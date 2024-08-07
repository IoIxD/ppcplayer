extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#ifdef __RETRO68__
#include "portaudio/portaudio.h"
#else
#include <portaudio.h>
#endif

#pragma once

namespace player
{
    class Player
    {
        PaError err;
        PaStream *stream;

        AVFormatContext *pFormatCtx;
        struct SwsContext *img_convert_ctx;
        struct SwsContext *sws_ctx = NULL;
        AVStream *videoStream = NULL;
        AVStream *audioStream = NULL;
        AVCodecParameters *videoPar = NULL;
        AVCodecParameters *audioPar = NULL;

        const AVCodec *videoCodec;
        const AVCodec *audioCodec;
        AVCodecContext *audioCodecCtx;
        AVCodecContext *videoCodecCtx;
        AVFrame *frame;
        AVPacket *packet;

        int vframe = 0;

    public:
        bool hasVideo = true;
        bool hasAudio = true;

        int realWidth;
        int realHeight;

        float widthDiff;
        float heightDiff;

        // Returns the time to wait after a decoding, in milliseconds.
        float framerate()
        {
            return this->videoCodecCtx->framerate.num;
        }

        AVFrame *pRGBFrame = NULL;
        Player(char *buf);

        // SDL_AudioSpec audio_spec = {0};

        void step();

        ~Player();
    };
};
