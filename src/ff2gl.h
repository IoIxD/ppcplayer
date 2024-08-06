#ifndef __FF2GL_H
#define __FF2GL_H
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
#include "GL/gl.h"

#include "macros.h"

#define __UNSUPPORTED_ENUM(enum)                                          \
    case enum:                                                            \
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported enum " #enum "\n"); \
        return 0;

#define RETURN_AND_PRINT(enum)              \
    printf("Selected GL Enum " #enum "\n"); \
    return enum;
GLint ffmpeg_pix_format_to_gl(int pix_format)
{
    switch (pix_format)
    {
    case AV_PIX_FMT_0RGB:
        RETURN_AND_PRINT(GL_RGB);

    case AV_PIX_FMT_RGB0:
    case AV_PIX_FMT_RGB24:
        RETURN_AND_PRINT(GL_RGB);
    case AV_PIX_FMT_RGB4:
    case AV_PIX_FMT_RGB444:

    case AV_PIX_FMT_RGB32_1:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGB32_1\n");
        return 0;
    case AV_PIX_FMT_RGB565:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGB565\n");
        return 0;

    case AV_PIX_FMT_BGR0:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR0\n");
        return 0;
    case AV_PIX_FMT_BGR24:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR24\n");
        return 0;
    case AV_PIX_FMT_BGR4:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR4\n");
        return 0;
    case AV_PIX_FMT_BGR444:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR444\n");
        return 0;

    case AV_PIX_FMT_BGR48:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR48\n");
        return 0;

    case AV_PIX_FMT_BGR4_BYTE:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR4_BYTE\n");
        return 0;
    case AV_PIX_FMT_BGR555:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR555\n");
        return 0;

    case AV_PIX_FMT_BGR565:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR565\n");
        return 0;

    case AV_PIX_FMT_BGR8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGR8\n");
        return 0;
    case AV_PIX_FMT_BGRA64:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BGRA64\n");
        return 0;

    case AV_PIX_FMT_RGBAF32:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGBAF32\n");
        return 0;

    case AV_PIX_FMT_RGBF32:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGBF32\n");
        return 0;

    case AV_PIX_FMT_ARGB:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_ARGB\n");
        return 0;
    case AV_PIX_FMT_AYUV64:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_AYUV64\n");
        return 0;

    case AV_PIX_FMT_BAYER_BGGR16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_BGGR16\n");
        return 0;

    case AV_PIX_FMT_BAYER_BGGR8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_BGGR8\n");
        return 0;
    case AV_PIX_FMT_BAYER_GBRG16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_GBRG16\n");
        return 0;

    case AV_PIX_FMT_BAYER_GBRG8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_GBRG8\n");
        return 0;
    case AV_PIX_FMT_BAYER_GRBG16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_GRBG16\n");
        return 0;

    case AV_PIX_FMT_BAYER_GRBG8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_GRBG8\n");
        return 0;
    case AV_PIX_FMT_BAYER_RGGB16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_RGGB16\n");
        return 0;

    case AV_PIX_FMT_BAYER_RGGB8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_BAYER_RGGB8\n");
        return 0;
    case AV_PIX_FMT_CUDA:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_CUDA\n");
        return 0;
    case AV_PIX_FMT_D3D11:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_D3D11\n");
        return 0;
    case AV_PIX_FMT_D3D11VA_VLD:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_D3D11VA_VLD\n");
        return 0;
    case AV_PIX_FMT_DRM_PRIME:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_DRM_PRIME\n");
        return 0;
    case AV_PIX_FMT_DXVA2_VLD:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_DXVA2_VLD\n");
        return 0;
    case AV_PIX_FMT_GBR24P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBR24P\n");
        return 0;
    case AV_PIX_FMT_GBRAP:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAP\n");
        return 0;
    case AV_PIX_FMT_GBRAP10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAP10\n");
        return 0;

    case AV_PIX_FMT_GBRAP12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAP12\n");
        return 0;

    case AV_PIX_FMT_GBRAP14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAP14\n");
        return 0;

    case AV_PIX_FMT_GBRAP16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAP16\n");
        return 0;

    case AV_PIX_FMT_GBRAPF32:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRAPF32\n");
        return 0;

    case AV_PIX_FMT_GBRP10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRP10\n");
        return 0;

    case AV_PIX_FMT_GBRP12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRP12\n");
        return 0;

    case AV_PIX_FMT_GBRP14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRP14\n");
        return 0;

    case AV_PIX_FMT_GBRP16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRP16\n");
        return 0;

    case AV_PIX_FMT_GBRP9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRP9\n");
        return 0;

    case AV_PIX_FMT_GBRPF32:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GBRPF32\n");
        return 0;

    case AV_PIX_FMT_GRAY10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY10\n");
        return 0;

    case AV_PIX_FMT_GRAY12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY12\n");
        return 0;

    case AV_PIX_FMT_GRAY14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY14\n");
        return 0;

    case AV_PIX_FMT_GRAY16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY16\n");
        return 0;

    case AV_PIX_FMT_GRAY8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY8\n");
        return 0;
    case AV_PIX_FMT_GRAY8A:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY8A\n");
        return 0;
    case AV_PIX_FMT_GRAY9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAY9\n");
        return 0;

    case AV_PIX_FMT_GRAYF32:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_GRAYF32\n");
        return 0;

    case AV_PIX_FMT_MEDIACODEC:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_MEDIACODEC\n");
        return 0;
    case AV_PIX_FMT_MMAL:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_MMAL\n");
        return 0;
    case AV_PIX_FMT_MONOBLACK:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_MONOBLACK\n");
        return 0;
    case AV_PIX_FMT_MONOWHITE:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_MONOWHITE\n");
        return 0;
    case AV_PIX_FMT_NB:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NB\n");
        return 0;
    case AV_PIX_FMT_NONE:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NONE\n");
        return 0;
    case AV_PIX_FMT_NV12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV12\n");
        return 0;
    case AV_PIX_FMT_NV16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV16\n");
        return 0;
    case AV_PIX_FMT_NV20:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV20\n");
        return 0;

    case AV_PIX_FMT_NV21:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV21\n");
        return 0;
    case AV_PIX_FMT_NV24:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV24\n");
        return 0;
    case AV_PIX_FMT_NV42:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_NV42\n");
        return 0;
    case AV_PIX_FMT_OPENCL:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_OPENCL\n");
        return 0;
    case AV_PIX_FMT_P010:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P010\n");
        return 0;

    case AV_PIX_FMT_P012:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P012\n");
        return 0;

    case AV_PIX_FMT_P016:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P016\n");
        return 0;

    case AV_PIX_FMT_P210:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P210\n");
        return 0;

    case AV_PIX_FMT_P212:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P212\n");
        return 0;

    case AV_PIX_FMT_P216:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P216\n");
        return 0;

    case AV_PIX_FMT_P410:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P410\n");
        return 0;

    case AV_PIX_FMT_P412:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P412\n");
        return 0;

    case AV_PIX_FMT_P416:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_P416\n");
        return 0;

    case AV_PIX_FMT_PAL8:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_PAL8\n");
        return 0;
    case AV_PIX_FMT_QSV:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_QSV\n");
        return 0;

    case AV_PIX_FMT_RGB48:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGB48\n");
        return 0;

    case AV_PIX_FMT_RGBA64:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_RGBA64\n");
        return 0;

    case AV_PIX_FMT_UYVY422:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_UYVY422\n");
        return 0;
    case AV_PIX_FMT_UYYVYY411:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_UYYVYY411\n");
        return 0;
    case AV_PIX_FMT_VAAPI:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VAAPI\n");
        return 0;
    case AV_PIX_FMT_VDPAU:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VDPAU\n");
        return 0;
    case AV_PIX_FMT_VIDEOTOOLBOX:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VIDEOTOOLBOX\n");
        return 0;
    case AV_PIX_FMT_VULKAN:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VULKAN\n");
        return 0;
    case AV_PIX_FMT_VUYA:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VUYA\n");
        return 0;
    case AV_PIX_FMT_VUYX:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_VUYX\n");
        return 0;
    case AV_PIX_FMT_X2BGR10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_X2BGR10\n");
        return 0;

    case AV_PIX_FMT_XV30:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_XV30\n");
        return 0;

    case AV_PIX_FMT_XV36:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_XV36\n");
        return 0;

    case AV_PIX_FMT_XYZ12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_XYZ12\n");
        return 0;

    case AV_PIX_FMT_Y210:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_Y210\n");
        return 0;

    case AV_PIX_FMT_Y212:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_Y212\n");
        return 0;

    case AV_PIX_FMT_YA16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YA16\n");
        return 0;

    case AV_PIX_FMT_YUV410P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV410P\n");
        return 0;
    case AV_PIX_FMT_YUV411P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV411P\n");
        return 0;
    case AV_PIX_FMT_YUV420P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P\n");
        return 0;
    case AV_PIX_FMT_YUV420P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P10\n");
        return 0;

    case AV_PIX_FMT_YUV420P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P12\n");
        return 0;

    case AV_PIX_FMT_YUV420P14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P14\n");
        return 0;

    case AV_PIX_FMT_YUV420P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P16\n");
        return 0;

    case AV_PIX_FMT_YUV420P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV420P9\n");
        return 0;

    case AV_PIX_FMT_YUV422P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P\n");
        return 0;
    case AV_PIX_FMT_YUV422P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P10\n");
        return 0;

    case AV_PIX_FMT_YUV422P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P12\n");
        return 0;

    case AV_PIX_FMT_YUV422P14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P14\n");
        return 0;

    case AV_PIX_FMT_YUV422P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P16\n");
        return 0;

    case AV_PIX_FMT_YUV422P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV422P9\n");
        return 0;

    case AV_PIX_FMT_YUV440P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV440P\n");
        return 0;
    case AV_PIX_FMT_YUV440P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV440P10\n");
        return 0;

    case AV_PIX_FMT_YUV440P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV440P12\n");
        return 0;

    case AV_PIX_FMT_YUV444P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P\n");
        return 0;
    case AV_PIX_FMT_YUV444P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P10\n");
        return 0;

    case AV_PIX_FMT_YUV444P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P12\n");
        return 0;

    case AV_PIX_FMT_YUV444P14:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P14\n");
        return 0;

    case AV_PIX_FMT_YUV444P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P16\n");
        return 0;

    case AV_PIX_FMT_YUV444P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUV444P9\n");
        return 0;

    case AV_PIX_FMT_YUVA420P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA420P\n");
        return 0;
    case AV_PIX_FMT_YUVA420P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA420P10\n");
        return 0;

    case AV_PIX_FMT_YUVA420P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA420P16\n");
        return 0;

    case AV_PIX_FMT_YUVA420P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA420P9\n");
        return 0;

    case AV_PIX_FMT_YUVA422P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA422P\n");
        return 0;
    case AV_PIX_FMT_YUVA422P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA422P10\n");
        return 0;

    case AV_PIX_FMT_YUVA422P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA422P12\n");
        return 0;

    case AV_PIX_FMT_YUVA422P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA422P16\n");
        return 0;

    case AV_PIX_FMT_YUVA422P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA422P9\n");
        return 0;

    case AV_PIX_FMT_YUVA444P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA444P\n");
        return 0;
    case AV_PIX_FMT_YUVA444P10:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA444P10\n");
        return 0;

    case AV_PIX_FMT_YUVA444P12:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA444P12\n");
        return 0;

    case AV_PIX_FMT_YUVA444P16:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA444P16\n");
        return 0;
    case AV_PIX_FMT_YUVA444P9:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVA444P9\n");
        return 0;
    case AV_PIX_FMT_YUVJ411P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVJ411P\n");
        return 0;
    case AV_PIX_FMT_YUVJ420P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVJ420P\n");
        return 0;
    case AV_PIX_FMT_YUVJ422P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVJ422P\n");
        return 0;
    case AV_PIX_FMT_YUVJ440P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVJ440P\n");
        return 0;
    case AV_PIX_FMT_YUVJ444P:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUVJ444P\n");
        return 0;
    case AV_PIX_FMT_YUYV422:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YUYV422\n");
        return 0;
    case AV_PIX_FMT_YVYU422:
        __THROW_FATAL_ERROR(1, "GL Error: Unsupported format AV_PIX_FMT_YVYU422\n");
        return 0;
    }
}

#endif