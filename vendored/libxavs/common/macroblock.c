/*****************************************************************************
 * macroblock.c: h264 encoder library
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
 * macroblock.c: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "common.h"

static const int dequant_mf[6][4][4] = {
  {{10, 13, 10, 13}, {13, 16, 13, 16}, {10, 13, 10, 13}, {13, 16, 13, 16}},
  {{11, 14, 11, 14}, {14, 18, 14, 18}, {11, 14, 11, 14}, {14, 18, 14, 18}},
  {{13, 16, 13, 16}, {16, 20, 16, 20}, {13, 16, 13, 16}, {16, 20, 16, 20}},
  {{14, 18, 14, 18}, {18, 23, 18, 23}, {14, 18, 14, 18}, {18, 23, 18, 23}},
  {{16, 20, 16, 20}, {20, 25, 20, 25}, {16, 20, 16, 20}, {20, 25, 20, 25}},
  {{18, 23, 18, 23}, {23, 29, 23, 29}, {18, 23, 18, 23}, {23, 29, 23, 29}}
};

int
xavs_mb_predict_intra4x4_mode (xavs_t * h, int idx)
{
  const int ma = h->mb.cache.intra4x4_pred_mode[xavs_scan8[idx] - 1];
  const int mb = h->mb.cache.intra4x4_pred_mode[xavs_scan8[idx] - 8];
  const int m = XAVS_MIN (xavs_mb_pred_mode8x8 (ma),
                          xavs_mb_pred_mode8x8 (mb));

  if (m < 0)
    return I_PRED_8x8_DC;

  return m;
}

int
xavs_mb_predict_non_zero_code (xavs_t * h, int idx)
{
  const int za = h->mb.cache.non_zero_count[xavs_scan8[idx] - 1];
  const int zb = h->mb.cache.non_zero_count[xavs_scan8[idx] - 8];

  int i_ret = za + zb;

  if (i_ret < 0x80)
  {
    i_ret = (i_ret + 1) >> 1;
  }
  return i_ret & 0x7f;
}

int
xavs_mb_transform_8x8_allowed (xavs_t * h)
{
  if (IS_SKIP (h->mb.i_type))
    return 0;
  if (h->mb.i_type == P_8x8 || h->mb.i_type == B_8x8)
  {
    int i;
    for (i = 0; i < 4; i++)
      if (!IS_SUB8x8 (h->mb.i_sub_partition[i]) || (h->mb.i_sub_partition[i] == D_DIRECT_8x8 && !h->sps->b_direct8x8_inference))
      {
        return 0;
      }
  }
  if (h->mb.i_type == B_DIRECT && !h->sps->b_direct8x8_inference)
    return 0;

  return 1;
}


void
xavs_mb_predict_mv (xavs_t * h, int i_list, int idx, int i_width, int mvp[2])
{
  int16_t mv_a[2], mv_b[2], mv_c[2];
  const int i8 = xavs_scan8[idx];
  const int i_ref = (h->sh.i_type == SLICE_TYPE_P) ? h->mb.cache.ref[i_list][i8] : 0;
  //int     i_lista= h->mb.cache.list[i8-1];
  int i_refa = h->mb.cache.ref[i_list][i8 - 1];
  //int16_t *mv_a  = h->mb.cache.mv[i_lista][i8 - 1];
  //mv_a[0]= h->mb.cache.mv[i_lista][i8 - 1][0];
  //mv_a[1]= h->mb.cache.mv[i_lista][i8 - 1][1];
  //int     i_listb= h->mb.cache.list[i8-8];
  int i_refb = h->mb.cache.ref[i_list][i8 - 8];
  //int16_t *mv_b  = h->mb.cache.mv[i_listb][i8 - 8];
  //int16_t mv_b[0]= h->mb.cache.mv[i_listb][i8 - 8][0];
  //int16_t mv_b[1]= h->mb.cache.mv[i_listb][i8 - 8][1];
  //int     i_listc= h->mb.cache.list[i8 - 8 + i_width];
  int i_refc = h->mb.cache.ref[i_list][i8 - 8 + i_width];
  //int16_t *mv_c  = h->mb.cache.mv[i_listc][i8 - 8 + i_width];
  //int16_t mv_c[0]= h->mb.cache.mv[i_listc][i8 - 8 + i_width][0];
  //int16_t mv_c[1]= h->mb.cache.mv[i_listc][i8 - 8 + i_width][1];

  int MV_A[2];
  int MV_B[2];
  int MV_C[2];

  int VAB;
  int VBC;
  int VCA;

  int FMV;
  int i_count;
  int BlockDistanceE, BlockDistanceA, BlockDistanceB, BlockDistanceC;

  mv_a[0] = h->mb.cache.mv[i_list][i8 - 1][0];
  mv_a[1] = h->mb.cache.mv[i_list][i8 - 1][1];

  mv_b[0] = h->mb.cache.mv[i_list][i8 - 8][0];
  mv_b[1] = h->mb.cache.mv[i_list][i8 - 8][1];

  mv_c[0] = h->mb.cache.mv[i_list][i8 - 8 + i_width][0];
  mv_c[1] = h->mb.cache.mv[i_list][i8 - 8 + i_width][1];

  BlockDistanceE = abs (h->fenc->i_poc - (i_list ? h->fref1[i_ref]->i_poc : h->fref0[i_ref]->i_poc));

  if ((idx & 0x03) == 3 || (i_width == 2 && (idx & 0x3) == 2) || (i_width == 4 && (idx & 0xF) == 8) || i_refc == -2)
  {
    i_refc = h->mb.cache.ref[i_list][i8 - 8 - 1];
    mv_c[0] = h->mb.cache.mv[i_list][i8 - 8 - 1][0];
    mv_c[1] = h->mb.cache.mv[i_list][i8 - 8 - 1][1];
  }

  if (i_refa == -1 || i_refa == -2)     //||i_lista!=i_list)
  {
    BlockDistanceA = 1;
    mv_a[0] = mv_a[1] = 0;
  }
  else
    BlockDistanceA = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refa]->i_poc : h->fref0[i_refa]->i_poc));

  if (i_refb == -1 || i_refb == -2)     //||i_listb!=i_list)
  {
    BlockDistanceB = 1;
    mv_b[0] = mv_b[1] = 0;
  }
  else
    BlockDistanceB = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refb]->i_poc : h->fref0[i_refb]->i_poc));

  if (i_refc == -1 || i_refc == -2)     //||i_listc!=i_list)
  {
    BlockDistanceC = 1;
    mv_c[0] = mv_c[1] = 0;
  }
  else
    BlockDistanceC = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refc]->i_poc : h->fref0[i_refc]->i_poc));


  if (h->mb.i_partition == D_16x8)
  {
    if (idx == 0 && i_refb == i_ref)
    {
      mvp[0] = mv_b[0];
      mvp[1] = mv_b[1];
      return;
    }
    else if (idx != 0 && i_refa == i_ref)
    {
      mvp[0] = mv_a[0];
      mvp[1] = mv_a[1];
      return;
    }
  }
  else if (h->mb.i_partition == D_8x16)
  {
    if (idx == 0 && i_refa == i_ref)
    {
      mvp[0] = mv_a[0];
      mvp[1] = mv_a[1];
      return;
    }
    else if (idx != 0 && i_refc == i_ref)
    {
      mvp[0] = mv_c[0];
      mvp[1] = mv_c[1];
      return;
    }
  }

  i_count = 0;

  if (i_refa != -1 && i_refa != -2)
    i_count++;
  if (i_refb != -1 && i_refb != -2)
    i_count++;
  if (i_refc != -1 && i_refc != -2)
    i_count++;

  if (i_count == 1)
  {
    if (i_refa != -1 && i_refa != -2)
    {
      mvp[0] = mv_a[0];
      mvp[1] = mv_a[1];
    }
    else if (i_refb != -1 && i_refb != -2)
    {
      mvp[0] = mv_b[0];
      mvp[1] = mv_b[1];
    }
    else
    {
      mvp[0] = mv_c[0];
      mvp[1] = mv_c[1];
    }
  }
  else
  {
    MV_A[0] = (abs (mv_a[0]) * BlockDistanceE * (512 / BlockDistanceA) + 256) >> 9;
    MV_A[1] = (abs (mv_a[1]) * BlockDistanceE * (512 / BlockDistanceA) + 256) >> 9;

    MV_B[0] = (abs (mv_b[0]) * BlockDistanceE * (512 / BlockDistanceB) + 256) >> 9;
    MV_B[1] = (abs (mv_b[1]) * BlockDistanceE * (512 / BlockDistanceB) + 256) >> 9;

    MV_C[0] = (abs (mv_c[0]) * BlockDistanceE * (512 / BlockDistanceC) + 256) >> 9;
    MV_C[1] = (abs (mv_c[1]) * BlockDistanceE * (512 / BlockDistanceC) + 256) >> 9;

    MV_A[0] = (mv_a[0] >= 0) ? MV_A[0] : (-MV_A[0]);
    MV_A[1] = (mv_a[1] >= 0) ? MV_A[1] : (-MV_A[1]);
    MV_B[0] = (mv_b[0] >= 0) ? MV_B[0] : (-MV_B[0]);
    MV_B[1] = (mv_b[1] >= 0) ? MV_B[1] : (-MV_B[1]);
    MV_C[0] = (mv_c[0] >= 0) ? MV_C[0] : (-MV_C[0]);
    MV_C[1] = (mv_c[1] >= 0) ? MV_C[1] : (-MV_C[1]);

    VAB = abs (MV_A[0] - MV_B[0]) + abs (MV_A[1] - MV_B[1]);
    VBC = abs (MV_B[0] - MV_C[0]) + abs (MV_B[1] - MV_C[1]);
    VCA = abs (MV_C[0] - MV_A[0]) + abs (MV_C[1] - MV_A[1]);

    FMV = xavs_median (VAB, VBC, VCA);

    if (FMV == VAB)
    {
      mvp[0] = MV_C[0];
      mvp[1] = MV_C[1];
    }
    else if (FMV == VBC)
    {
      mvp[0] = MV_A[0];
      mvp[1] = MV_A[1];
    }
    else
    {
      mvp[0] = MV_B[0];
      mvp[1] = MV_B[1];
    }
  }
}

void
xavs_mb_predict_mv_16x16 (xavs_t * h, int i_list, int i_ref, int mvp[2])
{
  int i_refa = h->mb.cache.ref[i_list][XAVS_SCAN8_0 - 1];
  int16_t *mv_a = h->mb.cache.mv[i_list][XAVS_SCAN8_0 - 1];
  int i_refb = h->mb.cache.ref[i_list][XAVS_SCAN8_0 - 8];
  int16_t *mv_b = h->mb.cache.mv[i_list][XAVS_SCAN8_0 - 8];
  int i_refc = h->mb.cache.ref[i_list][XAVS_SCAN8_0 - 8 + 4];
  int16_t *mv_c = h->mb.cache.mv[i_list][XAVS_SCAN8_0 - 8 + 4];

  int MV_A[2];
  int MV_B[2];
  int MV_C[2];

  int VAB;
  int VBC;
  int VCA;
  int FMV;
  int i_count;

  int BlockDistanceE, BlockDistanceA, BlockDistanceB, BlockDistanceC;

  BlockDistanceE = abs (h->fenc->i_poc - (i_list ? h->fref1[i_ref]->i_poc : h->fref0[i_ref]->i_poc));

  if (i_refc == -2)
  {
    i_refc = h->mb.cache.ref[i_list][XAVS_SCAN8_0 - 8 - 1];
    mv_c = h->mb.cache.mv[i_list][XAVS_SCAN8_0 - 8 - 1];
  }

  if (i_refa == -1 || i_refa == -2)
    BlockDistanceA = 1;
  else
    BlockDistanceA = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refa]->i_poc : h->fref0[i_refa]->i_poc));

  if (i_refb == -1 || i_refb == -2)
    BlockDistanceB = 1;
  else
    BlockDistanceB = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refb]->i_poc : h->fref0[i_refb]->i_poc));

  if (i_refc == -1 || i_refc == -2)
    BlockDistanceC = 1;
  else
    BlockDistanceC = abs (h->fenc->i_poc - (i_list ? h->fref1[i_refc]->i_poc : h->fref0[i_refc]->i_poc));

  i_count = 0;

  if (i_refa != -1 && i_refa != -2)
    i_count++;
  if (i_refb != -1 && i_refb != -2)
    i_count++;
  if (i_refc != -1 && i_refc != -2)
    i_count++;

  if (i_count == 1)
  {
    if (i_refa != -1 && i_refa != -2)
    {
      mvp[0] = mv_a[0];
      mvp[1] = mv_a[1];
    }
    else if (i_refb != -1 && i_refb != -2)
    {
      mvp[0] = mv_b[0];
      mvp[1] = mv_b[1];
    }
    else
    {
      mvp[0] = mv_c[0];
      mvp[1] = mv_c[1];
    }
  }
  else
  {
    MV_A[0] = (abs (mv_a[0]) * BlockDistanceE * (512 / BlockDistanceA) + 256) >> 9;
    MV_A[1] = (abs (mv_a[1]) * BlockDistanceE * (512 / BlockDistanceA) + 256) >> 9;

    MV_B[0] = (abs (mv_b[0]) * BlockDistanceE * (512 / BlockDistanceB) + 256) >> 9;
    MV_B[1] = (abs (mv_b[1]) * BlockDistanceE * (512 / BlockDistanceB) + 256) >> 9;

    MV_C[0] = (abs (mv_c[0]) * BlockDistanceE * (512 / BlockDistanceC) + 256) >> 9;
    MV_C[1] = (abs (mv_c[1]) * BlockDistanceE * (512 / BlockDistanceC) + 256) >> 9;

    MV_A[0] = (mv_a[0] > 0) ? MV_A[0] : (-MV_A[0]);
    MV_A[1] = (mv_a[1] > 0) ? MV_A[1] : (-MV_A[1]);
    MV_B[0] = (mv_b[0] > 0) ? MV_B[0] : (-MV_B[0]);
    MV_B[1] = (mv_b[1] > 0) ? MV_B[1] : (-MV_B[1]);
    MV_C[0] = (mv_c[0] > 0) ? MV_C[0] : (-MV_C[0]);
    MV_C[1] = (mv_c[1] > 0) ? MV_C[1] : (-MV_C[1]);

    VAB = abs (MV_A[0] - MV_B[0]) + abs (MV_A[1] - MV_B[1]);
    VBC = abs (MV_B[0] - MV_C[0]) + abs (MV_B[1] - MV_C[1]);
    VCA = abs (MV_C[0] - MV_A[0]) + abs (MV_C[1] - MV_A[1]);

    FMV = xavs_median (VAB, VBC, VCA);

    if (FMV == VAB)
    {
      mvp[0] = MV_C[0];
      mvp[1] = MV_C[1];
    }
    else if (FMV == VBC)
    {
      mvp[0] = MV_A[0];
      mvp[1] = MV_A[1];
    }
    else
    {
      mvp[0] = MV_B[0];
      mvp[1] = MV_B[1];
    }
  }
}

void
xavs_mb_predict_mv_pskip (xavs_t * h, int mv[2])
{
  int i_refa = h->mb.cache.ref[0][XAVS_SCAN8_0 - 1];
  int i_refb = h->mb.cache.ref[0][XAVS_SCAN8_0 - 8];
  int16_t *mv_a = h->mb.cache.mv[0][XAVS_SCAN8_0 - 1];
  int16_t *mv_b = h->mb.cache.mv[0][XAVS_SCAN8_0 - 8];

  if (i_refa == -2 || i_refb == -2 || (i_refa == 0 && mv_a[0] == 0 && mv_a[1] == 0) || (i_refb == 0 && mv_b[0] == 0 && mv_b[1] == 0))
  {
    mv[0] = mv[1] = 0;
  }
  else
  {
    xavs_mb_predict_mv_16x16 (h, 0, 0, mv);
  }
}

static int
xavs_mb_predict_mv_direct16x16_spatial (xavs_t * h)
{
  int ref;
  int mv[2][2];
  int i_list;
  int i8;
  //int b8x8;
  const int8_t *l1ref0 = &h->fref1[0]->ref[0][h->mb.i_b8_xy];
  //const int8_t *l1ref1 = &h->fref1[0]->ref[1][ h->mb.i_b8_xy ];
  const int16_t (*l1mv0)[2] = (const int16_t (*)[2]) &h->fref1[0]->mv[0][h->mb.i_b4_xy];
  //const int16_t (*l1mv1)[2] = (const int16_t (*)[2]) &h->fref1[0]->mv[1][ h->mb.i_b4_xy ];
  int mvRef[2], mvFw[2], mvBw[2];
  int BlockDistanceRef, BlockDistanceFw, BlockDistanceBw;

  const int type_col = h->fref1[0]->mb_type[h->mb.i_mb_xy];

  if (type_col == I_8x8)
  {
    for (i_list = 0; i_list < 2; i_list++)
    {
      xavs_mb_predict_mv_16x16 (h, i_list, 0, mv[i_list]);
      xavs_macroblock_cache_ref (h, 0, 0, 4, 4, i_list, 0);
      xavs_macroblock_cache_mv (h, 0, 0, 4, 4, i_list, mv[i_list][0], mv[i_list][1]);

    }
    return 1;
  }
  else
  {
    for (i8 = 0; i8 < 4; i8++)
    {
      const int x8 = i8 % 2;
      const int y8 = i8 / 2;
      const int o8 = x8 + y8 * h->mb.i_b8_stride;
      const int o4 = 2 * x8 + 2 * y8 * h->mb.i_b4_stride;

      mvRef[0] = l1mv0[o4][0];
      mvRef[1] = l1mv0[o4][1];
      ref = l1ref0[o8];

      BlockDistanceRef = (h->fref1[0]->i_poc - h->fref1[0]->ref_poc[0][ref] + 512) % 512;
      BlockDistanceFw = (h->fenc->i_poc - h->fref0[0]->i_poc + 512) % 512;
      BlockDistanceBw = (h->fref1[0]->i_poc - h->fenc->i_poc + 512) % 512;

      mvFw[0] = (mvRef[0] > 0) ? ((16384 / BlockDistanceRef) * (1 + mvRef[0] * BlockDistanceFw) - 1) >> 14 : -((16384 / BlockDistanceRef * (1 - mvRef[0] * BlockDistanceFw) - 1) >> 14);

      mvFw[1] = (mvRef[1] > 0) ? ((16384 / BlockDistanceRef) * (1 + mvRef[1] * BlockDistanceFw) - 1) >> 14 : -(((16384 / BlockDistanceRef) * (1 - mvRef[1] * BlockDistanceFw) - 1) >> 14);

      mvBw[0] = (mvRef[0] > 0) ? -(((16384 / BlockDistanceRef) * (1 + mvRef[0] * BlockDistanceBw) - 1) >> 14) : ((16384 / BlockDistanceRef) * (1 - mvRef[0] * BlockDistanceBw) - 1) >> 14;

      mvBw[1] = (mvRef[1] > 0) ? -(((16384 / BlockDistanceRef) * (1 + mvRef[1] * BlockDistanceBw) - 1) >> 14) : ((16384 / BlockDistanceRef) * (1 - mvRef[1] * BlockDistanceBw) - 1) >> 14;

      xavs_macroblock_cache_ref (h, 2 * x8, 2 * y8, 2, 2, 0, 0);
      xavs_macroblock_cache_ref (h, 2 * x8, 2 * y8, 2, 2, 1, 0);
      xavs_macroblock_cache_mv (h, 2 * x8, 2 * y8, 2, 2, 0, mvFw[0], mvFw[1]);
      xavs_macroblock_cache_mv (h, 2 * x8, 2 * y8, 2, 2, 1, mvBw[0], mvBw[1]);
    }
    return 1;
  }


}



int
xavs_mb_predict_mv_direct16x16 (xavs_t * h, int *b_changed)
{
  int b_available;
  if (h->param.analyse.i_direct_mv_pred == XAVS_DIRECT_PRED_NONE)
    return 0;
  else                          //if( h->sh.b_direct_spatial_mv_pred )
    b_available = xavs_mb_predict_mv_direct16x16_spatial (h);
  //else
  //b_available = xavs_mb_predict_mv_direct16x16_temporal( h );

  if (b_changed != NULL && b_available)
  {
    int type_col = h->fref1[0]->mb_type[h->mb.i_mb_xy];
    if (IS_INTRA (type_col) || type_col == P_SKIP)
    {
      *b_changed = h->mb.cache.direct_ref[0][0] != h->mb.cache.ref[0][XAVS_SCAN8_0]
        || h->mb.cache.direct_ref[1][0] != h->mb.cache.ref[1][XAVS_SCAN8_0]
        || *(uint32_t *) h->mb.cache.direct_mv[0][XAVS_SCAN8_0] != *(uint32_t *) h->mb.cache.mv[0][XAVS_SCAN8_0] || *(uint32_t *) h->mb.cache.direct_mv[1][XAVS_SCAN8_0] != *(uint32_t *) h->mb.cache.mv[1][XAVS_SCAN8_0];
    }
    else
    {
      int i, l;
      *b_changed = 0;
      for (l = 0; l < 2; l++)
        for (i = 0; i < 4; i++)
          *b_changed |= h->mb.cache.direct_ref[l][i] != h->mb.cache.ref[l][xavs_scan8[i * 4]];
      *b_changed = *b_changed || memcmp (h->mb.cache.direct_mv, h->mb.cache.mv, sizeof (h->mb.cache.mv));
    }
    if (!*b_changed)
      return b_available;
  }

  /* cache ref & mv */
  if (b_available)
  {
    int i, l;
    for (l = 0; l < 2; l++)
      for (i = 0; i < 4; i++)
        h->mb.cache.direct_ref[l][i] = h->mb.cache.ref[l][xavs_scan8[i * 4]];
    memcpy (h->mb.cache.direct_mv, h->mb.cache.mv, sizeof (h->mb.cache.mv));
  }

  return b_available;
}

