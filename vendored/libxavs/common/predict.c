/*****************************************************************************
 * predict.c: h264 encoder
 *****************************************************************************
 * Copyright (C) 2003-2008 x264 project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Loren Merritt <lorenm@u.washington.edu>
 *          Jason Garrett-Glaser <darkshikari@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * predict.c: xavs encoder
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include "common.h"


#ifdef HAVE_MMXEXT
#   include "i386/predict.h"
#endif

/****************************************************************************
 * 16x16 prediction for intra luma block
 ****************************************************************************/

#define PREDICT_16x16_DC(v) \
    for( i = 0; i < 16; i++ )\
    {\
        uint32_t *p = (uint32_t*)src;\
        *p++ = v;\
        *p++ = v;\
        *p++ = v;\
        *p++ = v;\
        src += FDEC_STRIDE;\
    }


/****************************************************************************
 * 8x8 prediction for intra chroma block
 ****************************************************************************/
#define SRC(x,y) src[(x)+(y)*FDEC_STRIDE]
#define PL(y) \
    const int l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;
#define PREDICT_8x8_LOAD_LEFT(have_tl) \
    const int l0 = ((have_tl || (i_neighbor&MB_TOPLEFT) ? SRC(-1,-1) : SRC(-1,0)) \
                     + 2*SRC(-1,0) + SRC(-1,1) + 2) >> 2; \
    PL(1) PL(2) PL(3) PL(4) PL(5) PL(6) \
    UNUSED const int l7 = (SRC(-1,6) + 2*SRC(-1,7) +((i_neighbor&MB_DOWNLEFT) ?\
                     SRC(-1,8) : SRC(-1,7))+2) >> 2;\

#define PDL(y) \
    l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;

#define PREDICT_8x8_LOAD_DOWNLEFT \
    int l8, l9, l10, l11, l12, l13, l14, l15; \
if(i_neighbor&MB_DOWNLEFT) { \
        PDL(8) PDL(9) PDL(10) PDL(11) PDL(12) PDL(13) PDL(14) \
        l15 = (SRC(-1,14) + 3*SRC(-1,15) + 2) >> 2; \
    } else l8=l9=l10=l11=l12=l13=l14=l15= SRC(-1,7);

#define PT(x) \
    const int t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;

#define PREDICT_8x8_LOAD_TOP(have_tl) \
    const int t0 = ((have_tl || (i_neighbor&MB_TOPLEFT) ? SRC(-1,-1) : SRC(0,-1)) \
                     + 2*SRC(0,-1) + SRC(1,-1) + 2) >> 2; \
    PT(1) PT(2) PT(3) PT(4) PT(5) PT(6) \
    UNUSED const int t7 = ((i_neighbor&MB_TOPRIGHT ? SRC(8,-1) : SRC(7,-1)) \
                     + 2*SRC(7,-1) + SRC(6,-1) + 2) >> 2;

#define PTR(x) \
    t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;

#define PREDICT_8x8_LOAD_TOPRIGHT \
    int t8, t9, t10, t11, t12, t13, t14, t15; \
    if(i_neighbor&MB_TOPRIGHT) { \
        PTR(8) PTR(9) PTR(10) PTR(11) PTR(12) PTR(13) PTR(14) \
        t15 = (SRC(14,-1) + 3*SRC(15,-1) + 2) >> 2; \
    } else t8=t9=t10=t11=t12=t13=t14=t15= SRC(7,-1);

#define PREDICT_8x8_LOAD_TOPLEFT \
    const int lt = (SRC(-1,0) + 2*SRC(-1,-1) + SRC(0,-1) + 2) >> 2;


#define PREDICT_8x8_DC(v) \
    int y; \
    for( y = 0; y < 8; y++ ) { \
        ((uint32_t*)src)[0] = \
        ((uint32_t*)src)[1] = v; \
        src += FDEC_STRIDE; \
    }

static void
predict_8x8c_dc_128 (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_DC (0x80808080);
}

static void
predict_8x8c_dc_left (uint8_t * src, int i_neighbor)
{                               //int i;
  PREDICT_8x8_LOAD_LEFT (0)
    //PREDICT_8x8_LOAD_LEFT(0)
#define ROW(y) ((uint32_t*)(src+y*FDEC_STRIDE))[0] =\
               ((uint32_t*)(src+y*FDEC_STRIDE))[1] = 0x01010101U * l##y
    ROW (0);
  ROW (1);
  ROW (2);
  ROW (3);
  ROW (4);
  ROW (5);
  ROW (6);
  ROW (7);
#undef ROW
}


