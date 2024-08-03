extern "C"
{
#ifndef __FF_H
#define __FF_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
    int audio_resampling(AVCodecContext *audio_decode_ctx, AVFrame *decoded_audio_frame,
                         enum AVSampleFormat out_sample_fmt, int out_channels, int out_sample_rate,
                         uint8_t *out_buf);

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
        AVCodecContext *codecCtx;
    } PQueue;

    static PQueue pq;

    void init_pq();
    int pq_empty();
    void pq_put(AVPacket packet);
    AVPacket pq_get();
    void pq_free();
    int audio_decode_frame(uint8_t *buf);
    void audio_callback(void *buffer, unsigned int frames);
#endif
}