void
xavs_mb_load_mv_direct8x8 (xavs_t * h, int idx)
{
  const int x = 2 * (idx % 2);
  const int y = 2 * (idx / 2);
  int l;
  xavs_macroblock_cache_ref (h, x, y, 2, 2, 0, h->mb.cache.direct_ref[0][idx]);
  xavs_macroblock_cache_ref (h, x, y, 2, 2, 1, h->mb.cache.direct_ref[1][idx]);
  for (l = 0; l < 2; l++)
  {
    *(uint64_t *) h->mb.cache.mv[l][xavs_scan8[idx * 4]] = *(uint64_t *) h->mb.cache.direct_mv[l][xavs_scan8[idx * 4]];
    *(uint64_t *) h->mb.cache.mv[l][xavs_scan8[idx * 4] + 8] = *(uint64_t *) h->mb.cache.direct_mv[l][xavs_scan8[idx * 4] + 8];
  }
}

/* This just improves encoder performance, it's not part of the spec */
void
xavs_mb_predict_mv_ref16x16 (xavs_t * h, int i_list, int i_ref, int mvc[8][2], int *i_mvc)
{
  int16_t (*mvr)[2] = h->mb.mvr[i_list][i_ref];
  int i = 0;

#define SET_MVP(mvp) { \
        mvc[i][0] = mvp[0]; \
        mvc[i][1] = mvp[1]; \
        i++; \
    }

  /* b_direct */
  if (h->sh.i_type == SLICE_TYPE_B && h->mb.cache.ref[i_list][xavs_scan8[12]] == i_ref)
  {
    SET_MVP (h->mb.cache.mv[i_list][xavs_scan8[12]]);
  }

  /* spatial predictors */
  if (h->mb.i_neighbour & MB_LEFT)
  {
    int i_mb_l = h->mb.i_mb_xy - 1;
    /* skip MBs didn't go through the whole search process, so mvr is undefined */
    if (!IS_SKIP (h->mb.type[i_mb_l]))
      SET_MVP (mvr[i_mb_l]);
  }
  if (h->mb.i_neighbour & MB_TOP)
  {
    int i_mb_t = h->mb.i_mb_xy - h->mb.i_mb_stride;
    if (!IS_SKIP (h->mb.type[i_mb_t]))
      SET_MVP (mvr[i_mb_t]);

    if (h->mb.i_neighbour & MB_TOPLEFT && !IS_SKIP (h->mb.type[i_mb_t - 1]))
      SET_MVP (mvr[i_mb_t - 1]);
    if (h->mb.i_mb_x < h->mb.i_mb_stride - 1 && !IS_SKIP (h->mb.type[i_mb_t + 1]))
      SET_MVP (mvr[i_mb_t + 1]);
  }
