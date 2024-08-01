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

#include "common/common.h"
#include "common/quant.h"
#include "macroblock.h"

/* (ref: JVT-B118)
 * xavs_mb_decimate_score: given dct coeffs it returns a score to see if we could empty this dct coeffs
 * to 0 (low score means set it to null)
 * Used in inter macroblock (luma and chroma)
 *  luma: for a 8x8 block: if score < 4 -> null
 *        for the complete mb: if score < 6 -> null
 *  chroma: for the complete mb: if score < 7 -> null
 */

uint16_t dequant_shifttable[64] = {
    14, 14, 14, 14, 14, 14, 14, 14,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 12, 12, 12, 12, 12, 12, 12,
    11, 11, 11, 11, 11, 11, 11, 11,
    11, 10, 10, 10, 10, 10, 10, 10,
    10, 9, 9, 9, 9, 9, 9, 9,
    9, 8, 8, 8, 8, 8, 8, 8,
    7, 7, 7, 7, 7, 7, 7, 7
};

static int
xavs_mb_decimate_score (int *dct, int i_max)
{
  static const int i_ds_table4[16] = {
    3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };
  static const int i_ds_table8[64] = {
    3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  };

  const int *ds_table = (i_max == 64) ? i_ds_table8 : i_ds_table4;
  int i_score = 0;
  int idx = i_max - 1;

  while (idx >= 0 && dct[idx] == 0)
    idx--;

  while (idx >= 0)
  {
    int i_run;

    if (abs (dct[idx--]) > 1)
      return 9;

    i_run = 0;
    while (idx >= 0 && dct[idx] == 0)
    {
      idx--;
      i_run++;
    }
    i_score += ds_table[i_run];
  }

  return i_score;
}

#define STORE_8x8_NNZ(idx,nz)\
{\
    *(uint16_t*)&h->mb.cache.non_zero_count[xavs_scan8[idx*4+0]] = nz * 0x0101;\
    *(uint16_t*)&h->mb.cache.non_zero_count[xavs_scan8[idx*4+2]] = nz * 0x0101;\
}


void
xavs_mb_encode_i8x8 (xavs_t * h, int idx, int i_qscale)
{
  int x = 8 * (idx & 1);
  int y = 8 * (idx >> 1);
  uint8_t *p_src = &h->mb.pic.p_fenc[0][x + y * FENC_STRIDE];
  uint8_t *p_dst = &h->mb.pic.p_fdec[0][x + y * FDEC_STRIDE];
  DECLARE_ALIGNED (int16_t, dct8x8[8][8], 16);

  h->dctf.sub8x8_dct8 (dct8x8, p_src, p_dst);

  if (h->mb.b_trellis)
    xavs_quant_8x8_trellis (h, dct8x8, CQM_8IY, i_qscale, 1);
  else
    //h->quantf.quant_8x8 (dct8x8, h->quant8_mf[CQM_8IY][i_qscale], h->quant8_bias[CQM_8IY][i_qscale], i_qscale);
    quant_8x8 (h,dct8x8, h->quant8_mf[CQM_8IY][i_qscale], h->quant8_bias[CQM_8IY][i_qscale], i_qscale);  

  scan_zigzag_8x8full (h->dct.luma8x8[idx], dct8x8);

  //h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8IY], i_qscale);
  h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8IY], i_qscale,dequant_shifttable); 
  h->dctf.add8x8_idct8 (p_dst, dct8x8);
}

