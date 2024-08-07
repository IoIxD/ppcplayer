extern "C"
{

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GL/gl.h"

#include "macros.h"
#include "audio.h"
}

#include "player.hpp"

PQueue pq;

#define SAMPLE_RATE (44100)
typedef struct
{
    int sample_rate;
} paTestData;

static paTestData data;

void pq_init()
{
    pq.head = NULL;
    pq.last = NULL;
    pq.size = 0;
    pq.codecCtx = NULL;
    // pthread_mutex_init(&pq.mutex, NULL);
    // pthread_cond_init(&pq.cond, NULL);
}

bool pq_empty()
{
    return pq.size == 0;
}

void pq_put(AVPacket packet)
{
    // pthread_mutex_lock(&pq.mutex);
    AVList *node = (AVList *)malloc(sizeof(AVList));
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
    // pthread_cond_signal(&pq.cond);
    // pthread_mutex_unlock(&pq.mutex);
}

AVPacket pq_get()
{
    // pthread_mutex_lock(&pq.mutex);
    while (pq_empty())
    {
        // pthread_cond_wait(&pq.cond, &pq.mutex);
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
    // pthread_mutex_unlock(&pq.mutex);
    return p;
}

void pq_free()
{
    // pthread_mutex_destroy(&pq.mutex);
    // pthread_cond_destroy(&pq.cond);
    //  Free all nodes
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

int audio_decode_frame(uint8_t *buf, int sample_rate)
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
    switch (ret)
    {
    case AVERROR(EAGAIN):
        av_frame_free(&frame);
        av_packet_unref(&packet);
        return -2;
    case AVERROR_EOF:
        av_frame_free(&frame);
        av_packet_unref(&packet);
        return -1;
    default:
        if (ret < 0)
        {
            av_frame_free(&frame);
            av_packet_unref(&packet);
            return -1;
        }
        break;
    }

    // Got frame
    uint f_size = frame->linesize[0];
    uint b_ch = f_size / pq.codecCtx->ch_layout.nb_channels;
    int res = audio_resampling(pq.codecCtx, frame, AV_SAMPLE_FMT_FLT, 2, SAMPLE_RATE, buf);

    av_frame_free(&frame);
    av_packet_unref(&packet);
    return res;
}

#ifdef __RETRO68__
int callback(void *input,
             void *output,
             unsigned long frames,
             PaTimestamp outTime,
             void *userData)
#else
int callback(const void *input, void *output, unsigned long frames, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
#endif
{

    // uint8_t *origin = (uint8_t *)output;
    paTestData *data = (paTestData *)userData;
    uint8_t *dbuf = (uint8_t *)output;
    static uint8_t audio_buf[19200];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;
    int len1 = -1;
    int audio_size = -2;
    int len = frames * sizeof(float) * 2; // Stereo
    static int jj = 0;
    ++jj;
    while (len > 0)
    {
        if (audio_buf_index >= audio_buf_size)
        {
            while (audio_size == -2)
            {
                audio_size = audio_decode_frame(audio_buf, data->sample_rate);
            }
            audio_buf_size = audio_size;
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        // hmmmm
        if (len1 <= 0)
        {
            len1 -= 1;
        }

        if (len1 > len)
        {
            len1 = len;
        }

        memcpy(dbuf, audio_buf + audio_buf_index, len1);

        len -= len1;
        dbuf += len1;
        audio_buf_index += len1;
    }
    return 0;
}

namespace player
{

    Player::Player(char *buf)
    {
        pq_init();
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
            this->hasVideo = false;
        }

        if (!audioStream)
        {
            this->hasAudio = false;
        }

        if (this->hasVideo)
        {
            videoCodec = avcodec_find_decoder(videoPar->codec_id);
            printf("CODEC: %s ID %d, Bit rate %lld\n", videoCodec->name, videoCodec->id,
                   videoPar->bit_rate);
            printf("FPS: %d/%d, TBR: %d/%d, TimeBase: %d/%d\n", videoStream->avg_frame_rate.num,
                   videoStream->avg_frame_rate.den, videoStream->r_frame_rate.num,
                   videoStream->r_frame_rate.den, videoStream->time_base.num, videoStream->time_base.den);
            videoCodecCtx = avcodec_alloc_context3(videoCodec);
            avcodec_parameters_to_context(videoCodecCtx, videoPar);
            avcodec_open2(videoCodecCtx, videoCodec, NULL);
        }

        audioCodec = avcodec_find_decoder(audioPar->codec_id);

        audioCodecCtx = avcodec_alloc_context3(audioCodec);
        pq.codecCtx = audioCodecCtx;
        if (audioCodecCtx == NULL)
        {
            __THROW_FATAL_ERROR(0, "audioCodecCtx is null");
        }
        avcodec_parameters_to_context(audioCodecCtx, audioPar);
        avcodec_open2(audioCodecCtx, audioCodec, NULL);

        frame = av_frame_alloc();
        packet = av_packet_alloc();

        if (this->hasVideo)
        {
            sws_ctx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, videoCodecCtx->pix_fmt,
                                     videoCodecCtx->width, videoCodecCtx->height, AV_PIX_FMT_RGB24,
                                     SWS_FAST_BILINEAR, 0, 0, 0);
            pRGBFrame = av_frame_alloc();
            pRGBFrame->format = AV_PIX_FMT_RGB24;
            pRGBFrame->width = pow(2, ceil(log(videoCodecCtx->width) / log(2)));
            pRGBFrame->height = pow(2, ceil(log(videoCodecCtx->height) / log(2)));
            this->realWidth = videoCodecCtx->width;
            this->realHeight = videoCodecCtx->height;

            this->widthDiff = (float)this->realWidth / (float)pRGBFrame->width;
            this->heightDiff = (float)this->realHeight / (float)pRGBFrame->height;
            av_frame_get_buffer(pRGBFrame, 0);
        }
        PA_COMMAND(Pa_Initialize());

#ifdef __RETRO68__
        PA_COMMAND(Pa_OpenDefaultStream(
                       &stream,                              /* passes back stream pointer */
                       0,                                    /* no input channels */
                       audioCodecCtx->ch_layout.nb_channels, /* stereo output */
                       paFloat32,                            /* 32 bit floating point output */
                       audioCodecCtx->sample_rate,           /* sample rate */
                       256,                                  /* frames per buffer */
                       0,                                    /* number of buffers, if zero then use default minimum */
                       callback,                             /* specify our custom callback */
                       &data);                               /* pass our data through to callback */
#else
        PA_COMMAND(Pa_OpenDefaultStream(
                       &stream, /* passes back stream pointer */
                       0,       /* no input channels */
                       audioCodecCtx->ch_layout.nb_channels,
                       paFloat32,                  /* stereo output */
                       audioCodecCtx->sample_rate, /* sample rate */
                       256,                        /* frames per buffer */
                       callback,                   /* specify our custom callback */
                       &data);                     /* pass our data through to ` */
#endif
        )

        data.sample_rate = audioPar->sample_rate;

        PA_COMMAND(Pa_StartStream(stream));
        //  SDL_PauseAudio(0);
    }
    void Player::step()
    {
        vframe++;
        while (av_read_frame(pFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoStream->index)
            {
                // Getting frame from video
                int ret = avcodec_send_packet(videoCodecCtx, packet);
                av_packet_unref(packet);
                if (ret < 0)
                {
                    // Error
                    printf("Error sending packet\n");
                    continue;
                }
                while (ret >= 0)
                {
                    ret = avcodec_receive_frame(videoCodecCtx, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    sws_scale(sws_ctx, (uint8_t const *const *)frame->data, frame->linesize, 0,
                              frame->height, pRGBFrame->data, pRGBFrame->linesize);
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
        if (this->hasVideo)
        {
            if (vframe >= videoStream->nb_frames)
            {
                exit(1);
            }
        }
    }

    Player::~Player()
    {
        PA_COMMAND(Pa_Terminate());
        av_frame_free(&frame);
        av_frame_free(&pRGBFrame);
        av_packet_unref(packet);
        av_packet_free(&packet);
        avcodec_free_context(&videoCodecCtx);
        sws_freeContext(sws_ctx);

        avformat_close_input(&pFormatCtx);
        pq_free();

        // SDL_CloseAudio();
        // SDL_Quit();
    }
}