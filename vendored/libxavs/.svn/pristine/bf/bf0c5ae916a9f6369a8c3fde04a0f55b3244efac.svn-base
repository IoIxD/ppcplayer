/*****************************************************************************
 * predict.h: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2003-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
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
 * predict.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_PREDICT_H_
#define _XAVS_PREDICT_H_

typedef void (*xavs_predict_t) (uint8_t * src, int i_neighbor);

enum intra_chroma_pred_e
{
  I_PRED_CHROMA_DC = 0,
  I_PRED_CHROMA_H = 1,
  I_PRED_CHROMA_V = 2,
  I_PRED_CHROMA_P = 3,

  I_PRED_CHROMA_DC_LEFT = 4,
  I_PRED_CHROMA_DC_TOP = 5,
  I_PRED_CHROMA_DC_128 = 6
};
static const int xavs_mb_pred_mode8x8c[7] = {
  I_PRED_CHROMA_DC, I_PRED_CHROMA_H, I_PRED_CHROMA_V, I_PRED_CHROMA_P,
  I_PRED_CHROMA_DC, I_PRED_CHROMA_DC, I_PRED_CHROMA_DC
};

/* must use the same numbering as intra4x4_pred_e */
enum intra8x8_pred_e
{
  I_PRED_8x8_V = 0,
  I_PRED_8x8_H = 1,
  I_PRED_8x8_DC = 2,
  I_PRED_8x8_DDL = 3,
  I_PRED_8x8_DDR = 4,

  I_PRED_8x8_DC_LEFT = 5,
  I_PRED_8x8_DC_TOP = 6,
  I_PRED_8x8_DC_128 = 7,
};
#define xavs_mb_pred_mode8x8(t) xavs_mb_pred_mode8x8[(t)+1]
static const int xavs_mb_pred_mode8x8[9] = {
  -1, I_PRED_8x8_V, I_PRED_8x8_H, I_PRED_8x8_DC,
  I_PRED_8x8_DDL, I_PRED_8x8_DDR, I_PRED_8x8_DC,
  I_PRED_8x8_DC, I_PRED_8x8_DC
};
void xavs_predict_8x8c_init (int cpu, xavs_predict_t pf[7]);
void xavs_predict_8x8_init (int cpu, xavs_predict_t pf[12]);

#endif