static void
predict_8x8c_dc_top (uint8_t * src, int i_neighbor)
{
  int i;
  PREDICT_8x8_LOAD_TOP (0) for (i = 0; i < 8; i++)
  {
    src[0] = t0;
    src[1] = t1;
    src[2] = t2;
    src[3] = t3;
    src[4] = t4;
    src[5] = t5;
    src[6] = t6;
    src[7] = t7;
    src += FDEC_STRIDE;
  }
}


static void
predict_8x8c_dc (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_LOAD_LEFT (0) PREDICT_8x8_LOAD_TOP (0)

  src[0] = xavs_clip_uint8 ((t0 + l0) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l0) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l0) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l0) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l0) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l0) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l0) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l0) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l1) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l1) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l1) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l1) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l1) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l1) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l1) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l1) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l2) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l2) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l2) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l2) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l2) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l2) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l2) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l2) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l3) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l3) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l3) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l3) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l3) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l3) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l3) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l3) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l4) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l4) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l4) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l4) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l4) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l4) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l4) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l4) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l5) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l5) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l5) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l5) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l5) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l5) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l5) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l5) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l6) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l6) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l6) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l6) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l6) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l6) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l6) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l6) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l7) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l7) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l7) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l7) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l7) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l7) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l7) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l7) >> 1);
}

static void
predict_8x8c_h (uint8_t * src, int i_neighbor)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    uint32_t v = 0x01010101 * src[-1];
    uint32_t *p = (uint32_t *) src;
    *p++ = v;
    *p++ = v;
    src += FDEC_STRIDE;
  }
}

static void
predict_8x8c_v (uint8_t * src, int i_neighbor)
{
  uint32_t v0 = *(uint32_t *) & src[0 - FDEC_STRIDE];
  uint32_t v1 = *(uint32_t *) & src[4 - FDEC_STRIDE];
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    uint32_t *p = (uint32_t *) src;
    *p++ = v0;
    *p++ = v1;
    src += FDEC_STRIDE;
  }
}

static void
predict_8x8c_p (uint8_t * src, int i_neighbor)
{
  int i;
  int x, y;
  int a, b, c;
  int H = 0;
  int V = 0;
  int i00;

  for (i = 0; i < 4; i++)
  {
    H += (i + 1) * (src[4 + i - FDEC_STRIDE] - src[2 - i - FDEC_STRIDE]);
    V += (i + 1) * (src[-1 + (i + 4) * FDEC_STRIDE] - src[-1 + (2 - i) * FDEC_STRIDE]);
  }

  a = 16 * (src[-1 + 7 * FDEC_STRIDE] + src[7 - FDEC_STRIDE]);
  b = (17 * H + 16) >> 5;
  c = (17 * V + 16) >> 5;
  i00 = a - 3 * b - 3 * c + 16;

  for (y = 0; y < 8; y++)
  {
    int pix = i00;
    for (x = 0; x < 8; x++)
    {
      src[x] = xavs_clip_uint8 (pix >> 5);
      pix += b;
    }
    src += FDEC_STRIDE;
    i00 += c;
  }
}

/****************************************************************************
 * 8x8 prediction for intra luma block
 ****************************************************************************/
#define SRC(x,y) src[(x)+(y)*FDEC_STRIDE]

#define PL(y) \
    const int l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;

#define PREDICT_8x8_LOAD_LEFT(have_tl) \
    const int l0 = ((have_tl || (i_neighbor&MB_TOPLEFT) ? SRC(-1,-1) : SRC(-1,0)) \
                     + 2*SRC(-1,0) + SRC(-1,1) + 2) >> 2; \
    PL(1) PL(2) PL(3) PL(4) PL(5) PL(6) \
    UNUSED const int l7 = (SRC(-1,6) + 2*SRC(-1,7) +((i_neighbor&MB_DOWNLEFT) ?\
                     SRC(-1,8) : SRC(-1,7))+2) >> 2;\

#define PDL(y) \
    l##y = (SRC(-1,y-1) + 2*SRC(-1,y) + SRC(-1,y+1) + 2) >> 2;

#define PREDICT_8x8_LOAD_DOWNLEFT \
    int l8, l9, l10, l11, l12, l13, l14, l15; \