#undef SET_MVP

  /* temporal predictors */
  if (h->fref0[0]->i_ref[0] > 0)
  {
    xavs_frame_t *l0 = h->fref0[0];
    int ref_col_cur, ref_col_prev = -1;
    int scale = 0;

#define SET_TMVP(dx, dy) { \
            int i_b4 = h->mb.i_b4_xy + dx*4 + dy*4*h->mb.i_b4_stride; \
            int i_b8 = h->mb.i_b8_xy + dx*2 + dy*2*h->mb.i_b8_stride; \
            ref_col_cur = l0->ref[0][i_b8]; \
            if( ref_col_cur >= 0 ) \
            { \
                /* TODO: calc once per frame and tablize? */\
                if( ref_col_cur != ref_col_prev ) \
                    scale = 256 * (h->fenc->i_poc - h->fref0[i_ref]->i_poc) \
                                / (l0->i_poc - l0->ref_poc[0][ref_col_cur]); \
                mvc[i][0] = l0->mv[0][i_b4][0] * scale / 256; \
                mvc[i][1] = l0->mv[0][i_b4][1] * scale / 256; \
                i++; \
                ref_col_prev = ref_col_cur; \
            } \
        }

    SET_TMVP (0, 0);
    if (h->mb.i_mb_x < h->sps->i_mb_width - 1)
      SET_TMVP (1, 0);
    if (h->mb.i_mb_y < h->sps->i_mb_height - 1)
      SET_TMVP (0, 1);