void
xavs_mb_encode_8x8_chroma (xavs_t * h, int b_inter, int i_qscale)
{
  int ch;

  for (ch = 0; ch < 2; ch++)
  {
    uint8_t *p_src = h->mb.pic.p_fenc[1 + ch];
    uint8_t *p_dst = h->mb.pic.p_fdec[1 + ch];
    int16_t dct8x8[2][8][8];

    if (h->mb.b_lossless)
    {
      sub_zigzag_8x8full (h->dct.chroma8x8[ch], p_src, p_dst);
      p_dst = p_src;
      continue;
    }

    h->dctf.sub8x8_dct8 (dct8x8[ch], p_src, p_dst);

    //h->quantf.quant_8x8 (dct8x8[ch], h->quant8_mf[CQM_8IC + b_inter][i_qscale], h->quant8_bias[CQM_8IC + b_inter][i_qscale], i_qscale);
    quant_8x8 ( h,dct8x8[ch], h->quant8_mf[CQM_8IC + b_inter][i_qscale], h->quant8_bias[CQM_8IC + b_inter][i_qscale], i_qscale);

    scan_zigzag_8x8full (h->dct.chroma8x8[ch], dct8x8[ch]);

    //h->quantf.dequant_8x8 (dct8x8[ch], h->dequant8_mf[CQM_8IC + b_inter], i_qscale);
    h->quantf.dequant_8x8 (dct8x8[ch], h->dequant8_mf[CQM_8IC + b_inter], i_qscale,dequant_shifttable);
    h->dctf.add8x8_idct8 (p_dst, dct8x8[ch]);
  }
}

static void
xavs_macroblock_encode_skip (xavs_t * h)
{
  int i;
  h->mb.i_cbp_luma = 0x00;
  h->mb.i_cbp_chroma = 0x00;

  for (i = 0; i < 16 + 8; i++)
  {
    h->mb.cache.non_zero_count[xavs_scan8[i]] = 0;
  }

  /* store cbp */
  h->mb.cbp[h->mb.i_mb_xy] = 0;
}

/*****************************************************************************
 * xavs_macroblock_encode_pskip:
 *  Encode an already marked skip block
 *****************************************************************************/
void
xavs_macroblock_encode_pskip (xavs_t * h)
{
  const int mvx = xavs_clip3 (h->mb.cache.mv[0][xavs_scan8[0]][0],
                              h->mb.mv_min[0], h->mb.mv_max[0]);
  const int mvy = xavs_clip3 (h->mb.cache.mv[0][xavs_scan8[0]][1],
                              h->mb.mv_min[1], h->mb.mv_max[1]);

  /* Motion compensation XXX probably unneeded */
  h->mc.mc_luma (h->mb.pic.p_fref[0][0], h->mb.pic.i_stride[0], h->mb.pic.p_fdec[0], FDEC_STRIDE, mvx, mvy, 16, 16);

  /* Chroma MC */
  h->mc.mc_chroma (h->mb.pic.p_fref[0][0][4], h->mb.pic.i_stride[1], h->mb.pic.p_fdec[1], FDEC_STRIDE, mvx, mvy, 8, 8);

  h->mc.mc_chroma (h->mb.pic.p_fref[0][0][5], h->mb.pic.i_stride[2], h->mb.pic.p_fdec[2], FDEC_STRIDE, mvx, mvy, 8, 8);

  xavs_macroblock_encode_skip (h);
}

/*****************************************************************************
 * xavs_macroblock_encode:
 *****************************************************************************/
