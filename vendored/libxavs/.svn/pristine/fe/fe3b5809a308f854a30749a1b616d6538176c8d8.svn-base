/*****************************************************************************
 * mc.h: h264 encoder library (Motion Compensation)
 *****************************************************************************
 * Copyright (C) 2004-2008 Loren Merritt <lorenm@u.washington.edu>
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
 * mc.h: xavs encoder library (Motion Compensation)
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_MC_H_
#define _XAVS_MC_H_

/* Do the MC
 * XXX: Only width = 4, 8 or 16 are valid
 * width == 4 -> height == 4 or 8
 * width == 8 -> height == 4 or 8 or 16
 * width == 16-> height == 8 or 16
 * */

typedef struct
{
  void (*mc_luma) (uint8_t **, int, uint8_t *, int, int mvx, int mvy, int i_width, int i_height);

  uint8_t *(*get_ref) (uint8_t **, int, uint8_t *, int *, int mvx, int mvy, int i_width, int i_height);

  void (*mc_chroma) (uint8_t *, int, uint8_t *, int, int mvx, int mvy, int i_width, int i_height);

  void (*avg[10]) (uint8_t * dst, int, uint8_t * src, int);
  void (*avg_weight[10]) (uint8_t * dst, int, uint8_t * src, int, int i_weight);

  /* only 16x16, 8x8, and 4x4 defined */
  void (*copy[7]) (uint8_t * dst, int, uint8_t * src, int, int i_height);
} xavs_mc_functions_t;

void xavs_mc_init (int cpu, xavs_mc_functions_t * pf);

#endif