#undef SET_TMVP
  }

  *i_mvc = i;
}

static inline void
xavs_mb_mc_0xywh (xavs_t * h, int x, int y, int width, int height)
{
  const int i8 = xavs_scan8[0] + x + 8 * y;
  const int i_ref = h->sh.i_type == SLICE_TYPE_P ? h->mb.cache.ref[0][i8] : 0;
  const int mvx = xavs_clip3 (h->mb.cache.mv[0][i8][0], h->mb.mv_min[0], h->mb.mv_max[0]);
  const int mvy = xavs_clip3 (h->mb.cache.mv[0][i8][1], h->mb.mv_min[1], h->mb.mv_max[1]);

  h->mc.mc_luma (h->mb.pic.p_fref[0][i_ref], h->mb.pic.i_stride[0], &h->mb.pic.p_fdec[0][4 * y * FDEC_STRIDE + 4 * x], FDEC_STRIDE, mvx + 4 * 4 * x, mvy + 4 * 4 * y, 4 * width, 4 * height);

  h->mc.mc_chroma (&h->mb.pic.p_fref[0][i_ref][4][2 * y * h->mb.pic.i_stride[1] + 2 * x], h->mb.pic.i_stride[1], &h->mb.pic.p_fdec[1][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, mvx, mvy, 2 * width, 2 * height);

  h->mc.mc_chroma (&h->mb.pic.p_fref[0][i_ref][5][2 * y * h->mb.pic.i_stride[2] + 2 * x], h->mb.pic.i_stride[2], &h->mb.pic.p_fdec[2][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, mvx, mvy, 2 * width, 2 * height);
}
static inline void
xavs_mb_mc_1xywh (xavs_t * h, int x, int y, int width, int height)
{
  const int i8 = xavs_scan8[0] + x + 8 * y;
  const int i_ref = 0;          //h->mb.cache.ref[1][i8];
  const int mvx = xavs_clip3 (h->mb.cache.mv[1][i8][0], h->mb.mv_min[0], h->mb.mv_max[0]);
  const int mvy = xavs_clip3 (h->mb.cache.mv[1][i8][1], h->mb.mv_min[1], h->mb.mv_max[1]);

  h->mc.mc_luma (h->mb.pic.p_fref[1][i_ref], h->mb.pic.i_stride[0], &h->mb.pic.p_fdec[0][4 * y * FDEC_STRIDE + 4 * x], FDEC_STRIDE, mvx + 4 * 4 * x, mvy + 4 * 4 * y, 4 * width, 4 * height);

  h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref][4][2 * y * h->mb.pic.i_stride[1] + 2 * x], h->mb.pic.i_stride[1], &h->mb.pic.p_fdec[1][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, mvx, mvy, 2 * width, 2 * height);

  h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref][5][2 * y * h->mb.pic.i_stride[2] + 2 * x], h->mb.pic.i_stride[2], &h->mb.pic.p_fdec[2][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, mvx, mvy, 2 * width, 2 * height);
}

static inline void
xavs_mb_mc_01xywh (xavs_t * h, int x, int y, int width, int height)
{
  //int mvBw[0], mvBw[1];
  int i;
  const int i8 = xavs_scan8[0] + x + 8 * y;

  const int i_ref0 = h->sh.i_type == SLICE_TYPE_P ? h->mb.cache.ref[0][i8] : 0;
  const int i_ref1 = 0;         //h->mb.cache.ref[1][i8];

  int BlockDistanceFw = h->fenc->i_poc - h->fref0[i_ref0]->i_poc;
  int BlockDistanceBw = h->fref1[0]->i_poc - h->fenc->i_poc;

  int mvx1;
  int mvy1;

  DECLARE_ALIGNED (uint8_t, tmp[16 * 16], 16);
  int i_mode = xavs_size2pixel[height][width];
  if (x == 0 && y == 0)
    i = 0;
  else if (x == 2 && y == 0)
    i = 1;
  else if (x == 0 && y == 2)
    i = 2;
  else
    i = 3;
  if (h->mb.i_type == B_SKIP || h->mb.i_type == B_DIRECT || h->mb.i_sub_partition[i] == D_DIRECT_8x8)
  {
    mvx1 = xavs_clip3 (h->mb.cache.mv[1][i8][0], h->mb.mv_min[0], h->mb.mv_max[0]);
    mvy1 = xavs_clip3 (h->mb.cache.mv[1][i8][1], h->mb.mv_min[1], h->mb.mv_max[1]);
  }
  else
  {
    mvx1 = xavs_clip3 ((-((h->mb.cache.mv[0][i8][0] * BlockDistanceBw * (512 / BlockDistanceFw) + 256) >> 9)), h->mb.mv_min[0], h->mb.mv_max[0]);
    mvy1 = xavs_clip3 ((-((h->mb.cache.mv[0][i8][1] * BlockDistanceBw * (512 / BlockDistanceFw) + 256) >> 9)), h->mb.mv_min[1], h->mb.mv_max[1]);
  }
  xavs_mb_mc_0xywh (h, x, y, width, height);

  h->mc.mc_luma (h->mb.pic.p_fref[1][i_ref1], h->mb.pic.i_stride[0], tmp, 16, mvx1 + 4 * 4 * x, mvy1 + 4 * 4 * y, 4 * width, 4 * height);

  if (h->param.analyse.b_weighted_bipred)
  {
    const int i_ref0 = h->mb.cache.ref[0][i8];
    const int weight = h->mb.bipred_weight[i_ref0][i_ref1];

    h->mc.avg_weight[i_mode] (&h->mb.pic.p_fdec[0][4 * y * FDEC_STRIDE + 4 * x], FDEC_STRIDE, tmp, 16, weight);

    h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref1][4][2 * y * h->mb.pic.i_stride[1] + 2 * x], h->mb.pic.i_stride[1], tmp, 16, mvx1, mvy1, 2 * width, 2 * height);
    h->mc.avg_weight[i_mode + 3] (&h->mb.pic.p_fdec[1][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, tmp, 16, weight);

    h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref1][5][2 * y * h->mb.pic.i_stride[2] + 2 * x], h->mb.pic.i_stride[2], tmp, 16, mvx1, mvy1, 2 * width, 2 * height);
    h->mc.avg_weight[i_mode + 3] (&h->mb.pic.p_fdec[2][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, tmp, 16, weight);
  }
  else
  {
    h->mc.avg[i_mode] (&h->mb.pic.p_fdec[0][4 * y * FDEC_STRIDE + 4 * x], FDEC_STRIDE, tmp, 16);

    h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref1][4][2 * y * h->mb.pic.i_stride[1] + 2 * x], h->mb.pic.i_stride[1], tmp, 16, mvx1, mvy1, 2 * width, 2 * height);
    h->mc.avg[i_mode + 3] (&h->mb.pic.p_fdec[1][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, tmp, 16);

    h->mc.mc_chroma (&h->mb.pic.p_fref[1][i_ref1][5][2 * y * h->mb.pic.i_stride[2] + 2 * x], h->mb.pic.i_stride[2], tmp, 16, mvx1, mvy1, 2 * width, 2 * height);
    h->mc.avg[i_mode + 3] (&h->mb.pic.p_fdec[2][2 * y * FDEC_STRIDE + 2 * x], FDEC_STRIDE, tmp, 16);
  }
}


