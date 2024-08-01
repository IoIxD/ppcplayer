/*****************************************************************************
 * cavlc.c: h264 encoder library
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
 * cavlc.c: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "common/common.h"
#include "common/vlc.h"
#include "macroblock.h"

static const uint8_t intra8x8_cbp_to_golomb[64] = {
  4, 16, 17, 19, 14, 9, 22, 8, 11, 21, 10, 7, 12, 6, 5, 1,
  35, 47, 48, 38, 46, 36, 50, 26, 45, 52, 41, 28, 37, 23, 31, 2,
  43, 51, 56, 39, 55, 33, 62, 27, 54, 60, 40, 32, 42, 24, 29, 3,
  49, 53, 57, 25, 58, 30, 59, 15, 61, 63, 44, 18, 34, 13, 20, 0
};
static const uint8_t intra4x4_cbp_to_golomb[48] = {
  3, 29, 30, 17, 31, 18, 37, 8, 32, 38, 19, 9, 20, 10, 11, 2,
  16, 33, 34, 21, 35, 22, 39, 4, 36, 40, 23, 5, 24, 6, 7, 1,
  41, 42, 43, 25, 44, 26, 46, 12, 45, 47, 27, 13, 28, 14, 15, 0
};
static const uint8_t inter_cbp_to_golomb[64] = {
  0, 19, 16, 15, 18, 11, 31, 13, 17, 30, 12, 9, 10, 7, 8, 1,
  4, 42, 38, 27, 39, 33, 59, 26, 40, 58, 35, 25, 29, 24, 28,
  3, 5, 51, 52, 37, 50, 43, 63, 44, 53, 62, 48, 47, 34, 45,
  49, 6, 14, 55, 56, 36, 54, 41, 60, 21, 57, 61, 46, 22, 32, 20, 23, 2
};

/*static const uint8_t inter_cbp_to_golomb[48]=
{
  0,  2,  3,  7,  4,  8, 17, 13,  5, 18,  9, 14, 10, 15, 16, 11,
  1, 32, 33, 36, 34, 37, 44, 40, 35, 45, 38, 41, 39, 42, 43, 19,
  6, 24, 25, 20, 26, 21, 46, 28, 27, 47, 22, 29, 23, 30, 31, 12
};*/
static const uint8_t mb_type_b_to_golomb[3][9] = {
  {4, 8, 12, 10, 6, 14, 16, 18, 20},    /* D_16x8 */
  {5, 9, 13, 11, 7, 15, 17, 19, 21},    /* D_8x16 */
  {1, -1, -1, -1, 2, -1, -1, -1, 3}     /* D_16x16 */
};
static const uint8_t sub_mb_type_p_to_golomb[4] = {
  3, 1, 2, 0
};
static const uint8_t sub_mb_type_b_to_golomb[13] = {
  10, 4, 5, 1, 11, 6, 7, 2, 12, 8, 9, 3, 0
};

/*static const uint8_t sub_mb_type_b_to_golomb[13]=
{
    10,  4,  5,  1, 11,  6,  7,  2, 12,  8,  9,  3,  0
};*/

#define BLOCK_INDEX_CHROMA_DC   (-1)
#define BLOCK_INDEX_LUMA_DC     (-2)

static inline void
bs_write_vlc (bs_t * s, vlc_t v)
{
  bs_write (s, v.i_size, v.i_bits);
}

static void
bs_write_ue_k (bs_t * s, unsigned int code_num, unsigned int grad, unsigned int max_levels)
{
  unsigned int level, res, numbits;

  res = 1UL << grad;
  level = 1UL;
  numbits = 1UL + grad;

  //find golomb level
  while (code_num >= res && level < max_levels)
  {
    code_num -= res;
    res = res << 1;
    level++;
    numbits += 2UL;
  }

  if (level >= max_levels)
  {
    if (code_num >= res)
      code_num = res - 1UL;     //crop if too large.
  }

  //set data bits
  bs_write (s, numbits, res | code_num);
}

/****************************************************************************
 * block_residual_write_cavlc:
 ****************************************************************************/
