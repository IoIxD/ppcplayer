extern "C"
{

#include "ff.hpp"

#include <stdbool.h>

    void
    init_pq()
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

    int audio_resampling(AVCodecContext *audio_decode_ctx, AVFrame *decoded_audio_frame,
                         enum AVSampleFormat out_sample_fmt, int out_channels, int out_sample_rate,
                         uint8_t *out_buf)
    {
        SwrContext *swr_ctx = NULL;
        int ret = 0;
        AVChannelLayout in_channel_layout = audio_decode_ctx->ch_layout;
        AVChannelLayout out_channel_layout = AV_CHANNEL_LAYOUT_STEREO;
        int out_nb_channels = 0;
        int out_linesize = 0;
        int in_nb_samples = 0;
        int out_nb_samples = 0;
        int max_out_nb_samples = 0;
        uint8_t **resampled_data = NULL;
        int resampled_data_size = 0;

        swr_ctx = swr_alloc();

        if (!swr_ctx)
        {
            printf("swr_alloc error.\n");
            return -1;
        }

        // set output audio channels based on the input audio channels
        if (out_channels == 1)
        {
            AVChannelLayout tmp = AV_CHANNEL_LAYOUT_MONO;
            out_channel_layout = tmp;
        }
        else if (out_channels == 2)
        {
            AVChannelLayout tmp = AV_CHANNEL_LAYOUT_STEREO;
            out_channel_layout = tmp;
        }
        else
        {
            AVChannelLayout tmp = AV_CHANNEL_LAYOUT_SURROUND;
            out_channel_layout = tmp;
        }

        // retrieve number of audio samples (per channel)
        in_nb_samples = decoded_audio_frame->nb_samples;
        if (in_nb_samples <= 0)
        {
            printf("in_nb_samples error.\n");
            return -1;
        }

        swr_alloc_set_opts2(&swr_ctx,
                            &out_channel_layout, out_sample_fmt, out_sample_rate, &in_channel_layout,
                            audio_decode_ctx->sample_fmt, audio_decode_ctx->sample_rate, 0, NULL);

        // Once all values have been set for the SwrContext, it must be initialized
        // with swr_init().
        ret = swr_init(swr_ctx);
        ;
        if (ret < 0)
        {
            printf("Failed to initialize the resampling context.\n");
            return -1;
        }

        max_out_nb_samples = out_nb_samples =
            av_rescale_rnd(in_nb_samples, out_sample_rate, audio_decode_ctx->sample_rate, AV_ROUND_UP);

        // check rescaling was successful
        if (max_out_nb_samples <= 0)
        {
            printf("av_rescale_rnd error.\n");
            return -1;
        }

        // get number of output audio channels
        out_nb_channels = out_channel_layout.nb_channels;

        ret = av_samples_alloc_array_and_samples(&resampled_data, &out_linesize, out_nb_channels,
                                                 out_nb_samples, out_sample_fmt, 0);

        if (ret < 0)
        {
            printf(
                "av_samples_alloc_array_and_samples() error: Could not allocate destination "
                "samples.\n");
            return -1;
        }

        // retrieve output samples number taking into account the progressive delay
        out_nb_samples =
            av_rescale_rnd(swr_get_delay(swr_ctx, audio_decode_ctx->sample_rate) + in_nb_samples,
                           out_sample_rate, audio_decode_ctx->sample_rate, AV_ROUND_UP);

        // check output samples number was correctly retrieved
        if (out_nb_samples <= 0)
        {
            printf("av_rescale_rnd error\n");
            return -1;
        }

        if (out_nb_samples > max_out_nb_samples)
        {
            // free memory block and set pointer to NULL
            av_free(resampled_data[0]);

            // Allocate a samples buffer for out_nb_samples samples
            ret = av_samples_alloc(resampled_data, &out_linesize, out_nb_channels, out_nb_samples,
                                   out_sample_fmt, 1);

            // check samples buffer correctly allocated
            if (ret < 0)
            {
                printf("av_samples_alloc failed.\n");
                return -1;
            }

            max_out_nb_samples = out_nb_samples;
        }

        if (swr_ctx)
        {
            // do the actual audio data resampling
            ret = swr_convert(swr_ctx, resampled_data, out_nb_samples,
                              (const uint8_t **)decoded_audio_frame->extended_data,
                              decoded_audio_frame->nb_samples);

            // check audio conversion was successful
            if (ret < 0)
            {
                printf("swr_convert_error.\n");
                return -1;
            }

            // Get the required buffer size for the given audio parameters
            resampled_data_size =
                av_samples_get_buffer_size(&out_linesize, out_nb_channels, ret, out_sample_fmt, 1);

            // check audio buffer size
            if (resampled_data_size < 0)
            {
                printf("av_samples_get_buffer_size error.\n");
                return -1;
            }
        }
        else
        {
            printf("swr_ctx null error.\n");
            return -1;
        }

        // copy the resampled data to the output buffer
        memcpy(out_buf, resampled_data[0], resampled_data_size);

        /*
         * Memory Cleanup.
         */
        if (resampled_data)
        {
            // free memory block and set pointer to NULL
            av_freep(&resampled_data[0]);
        }

        av_freep(&resampled_data);
        resampled_data = NULL;

        if (swr_ctx)
        {
            // Free the given SwrContext and set the pointer to NULL
            swr_free(&swr_ctx);
        }

        return resampled_data_size;
    }
}