static void
xavs_mb_mc_direct8x8 (xavs_t * h, int x, int y)
{
  const int i8 = xavs_scan8[0] + x + 8 * y;

  if (h->mb.cache.ref[0][i8] >= 0)
    if (h->mb.cache.ref[1][i8] >= 0)
      xavs_mb_mc_01xywh (h, x, y, 2, 2);
    else
      xavs_mb_mc_0xywh (h, x, y, 2, 2);
  else
    xavs_mb_mc_1xywh (h, x, y, 2, 2);
}

void
xavs_mb_mc_8x8 (xavs_t * h, int i8)
{
  const int x = 2 * (i8 & 1);
  const int y = 2 * (i8 >> 1);
  switch (h->mb.i_sub_partition[i8])
  {
  case D_L0_8x8:
    xavs_mb_mc_0xywh (h, x, y, 2, 2);
    break;
  case D_L1_8x8:
    xavs_mb_mc_1xywh (h, x, y, 2, 2);
    break;
  case D_BI_8x8:
    xavs_mb_mc_01xywh (h, x, y, 2, 2);
    break;
  case D_DIRECT_8x8:
    xavs_mb_mc_direct8x8 (h, x, y);
    break;
  }
}

void
xavs_mb_mc (xavs_t * h)
{
  if (h->mb.i_type == P_L0)
  {
    if (h->mb.i_partition == D_16x16)
    {
      xavs_mb_mc_0xywh (h, 0, 0, 4, 4);
    }
    else if (h->mb.i_partition == D_16x8)
    {
      xavs_mb_mc_0xywh (h, 0, 0, 4, 2);
      xavs_mb_mc_0xywh (h, 0, 2, 4, 2);
    }
    else if (h->mb.i_partition == D_8x16)
    {
      xavs_mb_mc_0xywh (h, 0, 0, 2, 4);
      xavs_mb_mc_0xywh (h, 2, 0, 2, 4);
    }
  }
  else if (h->mb.i_type == P_8x8 || h->mb.i_type == B_8x8)
  {
    int i;
    for (i = 0; i < 4; i++)
      xavs_mb_mc_8x8 (h, i);
  }
  else if (h->mb.i_type == B_SKIP || h->mb.i_type == B_DIRECT)
  {
    xavs_mb_mc_direct8x8 (h, 0, 0);
    xavs_mb_mc_direct8x8 (h, 2, 0);
    xavs_mb_mc_direct8x8 (h, 0, 2);
    xavs_mb_mc_direct8x8 (h, 2, 2);
  }
  else                          /* B_*x* */
  {
    int b_list0[2];
    int b_list1[2];

    int i;

    /* init ref list utilisations */
    for (i = 0; i < 2; i++)
    {
      b_list0[i] = xavs_mb_type_list0_table[h->mb.i_type][i];
      b_list1[i] = xavs_mb_type_list1_table[h->mb.i_type][i];
    }
    if (h->mb.i_partition == D_16x16)
    {
      if (b_list0[0] && b_list1[0])
        xavs_mb_mc_01xywh (h, 0, 0, 4, 4);
      else if (b_list0[0])
        xavs_mb_mc_0xywh (h, 0, 0, 4, 4);
      else if (b_list1[0])
        xavs_mb_mc_1xywh (h, 0, 0, 4, 4);
    }
    else if (h->mb.i_partition == D_16x8)
    {
      if (b_list0[0] && b_list1[0])
        xavs_mb_mc_01xywh (h, 0, 0, 4, 2);
      else if (b_list0[0])
        xavs_mb_mc_0xywh (h, 0, 0, 4, 2);
      else if (b_list1[0])
        xavs_mb_mc_1xywh (h, 0, 0, 4, 2);

      if (b_list0[1] && b_list1[1])
        xavs_mb_mc_01xywh (h, 0, 2, 4, 2);
      else if (b_list0[1])
        xavs_mb_mc_0xywh (h, 0, 2, 4, 2);
      else if (b_list1[1])
        xavs_mb_mc_1xywh (h, 0, 2, 4, 2);
    }
    else if (h->mb.i_partition == D_8x16)
    {
      if (b_list0[0] && b_list1[0])
        xavs_mb_mc_01xywh (h, 0, 0, 2, 4);
      else if (b_list0[0])
        xavs_mb_mc_0xywh (h, 0, 0, 2, 4);
      else if (b_list1[0])
        xavs_mb_mc_1xywh (h, 0, 0, 2, 4);

      if (b_list0[1] && b_list1[1])
        xavs_mb_mc_01xywh (h, 2, 0, 2, 4);
      else if (b_list0[1])
        xavs_mb_mc_0xywh (h, 2, 0, 2, 4);
      else if (b_list1[1])
        xavs_mb_mc_1xywh (h, 2, 0, 2, 4);
    }
  }
}

void
xavs_macroblock_slice_init (xavs_t * h, int i_slice_type)
{
  int i, j;

  h->mb.mv[0] = h->fdec->mv[0];
  h->mb.mv[1] = h->fdec->mv[1];
  h->mb.ref[0] = h->fdec->ref[0];
  h->mb.ref[1] = h->fdec->ref[1];
  h->mb.type = h->fdec->mb_type;

  h->fdec->i_ref[0] = h->i_ref0;
  h->fdec->i_ref[1] = h->i_ref1;
  for (i = 0; i < h->i_ref0; i++)
    h->fdec->ref_poc[0][i] = h->fref0[i]->i_poc;
  if (i_slice_type == SLICE_TYPE_B)
  {
    for (i = 0; i < h->i_ref1; i++)
      h->fdec->ref_poc[1][i] = h->fref1[i]->i_poc;

    h->mb.map_col_to_list0[-1] = -1;
    h->mb.map_col_to_list0[-2] = -2;
    for (i = 0; i < h->fref1[0]->i_ref[0]; i++)
    {
      int poc = h->fref1[0]->ref_poc[0][i];
      h->mb.map_col_to_list0[i] = -2;
      for (j = 0; j < h->i_ref0; j++)
        if (h->fref0[j]->i_poc == poc)
        {
          h->mb.map_col_to_list0[i] = j;
          break;
        }
    }
  }
  if (i_slice_type == SLICE_TYPE_P)
    memset (h->mb.cache.skip, 0, XAVS_SCAN8_SIZE * sizeof (int8_t));
}

void
xavs_macroblock_cache_init (xavs_t * h)
{
  int i, j;
  int i_mb_count = h->mb.i_mb_count;

  h->mb.i_mb_stride = h->sps->i_mb_width;
  h->mb.i_b8_stride = h->sps->i_mb_width * 2;
  h->mb.i_b4_stride = h->sps->i_mb_width * 4;

  h->mb.qp = xavs_malloc (i_mb_count * sizeof (int8_t));
  h->mb.cbp = xavs_malloc (i_mb_count * sizeof (int16_t));
  h->mb.skipbp = xavs_malloc (i_mb_count * sizeof (int8_t));
  h->mb.mb_transform_size = xavs_malloc (i_mb_count * sizeof (int8_t));

  /* 0 -> 3 top(4), 4 -> 6 : left(3) */
  h->mb.intra4x4_pred_mode = xavs_malloc (i_mb_count * 7 * sizeof (int8_t));

  /* all coeffs */
  h->mb.non_zero_count = xavs_malloc (i_mb_count * 24 * sizeof (uint8_t));

  for (i = 0; i < 2; i++)
  {
    int i_refs = (i ? 1 : h->param.i_frame_reference) + h->param.b_bframe_pyramid;
    for (j = 0; j < i_refs && j < 16; j++)
      h->mb.mvr[i][j] = xavs_malloc (2 * i_mb_count * sizeof (int16_t));
  }

  /* init with not avaiable (for top right idx=7,15) */
  memset (h->mb.cache.ref[0], -2, XAVS_SCAN8_SIZE * sizeof (int8_t));
  memset (h->mb.cache.ref[1], -2, XAVS_SCAN8_SIZE * sizeof (int8_t));
}

void
xavs_macroblock_cache_end (xavs_t * h)
{
  int i, j;
  for (i = 0; i < 2; i++)
  {
    int i_refs = i ? 1 + h->param.b_bframe_pyramid : h->param.i_frame_reference;
    for (j = 0; j < i_refs; j++)
      xavs_free (h->mb.mvr[i][j]);
  }

  xavs_free (h->mb.intra4x4_pred_mode);
  xavs_free (h->mb.non_zero_count);
  xavs_free (h->mb.mb_transform_size);
  xavs_free (h->mb.skipbp);
  xavs_free (h->mb.cbp);
  xavs_free (h->mb.qp);
}