static void
xavs_block_luma_write_cavlc (xavs_t * h, bs_t * s, int i_block_idx, int *l, int i_count)
{
  int level_arr[65], run_arr[65];
  int idx;
  int level, run;
  int ipos, icoef;
  int16_t curr_val;
  int symbol2D;
  int escape_level_diff;
  int tablenum;
  static const int incVlc_intra[7] = { 0, 1, 2, 4, 7, 10, 3000 };
  static const int incVlc_inter[7] = { 0, 1, 2, 3, 6, 9, 3000 };
  const char (*AVS_2DVLC_table_intra)[26][27];
  const char (*AVS_2DVLC_table_inter)[26][27];

  AVS_2DVLC_table_intra = AVS_2DVLC_INTRA_T;
  AVS_2DVLC_table_inter = AVS_2DVLC_INTER_T;

  /* first find i_last */
  run = -1;
  ipos = 0;
  for (icoef = 0; icoef < 64; icoef++)
  {
    run++;
    curr_val = l[icoef];
    if (curr_val != 0)
    {
      level_arr[ipos] = curr_val;
      run_arr[ipos] = run;
      run = -1;
      ipos++;
    }

  }
  idx = ipos;

  if (h->mb.i_type == I_8x8)
  {
    tablenum = 0;
    for (; idx >= 0; idx--)
    {
      if (!idx) //EOB
      {
        level = 0;
        run = 0;
      }
      else
      {
        level = level_arr[idx - 1];
        run = run_arr[idx - 1];
      }

      symbol2D = CODE2D_ESCAPE_SYMBOL_T; //symbol for out-of-table
      if (level > -27 && level < 27 && run < 26)
      {
        if (tablenum == 0)
          symbol2D = AVS_2DVLC_table_intra[tablenum][run][abs (level) - 1];
        else
          symbol2D = AVS_2DVLC_table_intra[tablenum][run][abs (level)];
        if (symbol2D >= 0 && level < 0)
          symbol2D++;
        if (symbol2D < 0)
          symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
      }
      else
      {
        symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
      }

      bs_write_ue_k (s, symbol2D, VLC_Golomb_Order_T[0][tablenum][0], VLC_Golomb_Order_T[0][tablenum][1]);


      if (!idx)
        break;

      if (symbol2D >= CODE2D_ESCAPE_SYMBOL_T)
      {
        escape_level_diff = abs (level) - ((run > MaxRun_T[0][tablenum]) ? 1 : RefAbsLevel_T[tablenum][run]);
        bs_write_ue_k (s, escape_level_diff, 1, 11);
      }

      if (abs (level) > incVlc_intra[tablenum])
      {
        if (abs (level) <= 2)
          tablenum = abs (level);
        else if (abs (level) <= 4)
          tablenum = 3;
        else if (abs (level) <= 7)
          tablenum = 4;
        else if (abs (level) <= 10)
          tablenum = 5;
        else
          tablenum = 6;
      }
    }
  }else {   //!intra
    tablenum = 0;
    for (; idx >= 0; idx--)
    {
      if (!idx)//EOB
      {
        level = 0;
        run = 0;
      }
      else
      {
        level = level_arr[idx - 1];
        run = run_arr[idx - 1];
      }

      symbol2D = CODE2D_ESCAPE_SYMBOL_T;        //symbol for out-of-table
      if (level > -27 && level < 27 && run < 26)
      {
        if (tablenum == 0)
          symbol2D = AVS_2DVLC_table_inter[tablenum][run][abs (level) - 1];
        else
          symbol2D = AVS_2DVLC_table_inter[tablenum][run][abs (level)];
        if (symbol2D >= 0 && level < 0)
          symbol2D++;
        if (symbol2D < 0)
          symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
      }
      else
      {
        symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
      }


      bs_write_ue_k (s, symbol2D, VLC_Golomb_Order_T[1][tablenum][0], VLC_Golomb_Order_T[1][tablenum][1]);

      if (!idx)
        break;

      if (symbol2D >= CODE2D_ESCAPE_SYMBOL_T)
      {
        escape_level_diff = abs (level) - ((run > MaxRun_T[1][tablenum]) ? 1 : RefAbsLevel_T[tablenum + 7][run]);

        bs_write_ue_k (s, escape_level_diff, 0, 11);
      }

      if (abs (level) > incVlc_inter[tablenum])
      {
        if (abs (level) <= 3)
          tablenum = abs (level);
        else if (abs (level) <= 6)
          tablenum = 4;
        else if (abs (level) <= 9)
          tablenum = 5;
        else
          tablenum = 6;
      }
    }
  }// inter
}

