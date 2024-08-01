/*****************************************************************************
 * macroblock.h: h264 encoder library
 *****************************************************************************
 * Copyright (C) 2005-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
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
 * macroblock.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_MACROBLOCK_H_
#define _XAVS_MACROBLOCK_H_

enum macroblock_position_e
{
  MB_LEFT = 0x01,
  MB_TOP = 0x02,
  MB_TOPRIGHT = 0x04,
  MB_TOPLEFT = 0x08,
  MB_DOWNLEFT = 0x10,

  //MB_PRIVATE  = 0x10,
};


/* XXX mb_type isn't the one written in the bitstream -> only internal usage */
#define IS_INTRA(type) ( (type) == I_4x4 || (type) == I_8x8 || (type) == I_16x16 )
#define IS_SKIP(type)  ( (type) == P_SKIP || (type) == B_SKIP )
#define IS_DIRECT(type)  ( (type) == B_DIRECT )
enum mb_class_e
{
  I_4x4 = 0,
  I_8x8 = 1,
  I_16x16 = 2,
  I_PCM = 3,

  P_L0 = 4,
  P_8x8 = 5,
  P_SKIP = 6,

  B_DIRECT = 7,
  B_L0_L0 = 8,
  B_L0_L1 = 9,
  B_L0_BI = 10,
  B_L1_L0 = 11,
  B_L1_L1 = 12,
  B_L1_BI = 13,
  B_BI_L0 = 14,
  B_BI_L1 = 15,
  B_BI_BI = 16,
  B_8x8 = 17,
  B_SKIP = 18,
};
static const int xavs_mb_type_fix[19] = {
  I_4x4, I_8x8, I_16x16, I_PCM,
  P_L0, P_8x8, P_SKIP,
  B_DIRECT, B_L0_L0, B_L0_L1, B_L0_BI, B_L1_L0, B_L1_L1,
  B_L1_BI, B_BI_L0, B_BI_L1, B_BI_BI, B_8x8, B_SKIP
};
static const int xavs_mb_type_list0_table_cavlc[19][2] = {
  {0, 0}, {0, 0}, {0, 0}, {0, 0},       /* INTRA */
  {1, 1},                       /* P_L0 */
  {0, 0},                       /* P_8x8 */
  {1, 1},                       /* P_SKIP */
  {0, 0},                       /* B_DIRECT */
  {1, 1}, {1, 0}, {1, 1},       /* B_L0_* */
  {0, 1}, {0, 0}, {0, 1},       /* B_L1_* */
  {1, 1}, {1, 0}, {1, 1},       /* B_BI_* */
  {0, 0},                       /* B_8x8 */
  {0, 0}                        /* B_SKIP */
};
static const int xavs_mb_type_list1_table_cavlc[19][2] = {
  {0, 0}, {0, 0}, {0, 0}, {0, 0},       /* INTRA */
  {0, 0},                       /* P_L0 */
  {0, 0},                       /* P_8x8 */
  {0, 0},                       /* P_SKIP */
  {0, 0},                       /* B_DIRECT */
  {0, 0}, {0, 1}, {0, 0},       /* B_L0_* */
  {1, 0}, {1, 1}, {1, 0},       /* B_L1_* */
  {0, 0}, {0, 1}, {0, 0},       /* B_BI_* */
  {0, 0},                       /* B_8x8 */
  {0, 0}                        /* B_SKIP */
};
static const int xavs_mb_type_list0_table[19][2] = {
  {0, 0}, {0, 0}, {0, 0}, {0, 0},       /* INTRA */
  {1, 1},                       /* P_L0 */
  {0, 0},                       /* P_8x8 */
  {1, 1},                       /* P_SKIP */
  {0, 0},                       /* B_DIRECT */
  {1, 1}, {1, 0}, {1, 1},       /* B_L0_* */
  {0, 1}, {0, 0}, {0, 1},       /* B_L1_* */
  {1, 1}, {1, 0}, {1, 1},       /* B_BI_* */
  {0, 0},                       /* B_8x8 */
  {0, 0}                        /* B_SKIP */
};
static const int xavs_mb_type_list1_table[19][2] = {
  {0, 0}, {0, 0}, {0, 0}, {0, 0},       /* INTRA */
  {0, 0},                       /* P_L0 */
  {0, 0},                       /* P_8x8 */
  {0, 0},                       /* P_SKIP */
  {0, 0},                       /* B_DIRECT */
  {0, 0}, {0, 1}, {0, 1},       /* B_L0_* */
  {1, 0}, {1, 1}, {1, 1},       /* B_L1_* */
  {1, 0}, {1, 1}, {1, 1},       /* B_BI_* */
  {0, 0},                       /* B_8x8 */
  {0, 0}                        /* B_SKIP */
};

