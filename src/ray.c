#include "ff.h"

#include "SDL.h"

int main()
{
    printf("Enter file name (must be relative to program path): \n");
    char *buf;
    scanf("%s", buf);

    SDL_Event event;
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_WM_SetCaption(buf, NULL);

    // int screenWidth = 1280;
    // int screenHeight = 720;

    // InitWindow(screenWidth, screenHeight, "RayPlayer");
    // SetWindowState(FLAG_WINDOW_RESIZABLE);
    // InitAudioDevice();
    init_pq();

    // Texture texture = {0};
    AVFormatContext *pFormatCtx = avformat_alloc_context();
    struct SwsContext *img_convert_ctx = sws_alloc_context();
    struct SwsContext *sws_ctx = NULL;
    avformat_open_input(&pFormatCtx, (const char *)buf, NULL, NULL);
    printf("CODEC: Format %s\n", pFormatCtx->iformat->long_name);
    avformat_find_stream_info(pFormatCtx, NULL);
    AVStream *videoStream = NULL;
    AVStream *audioStream = NULL;
    AVCodecParameters *videoPar = NULL;
    AVCodecParameters *audioPar = NULL;
    AVFrame *pRGBFrame = NULL;
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
        return -1;
    }

    // return with error in case no audio stream was found
    if (!audioStream)
    {
        printf("Could not find audio stream.\n");
        return -1;
    }
    const AVCodec *videoCodec = avcodec_find_decoder(videoPar->codec_id);
    const AVCodec *audioCodec = avcodec_find_decoder(audioPar->codec_id);
    printf("CODEC: %s ID %d, Bit rate %lld\n", videoCodec->name, videoCodec->id,
           videoPar->bit_rate);
    printf("FPS: %d/%d, TBR: %d/%d, TimeBase: %d/%d\n", videoStream->avg_frame_rate.num,
           videoStream->avg_frame_rate.den, videoStream->r_frame_rate.num,
           videoStream->r_frame_rate.den, videoStream->time_base.num, videoStream->time_base.den);

    AVCodecContext *audioCodecCtx = avcodec_alloc_context3(audioCodec);
    AVCodecContext *videoCodecCtx = avcodec_alloc_context3(videoCodec);
    pq.codecCtx = audioCodecCtx;
    avcodec_parameters_to_context(videoCodecCtx, videoPar);
    avcodec_parameters_to_context(audioCodecCtx, audioPar);
    avcodec_open2(videoCodecCtx, videoCodec, NULL);
    avcodec_open2(audioCodecCtx, audioCodec, NULL);

    AVFrame *frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();
    sws_ctx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
                             videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24,
                             SWS_FAST_BILINEAR, 0, 0, 0);

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    int rmask = 0xff000000;
    int gmask = 0x00ff0000;
    int bmask = 0x0000ff00;
    int amask = 0x000000ff;
#else
    int rmask = 0x000000ff;
    int gmask = 0x0000ff00;
    int bmask = 0x00ff0000;
    int amask = 0xff000000;
#endif

    SDL_SetVideoMode(videoPar->width, videoPar->height, 24, SDL_SWSURFACE);
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, videoCodecCtx->width, videoCodecCtx->height, 24, rmask, gmask, bmask, amask);

    SDL_Overlay *bmp = SDL_CreateYUVOverlay(videoCodecCtx->width, videoCodecCtx->height, SDL_YV12_OVERLAY, &surface);
    struct SwsContext *img_convert_context;
    img_convert_context = sws_getCachedContext(NULL,
                                               videoCodecCtx->width, videoCodecCtx->height,
                                               videoCodecCtx->pix_fmt,
                                               videoCodecCtx->width, videoCodecCtx->height,
                                               AV_PIX_FMT_YUV420P, SWS_BICUBIC,
                                               NULL, NULL, NULL);

    if (surface == NULL)
    {
        fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
        goto end;
    }

    // texture.height = videoCodecCtx->height;
    // texture.width = videoCodecCtx->width;
    // texture.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    // texture.mipmaps = 1;
    // texture.id =
    //     rlLoadTexture(NULL, texture.width, texture.height, texture.format, texture.mipmaps);
    //  SetTargetFPS(videoStream->avg_frame_rate.num / videoStream->avg_frame_rate.den);

    // AudioStream rayAStream = LoadAudioStream(44100, 32, 2);
    // SetAudioStreamCallback(rayAStream, audio_callback);
    // PlayAudioStream(rayAStream);

    pRGBFrame = av_frame_alloc();
    pRGBFrame->format = AV_PIX_FMT_YUV420P;
    pRGBFrame->width = videoCodecCtx->width;
    pRGBFrame->height = videoCodecCtx->height;
    av_frame_get_buffer(pRGBFrame, 0);
    int vframe = 0;

    while (1)
    {
        vframe++;

        // sprintf("VFrame %d\n", vframe);
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
                    // printf("Error sending packet\n");
                    continue;
                }
                while (ret >= 0)
                {
                    // printf("avcodec_receive_frame\n");
                    ret = avcodec_receive_frame(videoCodecCtx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    // printf("sws_scale\n");

                    SDL_LockYUVOverlay(bmp);

                    // Convert frame to YV12 pixel format for display in SDL overlay

                    pRGBFrame->data[0] = bmp->pixels[0];
                    pRGBFrame->data[1] = bmp->pixels[2]; // it's because YV12
                    pRGBFrame->data[2] = bmp->pixels[1];

                    pRGBFrame->linesize[0] = bmp->pitches[0];
                    pRGBFrame->linesize[1] = bmp->pitches[2];
                    pRGBFrame->linesize[2] = bmp->pitches[1];

                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
                              frame->height, pRGBFrame->data, pRGBFrame->linesize);

                    SDL_UnlockYUVOverlay(bmp);

                    SDL_Rect rect;
                    rect.x = 0;
                    rect.y = 0;
                    rect.w = videoCodecCtx->width;
                    rect.h = videoCodecCtx->height;
                    SDL_DisplayYUVOverlay(bmp, &rect);

                    //  UpdateTexture(texture, pRGBFrame->data[0]);
                }
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
    }

    // BeginDrawing();
    // ClearBackground(WHITE);

    // DrawTexturePro(texture, (Rectangle){0, 0, texture.width, texture.height},
    //               (Rectangle){0, 0, screenWidth, screenHeight}, (Vector2){0, 0}, 0, WHITE);

    // DrawFPS(0, 0);

    // EndDrawing();

    SDL_Quit();
    // UnloadTexture(texture);
    // UnloadAudioStream(rayAStream);

    // CloseWindow();
    // CloseAudioDevice();

    av_frame_free(&frame);
    av_frame_free(&pRGBFrame);
    av_packet_unref(packet);
    av_packet_free(&packet);
    avcodec_free_context(&videoCodecCtx);
    sws_freeContext(sws_ctx);

    avformat_close_input(&pFormatCtx);
    pq_free();

    printf("Program exit.\n");

end:
    getchar();
    return 0;
}