static void
xavs_block_chroma_write_cavlc (xavs_t * h, bs_t * s, int i_block_idx, int *l, int i_count)
{
  int level_arr[65], run_arr[65];
  int idx;
  int level, run;
  int ipos, icoef;
  int16_t curr_val;

  int symbol2D;
  int escape_level_diff;
  int tablenum;
  static const int incVlc_chroma[5] = { 0, 1, 2, 4, 3000 };
  const char (*AVS_2DVLC_table_chroma)[26][27];

  AVS_2DVLC_table_chroma = AVS_2DVLC_CHROMA_T;

  /* first find i_last */
  run = -1;
  ipos = 0;
  for (icoef = 0; icoef < 64; icoef++)
  {
    run++;
    curr_val = l[icoef];
    if (curr_val != 0)
    {
      level_arr[ipos] = curr_val;
      run_arr[ipos] = run;
      run = -1;
      ipos++;
    }
  }
  idx = ipos;

  tablenum = 0;
  for (; idx >= 0; idx--)
  {
    if (!idx)//EOB
    {
      level = 0;
      run = 0;
    }
    else
    {
      level = level_arr[idx - 1];
      run = run_arr[idx - 1];
    }

    symbol2D = CODE2D_ESCAPE_SYMBOL_T;  //symbol for out-of-table
    if (level > -27 && level < 27 && run < 26)
    {
      if (tablenum == 0)
        symbol2D = AVS_2DVLC_table_chroma[tablenum][run][abs (level) - 1];
      else
        symbol2D = AVS_2DVLC_table_chroma[tablenum][run][abs (level)];
      if (symbol2D >= 0 && level < 0)
        symbol2D++;
      if (symbol2D < 0)
        symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
    }
    else
    {
      symbol2D = (CODE2D_ESCAPE_SYMBOL_T + (run << 1) + ((level > 0) ? 1 : 0));
    }

    bs_write_ue_k (s, symbol2D, VLC_Golomb_Order_T[2][tablenum][0], VLC_Golomb_Order_T[2][tablenum][1]);


    if (!idx)
      break;

    if (symbol2D >= CODE2D_ESCAPE_SYMBOL_T)
    {
      escape_level_diff = abs (level) - ((run > MaxRun_T[2][tablenum]) ? 1 : RefAbsLevel_T[tablenum + 14][run]);

      bs_write_ue_k (s, escape_level_diff, 0, 11);
    }

    if (abs (level) > incVlc_chroma[tablenum])
    {
      if (abs (level) <= 2)
        tablenum = abs (level);
      else if (abs (level) <= 4)
        tablenum = 3;
      else
        tablenum = 4;
    }
  }
}

