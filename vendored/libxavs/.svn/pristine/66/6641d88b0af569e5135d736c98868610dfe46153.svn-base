/*****************************************************************************
 * quant.h: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2005-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
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
 * quant.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_QUANT_H_
#define _XAVS_QUANT_H_

typedef struct
{
  //int (*quant_8x8) (int16_t dct[8][8], int mf[64], uint16_t bias[64], int qp);
  int (*quant_8x8_core) (int16_t dct[8][8], int mf[64], uint16_t bias[64], int qp, int *p,int *q);

  //void (*dequant_8x8) (int16_t dct[8][8], int dequant_mf[64][8][8], int i_qp);
  void (*dequant_8x8) (int16_t dct[8][8], int dequant_mf[64][8][8], int i_qp, uint16_t dequant_shifttable[64]);
} xavs_quant_function_t;

void xavs_quant_init (xavs_t * h, int cpu, xavs_quant_function_t * pf);
int quant_8x8 (xavs_t * h,int16_t dct[8][8], int mf[64], uint16_t bias[64], int qp);

#endif
