#include "player.hpp"
namespace player
{

    Player::Player(char *buf)
    {
        init_pq();

        // Texture texture = {0};
        pFormatCtx = avformat_alloc_context();
        img_convert_ctx = sws_alloc_context();
        avformat_open_input(&pFormatCtx, (const char *)buf, NULL, NULL);
        printf("CODEC: Format %s\n", pFormatCtx->iformat->long_name);
        avformat_find_stream_info(pFormatCtx, NULL);

        for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
        {
            AVStream *tmpStream = pFormatCtx->streams[i];
            AVCodecParameters *tmpPar = tmpStream->codecpar;
            if (tmpPar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audioStream = tmpStream;
                audioPar = tmpPar;
                printf("CODEC: Audio sample rate %d, channels: %d\n", audioPar->sample_rate,
                       audioPar->ch_layout.nb_channels);
                continue;
            }
            if (tmpPar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                videoStream = tmpStream;
                videoPar = tmpPar;
                printf("CODEC: Resolution %d x %d, type: %d\n", videoPar->width,
                       videoPar->height, videoPar->codec_id);
                continue;
            }
        }
        if (!videoStream)
        {
            printf("Could not find video stream.\n");
            exit(-1);
        }

        // return with error in case no audio stream was found
        if (!audioStream)
        {
            printf("Could not find audio stream.\n");
            exit(-1);
        }
        videoCodec = avcodec_find_decoder(videoPar->codec_id);
        audioCodec = avcodec_find_decoder(audioPar->codec_id);
        printf("CODEC: %s ID %d, Bit rate %lld\n", videoCodec->name, videoCodec->id,
               videoPar->bit_rate);
        printf("FPS: %d/%d, TBR: %d/%d, TimeBase: %d/%d\n", videoStream->avg_frame_rate.num,
               videoStream->avg_frame_rate.den, videoStream->r_frame_rate.num,
               videoStream->r_frame_rate.den, videoStream->time_base.num, videoStream->time_base.den);

        audioCodecCtx = avcodec_alloc_context3(audioCodec);
        videoCodecCtx = avcodec_alloc_context3(videoCodec);
        pq.codecCtx = audioCodecCtx;
        avcodec_parameters_to_context(videoCodecCtx, videoPar);
        avcodec_parameters_to_context(audioCodecCtx, audioPar);
        avcodec_open2(videoCodecCtx, videoCodec, NULL);
        avcodec_open2(audioCodecCtx, audioCodec, NULL);

        frame = av_frame_alloc();
        packet = av_packet_alloc();
        sws_ctx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
                                 videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24,
                                 SWS_FAST_BILINEAR, 0, 0, 0);

        pRGBFrame = av_frame_alloc();
        pRGBFrame->format = AV_PIX_FMT_RGB24;
        pRGBFrame->width = videoCodecCtx->width;
        pRGBFrame->height = videoCodecCtx->height;
        av_frame_get_buffer(pRGBFrame, 0);
    }
    void Player::step()
    {
        vframe++;
        while (av_read_frame(pFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoStream->index)
            {
                // Getting frame from video
                // printf("avcodec_send_packet\n");
                int ret = avcodec_send_packet(videoCodecCtx, packet);
                if (ret < 0)
                {
                    // Error
                    printf("Error sending packet\n");
                    continue;
                }
                while (ret >= 0)
                {
                    // printf("avcodec_receive_frame\n");
                    ret = avcodec_receive_frame(videoCodecCtx, frame);
                    if (ret == AVERROR(EAGAIN))
                    {
                        break;
                    }
                    if (ret == AVERROR_EOF)
                    {
                        printf("end\n");
                        exit(0);
                    }
                    // printf("sws_scale\n");
                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
                              frame->height, pRGBFrame->data, pRGBFrame->linesize);
                }
                av_packet_unref(packet);
                break;
            }
            else if (packet->stream_index == audioStream->index)
            {
                // Getting audio data from audio
                AVPacket *cloned = av_packet_clone(packet);
                pq_put(*cloned);
            }
            av_packet_unref(packet);
        }
        if (vframe == videoStream->nb_frames)
        {
            exit(1);
        }
    }

    Player::~Player()
    {
        av_frame_free(&frame);
        av_frame_free(&pRGBFrame);
        av_packet_unref(packet);
        av_packet_free(&packet);
        avcodec_free_context(&videoCodecCtx);
        sws_freeContext(sws_ctx);

        avformat_close_input(&pFormatCtx);
        pq_free();
    }
}