static void
block_residual_write_cavlc (xavs_t * h, bs_t * s, int i_idx, int *l, int i_count)
{
  int level[16], run[16];
  int i_total, i_trailing;
  int i_total_zero;
  int i_last;
  unsigned int i_sign;

  int i;
  int i_zero_left;
  int i_suffix_length;

  /* first find i_last */
  i_last = i_count - 1;
  while (i_last >= 0 && l[i_last] == 0)
  {
    i_last--;
  }

  i_sign = 0;
  i_total = 0;
  i_trailing = 0;
  i_total_zero = 0;

  if (i_last >= 0)
  {
    int b_trailing = 1;
    int idx = 0;

    /* level and run and total */
    while (i_last >= 0)
    {
      level[idx] = l[i_last--];

      run[idx] = 0;
      while (i_last >= 0 && l[i_last] == 0)
      {
        run[idx]++;
        i_last--;
      }

      i_total++;
      i_total_zero += run[idx];

      if (b_trailing && abs (level[idx]) == 1 && i_trailing < 3)
      {
        i_sign <<= 1;
        if (level[idx] < 0)
        {
          i_sign |= 0x01;
        }

        i_trailing++;
      }
      else
      {
        b_trailing = 0;
      }

      idx++;
    }
  }

  /* total/trailing */
  if (i_idx == BLOCK_INDEX_CHROMA_DC)
  {
    bs_write_vlc (s, xavs_coeff_token[4][i_total * 4 + i_trailing]);
  }
  else
  {
    /* xavs_mb_predict_non_zero_code return 0 <-> (16+16+1)>>1 = 16 */
    static const int ct_index[17] = { 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
    int nC;

    if (i_idx == BLOCK_INDEX_LUMA_DC)
    {
      nC = xavs_mb_predict_non_zero_code (h, 0);
    }
    else
    {
      nC = xavs_mb_predict_non_zero_code (h, i_idx);
    }

    bs_write_vlc (s, xavs_coeff_token[ct_index[nC]][i_total * 4 + i_trailing]);
  }

  if (i_total <= 0)
  {
    return;
  }

  i_suffix_length = i_total > 10 && i_trailing < 3 ? 1 : 0;
  if (i_trailing > 0)
  {
    bs_write (s, i_trailing, i_sign);
  }
  for (i = i_trailing; i < i_total; i++)
  {
    int i_level_code;

    /* calculate level code */
    if (level[i] < 0)
    {
      i_level_code = -2 * level[i] - 1;
    }
    else                        /* if( level[i] > 0 ) */
    {
      i_level_code = 2 * level[i] - 2;
    }
    if (i == i_trailing && i_trailing < 3)
    {
      i_level_code -= 2;        /* as level[i] can't be 1 for the first one if i_trailing < 3 */
    }

    if ((i_level_code >> i_suffix_length) < 14)
    {
      bs_write_vlc (s, xavs_level_prefix[i_level_code >> i_suffix_length]);
      if (i_suffix_length > 0)
      {
        bs_write (s, i_suffix_length, i_level_code);
      }
    }
    else if (i_suffix_length == 0 && i_level_code < 30)
    {
      bs_write_vlc (s, xavs_level_prefix[14]);
      bs_write (s, 4, i_level_code - 14);
    }
    else if (i_suffix_length > 0 && (i_level_code >> i_suffix_length) == 14)
    {
      bs_write_vlc (s, xavs_level_prefix[14]);
      bs_write (s, i_suffix_length, i_level_code);
    }
    else
    {
      bs_write_vlc (s, xavs_level_prefix[15]);
      i_level_code -= 15 << i_suffix_length;
      if (i_suffix_length == 0)
      {
        i_level_code -= 15;
      }

      if (i_level_code >= (1 << 12) || i_level_code < 0)
      {
        xavs_log (h, XAVS_LOG_ERROR, "OVERFLOW levelcode=%d\n", i_level_code);
      }

      bs_write (s, 12, i_level_code);   /* check overflow ?? */
    }

    if (i_suffix_length == 0)
    {
      i_suffix_length++;
    }
    if (abs (level[i]) > (3 << (i_suffix_length - 1)) && i_suffix_length < 6)
    {
      i_suffix_length++;
    }
  }

  if (i_total < i_count)
  {
    if (i_idx == BLOCK_INDEX_CHROMA_DC)
    {
      bs_write_vlc (s, xavs_total_zeros_dc[i_total - 1][i_total_zero]);
    }
    else
    {
      bs_write_vlc (s, xavs_total_zeros[i_total - 1][i_total_zero]);
    }
  }

  for (i = 0, i_zero_left = i_total_zero; i < i_total - 1; i++)
  {
    int i_zl;

    if (i_zero_left <= 0)
    {
      break;
    }

    i_zl = XAVS_MIN (i_zero_left - 1, 6);

    bs_write_vlc (s, xavs_run_before[i_zl][run[i]]);

    i_zero_left -= run[i];
  }
}

static void
cavlc_qp_delta (xavs_t * h, bs_t * s)
{
  int i_dqp = h->mb.i_qp - h->mb.i_last_qp;
  if (i_dqp)
  {
    if (i_dqp < -32)
      i_dqp += 63;
    else if (i_dqp > 31)
      i_dqp -= 63;
  }
  bs_write_se (s, i_dqp);
}

static void
cavlc_mb_mvd (xavs_t * h, bs_t * s, int i_list, int idx, int width)
{
  int mvp[2];
  xavs_mb_predict_mv (h, i_list, idx, width, mvp);
  bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[idx]][0] - mvp[0]);
  bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[idx]][1] - mvp[1]);
}

