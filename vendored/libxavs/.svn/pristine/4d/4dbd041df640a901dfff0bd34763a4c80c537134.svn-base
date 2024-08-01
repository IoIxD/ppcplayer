/*****************************************************************************
 * frame.c: h264 encoder library
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
 * frame.c: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "common.h"

xavs_frame_t *
xavs_frame_new (xavs_t * h)
{
  xavs_frame_t *frame = xavs_malloc (sizeof (xavs_frame_t));
  int i, j;

  int i_mb_count = h->mb.i_mb_count;
  int i_stride;
  int i_lines;

  memset (frame, 0, sizeof (xavs_frame_t));

  /* allocate frame data (+64 for extra data for me) */
  i_stride = ((h->param.i_width + 15) & 0xfffff0) + 64;
  i_lines = ((h->param.i_height + 15) & 0xfffff0);

  frame->i_plane = 3;
  for (i = 0; i < 3; i++)
  {
    int i_divh = 1;
    int i_divw = 1;
    if (i > 0)
    {
      if (h->param.i_csp == XAVS_CSP_I420)
        i_divh = i_divw = 2;
      else if (h->param.i_csp == XAVS_CSP_I422)
        i_divw = 2;
    }
    frame->i_stride[i] = i_stride / i_divw;
    frame->i_lines[i] = i_lines / i_divh;
    frame->buffer[i] = xavs_malloc (frame->i_stride[i] * (frame->i_lines[i] + 64 / i_divh));

    frame->plane[i] = ((uint8_t *) frame->buffer[i]) + frame->i_stride[i] * 32 / i_divh + 32 / i_divw;
  }
  frame->i_stride[3] = 0;
  frame->i_lines[3] = 0;
  frame->buffer[3] = NULL;
  frame->plane[3] = NULL;

  frame->filtered[0] = frame->plane[0];
  for (i = 0; i < 3; i++)
  {
    frame->buffer[4 + i] = xavs_malloc (frame->i_stride[0] * (frame->i_lines[0] + 64));

    frame->filtered[i + 1] = ((uint8_t *) frame->buffer[4 + i]) + frame->i_stride[0] * 32 + 32;
  }

  if (h->frames.b_have_lowres)
  {
    frame->i_stride_lowres = frame->i_stride[0] / 2 + 32;
    frame->i_lines_lowres = frame->i_lines[0] / 2;
    for (i = 0; i < 4; i++)
    {
      frame->buffer[7 + i] = xavs_malloc (frame->i_stride_lowres * (frame->i_lines[0] / 2 + 64));
      frame->lowres[i] = ((uint8_t *) frame->buffer[7 + i]) + frame->i_stride_lowres * 32 + 32;
    }
  }

  if (h->param.analyse.i_me_method == XAVS_ME_ESA)
  {
    frame->buffer[11] = xavs_malloc (frame->i_stride[0] * (frame->i_lines[0] + 64) * sizeof (uint16_t));
    frame->integral = (uint16_t *) frame->buffer[11] + frame->i_stride[0] * 32 + 32;
  }

  frame->i_poc = -1;
  frame->i_type = XAVS_TYPE_AUTO;
  frame->i_qpplus1 = 0;
  frame->i_pts = -1;
  frame->i_frame = -1;
  frame->i_frame_num = -1;

  frame->mb_type = xavs_malloc (i_mb_count * sizeof (int8_t));
  frame->mv[0] = xavs_malloc (2 * 16 * i_mb_count * sizeof (int16_t));
  frame->ref[0] = xavs_malloc (4 * i_mb_count * sizeof (int8_t));
  if (h->param.i_bframe)
  {
    frame->mv[1] = xavs_malloc (2 * 16 * i_mb_count * sizeof (int16_t));
    frame->ref[1] = xavs_malloc (4 * i_mb_count * sizeof (int8_t));
  }
  else
  {
    frame->mv[1] = NULL;
    frame->ref[1] = NULL;
  }

  frame->i_row_bits = xavs_malloc (i_lines / 16 * sizeof (int));
  frame->i_row_qp = xavs_malloc (i_lines / 16 * sizeof (int));
  for (i = 0; i < h->param.i_bframe + 2; i++)
    for (j = 0; j < h->param.i_bframe + 2; j++)
      frame->i_row_satds[i][j] = xavs_malloc (i_lines / 16 * sizeof (int));

  xavs_pthread_mutex_init(&frame->mutex, NULL);
  return frame;
}