void
xavs_macroblock_encode (xavs_t * h)
{
  int i_qp = h->mb.i_qp;
  int i;

  if (h->mb.i_type == P_SKIP)
  {
    /* A bit special */
    xavs_macroblock_encode_pskip (h);
    return;
  }
  if (h->mb.i_type == B_SKIP)
  {
    /* XXX motion compensation is probably unneeded */
    xavs_mb_mc (h);
    xavs_macroblock_encode_skip (h);
    return;
  }

  if (h->mb.i_type == I_8x8)
  {
    h->mb.b_transform_8x8 = 1;
    for (i = 0; i < 4; i++)
    {
      uint8_t *p_dst = &h->mb.pic.p_fdec[0][8 * (i & 1) + 8 * (i >> 1) * FDEC_STRIDE];
      int i_mode = h->mb.cache.intra4x4_pred_mode[xavs_scan8[4 * i]];

      h->predict_8x8[i_mode] (p_dst, h->mb.i_neighbour8[i]);
      xavs_mb_encode_i8x8 (h, i, i_qp);
    }
  }
  else                          /* Inter MB */
  {
    int idx;
    /* Motion compensation */
    xavs_mb_mc (h);

    if (h->mb.b_lossless)
    {

    }
    else
    {
      int16_t dct8x8[4][8][8];
      h->dctf.sub16x16_dct8 (dct8x8, h->mb.pic.p_fenc[0], h->mb.pic.p_fdec[0]);

      for (idx = 0; idx < 4; idx++)
      {

        if (h->mb.b_noise_reduction)
          xavs_denoise_dct (h, (int16_t *) dct8x8[idx]);
        if (h->mb.b_trellis)
          xavs_quant_8x8_trellis (h, dct8x8[idx], CQM_8PY, i_qp, 0);
        else
          //h->quantf.quant_8x8 (dct8x8[idx], h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);
          quant_8x8 ( h,dct8x8[idx], h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);

        scan_zigzag_8x8full (h->dct.luma8x8[idx], dct8x8[idx]);
      }

      for (idx = 0; idx < 4; idx++)
      {
        //h->quantf.dequant_8x8 (dct8x8[idx], h->dequant8_mf[CQM_8PY], i_qp);
        h->quantf.dequant_8x8 (dct8x8[idx], h->dequant8_mf[CQM_8PY], i_qp,dequant_shifttable);
        h->dctf.add8x8_idct8 (&h->mb.pic.p_fdec[0][(idx & 1) * 8 + (idx >> 1) * 8 * FDEC_STRIDE], dct8x8[idx]);
      }
    }
  }

  /* encode chroma */
  i_qp = i_chroma_qp_table[xavs_clip3 (i_qp /*+ h->pps->i_chroma_qp_index_offset */ , 0, 63)];
  if (IS_INTRA (h->mb.i_type))
  {
    const int i_mode = h->mb.i_chroma_pred_mode;

    h->predict_8x8c[i_mode] (h->mb.pic.p_fdec[1], h->mb.i_neighbour);
    h->predict_8x8c[i_mode] (h->mb.pic.p_fdec[2], h->mb.i_neighbour);
  }

  /* encode the 8x8 blocks */
  xavs_mb_encode_8x8_chroma (h, !IS_INTRA (h->mb.i_type), i_qp);

  /* Calculate the Luma/Chroma patern and non_zero_count */
  h->mb.i_cbp_luma = 0x00;

  if (1)                        //h->mb.b_transform_8x8
  {
    /* coded_block_flag is enough for CABAC.
     * the full non_zero_count is done only in CAVLC. */
    for (i = 0; i < 4; i++)
    {
      const int nz = array_non_zero (h->dct.luma8x8[i], 64);
      if (nz > 0)
        h->mb.i_cbp_luma |= 1 << i;
    }
  }

  /* Calculate the chroma patern */
  h->mb.i_cbp_chroma = 0x00;
  for (i = 0; i < 2; i++)
  {
    const int nz = array_non_zero_count (h->dct.chroma8x8[i], 64);
    if (nz > 0)
    {
      h->mb.i_cbp_chroma |= 1 << i;     /* dc+ac (we can't do only ac) */
    }
  }

  /* store cbp */
  h->mb.cbp[h->mb.i_mb_xy] = (h->mb.i_cbp_luma << 2) | h->mb.i_cbp_chroma;

  /* Check for P_SKIP
   * XXX: in the me perhaps we should take xavs_mb_predict_mv_pskip into account
   *      (if multiple mv give same result)*/
  if (h->mb.i_type == P_L0 && h->mb.i_partition == D_16x16 && h->mb.i_cbp_luma == 0x00 && h->mb.i_cbp_chroma == 0x00 && h->mb.cache.ref[0][xavs_scan8[0]] == 0)
  {
    int mvp[2];

    xavs_mb_predict_mv_pskip (h, mvp);
    if (h->mb.cache.mv[0][xavs_scan8[0]][0] == mvp[0] && h->mb.cache.mv[0][xavs_scan8[0]][1] == mvp[1])
    {
      h->mb.i_type = P_SKIP;
    }
  }

  /* Check for B_SKIP */
  if (h->mb.i_type == B_DIRECT && h->mb.i_cbp_luma == 0x00 && h->mb.i_cbp_chroma == 0x00)
  {
    h->mb.i_type = B_SKIP;
  }
}