static void
cavlc_mb8x8_mvd (xavs_t * h, bs_t * s, int i_list, int i)
{
  if (!xavs_mb_partition_listX_table_cavlc[i_list][h->mb.i_sub_partition[i]])
    return;

  switch (h->mb.i_sub_partition[i])
  {
  case D_L0_8x8:
  case D_L1_8x8:
  case D_BI_8x8:
    cavlc_mb_mvd (h, s, i_list, 4 * i, 2);
    break;
  case D_L0_8x4:
  case D_L1_8x4:
  case D_BI_8x4:
    cavlc_mb_mvd (h, s, i_list, 4 * i + 0, 2);
    cavlc_mb_mvd (h, s, i_list, 4 * i + 2, 2);
    break;
  case D_L0_4x8:
  case D_L1_4x8:
  case D_BI_4x8:
    cavlc_mb_mvd (h, s, i_list, 4 * i + 0, 1);
    cavlc_mb_mvd (h, s, i_list, 4 * i + 1, 1);
    break;
  case D_L0_4x4:
  case D_L1_4x4:
  case D_BI_4x4:
    cavlc_mb_mvd (h, s, i_list, 4 * i + 0, 1);
    cavlc_mb_mvd (h, s, i_list, 4 * i + 1, 1);
    cavlc_mb_mvd (h, s, i_list, 4 * i + 2, 1);
    cavlc_mb_mvd (h, s, i_list, 4 * i + 3, 1);
    break;
  }
}

static inline void
xavs_macroblock_luma_write_cavlc (xavs_t * h, bs_t * s, int i8start, int i8end)
{
  int i8, i4, i;
  if (h->mb.b_transform_8x8)
  {
    /* shuffle 8x8 dct coeffs into 4x4 lists */
    for (i8 = i8start; i8 <= i8end; i8++)
      if (h->mb.i_cbp_luma & (1 << i8))
        for (i4 = 0; i4 < 4; i4++)
        {
          for (i = 0; i < 16; i++)
            h->dct.block[i4 + i8 * 4].luma4x4[i] = h->dct.luma8x8[i8][i4 + i * 4];
          h->mb.cache.non_zero_count[xavs_scan8[i4 + i8 * 4]] = array_non_zero_count (h->dct.block[i4 + i8 * 4].luma4x4, 16);
        }
  }

  for (i8 = i8start; i8 <= i8end; i8++)
    if (h->mb.i_cbp_luma & (1 << i8))
      for (i4 = 0; i4 < 4; i4++)
        block_residual_write_cavlc (h, s, i4 + i8 * 4, h->dct.block[i4 + i8 * 4].luma4x4, 16);
}

/*****************************************************************************
 * xavs_macroblock_write:
 *****************************************************************************/