void
xavs_macroblock_cache_load (xavs_t * h, int i_mb_x, int i_mb_y)
{
  const int i_mb_4x4 = 4 * (i_mb_y * h->mb.i_b4_stride + i_mb_x);
  const int i_mb_8x8 = 2 * (i_mb_y * h->mb.i_b8_stride + i_mb_x);

  int i_mb_xy = i_mb_y * h->mb.i_mb_stride + i_mb_x;
  int i_top_xy = i_mb_xy - h->mb.i_mb_stride;
  int i_left_xy = -1;
  int i_top_type = -1;          /* gcc warn */
  int i_left_type = -1;

  int i;

  /* init index */
  h->mb.i_mb_x = i_mb_x;
  h->mb.i_mb_y = i_mb_y;
  h->mb.i_mb_xy = i_mb_xy;
  h->mb.i_b8_xy = i_mb_8x8;
  h->mb.i_b4_xy = i_mb_4x4;
  h->mb.i_neighbour = 0;

  /* fdec:      fenc:
   * yyyyyyy
   * yYYYY      YYYY
   * yYYYY      YYYY
   * yYYYY      YYYY
   * yYYYY      YYYY
   * uuu vvv    UUVV
   * uUU vVV    UUVV
   * uUU vVV
   */
  h->mb.pic.p_fenc[0] = h->mb.pic.fenc_buf;
  h->mb.pic.p_fenc[1] = h->mb.pic.fenc_buf + 16 * FENC_STRIDE;
  h->mb.pic.p_fenc[2] = h->mb.pic.fenc_buf + 16 * FENC_STRIDE + 8;
  h->mb.pic.p_fdec[0] = h->mb.pic.fdec_buf + 2 * FDEC_STRIDE;
  h->mb.pic.p_fdec[1] = h->mb.pic.fdec_buf + 19 * FDEC_STRIDE;
  h->mb.pic.p_fdec[2] = h->mb.pic.fdec_buf + 19 * FDEC_STRIDE + 16;

  /* load picture pointers */
  for (i = 0; i < 3; i++)
  {
    const int w = (i == 0 ? 16 : 8);
    const int i_stride = h->fdec->i_stride[i];
    const uint8_t *plane_fdec = &h->fdec->plane[i][w * (i_mb_x + i_mb_y * i_stride)];
    int j;

    h->mb.pic.i_stride[i] = i_stride;

    h->mc.copy[i ? PIXEL_8x8 : PIXEL_16x16] (h->mb.pic.p_fenc[i], FENC_STRIDE, &h->fenc->plane[i][w * (i_mb_x + i_mb_y * i_stride)], i_stride, w);
    memcpy (&h->mb.pic.p_fdec[i][-1 - FDEC_STRIDE], &plane_fdec[-1 - i_stride], w * 3 / 2 + 1);
    for (j = 0; j < w; j++)
      h->mb.pic.p_fdec[i][-1 + j * FDEC_STRIDE] = plane_fdec[-1 + j * i_stride];

    for (j = 0; j < h->i_ref0; j++)
    {
      h->mb.pic.p_fref[0][j][i == 0 ? 0 : i + 3] = &h->fref0[j]->plane[i][w * (i_mb_x + i_mb_y * i_stride)];
      h->mb.pic.p_fref[0][j][i + 1] = &h->fref0[j]->filtered[i + 1][16 * (i_mb_x + i_mb_y * h->fdec->i_stride[0])];
    }
    for (j = 0; j < h->i_ref1; j++)
    {
      h->mb.pic.p_fref[1][j][i == 0 ? 0 : i + 3] = &h->fref1[j]->plane[i][w * (i_mb_x + i_mb_y * i_stride)];
      h->mb.pic.p_fref[1][j][i + 1] = &h->fref1[j]->filtered[i + 1][16 * (i_mb_x + i_mb_y * h->fdec->i_stride[0])];
    }
  }

  if (h->fdec->integral)
  {
    for (i = 0; i < h->i_ref0; i++)
      h->mb.pic.p_integral[0][i] = &h->fref0[i]->integral[16 * (i_mb_x + i_mb_y * h->fdec->i_stride[0])];
    for (i = 0; i < h->i_ref1; i++)
      h->mb.pic.p_integral[1][i] = &h->fref1[i]->integral[16 * (i_mb_x + i_mb_y * h->fdec->i_stride[0])];
  }

  /* load cache */
  if (i_mb_xy >= h->sh.i_first_mb + h->mb.i_mb_stride)
  {
    h->mb.i_mb_type_top = i_top_type = h->mb.type[i_top_xy];

    h->mb.i_neighbour |= MB_TOP;

    /* load intra4x4 */
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[0] - 8] = h->mb.intra4x4_pred_mode[i_top_xy][0];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[1] - 8] = h->mb.intra4x4_pred_mode[i_top_xy][1];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[4] - 8] = h->mb.intra4x4_pred_mode[i_top_xy][2];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[5] - 8] = h->mb.intra4x4_pred_mode[i_top_xy][3];

    /* load non_zero_count */
    h->mb.cache.non_zero_count[xavs_scan8[0] - 8] = h->mb.non_zero_count[i_top_xy][10];
    h->mb.cache.non_zero_count[xavs_scan8[1] - 8] = h->mb.non_zero_count[i_top_xy][11];
    h->mb.cache.non_zero_count[xavs_scan8[4] - 8] = h->mb.non_zero_count[i_top_xy][14];
    h->mb.cache.non_zero_count[xavs_scan8[5] - 8] = h->mb.non_zero_count[i_top_xy][15];

    h->mb.cache.non_zero_count[xavs_scan8[16 + 0] - 8] = h->mb.non_zero_count[i_top_xy][16 + 2];
    h->mb.cache.non_zero_count[xavs_scan8[16 + 1] - 8] = h->mb.non_zero_count[i_top_xy][16 + 3];

    h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 0] - 8] = h->mb.non_zero_count[i_top_xy][16 + 4 + 2];
    h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 1] - 8] = h->mb.non_zero_count[i_top_xy][16 + 4 + 3];
  }
  else
  {
    h->mb.i_mb_type_top = -1;

    /* load intra4x4 */
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[0] - 8] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[1] - 8] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[4] - 8] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[5] - 8] = -1;

    /* load non_zero_count */
    h->mb.cache.non_zero_count[xavs_scan8[0] - 8] =
      h->mb.cache.non_zero_count[xavs_scan8[1] - 8] =
      h->mb.cache.non_zero_count[xavs_scan8[4] - 8] =
      h->mb.cache.non_zero_count[xavs_scan8[5] - 8] = h->mb.cache.non_zero_count[xavs_scan8[16 + 0] - 8] = h->mb.cache.non_zero_count[xavs_scan8[16 + 1] - 8] = h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 0] - 8] = h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 1] - 8] = 0x80;

  }

  if (i_mb_x > 0 && i_mb_xy > h->sh.i_first_mb)
  {
    i_left_xy = i_mb_xy - 1;
    h->mb.i_mb_type_left = i_left_type = h->mb.type[i_left_xy];

    h->mb.i_neighbour |= MB_LEFT;

    /* load intra4x4 */
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[0] - 1] = h->mb.intra4x4_pred_mode[i_left_xy][4];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[2] - 1] = h->mb.intra4x4_pred_mode[i_left_xy][5];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[8] - 1] = h->mb.intra4x4_pred_mode[i_left_xy][6];
    h->mb.cache.intra4x4_pred_mode[xavs_scan8[10] - 1] = h->mb.intra4x4_pred_mode[i_left_xy][3];

    /* load non_zero_count */
    h->mb.cache.non_zero_count[xavs_scan8[0] - 1] = h->mb.non_zero_count[i_left_xy][5];
    h->mb.cache.non_zero_count[xavs_scan8[2] - 1] = h->mb.non_zero_count[i_left_xy][7];
    h->mb.cache.non_zero_count[xavs_scan8[8] - 1] = h->mb.non_zero_count[i_left_xy][13];
    h->mb.cache.non_zero_count[xavs_scan8[10] - 1] = h->mb.non_zero_count[i_left_xy][15];

    h->mb.cache.non_zero_count[xavs_scan8[16 + 0] - 1] = h->mb.non_zero_count[i_left_xy][16 + 1];
    h->mb.cache.non_zero_count[xavs_scan8[16 + 2] - 1] = h->mb.non_zero_count[i_left_xy][16 + 3];

    h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 0] - 1] = h->mb.non_zero_count[i_left_xy][16 + 4 + 1];
    h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 2] - 1] = h->mb.non_zero_count[i_left_xy][16 + 4 + 3];
  }
  else
  {
    h->mb.i_mb_type_left = -1;

    h->mb.cache.intra4x4_pred_mode[xavs_scan8[0] - 1] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[2] - 1] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[8] - 1] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[10] - 1] = -1;

    /* load non_zero_count */
    h->mb.cache.non_zero_count[xavs_scan8[0] - 1] =
      h->mb.cache.non_zero_count[xavs_scan8[2] - 1] =
      h->mb.cache.non_zero_count[xavs_scan8[8] - 1] =
      h->mb.cache.non_zero_count[xavs_scan8[10] - 1] = h->mb.cache.non_zero_count[xavs_scan8[16 + 0] - 1] = h->mb.cache.non_zero_count[xavs_scan8[16 + 2] - 1] = h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 0] - 1] = h->mb.cache.non_zero_count[xavs_scan8[16 + 4 + 2] - 1] = 0x80;
  }

  if (i_mb_x < h->sps->i_mb_width - 1 && i_top_xy + 1 >= h->sh.i_first_mb)
  {
    h->mb.i_neighbour |= MB_TOPRIGHT;
    h->mb.i_mb_type_topright = h->mb.type[i_top_xy + 1];
  }
  else
    h->mb.i_mb_type_topright = -1;
  if (i_mb_x > 0 && i_top_xy - 1 >= h->sh.i_first_mb)
  {
    h->mb.i_neighbour |= MB_TOPLEFT;
    h->mb.i_mb_type_topleft = h->mb.type[i_top_xy - 1];
  }
  else
    h->mb.i_mb_type_topleft = -1;

  if (h->param.analyse.b_transform_8x8)
  {
    h->mb.cache.i_neighbour_transform_size = (i_left_type >= 0 && h->mb.mb_transform_size[i_left_xy]) + (i_top_type >= 0 && h->mb.mb_transform_size[i_top_xy]);
  }

  /* load ref/mv/mvd */
  if (h->sh.i_type != SLICE_TYPE_I)
  {
    const int s8x8 = h->mb.i_b8_stride;
    const int s4x4 = h->mb.i_b4_stride;

    int i_list;

    for (i_list = 0; i_list < (h->sh.i_type == SLICE_TYPE_B ? 2 : 1); i_list++)
    {
      /*
         h->mb.cache.ref[i_list][xavs_scan8[5 ]+1] =
         h->mb.cache.ref[i_list][xavs_scan8[7 ]+1] =
         h->mb.cache.ref[i_list][xavs_scan8[13]+1] = -2;
       */

      if (h->mb.i_neighbour & MB_TOPLEFT)
      {
        const int i8 = xavs_scan8[0] - 1 - 1 * 8;
        const int ir = i_mb_8x8 - s8x8 - 1;
        const int iv = i_mb_4x4 - s4x4 - 1;
        h->mb.cache.ref[i_list][i8] = h->mb.ref[i_list][ir];
        //h->mb.cache.list[i8]  = h->mb.list[ir];

        h->mb.cache.mv[i_list][i8][0] = h->mb.mv[i_list][iv][0];
        h->mb.cache.mv[i_list][i8][1] = h->mb.mv[i_list][iv][1];
      }
      else
      {
        const int i8 = xavs_scan8[0] - 1 - 1 * 8;
        h->mb.cache.ref[i_list][i8] = -2;
        h->mb.cache.mv[i_list][i8][0] = 0;
        h->mb.cache.mv[i_list][i8][1] = 0;
      }

      if (h->mb.i_neighbour & MB_TOP)
      {
        const int i8 = xavs_scan8[0] - 8;
        const int ir = i_mb_8x8 - s8x8;
        const int iv = i_mb_4x4 - s4x4;

        h->mb.cache.ref[i_list][i8 + 0] = h->mb.cache.ref[i_list][i8 + 1] = h->mb.ref[i_list][ir + 0];
        h->mb.cache.ref[i_list][i8 + 2] = h->mb.cache.ref[i_list][i8 + 3] = h->mb.ref[i_list][ir + 1];

        //h->mb.cache.list[i8+0]  = 
        //h->mb.cache.list[i8+1]        = h->mb.list[ir+0];
        //h->mb.cache.list[i8+2]  = 
        //h->mb.cache.list[i8+3]        = h->mb.list[ir+1];

        for (i = 0; i < 4; i++)
        {
          h->mb.cache.mv[i_list][i8 + i][0] = h->mb.mv[i_list][iv + i][0];
          h->mb.cache.mv[i_list][i8 + i][1] = h->mb.mv[i_list][iv + i][1];
        }
      }
      else
      {
        const int i8 = xavs_scan8[0] - 8;
        for (i = 0; i < 4; i++)
        {
          h->mb.cache.ref[i_list][i8 + i] = -2;
          h->mb.cache.mv[i_list][i8 + i][0] = h->mb.cache.mv[i_list][i8 + i][1] = 0;
        }
      }

      if (h->mb.i_neighbour & MB_TOPRIGHT)
      {
        const int i8 = xavs_scan8[0] + 4 - 1 * 8;
        const int ir = i_mb_8x8 - s8x8 + 2;
        const int iv = i_mb_4x4 - s4x4 + 4;

        h->mb.cache.ref[i_list][i8] = h->mb.ref[i_list][ir];
        //h->mb.cache.list[i8]  = h->mb.list[ir];
        h->mb.cache.mv[i_list][i8][0] = h->mb.mv[i_list][iv][0];
        h->mb.cache.mv[i_list][i8][1] = h->mb.mv[i_list][iv][1];
      }
      else
      {
        const int i8 = xavs_scan8[0] + 4 - 1 * 8;
        h->mb.cache.ref[i_list][i8] = -2;
        h->mb.cache.mv[i_list][i8][0] = 0;
        h->mb.cache.mv[i_list][i8][1] = 0;
      }

      if (h->mb.i_neighbour & MB_LEFT)
      {
        const int i8 = xavs_scan8[0] - 1;
        const int ir = i_mb_8x8 - 1;
        const int iv = i_mb_4x4 - 1;

        h->mb.cache.ref[i_list][i8 + 0 * 8] = h->mb.cache.ref[i_list][i8 + 1 * 8] = h->mb.ref[i_list][ir + 0 * s8x8];
        h->mb.cache.ref[i_list][i8 + 2 * 8] = h->mb.cache.ref[i_list][i8 + 3 * 8] = h->mb.ref[i_list][ir + 1 * s8x8];

        //h->mb.cache.list[i8+0*8]  = 
        //h->mb.cache.list[i8+1*8]      = h->mb.list[ir+0*s8x8];
        //h->mb.cache.list[i8+2*8]  =
        //h->mb.cache.list[i8+3*8]      = h->mb.list[ir+1*s8x8];

        for (i = 0; i < 4; i++)
        {
          h->mb.cache.mv[i_list][i8 + i * 8][0] = h->mb.mv[i_list][iv + i * s4x4][0];
          h->mb.cache.mv[i_list][i8 + i * 8][1] = h->mb.mv[i_list][iv + i * s4x4][1];
        }
      }
      else
      {
        const int i8 = xavs_scan8[0] - 1;
        for (i = 0; i < 4; i++)
        {
          h->mb.cache.ref[i_list][i8 + i * 8] = -2;
          h->mb.cache.mv[i_list][i8 + i * 8][0] = h->mb.cache.mv[i_list][i8 + i * 8][1] = 0;
        }
      }

    }

  }

  h->mb.i_neighbour4[0] = h->mb.i_neighbour8[0] = (h->mb.i_neighbour & (MB_TOP | MB_LEFT | MB_TOPLEFT)) | ((h->mb.i_neighbour & MB_TOP) ? MB_TOPRIGHT : 0) | ((h->mb.i_neighbour & MB_LEFT) ? MB_DOWNLEFT : 0);
  h->mb.i_neighbour4[4] = h->mb.i_neighbour4[1] = MB_LEFT | ((h->mb.i_neighbour & MB_TOP) ? (MB_TOP | MB_TOPLEFT | MB_TOPRIGHT) : 0);
  h->mb.i_neighbour4[2] = h->mb.i_neighbour4[8] = h->mb.i_neighbour4[10] = h->mb.i_neighbour8[2] = MB_TOP | MB_TOPRIGHT | ((h->mb.i_neighbour & MB_LEFT) ? (MB_LEFT | MB_TOPLEFT) : 0);
  h->mb.i_neighbour4[3] = h->mb.i_neighbour4[7] = h->mb.i_neighbour4[11] = h->mb.i_neighbour4[13] = h->mb.i_neighbour4[15] = h->mb.i_neighbour8[3] = MB_LEFT | MB_TOP | MB_TOPLEFT;
  h->mb.i_neighbour4[5] = h->mb.i_neighbour8[1] = MB_LEFT | (h->mb.i_neighbour & MB_TOPRIGHT) | ((h->mb.i_neighbour & MB_TOP) ? MB_TOP | MB_TOPLEFT : 0);
  h->mb.i_neighbour4[6] = h->mb.i_neighbour4[9] = h->mb.i_neighbour4[12] = h->mb.i_neighbour4[14] = MB_LEFT | MB_TOP | MB_TOPLEFT | MB_TOPRIGHT;
}