/*****************************************************************************
 * xavs_macroblock_probe_skip:
 *  Check if the current MB could be encoded as a [PB]_SKIP (it supposes you use
 *  the previous QP
 *****************************************************************************/
int
xavs_macroblock_probe_skip (xavs_t * h, int b_bidir)
{
  DECLARE_ALIGNED (int16_t, dct8x8[4][8][8], 16);
  DECLARE_ALIGNED (int, dctscan[64], 16);

  int i_qp = h->mb.i_qp;
  int mvp[2];
  int ch;

  int i8x8;
  int i_decimate_mb;

  if (!b_bidir)
  {
    /* Get the MV */
    xavs_mb_predict_mv_pskip (h, mvp);
    mvp[0] = xavs_clip3 (mvp[0], h->mb.mv_min[0], h->mb.mv_max[0]);
    mvp[1] = xavs_clip3 (mvp[1], h->mb.mv_min[1], h->mb.mv_max[1]);

    /* Motion compensation */
    h->mc.mc_luma (h->mb.pic.p_fref[0][0], h->mb.pic.i_stride[0], h->mb.pic.p_fdec[0], FDEC_STRIDE, mvp[0], mvp[1], 16, 16);
  }

  /* get luma diff */
  h->dctf.sub16x16_dct8 (dct8x8, h->mb.pic.p_fenc[0], h->mb.pic.p_fdec[0]);

  /* encode each 8x8 block */
  for (i8x8 = 0, i_decimate_mb = 0; i8x8 < 4; i8x8++)
  {
    //h->quantf.quant_8x8 (dct8x8[i8x8], h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);
    quant_8x8 ( h,dct8x8[i8x8], h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);

    scan_zigzag_8x8full (dctscan, dct8x8[i8x8]);

    i_decimate_mb = xavs_mb_decimate_score (dctscan, 64);

    if (i_decimate_mb >= 6)
    {
      /* not as P_SKIP for this 8x8 block */
      return 0;
    }
  }

  /* encode chroma */
  i_qp = i_chroma_qp_table[xavs_clip3 (i_qp + h->pps->i_chroma_qp_index_offset, 0, 63)];

  i_decimate_mb = 0;

  for (ch = 0; ch < 2; ch++)
  {
    uint8_t *p_src = h->mb.pic.p_fenc[1 + ch];
    uint8_t *p_dst = h->mb.pic.p_fdec[1 + ch];

    if (!b_bidir)
    {
      h->mc.mc_chroma (h->mb.pic.p_fref[0][0][4 + ch], h->mb.pic.i_stride[1 + ch], h->mb.pic.p_fdec[1 + ch], FDEC_STRIDE, mvp[0], mvp[1], 8, 8);
    }

    h->dctf.sub8x8_dct8 (dct8x8[0], p_src, p_dst);

    /* calculate dct coeffs */
    //h->quantf.quant_8x8 (dct8x8[0], h->quant8_mf[CQM_8PC][i_qp], h->quant8_bias[CQM_8PC][i_qp], i_qp);
    quant_8x8 (h,dct8x8[0], h->quant8_mf[CQM_8PC][i_qp], h->quant8_bias[CQM_8PC][i_qp], i_qp);

    scan_zigzag_8x8full (dctscan, dct8x8[0]);

    i_decimate_mb += xavs_mb_decimate_score (dctscan, 64);
    if (i_decimate_mb >= 7)
    {
      return 0;
    }
  }

  return 1;
}

/****************************************************************************
 * DCT-domain noise reduction / adaptive deadzone
 * from libavcodec
 ****************************************************************************/
void
xavs_noise_reduction_update (xavs_t * h)
{
  int cat, i;
  for (cat = 0; cat < 2; cat++)
  {
    int size = cat ? 64 : 16;
    const int *weight = cat ? xavs_dct8_weight2_tab : xavs_dct4_weight2_tab;

    if (h->nr_count[cat] > (uint32_t) (cat ? (1 << 16) : (1 << 18)))
    {
      for (i = 0; i < size; i++)
        h->nr_residual_sum[cat][i] >>= 1;
      h->nr_count[cat] >>= 1;
    }

    for (i = 0; i < size; i++)
      h->nr_offset[cat][i] = (uint32_t) (((uint64_t) h->param.analyse.i_noise_reduction * h->nr_count[cat] + h->nr_residual_sum[cat][i] / 2) / ((uint64_t) h->nr_residual_sum[cat][i] * weight[i] / 256 + 1));
  }
}