void
xavs_macroblock_write_cavlc (xavs_t * h, bs_t * s)
{
  const int i_mb_type = h->mb.i_type;
  int i;

#ifndef RDO_SKIP_BS
  const int i_mb_pos_start = bs_pos (s);
  int i_mb_pos_tex;
#endif

  if (i_mb_type == I_8x8)
  {
    int di = i_mb_type == I_8x8 ? 4 : 1;

    switch (h->sh.i_type)
    {
    case SLICE_TYPE_I:
      //i_mb_i_offset = 0;
      break;
    case SLICE_TYPE_P:
      //i_mb_i_offset = 5;
      bs_write_ue (s, 4 + intra8x8_cbp_to_golomb[(h->mb.i_cbp_chroma << 4) | h->mb.i_cbp_luma]);

      break;
    case SLICE_TYPE_B:
      bs_write_ue (s, 23 + intra8x8_cbp_to_golomb[(h->mb.i_cbp_chroma << 4) | h->mb.i_cbp_luma]);
      // i_mb_i_offset = 23;
      break;
    default:
      xavs_log(h, XAVS_LOG_ERROR, "internal error or slice unsupported\n" );
      return;
    }

    /* Prediction: Luma */
    for (i = 0; i < 16; i += di)
    {
      int i_pred = xavs_mb_predict_intra4x4_mode (h, i);
      int i_mode = xavs_mb_pred_mode8x8 (h->mb.cache.intra4x4_pred_mode[xavs_scan8[i]]);

      if (i_pred == i_mode)
      {
        bs_write1 (s, 1);       /* b_prev_intra4x4_pred_mode */
      }
      else
      {
        bs_write1 (s, 0);       /* b_prev_intra4x4_pred_mode */
        if (i_mode < i_pred)
        {
          bs_write (s, 2, i_mode);
        }
        else
        {
          bs_write (s, 2, i_mode - 1);
        }
      }
    }
    bs_write_ue (s, xavs_mb_pred_mode8x8c[h->mb.i_chroma_pred_mode]);
  }
  else if (i_mb_type == P_L0)
  {
    int mvp[2];

    if (h->mb.i_partition == D_16x16)
    {
      bs_write_ue (s, 0);

      if (h->pbh.b_picture_reference_flag == 0) //h->sh.i_num_ref_idx_l0_active > 1 )
      {
        bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[0]]);
      }

      xavs_mb_predict_mv (h, 0, 0, 4, mvp);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][0] - mvp[0]);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][1] - mvp[1]);
    }
    else if (h->mb.i_partition == D_16x8)
    {
      bs_write_ue (s, 1);
      if (h->pbh.b_picture_reference_flag == 0) //h->sh.i_num_ref_idx_l0_active > 1 )
      {
        bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[0]]);
        bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[8]]);
      }

      xavs_mb_predict_mv (h, 0, 0, 4, mvp);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][0] - mvp[0]);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][1] - mvp[1]);

      xavs_mb_predict_mv (h, 0, 8, 4, mvp);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[8]][0] - mvp[0]);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[8]][1] - mvp[1]);
    }
    else if (h->mb.i_partition == D_8x16)
    {
      bs_write_ue (s, 2);
      if (h->pbh.b_picture_reference_flag == 0) //h->sh.i_num_ref_idx_l0_active > 1 )
      {
        bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[0]]);
        bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[4]]);
      }

      xavs_mb_predict_mv (h, 0, 0, 2, mvp);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][0] - mvp[0]);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[0]][1] - mvp[1]);

      xavs_mb_predict_mv (h, 0, 4, 2, mvp);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[4]][0] - mvp[0]);
      bs_write_se (s, h->mb.cache.mv[0][xavs_scan8[4]][1] - mvp[1]);
    }
  }
  else if (i_mb_type == P_8x8)
  {
    int b_sub_ref0;

    bs_write_ue (s, 3);
    b_sub_ref0 = 1;
    /* sub mb type */
    /* ref0 */
    if (h->pbh.b_picture_reference_flag == 0)   //h->sh.i_num_ref_idx_l0_active > 1 && b_sub_ref0 )
    {
      bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[0]]);
      bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[4]]);
      bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[8]]);
      bs_write1 (s, h->mb.cache.ref[0][xavs_scan8[12]]);
    }

    for (i = 0; i < 4; i++)
      cavlc_mb8x8_mvd (h, s, 0, i);
  }
  else if (i_mb_type == B_8x8)
  {
    bs_write_ue (s, 22);

    /* sub mb type */
    for (i = 0; i < 4; i++)
    {
      bs_write (s, 2, sub_mb_type_b_to_golomb[h->mb.i_sub_partition[i]]);
    }
    /* ref */

    /* mvd */
    for (i = 0; i < 4; i++)
      cavlc_mb8x8_mvd (h, s, 0, i);
    for (i = 0; i < 4; i++)
      cavlc_mb8x8_mvd (h, s, 1, i);
  }
  else if (i_mb_type != B_DIRECT)
  {
    /* All B mode */
    /* Motion Vector */
    int i_list;
    int mvp[2];

    int b_list[2][2];

    /* init ref list utilisations */
    for (i = 0; i < 2; i++)
    {
      b_list[0][i] = xavs_mb_type_list0_table_cavlc[i_mb_type][i];
      b_list[1][i] = xavs_mb_type_list1_table_cavlc[i_mb_type][i];
    }


    bs_write_ue (s, mb_type_b_to_golomb[h->mb.i_partition - D_16x8][i_mb_type - B_L0_L0]);

    for (i_list = 0; i_list < 2; i_list++)
    {
      switch (h->mb.i_partition)
      {
      case D_16x16:
        if (b_list[i_list][0])
        {
          xavs_mb_predict_mv (h, i_list, 0, 4, mvp);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][0] - mvp[0]);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][1] - mvp[1]);
        }
        break;
      case D_16x8:
        if (b_list[i_list][0])
        {
          xavs_mb_predict_mv (h, i_list, 0, 4, mvp);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][0] - mvp[0]);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][1] - mvp[1]);
        }
        if (b_list[i_list][1])
        {
          xavs_mb_predict_mv (h, i_list, 8, 4, mvp);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[8]][0] - mvp[0]);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[8]][1] - mvp[1]);
        }
        break;
      case D_8x16:
        if (b_list[i_list][0])
        {
          xavs_mb_predict_mv (h, i_list, 0, 2, mvp);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][0] - mvp[0]);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[0]][1] - mvp[1]);
        }
        if (b_list[i_list][1])
        {
          xavs_mb_predict_mv (h, i_list, 4, 2, mvp);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[4]][0] - mvp[0]);
          bs_write_se (s, h->mb.cache.mv[i_list][xavs_scan8[4]][1] - mvp[1]);
        }
        break;
      }
    }
  }
  else if (i_mb_type == B_DIRECT)
  {
    bs_write_ue (s, 0);
  }
  else
  {
    xavs_log (h, XAVS_LOG_ERROR, "invalid/unhandled mb_type\n");
    return;
  }