void
xavs_macroblock_cache_save (xavs_t * h)
{
  const int i_mb_xy = h->mb.i_mb_xy;
  const int i_mb_type = xavs_mb_type_fix[h->mb.i_type];
  const int s8x8 = h->mb.i_b8_stride;
  const int s4x4 = h->mb.i_b4_stride;
  const int i_mb_4x4 = h->mb.i_b4_xy;
  const int i_mb_8x8 = h->mb.i_b8_xy;

  int i;

  for (i = 0; i < 3; i++)
  {
    int w = i ? 8 : 16;
    h->mc.copy[i ? PIXEL_8x8 : PIXEL_16x16] (&h->fdec->plane[i][w * (h->mb.i_mb_x + h->mb.i_mb_y * h->fdec->i_stride[i])], h->fdec->i_stride[i], h->mb.pic.p_fdec[i], FDEC_STRIDE, w);
  }

  h->mb.type[i_mb_xy] = i_mb_type;

  if (h->mb.i_type != I_16x16 && h->mb.i_cbp_luma == 0 && h->mb.i_cbp_chroma == 0)
    h->mb.i_qp = h->mb.i_last_qp;

  h->mb.qp[i_mb_xy] = h->mb.i_qp;

  h->mb.i_last_dqp = h->mb.i_qp - h->mb.i_last_qp;
  h->mb.i_last_qp = h->mb.i_qp;

  /* save intra4x4 */
  if (i_mb_type == I_8x8)

  {
    h->mb.intra4x4_pred_mode[i_mb_xy][0] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[10]];
    h->mb.intra4x4_pred_mode[i_mb_xy][1] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[11]];
    h->mb.intra4x4_pred_mode[i_mb_xy][2] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[14]];
    h->mb.intra4x4_pred_mode[i_mb_xy][3] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[15]];
    h->mb.intra4x4_pred_mode[i_mb_xy][4] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[5]];
    h->mb.intra4x4_pred_mode[i_mb_xy][5] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[7]];
    h->mb.intra4x4_pred_mode[i_mb_xy][6] = h->mb.cache.intra4x4_pred_mode[xavs_scan8[13]];
  }
  else
  {
    h->mb.intra4x4_pred_mode[i_mb_xy][0] = h->mb.intra4x4_pred_mode[i_mb_xy][1] = h->mb.intra4x4_pred_mode[i_mb_xy][2] = h->mb.intra4x4_pred_mode[i_mb_xy][3] = h->mb.intra4x4_pred_mode[i_mb_xy][4] = h->mb.intra4x4_pred_mode[i_mb_xy][5] = h->mb.intra4x4_pred_mode[i_mb_xy][6] = -1;        //I_PRED_8x8_DC;
  }

  if (i_mb_type == I_PCM)
  {
    h->mb.cbp[i_mb_xy] = 0x72f; /* all set */
    for (i = 0; i < 16 + 2 * 4; i++)
    {
      h->mb.non_zero_count[i_mb_xy][i] = 16;
    }
  }
  else
  {
    /* save non zero count */
    for (i = 0; i < 16 + 2 * 4; i++)
    {
      h->mb.non_zero_count[i_mb_xy][i] = h->mb.cache.non_zero_count[xavs_scan8[i]];
    }
  }

  if ( /*( h->sh.i_type == SLICE_TYPE_P||h->sh.i_type == SLICE_TYPE_B)&& */ !IS_INTRA (i_mb_type))
  {
    int i_list;
    for (i_list = 0; i_list < (h->sh.i_type == SLICE_TYPE_B ? 2 : 1); i_list++)
    {
      int y, x;

      h->mb.ref[i_list][i_mb_8x8 + 0 + 0 * s8x8] = h->mb.cache.ref[i_list][xavs_scan8[0]];
      h->mb.ref[i_list][i_mb_8x8 + 1 + 0 * s8x8] = h->mb.cache.ref[i_list][xavs_scan8[4]];
      h->mb.ref[i_list][i_mb_8x8 + 0 + 1 * s8x8] = h->mb.cache.ref[i_list][xavs_scan8[8]];
      h->mb.ref[i_list][i_mb_8x8 + 1 + 1 * s8x8] = h->mb.cache.ref[i_list][xavs_scan8[12]];
      //h->mb.list [i_mb_8x8+0+0*s8x8] = h->mb.cache.list[xavs_scan8[0]];
      //h->mb.list [i_mb_8x8+1+0*s8x8] = h->mb.cache.list[xavs_scan8[4]];
      //h->mb.list [i_mb_8x8+0+1*s8x8] = h->mb.cache.list[xavs_scan8[8]];
      //h->mb.list [i_mb_8x8+1+1*s8x8] = h->mb.cache.list[xavs_scan8[12]];
      for (y = 0; y < 4; y++)
      {
        for (x = 0; x < 4; x++)
        {
          h->mb.mv[i_list][i_mb_4x4 + x + y * s4x4][0] = h->mb.cache.mv[i_list][xavs_scan8[0] + x + 8 * y][0];
          h->mb.mv[i_list][i_mb_4x4 + x + y * s4x4][1] = h->mb.cache.mv[i_list][xavs_scan8[0] + x + 8 * y][1];
        }
      }
    }
  }
  else
  {
    int i_list;
    for (i_list = 0; i_list < (h->sh.i_type == SLICE_TYPE_B ? 2 : 1); i_list++)
    {
      int y, x;

      h->mb.ref[i_list][i_mb_8x8 + 0 + 0 * s8x8] = h->mb.ref[i_list][i_mb_8x8 + 1 + 0 * s8x8] = h->mb.ref[i_list][i_mb_8x8 + 0 + 1 * s8x8] = h->mb.ref[i_list][i_mb_8x8 + 1 + 1 * s8x8] = -1;

      for (y = 0; y < 4; y++)
      {
        for (x = 0; x < 4; x++)
        {
          h->mb.mv[i_list][i_mb_4x4 + x + y * s4x4][0] = 0;
          h->mb.mv[i_list][i_mb_4x4 + x + y * s4x4][1] = 0;
        }
      }
    }
  }
}

