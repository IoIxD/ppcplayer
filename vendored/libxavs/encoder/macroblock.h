/*****************************************************************************
 * macroblock.h: h264 encoder library
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
 * macroblock.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_ENC_MACROBLOCK_H_
#define _XAVS_ENC_MACROBLOCK_H_

#include "common/macroblock.h"

int xavs_macroblock_probe_skip (xavs_t * h, int b_bidir);

static inline int
xavs_macroblock_probe_pskip (xavs_t * h)
{
  return xavs_macroblock_probe_skip (h, 0);
}
static inline int
xavs_macroblock_probe_bskip (xavs_t * h)
{
  return xavs_macroblock_probe_skip (h, 1);
}

void xavs_macroblock_encode (xavs_t * h);
void xavs_macroblock_write_cavlc (xavs_t * h, bs_t * s);

void xavs_macroblock_encode_p8x8 (xavs_t * h, int i8);

void xavs_quant_8x8_trellis (xavs_t * h, int16_t dct[8][8], int i_quant_cat, int i_qp, int b_intra);

void xavs_noise_reduction_update (xavs_t * h);
void xavs_denoise_dct (xavs_t * h, int16_t * dct);
void xavs_mb_encode_8x8_chroma (xavs_t * h, int b_inter, int i_qscale);

static inline int
array_non_zero (int *v, int i_count)
{
  int i;
  for (i = 0; i < i_count; i++)
    if (v[i])
      return 1;
  return 0;
}

static inline int
array_non_zero_count (int *v, int i_count)
{
  int i;
  int i_nz;

  for (i = 0, i_nz = 0; i < i_count; i++)
    if (v[i])
      i_nz++;

  return i_nz;
}


#endif