#ifndef RDO_SKIP_BS
  i_mb_pos_tex = bs_pos (s);
  h->stat.frame.i_mv_bits += i_mb_pos_tex - i_mb_pos_start;
#endif

  /* Coded block patern */
  if ((i_mb_type == I_8x8) && (h->sh.i_type == SLICE_TYPE_I))
  {
    bs_write_ue (s, intra8x8_cbp_to_golomb[(h->mb.i_cbp_chroma << 4) | h->mb.i_cbp_luma]);
  }else if (!IS_INTRA (i_mb_type)){
    bs_write_ue (s, inter_cbp_to_golomb[(h->mb.i_cbp_chroma << 4) | h->mb.i_cbp_luma]);
  }

  //write qp_delta 
  if (((h->mb.i_cbp_chroma << 4) | h->mb.i_cbp_luma) && (h->mb.b_variable_qp == 1))
    cavlc_qp_delta (h, s);

  //write coeff  
  for (i = 0; i < 4; i++)
  {
    if (h->mb.i_cbp_luma & (1 << i))
      xavs_block_luma_write_cavlc (h, s, i, h->dct.luma8x8[i], 64);
  }

  for (i = 0; i < 2; i++)
  {
    if (h->mb.i_cbp_chroma & (1 << i))
      xavs_block_chroma_write_cavlc (h, s, 4 + i, h->dct.chroma8x8[i], 64);
  }

#ifndef RDO_SKIP_BS
  h->stat.frame.i_tex_bits += bs_pos (s) - i_mb_pos_tex;
#endif
}

#ifdef RDO_SKIP_BS
/*****************************************************************************
 * RD only; doesn't generate a valid bitstream
 * doesn't write cbp or chroma dc (I don't know how much this matters)
 * works on all partition sizes except 16x16
 * for sub8x8, call once per 8x8 block
 *****************************************************************************/