#define IS_SUB4x4(type) ( (type ==D_L0_4x4)||(type ==D_L1_4x4)||(type ==D_BI_4x4))
#define IS_SUB4x8(type) ( (type ==D_L0_4x8)||(type ==D_L1_4x8)||(type ==D_BI_4x8))
#define IS_SUB8x4(type) ( (type ==D_L0_8x4)||(type ==D_L1_8x4)||(type ==D_BI_8x4))
#define IS_SUB8x8(type) ( (type ==D_L0_8x8)||(type ==D_L1_8x8)||(type ==D_BI_8x8)||(type ==D_DIRECT_8x8))
enum mb_partition_e
{
  /* sub partition type for P_8x8 and B_8x8 */
  D_L0_4x4 = 0,
  D_L0_8x4 = 1,
  D_L0_4x8 = 2,
  D_L0_8x8 = 3,

  /* sub partition type for B_8x8 only */
  D_L1_4x4 = 4,
  D_L1_8x4 = 5,
  D_L1_4x8 = 6,
  D_L1_8x8 = 7,

  D_BI_4x4 = 8,
  D_BI_8x4 = 9,
  D_BI_4x8 = 10,
  D_BI_8x8 = 11,
  D_DIRECT_8x8 = 12,

  /* partition */
  D_8x8 = 13,
  D_16x8 = 14,
  D_8x16 = 15,
  D_16x16 = 16,
};

static const int xavs_mb_partition_listX_table_cavlc[2][17] = { {
                                                                 1, 1, 1, 1,    /* D_L0_* */
                                                                 0, 0, 0, 0,    /* D_L1_* */
                                                                 1, 1, 1, 1,    /* D_BI_* */
                                                                 0,     /* D_DIRECT_8x8 */
                                                                 0, 0, 0, 0     /* 8x8 .. 16x16 */
                                                                 },
{
 0, 0, 0, 0,                    /* D_L0_* */
 1, 1, 1, 1,                    /* D_L1_* */
 0, 0, 0, 0,                    /* D_BI_* */
 0,                             /* D_DIRECT_8x8 */
 0, 0, 0, 0                     /* 8x8 .. 16x16 */
 }
};
static const int xavs_mb_partition_listX_table[2][17] = { {
                                                           1, 1, 1, 1,  /* D_L0_* */
                                                           0, 0, 0, 0,  /* D_L1_* */
                                                           1, 1, 1, 1,  /* D_BI_* */
                                                           0,   /* D_DIRECT_8x8 */
                                                           0, 0, 0, 0   /* 8x8 .. 16x16 */
                                                           },
{
 0, 0, 0, 0,                    /* D_L0_* */
 1, 1, 1, 1,                    /* D_L1_* */
 1, 1, 1, 1,                    /* D_BI_* */
 0,                             /* D_DIRECT_8x8 */
 0, 0, 0, 0                     /* 8x8 .. 16x16 */
 }
};
static const int xavs_mb_partition_count_table[17] = {
  /* sub L0 */
  4, 2, 2, 1,
  /* sub L1 */
  4, 2, 2, 1,
  /* sub BI */
  4, 2, 2, 1,
  /* Direct */
  1,
  /* Partition */
  4, 2, 2, 1
};
static const int xavs_mb_partition_pixel_table[17] = {
  6, 4, 5, 3, 6, 4, 5, 3, 6, 4, 5, 3, 3, 3, 1, 2, 0
};

/* zigzags are transposed with respect to the tables in the standard */
static const int xavs_zigzag_scan4[16] = {
  0, 4, 1, 2, 5, 8, 12, 9, 6, 3, 7, 10, 13, 14, 11, 15
};
static const int xavs_zigzag_scan8[64] = {
  0, 8, 1, 2, 9, 16, 24, 17, 10, 3, 4, 11, 18, 25, 32, 40,
  33, 26, 19, 12, 5, 6, 13, 20, 27, 34, 41, 48, 56, 49, 42, 35,
  28, 21, 14, 7, 15, 22, 29, 36, 43, 50, 57, 58, 51, 44, 37, 30,
  23, 31, 38, 45, 52, 59, 60, 53, 46, 39, 47, 54, 61, 62, 55, 63
};

static const uint8_t block_idx_x[16] = {
  0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3
};
static const uint8_t block_idx_y[16] = {
  0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3
};
static const uint8_t block_idx_xy[4][4] = {
  {0, 2, 8, 10},
  {1, 3, 9, 11},
  {4, 6, 12, 14},
  {5, 7, 13, 15}
};

static const int i_chroma_qp_table[64] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39,
  40, 41, 42, 42, 43, 43, 44, 44, 45, 45,
  46, 46, 47, 47, 48, 48, 48, 49, 49, 49, 50, 50, 50, 51
};

enum cabac_ctx_block_cat_e
{
  DCT_LUMA_DC = 0,
  DCT_LUMA_AC = 1,
  DCT_LUMA_4x4 = 2,
  DCT_CHROMA_DC = 3,
  DCT_CHROMA_AC = 4,
  DCT_LUMA_8x8 = 5,
};


void xavs_macroblock_cache_init (xavs_t * h);
void xavs_macroblock_slice_init (xavs_t * h, int i_slice_type);
void xavs_macroblock_cache_load (xavs_t * h, int i_mb_x, int i_mb_y);
void xavs_macroblock_cache_save (xavs_t * h);
void xavs_macroblock_cache_end (xavs_t * h);