void
xavs_macroblock_bipred_init (xavs_t * h)
{
  int i_ref0, i_ref1;
  for (i_ref0 = 0; i_ref0 < h->i_ref0; i_ref0++)
  {
    int poc0 = h->fref0[i_ref0]->i_poc;
    for (i_ref1 = 0; i_ref1 < h->i_ref1; i_ref1++)
    {
      int dist_scale_factor;
      int poc1 = h->fref1[i_ref1]->i_poc;
      int td = xavs_clip3 (poc1 - poc0, -128, 127);
      if (td == 0 /* || pic0 is a long-term ref */ )
        dist_scale_factor = 256;
      else
      {
        int tb = xavs_clip3 (h->fdec->i_poc - poc0, -128, 127);
        int tx = (16384 + (abs (td) >> 1)) / td;
        dist_scale_factor = xavs_clip3 ((tb * tx + 32) >> 6, -1024, 1023);
      }
      h->mb.dist_scale_factor[i_ref0][i_ref1] = dist_scale_factor;

      dist_scale_factor >>= 2;
      if (h->param.analyse.b_weighted_bipred && dist_scale_factor >= -64 && dist_scale_factor <= 128)
        h->mb.bipred_weight[i_ref0][i_ref1] = 64 - dist_scale_factor;
      else
        h->mb.bipred_weight[i_ref0][i_ref1] = 32;
    }
  }
}