int
xavs_partition_size_cavlc (xavs_t * h, int i8, int i_pixel)
{
  bs_t s;
  const int i_mb_type = h->mb.i_type;
  int b_8x16 = h->mb.i_partition == D_8x16;

  s.i_bits_encoded = 0;

  if (i_mb_type == P_8x8)
  {
    bs_write_ue (&s, sub_mb_type_p_to_golomb[h->mb.i_sub_partition[i8]]);
    if (h->sh.i_num_ref_idx_l0_active > 1)
      bs_write_te (&s, h->sh.i_num_ref_idx_l0_active - 1, h->mb.cache.ref[0][xavs_scan8[4 * i8]]);
    cavlc_mb8x8_mvd (h, &s, 0, i8);
  }
  else if (i_mb_type == P_L0)
  {
    if (h->sh.i_num_ref_idx_l0_active > 1)
      bs_write_te (&s, h->sh.i_num_ref_idx_l0_active - 1, h->mb.cache.ref[0][xavs_scan8[4 * i8]]);
    if (h->mb.i_partition == D_16x8)
      cavlc_mb_mvd (h, &s, 0, 4 * i8, 4);
    else                        //8x16
      cavlc_mb_mvd (h, &s, 0, 4 * i8, 2);
  }
  else if (i_mb_type > B_DIRECT && i_mb_type < B_8x8)
  {
    if (xavs_mb_type_list0_table_cavlc[i_mb_type][0])
      cavlc_mb_mvd (h, &s, 0, 4 * i8, 4 >> b_8x16);
    if (xavs_mb_type_list0_table_cavlc[i_mb_type][1])
      cavlc_mb_mvd (h, &s, 0, 4 * i8, 4 >> b_8x16);
    if (xavs_mb_type_list1_table_cavlc[i_mb_type][0])
      cavlc_mb_mvd (h, &s, 1, 4 * i8, 4 >> b_8x16);
    if (xavs_mb_type_list1_table_cavlc[i_mb_type][1])
      cavlc_mb_mvd (h, &s, 1, 4 * i8, 4 >> b_8x16);
  }

  else if (i_mb_type == B_8x8)
  {
    bs_write_ue (&s, sub_mb_type_b_to_golomb[h->mb.i_sub_partition[i8]]);

    cavlc_mb8x8_mvd (h, &s, 0, i8);
    cavlc_mb8x8_mvd (h, &s, 1, i8);
  }
  else
  {
    xavs_log (h, XAVS_LOG_ERROR, "invalid/unhandled mb_type\n");
    return 0;
  }
  return s.i_bits_encoded;
}
static int
cavlc_intra8x8_pred_size (xavs_t * h, int i4, int i_mode)
{
  if (xavs_mb_predict_intra4x4_mode (h, i4) == xavs_mb_pred_mode8x8 (i_mode))
    return 1;
  else
    return 4;
}
static int
xavs_partition_i8x8_size_cavlc (xavs_t * h, int i8, int i_mode)
{
  //TBD 
  int i4, i;
  h->out.bs.i_bits_encoded = cavlc_intra8x8_pred_size (h, 4 * i8, i_mode);
  for (i4 = 0; i4 < 4; i4++)
  {
    for (i = 0; i < 16; i++)
      h->dct.block[i4 + i8 * 4].luma4x4[i] = h->dct.luma8x8[i8][i4 + i * 4];
    h->mb.cache.non_zero_count[xavs_scan8[i4 + i8 * 4]] = array_non_zero_count (h->dct.block[i4 + i8 * 4].luma4x4, 16);

    xavs_block_luma_write_cavlc (h, &h->out.bs, i, h->dct.luma8x8[i8], 64);
  }

  return h->out.bs.i_bits_encoded;
}

static int
xavs_i8x8_chroma_size_cavlc (xavs_t * h)
{
  h->out.bs.i_bits_encoded = bs_size_ue (xavs_mb_pred_mode8x8c[h->mb.i_chroma_pred_mode]);
  if (h->mb.i_cbp_chroma)
  {
    xavs_block_chroma_write_cavlc (h, &h->out.bs, 4 + 0, h->dct.chroma8x8[0], 64);
    xavs_block_chroma_write_cavlc (h, &h->out.bs, 4 + 1, h->dct.chroma8x8[1], 64);

  }
  return h->out.bs.i_bits_encoded;
}
#endif