void
xavs_frame_delete (xavs_frame_t * frame)
{
  int i, j;
  if (frame == NULL)
    return;

  for (i = 0; i < frame->i_plane; i++)
    xavs_free (frame->buffer[i]);
  for (i = 4; i < 12; i++)      /* filtered planes */
    xavs_free (frame->buffer[i]);
  for (i = 0; i < XAVS_BFRAME_MAX + 2; i++)
    for (j = 0; j < XAVS_BFRAME_MAX + 2; j++)
      xavs_free (frame->i_row_satds[i][j]);
  xavs_free (frame->i_row_bits);
  xavs_free (frame->i_row_qp);
  xavs_free (frame->mb_type);
  xavs_free (frame->mv[0]);
  xavs_free (frame->mv[1]);
  xavs_free (frame->ref[0]);
  xavs_free (frame->ref[1]);
  xavs_free (frame);
}

void
xavs_frame_copy_picture (xavs_t * h, xavs_frame_t * dst, xavs_picture_t * src)
{
  dst->i_type = src->i_type;
  dst->i_qpplus1 = src->i_qpplus1;
  dst->i_pts = src->i_pts;

  switch (src->img.i_csp & XAVS_CSP_MASK)
  {
  case XAVS_CSP_I420:
    h->csp.i420 (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_YV12:
    h->csp.yv12 (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_I422:
    h->csp.i422 (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_I444:
    h->csp.i444 (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_YUYV:
    h->csp.yuyv (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_RGB:
    h->csp.rgb (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_BGR:
    h->csp.bgr (dst, &src->img, h->param.i_width, h->param.i_height);
    break;
  case XAVS_CSP_BGRA:
    h->csp.bgra (dst, &src->img, h->param.i_width, h->param.i_height);
    break;

  default:
    xavs_log (h, XAVS_LOG_ERROR, "Arg invalid CSP\n");
    break;
  }
}

void
xavs_frame_put (xavs_frame_t ** list, xavs_frame_t * frame)
{
  int i = 0;
  while (list[i])
    i++;
  list[i] = frame;
}

void
xavs_frame_push (xavs_frame_t ** list, xavs_frame_t * frame)
{
  int i = 0;
  while (list[i])
    i++;
  while (i--)
    list[i + 1] = list[i];
  list[0] = frame;
}

xavs_frame_t * xavs_frame_get (xavs_frame_t ** list)
{
  xavs_frame_t *frame = list[0];
  int i;
  for (i = 0; list[i]; i++)
    list[i] = list[i + 1];
  return frame;
}

xavs_frame_t * xavs_frame_get_unused (xavs_t *h)
{
  xavs_frame_t *frame;
  if (h->frames.unused[0])
    frame = xavs_frame_get(h->frames.unused);
  else 
    frame = xavs_frame_new(h);

  if (frame)
  {
    frame->b_kept_as_ref = 0;
    frame->b_last_minigop_bframe = 0;
    frame->i_reference_count = 1;
  }

  return frame;
}

void xavs_frame_put_unused (xavs_t *h, xavs_frame_t *frame)
{
  assert (frame->i_reference_count > 0);
  if (--frame->i_reference_count == 0)
    xavs_frame_put(h->frames.unused, frame);
}

static void
plane_expand_border (uint8_t * pix, int i_stride, int i_height, int i_pad)
{
#define PPIXEL(x, y) ( pix + (x) + (y)*i_stride )
  const int i_width = i_stride - 2 * i_pad;
  int y;

  for (y = 0; y < i_height; y++)
  {
    /* left band */
    memset (PPIXEL (-i_pad, y), PPIXEL (0, y)[0], i_pad);
    /* right band */
    memset (PPIXEL (i_width, y), PPIXEL (i_width - 1, y)[0], i_pad);
  }
  /* upper band */
  for (y = 0; y < i_pad; y++)
    memcpy (PPIXEL (-i_pad, -y - 1), PPIXEL (-i_pad, 0), i_stride);
  /* lower band */
  for (y = 0; y < i_pad; y++)
    memcpy (PPIXEL (-i_pad, i_height + y), PPIXEL (-i_pad, i_height - 1), i_stride);
#undef PPIXEL
}

void
xavs_frame_expand_border (xavs_frame_t * frame)
{
  int i;
  for (i = 0; i < frame->i_plane; i++)
  {
    int i_pad = i ? 16 : 32;
    plane_expand_border (frame->plane[i], frame->i_stride[i], frame->i_lines[i], i_pad);
  }
}

void
xavs_frame_expand_border_filtered (xavs_frame_t * frame)
{
  /* during filtering, 8 extra pixels were filtered on each edge. 
     we want to expand border from the last filtered pixel */
  int i;
  for (i = 1; i < 4; i++)
    plane_expand_border (frame->filtered[i] - 8 * frame->i_stride[0] - 8, frame->i_stride[0], frame->i_lines[0] + 2 * 8, 24);
}

void
xavs_frame_expand_border_lowres (xavs_frame_t * frame)
{
  int i;
  for (i = 0; i < 4; i++)
    plane_expand_border (frame->lowres[i], frame->i_stride_lowres, frame->i_lines_lowres, 32);
}

void
xavs_frame_expand_border_mod16 (xavs_t * h, xavs_frame_t * frame)
{
  int i, y;
  for (i = 0; i < frame->i_plane; i++)
  {
    int i_subsample = i ? 1 : 0;
    int i_width = h->param.i_width >> i_subsample;
    int i_height = h->param.i_height >> i_subsample;
    int i_padx = (h->sps->i_mb_width * 16 - h->param.i_width) >> i_subsample;
    int i_pady = (h->sps->i_mb_height * 16 - h->param.i_height) >> i_subsample;

    if (i_padx)
    {
      for (y = 0; y < i_height; y++)
        memset (&frame->plane[i][y * frame->i_stride[i] + i_width], frame->plane[i][y * frame->i_stride[i] + i_width - 1], i_padx);
    }
    if (i_pady)
    {
      for (y = i_height; y < i_height + i_pady; y++);
      memcpy (&frame->plane[i][y * frame->i_stride[i]], &frame->plane[i][(i_height - 1) * frame->i_stride[i]], i_width + i_padx);
    }
  }
}

/* Deblocking filter */
static inline int
clip_uint8 (int a)
{
  if (a & (~255))
    return (-a) >> 31;
  else
    return a;
}

static const int i_alpha_table[64] = {
  0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 1, 2, 2, 2, 3, 3,
  4, 4, 5, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 15, 16, 18, 20,
  22, 24, 26, 28, 30, 33, 33, 35,
  35, 36, 37, 37, 39, 39, 42, 44,
  46, 48, 50, 52, 53, 54, 55, 56,
  57, 58, 59, 60, 61, 62, 63, 64
};

static const int i_beta_table[64] = {
  0, 0, 0, 0, 0, 0, 1, 1,
  1, 1, 1, 1, 1, 2, 2, 2,
  2, 2, 3, 3, 3, 3, 4, 4,
  4, 4, 5, 5, 5, 5, 6, 6,
  6, 7, 7, 7, 8, 8, 8, 9,
  9, 10, 10, 11, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22,
  23, 23, 24, 24, 25, 25, 26, 27
};

static const int CLIP_TAB[64] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 2, 2,
  2, 2, 2, 2, 2, 2, 3, 3,
  3, 3, 3, 3, 3, 4, 4, 4,
  5, 5, 5, 6, 6, 6, 7, 7,
  7, 7, 8, 8, 8, 9, 9, 9
};

static inline void
deblock_luma_intra_c (uint8_t * pix, int xstride, int ystride, int alpha, int beta)
{
  int d;
  for (d = 0; d < 16; d++)      //the picture's height(vertical),three pix at each side
  {
    const int p2 = pix[-3 * xstride];
    const int p1 = pix[-2 * xstride];
    const int p0 = pix[-1 * xstride];
    const int q0 = pix[0 * xstride];
    const int q1 = pix[1 * xstride];
    const int q2 = pix[2 * xstride];

    if (abs (p0 - q0) < alpha && abs (p1 - p0) < beta && abs (q1 - q0) < beta)
    {
      int ap = (abs (p2 - p0) < beta);
      int aq = (abs (q2 - q0) < beta);
      int k = (abs (p0 - q0) < ((alpha >> 2) + 2));

      if (ap & k)
      {
        pix[-1 * xstride] = (p1 + 2 * p0 + q0 + 2) >> 2;        /*P0 */
        pix[-2 * xstride] = (2 * p1 + p0 + q0 + 2) >> 2;        /*p1 */
      }
      else
      {
        pix[-1 * xstride] = (2 * p1 + p0 + q0 + 2) >> 2;        /*p0 */

      }
      if (aq & k)
      {
        pix[0 * xstride] = (q1 + 2 * q0 + p0 + 2) >> 2; /*q0 */
        pix[1 * xstride] = (2 * q1 + q0 + p0 + 2) >> 2; /*q1 */
      }
      else
      {
        pix[0 * xstride] = (2 * q1 + q0 + p0 + 2) >> 2; /*q0 */
      }
    }
    pix += ystride;
  }
}

static inline void
deblock_chroma_intra_c (uint8_t * pix, int xstride, int ystride, int alpha, int beta)
{
  int d;
  for (d = 0; d < 8; d++)
  {
    const int p2 = pix[-3 * xstride];
    const int p1 = pix[-2 * xstride];
    const int p0 = pix[-1 * xstride];
    const int q0 = pix[0 * xstride];
    const int q1 = pix[1 * xstride];
    const int q2 = pix[2 * xstride];

    if (abs (p0 - q0) < alpha && abs (p1 - p0) < beta && abs (q1 - q0) < beta)
    {
      int ap = (abs (p2 - p0) < beta);
      int aq = (abs (q2 - q0) < beta);
      int k = (abs (p0 - q0) < ((alpha >> 2) + 2));

      if (ap && k)
        pix[-1 * xstride] = (p1 + 2 * p0 + q0 + 2) >> 2;        /*p0 */
      else
        pix[-1 * xstride] = (2 * p1 + p0 + q0 + 2) >> 2;        /*p0 */
      if (aq & k)
        pix[0 * xstride] = (q1 + 2 * q0 + p0 + 2) >> 2; /*q0 */
      else
        pix[0 * xstride] = (2 * q1 + q0 + p0 + 2) >> 2; /*q0 */
    }
    pix += ystride;
  }
}

static inline void
deblock_luma_c (uint8_t * pix, int xstride, int ystride, int alpha, int beta, int8_t * tc0)
{
  int i, d;
  for (i = 0; i < 2; i++)
  {
    const int8_t tc = tc0[i];
    if (tc < 0)
    {
      pix += 8 * ystride;
      continue;
    }

    for (d = 0; d < 8; d++)
    {
      const int p2 = pix[-3 * xstride];
      const int p1 = pix[-2 * xstride];
      const int p0 = pix[-1 * xstride];
      const int q0 = pix[0 * xstride];
      const int q1 = pix[1 * xstride];
      const int q2 = pix[2 * xstride];

      int ap = abs (p2 - p0);
      int aq = abs (q2 - q0);

      if (abs (p0 - q0) < alpha && abs (p1 - p0) < beta && abs (q1 - q0) < beta)
      {
        int delta = xavs_clip3 ((((q0 - p0) * 3 + (p1 - q1) + 4) >> 3), -tc, tc);

        pix[-1 * xstride] = clip_uint8 (p0 + delta);    /*p0 */
        pix[0 * xstride] = clip_uint8 (q0 - delta);     /*q0 */

        if (ap < beta)
          pix[-2 * xstride] = clip_uint8 (p1 + xavs_clip3 ((((pix[-1 * xstride] - p1) * 3 + (p2 - pix[0 * xstride]) + 4) >> 3), -tc, tc));      /*p1 */
        if (aq < beta)
          pix[1 * xstride] = clip_uint8 (q1 - xavs_clip3 ((((q1 - pix[0 * xstride]) * 3 + (pix[-1 * xstride] - q2) + 4) >> 3), -tc, tc));       /*q1 */
      }
      pix += ystride;

    }
  }
}

static inline void
deblock_chroma_c (uint8_t * pix, int xstride, int ystride, int alpha, int beta, int8_t * tc0)
{
  int i, d;

  for (i = 0; i < 2; i++)
  {
    const int8_t tc = tc0[i];
    if (tc < 0)
    {
      pix += 4 * ystride;
      continue;
    }
    for (d = 0; d < 4; d++)
    {
      const int p1 = pix[-2 * xstride];
      const int p0 = pix[-1 * xstride];
      const int q0 = pix[0 * xstride];
      const int q1 = pix[1 * xstride];

      if (abs (p0 - q0) < alpha && abs (p1 - p0) < beta && abs (q1 - q0) < beta)
      {
        int delta = xavs_clip3 ((((q0 - p0) * 3 + (p1 - q1) + 4) >> 3), -tc, tc);

        pix[-1 * xstride] = clip_uint8 (p0 + delta);    /*p0 */
        pix[0 * xstride] = clip_uint8 (q0 - delta);     /*q0 */
        /*if the edge£¬no deblocking for p1 and q1 */
      }
      pix += ystride;
    }
  }
}

static void
deblock_v_luma_intra_c (uint8_t * pix, int stride, int alpha, int beta)
{
  deblock_luma_intra_c (pix, stride, 1, alpha, beta);
}
static void
deblock_h_luma_intra_c (uint8_t * pix, int stride, int alpha, int beta)
{
  deblock_luma_intra_c (pix, 1, stride, alpha, beta);
}
static void
deblock_v_chroma_intra_c (uint8_t * pix, int stride, int alpha, int beta)
{
  deblock_chroma_intra_c (pix, stride, 1, alpha, beta);
}

static void
deblock_h_chroma_intra_c (uint8_t * pix, int stride, int alpha, int beta)
{
  deblock_chroma_intra_c (pix, 1, stride, alpha, beta);
}
static void
deblock_v_luma_c (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0)
{
  deblock_luma_c (pix, stride, 1, alpha, beta, tc0);
}
static void
deblock_h_luma_c (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0)
{
  deblock_luma_c (pix, 1, stride, alpha, beta, tc0);
}
static void
deblock_v_chroma_c (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0)
{
  deblock_chroma_c (pix, stride, 1, alpha, beta, tc0);
}

static void
deblock_h_chroma_c (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0)
{
  deblock_chroma_c (pix, 1, stride, alpha, beta, tc0);
}

static inline void
xavs_deblock_edge (xavs_t * h, uint8_t * pix, int i_stride, int bS[2], int i_qp, int b_chroma, xavs_deblock_inter_t pf_inter, xavs_deblock_intra_t pf_intra)
{
  int i;
  const int index_a = xavs_clip3 (i_qp + h->sh.i_alpha_c0_offset, 0, 63);
  const int index_b = xavs_clip3 (i_qp + h->sh.i_beta_offset, 0, 63);
  const int alpha = i_alpha_table[index_a];
  const int beta = i_beta_table[index_b];

  if (bS[0] < 2)
  {
    int8_t tc[2];
    for (i = 0; i < 2; i++)
      tc[i] = bS[i] ? CLIP_TAB[index_a] : -1;
    pf_inter (pix, i_stride, alpha, beta, tc);
  }
  else
  {
    pf_intra (pix, i_stride, alpha, beta);
  }
}

void
xavs_frame_deblocking_filter (xavs_t * h, int i_slice_type)
{
  /*8x8 block width, h->mb.i_mb_stride is the MB numbers for one row */
  const int s8x8 = 2 * h->mb.i_mb_stride;
  const int s4x4 = 4 * h->mb.i_mb_stride;
  int mb_y;
  int mb_x;

  for (mb_y = 0, mb_x = 0; mb_y < h->sps->i_mb_height;)
  {
    const int mb_xy = mb_y * h->mb.i_mb_stride + mb_x;  /*the MB order in the picture */
    const int mb_8x8 = 2 * s8x8 * mb_y + 2 * mb_x;      /*the 8x8 block order in the picture */
    const int mb_4x4 = 4 * s4x4 * mb_y + 4 * mb_x;
    int i_edge, i_dir;
    int i_qp, i_qpn;

    /*current MB start position for luma and chroma */
    int i_pix_y[3] = { 16 * mb_y * h->fdec->i_stride[0] + 16 * mb_x,
      8 * mb_y * h->fdec->i_stride[1] + 8 * mb_x,
      8 * mb_y * h->fdec->i_stride[2] + 8 * mb_x
    };
    /*i_dir == 0 -> vertical edge
       i_dir == 1  -> horizontal edge */

    for (i_dir = 0; i_dir < 2; i_dir++)
    {
      int i_start = ((i_dir == 0 && mb_x != 0) || (i_dir == 1 && mb_y != 0)) ? 0 : 1;
      /*if edge, no deblocking is required */
      for (i_edge = i_start; i_edge < 2; i_edge++)
      {
        int mbn_xy, mbn_8x8, mbn_4x4;
        int bS[2];              /*strength of each edge */

        mbn_xy = i_edge > 0 ? mb_xy : (i_dir == 0 ? mb_xy - 1 : mb_xy - h->mb.i_mb_stride);
        mbn_8x8 = i_edge > 0 ? mb_8x8 : (i_dir == 0 ? mb_8x8 - 2 : mb_8x8 - 2 * s8x8);
        mbn_4x4 = i_edge > 0 ? mb_4x4 : (i_dir == 0 ? mb_4x4 - 4 : mb_4x4 - 4 * s4x4);

        /*Get bs for each 8px for the current edge */
        if (IS_INTRA (h->mb.type[mb_xy]) || IS_INTRA (h->mb.type[mbn_xy]))
        {
          bS[0] = bS[1] = 2;
        }
        else
        {
          int i;
          for (i = 0; i < 2; i++)
          {
            int x = i_dir == 0 ? i_edge : i;
            int y = i_dir == 0 ? i : i_edge;
            int xn = (x - (i_dir == 0 ? 1 : 0)) & 0x01;
            int yn = (y - (i_dir == 0 ? 0 : 1)) & 0x01;
            int i8p = mb_8x8 + x + y * s8x8;
            int i8q = mbn_8x8 + xn + yn * s8x8; //2)
            int k;
            int i4p = mb_4x4 + 2 * x + 2 * y * s4x4;
            int i4q = mbn_4x4 + 2 * xn + 2 * yn * s4x4;

            bS[i] = 0;

            for (k = 0; k < 1 + (i_slice_type == SLICE_TYPE_B); k++)
            {
              if (h->mb.ref[k][i8p] != h->mb.ref[k][i8q] || abs (h->mb.mv[k][i4p][0] - h->mb.mv[k][i4q][0]) >= 4 || abs (h->mb.mv[k][i4p][1] - h->mb.mv[k][i4q][1]) >= 4)
              {
                bS[i] = 1;
                break;
              }
            }
          }
        }

        /*filter */
        /*Y plane */
        i_qp = h->mb.qp[mb_xy];
        i_qpn = h->mb.qp[mbn_xy];

        if (i_dir == 0)
        {
          /*Vertical edge Y */
          xavs_deblock_edge (h, &h->fdec->plane[0][i_pix_y[0] + 8 * i_edge], h->fdec->i_stride[0], bS, (i_qp + i_qpn + 1) >> 1, 0, h->loopf.deblock_h_luma, h->loopf.deblock_h_luma_intra);

          if (!(i_edge & 1))
          {
            //U/V planes
            int i_qpc = (i_chroma_qp_table[xavs_clip3 (i_qp, 0, 63)] + i_chroma_qp_table[xavs_clip3 (i_qpn, 0, 63)] + 1) >> 1;
            xavs_deblock_edge (h, &h->fdec->plane[1][i_pix_y[1] + 4 * i_edge], h->fdec->i_stride[1], bS, i_qpc, 1, h->loopf.deblock_h_chroma, h->loopf.deblock_h_chroma_intra);
            xavs_deblock_edge (h, &h->fdec->plane[2][i_pix_y[2] + 4 * i_edge], h->fdec->i_stride[2], bS, i_qpc, 1, h->loopf.deblock_h_chroma, h->loopf.deblock_h_chroma_intra);
          }
        }
        else
        {
          //horizontal edge
          xavs_deblock_edge (h, &h->fdec->plane[0][i_pix_y[0] + 8 * i_edge * h->fdec->i_stride[0]], h->fdec->i_stride[0], bS, (i_qp + i_qpn + 1) >> 1, 0, h->loopf.deblock_v_luma, h->loopf.deblock_v_luma_intra);
          //U/V planes
          if (!(i_edge & 1))
          {
            int i_qpc = (i_chroma_qp_table[xavs_clip3 (i_qp, 0, 63)] + i_chroma_qp_table[xavs_clip3 (i_qpn, 0, 63)] + 1) >> 1;
            xavs_deblock_edge (h, &h->fdec->plane[1][i_pix_y[1] + 4 * i_edge * h->fdec->i_stride[1]], h->fdec->i_stride[1], bS, i_qpc, 1, h->loopf.deblock_v_chroma, h->loopf.deblock_v_chroma_intra);
            xavs_deblock_edge (h, &h->fdec->plane[2][i_pix_y[2] + 4 * i_edge * h->fdec->i_stride[2]], h->fdec->i_stride[2], bS, i_qpc, 1, h->loopf.deblock_v_chroma, h->loopf.deblock_v_chroma_intra);

          }
        }
      }
    }

    mb_x++;
    if (mb_x >= h->sps->i_mb_width)
    {
      mb_x = 0;
      mb_y++;
    }
  }
}


#ifdef HAVE_MMXEXT
void xavs_deblock_v_chroma_mmxext (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
void xavs_deblock_h_chroma_mmxext (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
void xavs_deblock_v_chroma_intra_mmxext (uint8_t * pix, int stride, int alpha, int beta);
void xavs_deblock_h_chroma_intra_mmxext (uint8_t * pix, int stride, int alpha, int beta);
void xavs_deblock_v_luma_intra_sse2 (uint8_t * pix, int stride, int alpha, int beta);
void xavs_deblock_h_luma_intra_sse2 (uint8_t * pix, int stride, int alpha, int beta); 
#endif

#ifdef ARCH_X86_64
void xavs_deblock_v_luma_sse2 (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
void xavs_deblock_h_luma_sse2 (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
#elif defined( HAVE_MMXEXT )
/*
void xavs_deblock_h_luma_mmxext (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
void xavs_deblock_v8_luma_mmxext (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);

void
xavs_deblock_v_luma_mmxext (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0)
{
  xavs_deblock_v8_luma_mmxext (pix, stride, alpha, beta, tc0);
  xavs_deblock_v8_luma_mmxext (pix + 8, stride, alpha, beta, tc0 + 2);
}
*/
#endif

void
xavs_deblock_init (int cpu, xavs_deblock_function_t * pf)
{
  pf->deblock_v_luma = deblock_v_luma_c;
  pf->deblock_h_luma = deblock_h_luma_c;
  pf->deblock_v_chroma = deblock_v_chroma_c;
  pf->deblock_h_chroma = deblock_h_chroma_c;
  pf->deblock_v_luma_intra = deblock_v_luma_intra_c;
  pf->deblock_h_luma_intra = deblock_h_luma_intra_c;
  pf->deblock_v_chroma_intra = deblock_v_chroma_intra_c;
  pf->deblock_h_chroma_intra = deblock_h_chroma_intra_c;

#ifdef HAVE_MMXEXT
  if (cpu & XAVS_CPU_SSE)
  {
     pf->deblock_v_chroma = xavs_deblock_v_chroma_mmxext;
     pf->deblock_h_chroma = xavs_deblock_h_chroma_mmxext;
  }

  if (cpu & XAVS_CPU_MMXEXT)
  {
    pf->deblock_v_chroma = xavs_deblock_v_chroma_mmxext;
    pf->deblock_h_chroma = xavs_deblock_h_chroma_mmxext;
    pf->deblock_v_chroma_intra = xavs_deblock_v_chroma_intra_mmxext;
    pf->deblock_h_chroma_intra = xavs_deblock_h_chroma_intra_mmxext;

#ifdef ARCH_X86_64
    if (cpu & XAVS_CPU_SSE2)
    {
      pf->deblock_v_luma = xavs_deblock_v_luma_sse2;
      pf->deblock_h_luma = xavs_deblock_h_luma_sse2;
    }
#else
/*    
    pf->deblock_v_luma = xavs_deblock_v_luma_mmxext;
    pf->deblock_h_luma = xavs_deblock_h_luma_mmxext;
*/
#endif
  }
#endif
}

int xavs_synch_frame_list_init( xavs_synch_frame_list_t *slist, int max_size )
{
    if( max_size < 0 )
        return -1;
    slist->i_max_size = max_size;
    slist->i_size = 0;
    CHECKED_MALLOCZERO( slist->list, (max_size+1) * sizeof(xavs_frame_t*) );
    if( xavs_pthread_mutex_init( &slist->mutex, NULL ) ||
        xavs_pthread_cond_init( &slist->cv_fill, NULL ) ||
        xavs_pthread_cond_init( &slist->cv_empty, NULL ) )
        return -1;
    return 0;
fail:
    return -1;
}

void xavs_synch_frame_list_delete( xavs_synch_frame_list_t *slist )
{
    int i;
    if (!slist)
        return;

    xavs_pthread_mutex_destroy( &slist->mutex );
    xavs_pthread_cond_destroy( &slist->cv_fill );
    xavs_pthread_cond_destroy( &slist->cv_empty );

    for (i = 0; i < slist->i_max_size; i++) 
    {
        if (slist->list[i])
            xavs_frame_delete(slist->list[i]);
    }
    xavs_free(slist->list);
}

void xavs_synch_frame_list_push( xavs_synch_frame_list_t *slist, xavs_frame_t *frame )
{
    xavs_pthread_mutex_lock( &slist->mutex );
    while( slist->i_size == slist->i_max_size )
        xavs_pthread_cond_wait( &slist->cv_empty, &slist->mutex );
    slist->list[ slist->i_size++ ] = frame;
    xavs_pthread_mutex_unlock( &slist->mutex );
    xavs_pthread_cond_broadcast( &slist->cv_fill );
}

