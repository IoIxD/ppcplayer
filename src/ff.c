#include "ff.h"

#include <stdbool.h>

void init_pq()
{
    pq.head = NULL;
    pq.last = NULL;
    pq.size = 0;
    pq.codecCtx = NULL;
}

int pq_empty()
{
    return pq.size == 0;
}

void pq_put(AVPacket packet)
{
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