if(i_neighbor&MB_DOWNLEFT) { \
        PDL(8) PDL(9) PDL(10) PDL(11) PDL(12) PDL(13) PDL(14) \
        l15 = (SRC(-1,14) + 3*SRC(-1,15) + 2) >> 2; \
    } else l8=l9=l10=l11=l12=l13=l14=l15= SRC(-1,7);

#define PT(x) \
    const int t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;

#define PREDICT_8x8_LOAD_TOP(have_tl) \
    const int t0 = ((have_tl || (i_neighbor&MB_TOPLEFT) ? SRC(-1,-1) : SRC(0,-1)) \
                     + 2*SRC(0,-1) + SRC(1,-1) + 2) >> 2; \
    PT(1) PT(2) PT(3) PT(4) PT(5) PT(6) \
    UNUSED const int t7 = ((i_neighbor&MB_TOPRIGHT ? SRC(8,-1) : SRC(7,-1)) \
                     + 2*SRC(7,-1) + SRC(6,-1) + 2) >> 2;

#define PTR(x) \
    t##x = (SRC(x-1,-1) + 2*SRC(x,-1) + SRC(x+1,-1) + 2) >> 2;

#define PREDICT_8x8_LOAD_TOPRIGHT \
    int t8, t9, t10, t11, t12, t13, t14, t15; \
    if(i_neighbor&MB_TOPRIGHT) { \
        PTR(8) PTR(9) PTR(10) PTR(11) PTR(12) PTR(13) PTR(14) \
        t15 = (SRC(14,-1) + 3*SRC(15,-1) + 2) >> 2; \
    } else t8=t9=t10=t11=t12=t13=t14=t15= SRC(7,-1);

#define PREDICT_8x8_LOAD_TOPLEFT \
    const int lt = (SRC(-1,0) + 2*SRC(-1,-1) + SRC(0,-1) + 2) >> 2;


#define PREDICT_8x8_DC(v) \
    int y; \
    for( y = 0; y < 8; y++ ) { \
        ((uint32_t*)src)[0] = \
        ((uint32_t*)src)[1] = v; \
        src += FDEC_STRIDE; \
    }

static void
predict_8x8_dc_128 (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_DC (0x80808080);
}

static void
predict_8x8_dc_left (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_LOAD_LEFT (0)
#define ROW(y) ((uint32_t*)(src+y*FDEC_STRIDE))[0] =\
               ((uint32_t*)(src+y*FDEC_STRIDE))[1] = 0x01010101U * l##y
  ROW (0);
  ROW (1);
  ROW (2);
  ROW (3);
  ROW (4);
  ROW (5);
  ROW (6);
  ROW (7);
#undef ROW
}


static void
predict_8x8_dc_top (uint8_t * src, int i_neighbor)
{
  int i;
  PREDICT_8x8_LOAD_TOP (0) for (i = 0; i < 8; i++)
  {
    src[0] = t0;
    src[1] = t1;
    src[2] = t2;
    src[3] = t3;
    src[4] = t4;
    src[5] = t5;
    src[6] = t6;
    src[7] = t7;
    src += FDEC_STRIDE;
  }
}


static void
predict_8x8_dc (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_LOAD_LEFT (0) PREDICT_8x8_LOAD_TOP (0) src[0] = xavs_clip_uint8 ((t0 + l0) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l0) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l0) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l0) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l0) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l0) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l0) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l0) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l1) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l1) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l1) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l1) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l1) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l1) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l1) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l1) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l2) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l2) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l2) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l2) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l2) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l2) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l2) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l2) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l3) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l3) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l3) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l3) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l3) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l3) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l3) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l3) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l4) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l4) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l4) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l4) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l4) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l4) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l4) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l4) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l5) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l5) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l5) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l5) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l5) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l5) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l5) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l5) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l6) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l6) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l6) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l6) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l6) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l6) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l6) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l6) >> 1);
  src += FDEC_STRIDE;

  src[0] = xavs_clip_uint8 ((t0 + l7) >> 1);
  src[1] = xavs_clip_uint8 ((t1 + l7) >> 1);
  src[2] = xavs_clip_uint8 ((t2 + l7) >> 1);
  src[3] = xavs_clip_uint8 ((t3 + l7) >> 1);
  src[4] = xavs_clip_uint8 ((t4 + l7) >> 1);
  src[5] = xavs_clip_uint8 ((t5 + l7) >> 1);
  src[6] = xavs_clip_uint8 ((t6 + l7) >> 1);
  src[7] = xavs_clip_uint8 ((t7 + l7) >> 1);
}