void xavs_macroblock_bipred_init (xavs_t * h);

/* xavs_mb_predict_mv_16x16:
 *      set mvp with predicted mv for D_16x16 block
 *      h->mb. need only valid values from other blocks */
void xavs_mb_predict_mv_16x16 (xavs_t * h, int i_list, int i_ref, int mvp[2]);
/* xavs_mb_predict_mv_pskip:
 *      set mvp with predicted mv for P_SKIP
 *      h->mb. need only valid values from other blocks */
void xavs_mb_predict_mv_pskip (xavs_t * h, int mv[2]);
/* xavs_mb_predict_mv:
 *      set mvp with predicted mv for all blocks except SKIP and DIRECT
 *      h->mb. need valid ref/partition/sub of current block to be valid
 *      and valid mv/ref from other blocks. */
void xavs_mb_predict_mv (xavs_t * h, int i_list, int idx, int i_width, int mvp[2]);
/* xavs_mb_predict_mv_direct16x16:
 *      set h->mb.cache.mv and h->mb.cache.ref for B_SKIP or B_DIRECT
 *      h->mb. need only valid values from other blocks.
 *      return 1 on success, 0 on failure.
 *      if b_changed != NULL, set it to whether refs or mvs differ from
 *      before this functioncall. */
int xavs_mb_predict_mv_direct16x16 (xavs_t * h, int *b_changed);
/* xavs_mb_load_mv_direct8x8:
 *      set h->mb.cache.mv and h->mb.cache.ref for B_DIRECT
 *      must be called only after xavs_mb_predict_mv_direct16x16 */
void xavs_mb_load_mv_direct8x8 (xavs_t * h, int idx);
/* xavs_mb_predict_mv_ref16x16:
 *      set mvc with D_16x16 prediction.
 *      uses all neighbors, even those that didn't end up using this ref.
 *      h->mb. need only valid values from other blocks */
void xavs_mb_predict_mv_ref16x16 (xavs_t * h, int i_list, int i_ref, int mvc[8][2], int *i_mvc);


int xavs_mb_predict_intra4x4_mode (xavs_t * h, int idx);
int xavs_mb_predict_non_zero_code (xavs_t * h, int idx);

/* xavs_mb_transform_8x8_allowed:
 *      check whether any partition is smaller than 8x8 (or at least
 *      might be, according to just partition type.)
 *      doesn't check for intra or cbp */
int xavs_mb_transform_8x8_allowed (xavs_t * h);

void xavs_mb_encode_i8x8 (xavs_t * h, int idx, int i_qscale);

void xavs_mb_mc (xavs_t * h);
void xavs_mb_mc_8x8 (xavs_t * h, int i8);


static inline void
xavs_macroblock_cache_ref (xavs_t * h, int x, int y, int width, int height, int i_list, int ref)
{
  int dy, dx;
  for (dy = 0; dy < height; dy++)
  {
    for (dx = 0; dx < width; dx++)
    {
      h->mb.cache.ref[i_list][XAVS_SCAN8_0 + x + dx + 8 * (y + dy)] = ref;
    }
  }
}

static inline void
xavs_macroblock_cache_mv (xavs_t * h, int x, int y, int width, int height, int i_list, int mvx, int mvy)
{
  int dy, dx;
  for (dy = 0; dy < height; dy++)
  {
    for (dx = 0; dx < width; dx++)
    {
      h->mb.cache.mv[i_list][XAVS_SCAN8_0 + x + dx + 8 * (y + dy)][0] = mvx;
      h->mb.cache.mv[i_list][XAVS_SCAN8_0 + x + dx + 8 * (y + dy)][1] = mvy;
    }
  }
}
static inline void
xavs_macroblock_cache_mvd (xavs_t * h, int x, int y, int width, int height, int i_list, int mdx, int mdy)
{
  int dy, dx;
  for (dy = 0; dy < height; dy++)
  {
    for (dx = 0; dx < width; dx++)
    {
      h->mb.cache.mvd[i_list][XAVS_SCAN8_0 + x + dx + 8 * (y + dy)][0] = mdx;
      h->mb.cache.mvd[i_list][XAVS_SCAN8_0 + x + dx + 8 * (y + dy)][1] = mdy;
    }
  }
}
static inline void
xavs_macroblock_cache_skip (xavs_t * h, int x, int y, int width, int height, int b_skip)
{
  int dy, dx;
  for (dy = 0; dy < height; dy++)
  {
    for (dx = 0; dx < width; dx++)
    {
      h->mb.cache.skip[XAVS_SCAN8_0 + x + dx + 8 * (y + dy)] = b_skip;
    }
  }
}
static inline void
xavs_macroblock_cache_intra8x8_pred (xavs_t * h, int x, int y, int i_mode)
{
  int *cache = &h->mb.cache.intra4x4_pred_mode[XAVS_SCAN8_0 + x + 8 * y];
  cache[0] = cache[1] = cache[8] = cache[9] = i_mode;
}

#endif
