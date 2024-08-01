/*****************************************************************************
 * pixel.h: h264 encoder library
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
 * pixel.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_PIXEL_H_
#define _XAVS_PIXEL_H_

typedef int (*xavs_pixel_cmp_t) (uint8_t *, int, uint8_t *, int);
typedef int (*xavs_pixel_cmp_pde_t) (uint8_t *, int, uint8_t *, int, int);
typedef void (*xavs_pixel_cmp_x3_t) (uint8_t *, uint8_t *, uint8_t *, uint8_t *, int, int[3]);
typedef void (*xavs_pixel_cmp_x4_t) (uint8_t *, uint8_t *, uint8_t *, uint8_t *, uint8_t *, int, int[4]);

enum
{
  PIXEL_16x16 = 0,
  PIXEL_16x8 = 1,
  PIXEL_8x16 = 2,
  PIXEL_8x8 = 3,
  PIXEL_8x4 = 4,
  PIXEL_4x8 = 5,
  PIXEL_4x4 = 6,
  PIXEL_4x2 = 7,
  PIXEL_2x4 = 8,
  PIXEL_2x2 = 9,
};

static const struct
{
  int w;
  int h;
} xavs_pixel_size[7] =
{
  {
  16, 16},
  {
  16, 8},
  {
  8, 16},
  {
  8, 8},
  {
  8, 4},
  {
  4, 8},
  {
  4, 4}
};

static const int xavs_size2pixel[5][5] = {
  {0,},
  {0, PIXEL_4x4, PIXEL_8x4, 0, 0},
  {0, PIXEL_4x8, PIXEL_8x8, 0, PIXEL_16x8},
  {0,},
  {0, 0, PIXEL_8x16, 0, PIXEL_16x16}
};

typedef struct
{
  xavs_pixel_cmp_t sad[7];
  xavs_pixel_cmp_t ssd[7];
  xavs_pixel_cmp_t satd[7];
  xavs_pixel_cmp_t sa8d[4];
  xavs_pixel_cmp_t mbcmp[7];    /* either satd or sad for subpel refine and mode decision */

  /* partial distortion elimination:
   * terminate early if partial score is worse than a threshold.
   * may be NULL, in which case just use sad instead. */
  xavs_pixel_cmp_pde_t sad_pde[7];

  /* multiple parallel calls to sad. */
  xavs_pixel_cmp_x3_t sad_x3[7];
  xavs_pixel_cmp_x4_t sad_x4[7];
} xavs_pixel_function_t;

void xavs_pixel_init (int cpu, xavs_pixel_function_t * pixf);
int64_t xavs_pixel_ssd_wxh (xavs_pixel_function_t * pf, uint8_t * pix1, int i_pix1, uint8_t * pix2, int i_pix2, int i_width, int i_height);

#endif
