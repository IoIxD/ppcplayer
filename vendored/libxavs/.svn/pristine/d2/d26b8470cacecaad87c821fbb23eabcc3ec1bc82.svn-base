/*****************************************************************************
 * rdo.c: h264 encoder library (rate-distortion optimization)
 *****************************************************************************
 * Copyright (C) 2005-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
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
 * rdo.c: xavs encoder library (rate-distortion optimization)
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#define RDO_SKIP_BS

/* CAVLC: produces exactly the same bit count as a normal encode */
/* this probably still leaves some unnecessary computations */
#define bs_write1(s,v)     ((s)->i_bits_encoded += 1)
#define bs_write(s,n,v)    ((s)->i_bits_encoded += (n))
#define bs_write_ue(s,v)   ((s)->i_bits_encoded += bs_size_ue(v))
#define bs_write_se(s,v)   ((s)->i_bits_encoded += bs_size_se(v))
#define bs_write_te(s,v,l) ((s)->i_bits_encoded += bs_size_te(v,l))
#define xavs_macroblock_write_cavlc  xavs_macroblock_size_cavlc
#include "cavlc.c"

static int
ssd_mb (xavs_t * h)
{
  return h->pixf.ssd[PIXEL_16x16] (h->mb.pic.p_fenc[0], FENC_STRIDE,
                                   h->mb.pic.p_fdec[0], FDEC_STRIDE) + h->pixf.ssd[PIXEL_8x8] (h->mb.pic.p_fenc[1], FENC_STRIDE, h->mb.pic.p_fdec[1], FDEC_STRIDE) + h->pixf.ssd[PIXEL_8x8] (h->mb.pic.p_fenc[2], FENC_STRIDE, h->mb.pic.p_fdec[2], FDEC_STRIDE);
}

static int
ssd_plane (xavs_t * h, int size, int p, int x, int y)
{
  return h->pixf.ssd[size] (h->mb.pic.p_fenc[p] + x + y * FENC_STRIDE, FENC_STRIDE, h->mb.pic.p_fdec[p] + x + y * FDEC_STRIDE, FDEC_STRIDE);
}

static int
xavs_rd_cost_mb (xavs_t * h, int i_lambda2)
{
  int b_transform_bak = h->mb.b_transform_8x8;
  int i_ssd;
  int i_bits;

  xavs_macroblock_encode (h);

  i_ssd = ssd_mb (h);

  if (IS_SKIP (h->mb.i_type))
  {
    i_bits = 1 * i_lambda2;
  }
  else
  {
    bs_t bs_tmp = h->out.bs;
    bs_tmp.i_bits_encoded = 0;
    xavs_macroblock_size_cavlc (h, &bs_tmp);
    i_bits = bs_tmp.i_bits_encoded * i_lambda2;
  }

  h->mb.b_transform_8x8 = b_transform_bak;

  return i_ssd + i_bits;
}

int
xavs_rd_cost_part (xavs_t * h, int i_lambda2, int i8, int i_pixel)
{
  int i_ssd, i_bits;
  int chromassd;

  if (i_pixel == PIXEL_16x16)
  {
    int type_bak = h->mb.i_type;
    int i_cost = xavs_rd_cost_mb (h, i_lambda2);
    h->mb.i_type = type_bak;
    return i_cost;
  }

  h->mb.i_cbp_luma = 0;

  xavs_macroblock_encode_p8x8 (h, i8);
  if (i_pixel == PIXEL_16x8)
    xavs_macroblock_encode_p8x8 (h, i8 + 1);
  if (i_pixel == PIXEL_8x16)
    xavs_macroblock_encode_p8x8 (h, i8 + 2);

  chromassd = ssd_plane (h, i_pixel + 3, 1, (i8 & 1) * 4, (i8 >> 1) * 4) + ssd_plane (h, i_pixel + 3, 2, (i8 & 1) * 4, (i8 >> 1) * 4);
  i_ssd = ssd_plane (h, i_pixel, 0, (i8 & 1) * 8, (i8 >> 1) * 8) + chromassd;

  i_bits = xavs_partition_size_cavlc (h, i8, i_pixel) * i_lambda2;

  return i_ssd + i_bits;
}

int
xavs_rd_cost_i8x8 (xavs_t * h, int i_lambda2, int i8, int i_mode)
{
  int i_ssd, i_bits;

  xavs_mb_encode_i8x8 (h, i8, h->mb.i_qp);
  i_ssd = ssd_plane (h, PIXEL_8x8, 0, (i8 & 1) * 8, (i8 >> 1) * 8);

  i_bits = xavs_partition_i8x8_size_cavlc (h, i8, i_mode) * i_lambda2;

  return i_ssd + i_bits;
}

static int
xavs_rd_cost_i8x8_chroma (xavs_t * h, int i_lambda2, int i_mode, int b_dct)
{
  int i_ssd, i_bits;

  if (b_dct)
    xavs_mb_encode_8x8_chroma (h, 0, h->mb.i_chroma_qp);

  i_ssd = ssd_plane (h, PIXEL_8x8, 1, 0, 0) + ssd_plane (h, PIXEL_8x8, 2, 0, 0);

  h->mb.i_chroma_pred_mode = i_mode;

  i_bits = xavs_i8x8_chroma_size_cavlc (h) * i_lambda2;

  return i_ssd + i_bits;
}

/****************************************************************************
 * Trellis RD quantization
 ****************************************************************************/

//#define TRELLIS_SCORE_MAX (1ULL<<50)
#define TRELLIS_SCORE_MAX (1uI64<<50)   //lsp051219 gcc ULL = vc uI64
#define CABAC_SIZE_BITS 8
#define SSD_WEIGHT_BITS 5
#define LAMBDA_BITS 4

// node ctx: 0..3: abslevel1 (with abslevelgt1 == 0).
//           4..7: abslevelgt1 + 3 (and abslevel1 doesn't matter).
/* map node ctx => cabac ctx for level=1 */
static const int coeff_abs_level1_ctx[8] = { 1, 2, 3, 4, 0, 0, 0, 0 };

/* map node ctx => cabac ctx for level>1 */
static const int coeff_abs_levelgt1_ctx[8] = { 5, 5, 5, 5, 6, 7, 8, 9 };
static const int coeff_abs_level_transition[2][8] = {
/* update node.ctx after coding a level=1 */
  {1, 2, 3, 3, 4, 5, 6, 7},
/* update node.ctx after coding a level>1 */
  {4, 4, 4, 4, 5, 6, 7, 7}
};

static const int lambda2_tab[6] = { 1024, 1290, 1625, 2048, 2580, 3251 };

typedef struct
{
  uint64_t score;
  int level_idx;                // index into level_tree[]
  uint8_t cabac_state[10];      //just the contexts relevant to coding abs_level_m1
} trellis_node_t;

void
xavs_quant_8x8_trellis (xavs_t * h, int16_t dct[8][8], int i_quant_cat, int i_qp, int b_intra)
{
  //const int i_qbits = i_qp / 6;
  //const int i_mf = i_qp % 6;
  //const int i_lambda_mult = b_intra ? 65 : 85;
  //const int i_lambda2 = ((lambda2_tab[i_mf] * i_lambda_mult*i_lambda_mult / 10000)
  //                      << (2*i_qbits)) >> LAMBDA_BITS;

  /*to be finished */
}