void
xavs_denoise_dct (xavs_t * h, int16_t * dct)
{
  const int cat = h->mb.b_transform_8x8;
  int i;

  h->nr_count[cat]++;

  for (i = (cat ? 63 : 15); i >= 1; i--)
  {
    int level = dct[i];
    if (level)
    {
      if (level > 0)
      {
        h->nr_residual_sum[cat][i] += level;
        level -= h->nr_offset[cat][i];
        if (level < 0)
          level = 0;
      }
      else
      {
        h->nr_residual_sum[cat][i] -= level;
        level += h->nr_offset[cat][i];
        if (level > 0)
          level = 0;
      }
      dct[i] = level;
    }
  }
}

/*****************************************************************************
 * RD only; 4 calls to this do not make up for one macroblock_encode.
 * doesn't transform chroma dc.
 *****************************************************************************/
void
xavs_macroblock_encode_p8x8 (xavs_t * h, int i8)
{
  int i_qp = h->mb.i_qp;
  uint8_t *p_fenc = h->mb.pic.p_fenc[0] + (i8 & 1) * 8 + (i8 >> 1) * 8 * FENC_STRIDE;
  uint8_t *p_fdec = h->mb.pic.p_fdec[0] + (i8 & 1) * 8 + (i8 >> 1) * 8 * FDEC_STRIDE;
  int i_decimate_8x8 = 0;
  int nnz8x8 = 1;
  int ch;

  DECLARE_ALIGNED (int16_t, dct8x8[8][8], 16);

  xavs_mb_mc_8x8 (h, i8);

  h->dctf.sub8x8_dct8 (dct8x8, p_fenc, p_fdec);
  //h->quantf.quant_8x8 (dct8x8, h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);
  quant_8x8 (h,dct8x8, h->quant8_mf[CQM_8PY][i_qp], h->quant8_bias[CQM_8PY][i_qp], i_qp);
  scan_zigzag_8x8full (h->dct.luma8x8[i8], dct8x8);
  i_decimate_8x8 = xavs_mb_decimate_score (h->dct.luma8x8[i8], 64);

  if (i_decimate_8x8 < 4)
  {
    memset (h->dct.luma8x8[i8], 0, sizeof (h->dct.luma8x8[i8]));
    nnz8x8 = 0;
  }
  if (nnz8x8)
  {
   //h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8PY], i_qp);
    h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8PY], i_qp,dequant_shifttable);
    h->dctf.add8x8_idct8 (p_fdec, dct8x8);
  }

  i_qp = i_chroma_qp_table[xavs_clip3 (i_qp + h->pps->i_chroma_qp_index_offset, 0, 63)];


  for (ch = 0; ch < 2; ch++)
  {
    p_fenc = h->mb.pic.p_fenc[1 + ch];
    p_fdec = h->mb.pic.p_fdec[1 + ch];

    h->dctf.sub8x8_dct8 (dct8x8, p_fenc, p_fdec);

    //h->quantf.quant_8x8 (dct8x8, h->quant8_mf[CQM_8PC][i_qp], h->quant8_bias[CQM_8PC][i_qp], i_qp);
    quant_8x8 ( h,dct8x8, h->quant8_mf[CQM_8PC][i_qp], h->quant8_bias[CQM_8PC][i_qp], i_qp);
    scan_zigzag_8x8full (h->dct.chroma8x8[ch], dct8x8);
    //h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8PC], i_qp);
    h->quantf.dequant_8x8 (dct8x8, h->dequant8_mf[CQM_8PC], i_qp,dequant_shifttable);
    h->dctf.add8x8_idct8 (p_fdec, dct8x8);
  }

  if (nnz8x8)
    h->mb.i_cbp_luma |= (1 << i8);
  else
    h->mb.i_cbp_luma &= ~(1 << i8);
  h->mb.i_cbp_chroma = 0x02;
}