static void
predict_8x8_h (uint8_t * src, int i_neighbor)
{
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    uint32_t v = 0x01010101 * src[-1];
    uint32_t *p = (uint32_t *) src;
    *p++ = v;
    *p++ = v;
    src += FDEC_STRIDE;
  }
}

static void
predict_8x8_v (uint8_t * src, int i_neighbor)
{
  uint32_t v0 = *(uint32_t *) & src[0 - FDEC_STRIDE];
  uint32_t v1 = *(uint32_t *) & src[4 - FDEC_STRIDE];
  uint8_t i;

  for (i = 0; i < 8; i++)
  {
    uint32_t *p = (uint32_t *) src;
    *p++ = v0;
    *p++ = v1;
    src += FDEC_STRIDE;
  }
}

static void
predict_8x8_ddl (uint8_t * src, int i_neighbor)
{
  int l8, l9, l10, l11, l12, l13, l14, l15;

  PREDICT_8x8_LOAD_TOP (1) PREDICT_8x8_LOAD_LEFT (1) PREDICT_8x8_LOAD_TOPRIGHT
  if (i_neighbor & MB_DOWNLEFT)
  {
    PDL (8) PDL (9) PDL (10) PDL (11) PDL (12) PDL (13) PDL (14) l15 = (SRC (-1, 14) + 3 * SRC (-1, 15) + 2) >> 2;
  }
  else
    l8 = l9 = l10 = l11 = l12 = l13 = l14 = l15 = SRC (-1, 7);

  SRC (0, 0) = (l1 + t1) >> 1;
  SRC (0, 1) = SRC (1, 0) = (l2 + t2) >> 1;
  SRC (0, 2) = SRC (1, 1) = SRC (2, 0) = (l3 + t3) >> 1;
  SRC (0, 3) = SRC (1, 2) = SRC (2, 1) = SRC (3, 0) = (l4 + t4) >> 1;
  SRC (0, 4) = SRC (1, 3) = SRC (2, 2) = SRC (3, 1) = SRC (4, 0) = (l5 + t5) >> 1;
  SRC (0, 5) = SRC (1, 4) = SRC (2, 3) = SRC (3, 2) = SRC (4, 1) = SRC (5, 0) = (l6 + t6) >> 1;
  SRC (0, 6) = SRC (1, 5) = SRC (2, 4) = SRC (3, 3) = SRC (4, 2) = SRC (5, 1) = SRC (6, 0) = (l7 + t7) >> 1;
  SRC (0, 7) = SRC (1, 6) = SRC (2, 5) = SRC (3, 4) = SRC (4, 3) = SRC (5, 2) = SRC (6, 1) = SRC (7, 0) = (l8 + t8) >> 1;
  SRC (1, 7) = SRC (2, 6) = SRC (3, 5) = SRC (4, 4) = SRC (5, 3) = SRC (6, 2) = SRC (7, 1) = (l9 + t9) >> 1;
  SRC (2, 7) = SRC (3, 6) = SRC (4, 5) = SRC (5, 4) = SRC (6, 3) = SRC (7, 2) = (l10 + t10) >> 1;
  SRC (3, 7) = SRC (4, 6) = SRC (5, 5) = SRC (6, 4) = SRC (7, 3) = (l11 + t11) >> 1;
  SRC (4, 7) = SRC (5, 6) = SRC (6, 5) = SRC (7, 4) = (l12 + t12) >> 1;
  SRC (5, 7) = SRC (6, 6) = SRC (7, 5) = (l13 + t13) >> 1;
  SRC (6, 7) = SRC (7, 6) = (l14 + t14) >> 1;
  SRC (7, 7) = (l15 + t15) >> 1;
}

