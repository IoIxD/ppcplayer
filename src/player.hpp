#include "ff.hpp"
#pragma once

namespace player
{
    class Player
    {

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

        int framerate()
        {
            return this->videoCodecCtx->framerate.num;
        }

        AVFrame *pRGBFrame = NULL;
        Player(char *buf);

        void step();

        ~Player();
    };
};
