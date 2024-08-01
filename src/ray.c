#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

typedef struct AVList
{
    AVPacket self;
    struct AVList *next;
} AVList;

typedef struct
{
    AVList *head;
    AVList *last;
    int size;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    AVCodecContext *codecCtx;
} PQueue;

PQueue pq;

void init_pq()
{
    pq.head = NULL;
    pq.last = NULL;
    pq.size = 0;
    pq.codecCtx = NULL;
}

bool pq_empty()
{
    return pq.size == 0;
}

void pq_put(AVPacket packet)
{
    printf("putting...\n");

    AVList *node = malloc(sizeof(AVList));
    node->self = packet;
    node->next = NULL;
    if (pq_empty())
    {
        pq.head = node;
        pq.last = node;
    }
    else
    {
        pq.last->next = node;
        pq.last = node;
    }
    pq.size++;
}

AVPacket pq_get()
{
    printf("getting...\n");

    while (pq_empty())
    {
    }
    AVList *node = pq.head;
    pq.head = pq.head->next;
    AVPacket p = node->self;
    free(node);
    if (pq.head == NULL)
    {
        pq.last = NULL;
    }
    pq.size--;
    return p;
}

void pq_free()
{
    // Free all nodes
    printf("total %d node found, destroying them...\n", pq.size);
    AVList *node = pq.head;
    while (node != NULL)
    {
        AVList *next = node->next;
        av_packet_unref(&node->self);
        free(node);
        node = next;
    }
}

int audio_decode_frame(uint8_t *buf)
{
    AVPacket packet = pq_get();
    int ret = avcodec_send_packet(pq.codecCtx, &packet);
    if (ret < 0)
    {
        printf("Error sending a packet for decoding\n");
        av_packet_unref(&packet);
        return -1;
    }
    AVFrame *frame = av_frame_alloc();

    ret = avcodec_receive_frame(pq.codecCtx, frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
    {
        av_frame_free(&frame);
        av_packet_unref(&packet);
        return -1;
    }
    else if (ret < 0)
    {
        printf("Error during decoding\n");
        av_frame_free(&frame);
        av_packet_unref(&packet);
        return -1;
    }
    // Got frame
    // uint f_size = frame->linesize[0];
    // uint b_ch = f_size / pq.codecCtx->ch_layout.nb_channels;
    int res = audio_resampling(pq.codecCtx, frame, AV_SAMPLE_FMT_FLT, 2, 44100, buf);

    av_frame_free(&frame);
    av_packet_unref(&packet);
    return res;
}

void audio_callback(void *buffer, unsigned int frames)
{
    // uint8_t *origin = (uint8_t *)buffer;
    uint8_t *dbuf = (uint8_t *)buffer;
    static uint8_t audio_buf[19200];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;
    int len1 = -1;
    int audio_size = -1;
    int len = frames * sizeof(float) * 2; // Stereo
    static int jj = 0;
    ++jj;
    printf("AFrame: %d, %d\n", jj, pq.size);
    while (len > 0)
    {
        if (audio_buf_index >= audio_buf_size)
        {
            audio_size = audio_decode_frame(audio_buf);
            if (audio_size < 0)
            {
                // output silence
                printf("Skipped one frame.\n");
                continue;
            }
            else
            {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;

        if (len1 > len)
        {
            len1 = len;
        }

        memcpy(dbuf, audio_buf + audio_buf_index, len1);

        len -= len1;
        dbuf += len1;
        audio_buf_index += len1;
    }
}

int main()
{
    char * buf;
    scanf("%s",buf);
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
    pRGBFrame->format = AV_PIX_FMT_RGB24;
    pRGBFrame->width = videoCodecCtx->width;
    pRGBFrame->height = videoCodecCtx->height;
    av_frame_get_buffer(pRGBFrame, 0);
    int vframe = 0;
    // while (!WindowShouldClose())
    while (1)
    {
        vframe++;
        // sprintf("VFrame %d\n", vframe);
        while (av_read_frame(pFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoStream->index)
            {
                // Getting frame from video
                printf("avcodec_send_packet\n");
                int ret = avcodec_send_packet(videoCodecCtx, packet);
                if (ret < 0)
                {
                    // Error
                    printf("Error sending packet\n");
                    continue;
                }
                while (ret >= 0)
                {
                    printf("avcodec_receive_frame\n");
                    ret = avcodec_receive_frame(videoCodecCtx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    printf("sws_scale\n");
                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
                              frame->height, pRGBFrame->data, pRGBFrame->linesize);
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

        // BeginDrawing();
        // ClearBackground(WHITE);

        // DrawTexturePro(texture, (Rectangle){0, 0, texture.width, texture.height},
        //               (Rectangle){0, 0, screenWidth, screenHeight}, (Vector2){0, 0}, 0, WHITE);

        // DrawFPS(0, 0);

        // EndDrawing();
    }
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
    getchar();
    return 0;
}