static void
predict_8x8_ddr (uint8_t * src, int i_neighbor)
{
  PREDICT_8x8_LOAD_TOP (1) PREDICT_8x8_LOAD_LEFT (1) PREDICT_8x8_LOAD_TOPLEFT SRC (0, 7) = l6;  //(l7 + 2*l6 + l5 + 2) >> 2;
  SRC (0, 6) = SRC (1, 7) = l5; //(l6 + 2*l5 + l4 + 2) >> 2;
  SRC (0, 5) = SRC (1, 6) = SRC (2, 7) = l4;    //(l5 + 2*l4 + l3 + 2) >> 2;
  SRC (0, 4) = SRC (1, 5) = SRC (2, 6) = SRC (3, 7) = l3;       //(l4 + 2*l3 + l2 + 2) >> 2;
  SRC (0, 3) = SRC (1, 4) = SRC (2, 5) = SRC (3, 6) = SRC (4, 7) = l2;  //(l3 + 2*l2 + l1 + 2) >> 2;
  SRC (0, 2) = SRC (1, 3) = SRC (2, 4) = SRC (3, 5) = SRC (4, 6) = SRC (5, 7) = l1;     //(l2 + 2*l1 + l0 + 2) >> 2;
  SRC (0, 1) = SRC (1, 2) = SRC (2, 3) = SRC (3, 4) = SRC (4, 5) = SRC (5, 6) = SRC (6, 7) = l0;        //(l1 + 2*l0 + lt + 2) >> 2;
  SRC (0, 0) = SRC (1, 1) = SRC (2, 2) = SRC (3, 3) = SRC (4, 4) = SRC (5, 5) = SRC (6, 6) = SRC (7, 7) = lt;   //(l0 + 2*lt + t0 + 2) >> 2;
  SRC (1, 0) = SRC (2, 1) = SRC (3, 2) = SRC (4, 3) = SRC (5, 4) = SRC (6, 5) = SRC (7, 6) = t0;        //(lt + 2*t0 + t1 + 2) >> 2;
  SRC (2, 0) = SRC (3, 1) = SRC (4, 2) = SRC (5, 3) = SRC (6, 4) = SRC (7, 5) = t1;     //(t0 + 2*t1 + t2 + 2) >> 2;
  SRC (3, 0) = SRC (4, 1) = SRC (5, 2) = SRC (6, 3) = SRC (7, 4) = t2;  //(t1 + 2*t2 + t3 + 2) >> 2;
  SRC (4, 0) = SRC (5, 1) = SRC (6, 2) = SRC (7, 3) = t3;       //(t2 + 2*t3 + t4 + 2) >> 2;
  SRC (5, 0) = SRC (6, 1) = SRC (7, 2) = t4;    //(t3 + 2*t4 + t5 + 2) >> 2;
  SRC (6, 0) = SRC (7, 1) = t5; //(t4 + 2*t5 + t6 + 2) >> 2;
  SRC (7, 0) = t6;              //(t5 + 2*t6 + t7 + 2) >> 2;

}

/****************************************************************************
 * Exported functions:
 ****************************************************************************/
void
xavs_predict_8x8c_init (int cpu, xavs_predict_t pf[7])
{
  pf[I_PRED_CHROMA_V] = predict_8x8c_v;
  pf[I_PRED_CHROMA_H] = predict_8x8c_h;
  pf[I_PRED_CHROMA_DC] = predict_8x8c_dc;
  pf[I_PRED_CHROMA_P] = predict_8x8c_p;
  pf[I_PRED_CHROMA_DC_LEFT] = predict_8x8c_dc_left;
  pf[I_PRED_CHROMA_DC_TOP] = predict_8x8c_dc_top;
  pf[I_PRED_CHROMA_DC_128] = predict_8x8c_dc_128;

#ifdef HAVE_MMXEXT
  if (cpu & XAVS_CPU_MMXEXT)
  {
    xavs_predict_8x8c_init_mmxext (pf);
  }
#endif
}

void
xavs_predict_8x8_init (int cpu, xavs_predict_t pf[12])
{
  pf[I_PRED_8x8_V] = predict_8x8_v;
  pf[I_PRED_8x8_H] = predict_8x8_h;
  pf[I_PRED_8x8_DC] = predict_8x8_dc;
  pf[I_PRED_8x8_DDL] = predict_8x8_ddl;
  pf[I_PRED_8x8_DDR] = predict_8x8_ddr;
  pf[I_PRED_8x8_DC_LEFT] = predict_8x8_dc_left;
  pf[I_PRED_8x8_DC_TOP] = predict_8x8_dc_top;
  pf[I_PRED_8x8_DC_128] = predict_8x8_dc_128;

#ifdef HAVE_MMXEXT
  if (cpu & XAVS_CPU_MMXEXT)
  {
    xavs_predict_8x8_init_mmxext (pf);
  }
#endif
#ifdef HAVE_SSE2
  if (cpu & XAVS_CPU_SSE2)
  {
    xavs_predict_8x8_init_sse2 (pf);
  }
#endif
}
