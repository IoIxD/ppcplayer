/*****************************************************************************
 * common.h: h264 encoder
 *****************************************************************************
 * Copyright (C) 2003-2008 x264 project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Loren Merritt <lorenm@u.washington.edu>
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
 * common.h: xavs encoder
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_COMMON_H_
#define _XAVS_COMMON_H_

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include <stdarg.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#define XAVS_VERSION ""         // no configure script for msvc
#endif

/* threads */
#ifdef __WIN32__
#include <windows.h>
#define pthread_t               HANDLE
#define pthread_create(t,u,f,d) *(t)=CreateThread(NULL,0,f,d,0,NULL)
#define pthread_join(t,s)       { WaitForSingleObject(t,INFINITE); \
                                  CloseHandle(t); }
static int pthread_num_processors_np()
{
   SYSTEM_INFO si;
   GetSystemInfo(&si);
   return (int)(si.dwNumberOfProcessors);
}
#define HAVE_PTHREAD 1

#elif defined(SYS_BEOS)
#include <kernel/OS.h>
#define pthread_t               thread_id
#define pthread_create(t,u,f,d) { *(t)=spawn_thread(f,"",10,d); \
                                  resume_thread(*(t)); }
#define pthread_join(t,s)       { long tmp; \
                                  wait_for_thread(t,(s)?(long*)(s):&tmp); }
#define HAVE_PTHREAD 1

#elif defined(HAVE_PTHREAD)
#include <pthread.h>
#endif

/****************************************************************************
 * Macros
 ****************************************************************************/
#define XAVS_MIN(a,b) ( (a)<(b) ? (a) : (b) )
#define XAVS_MAX(a,b) ( (a)>(b) ? (a) : (b) )
#define XAVS_MIN3(a,b,c) XAVS_MIN((a),XAVS_MIN((b),(c)))
#define XAVS_MAX3(a,b,c) XAVS_MAX((a),XAVS_MAX((b),(c)))
#define XAVS_MIN4(a,b,c,d) XAVS_MIN((a),XAVS_MIN3((b),(c),(d)))
#define XAVS_MAX4(a,b,c,d) XAVS_MAX((a),XAVS_MAX3((b),(c),(d)))
#define XCHG(type,a,b) { type t = a; a = b; b = t; }
#define FIX8(f) ((int)(f*(1<<8)+.5))

#define CHECKED_MALLOC( var, size )\
	do {\
	var = xavs_malloc( size );\
	if( !var )\
	goto fail;\
	} while( 0 )
#define CHECKED_MALLOCZERO( var, size )\
	do {\
	CHECKED_MALLOC( var, size );\
	memset( var, 0, size );\
	} while( 0 )


#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#define XAVS_THREAD_MAX 16
#define XAVS_BFRAME_MAX 16
#define XAVS_LOOKAHEAD_MAX 100
#define XAVS_NAL_MAX (4 + XAVS_THREAD_MAX)

/****************************************************************************
 * Includes
 ****************************************************************************/
#include "osdep.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "xavs.h"
#include "bs.h"
#include "set.h"
#include "predict.h"
#include "pixel.h"
#include "mc.h"
#include "frame.h"
#include "dct.h"
#include "csp.h"
#include "quant.h"
#include "cpu.h"

/****************************************************************************
 * Generals functions
 ****************************************************************************/
/* xavs_malloc : will do or emulate a memalign
 * XXX you HAVE TO use xavs_free for buffer allocated
 * with xavs_malloc
 */
void *xavs_malloc (int);
void *xavs_realloc (void *p, int i_size);
void xavs_free (void *);

/* xavs_slurp_file: malloc space for the whole file and read it */
char *xavs_slurp_file (const char *filename);

/* mdate: return the current date in microsecond */
int64_t xavs_mdate (void);

/* xavs_param2string: return a (malloced) string containing most of
 * the encoding options */
char *xavs_param2string (xavs_param_t * p, int b_res);
void xavs_param_default (xavs_param_t * param);

/* log */
void xavs_log (xavs_t * h, int i_level, const char *psz_fmt, ...);

void xavs_reduce_fraction (int *n, int *d);

static inline int
xavs_clip3 (int v, int i_min, int i_max)
{
  return ((v < i_min) ? i_min : (v > i_max) ? i_max : v);
}
static inline float
xavs_clip3f (float v, float f_min, float f_max)
{
  return ((v < f_min) ? f_min : (v > f_max) ? f_max : v);
}
static inline double
xavs_clip3lf (double v, double f_min, double f_max)
{
  return ((v < f_min) ? f_min : (v > f_max) ? f_max : v);
}

static inline uint8_t
xavs_clip_uint8 (int x)
{
  return x & (~255) ? (-x) >> 31 : x;
}


static inline int
xavs_median (int a, int b, int c)
{
  int min = a, max = a;
  if (b < min)
    min = b;
  else
    max = b;                    /* no need to do 'b > max' (more consuming than always doing affectation) */

  if (c < min)
    min = c;
  else if (c > max)
    max = c;

  return a + b + c - min - max;
}


/****************************************************************************
 *
 ****************************************************************************/
enum slice_type_e
{
  SLICE_TYPE_P = 0,
  SLICE_TYPE_B = 1,
  SLICE_TYPE_I = 2,
  SLICE_TYPE_SP = 3,
  SLICE_TYPE_SI = 4
};

static const char slice_type_to_char[] = { 'P', 'B', 'I', 'S', 'S' };

typedef struct
{
  int i_slice_start_code;       // slice start code
  int i_slice_vertical_position;
  int i_slice_vertical_position_extension;      //
  int b_fixed_slice_qp;         //slice fixe qp, 1 means fixed qp in slice, otherwise variable qp
  int i_slice_qp;               //quantization parameters of current slice
  int b_slice_weighting_flag;   //
  int i_luma_scale[4];          //
  int i_luma_shift[4];          //
  //int  i_marker_bit;//
  int i_chroma_scale[4];        //
  int i_chroma_shift[4];        //
  //int  i_marker_bit;//
  int b_mb_weighting_flag;      //MB weighted prediction flag
  int i_mb_skip_run;            //
  int b_picture_fixed_qp;

  xavs_sps_t *sps;
  xavs_pps_t *pps;

  int i_type;
  int i_first_mb;
  int i_last_mb;

  int i_pps_id;

  int i_frame_num;

  int b_field_pic;
  int b_bottom_field;

  int i_idr_pic_id;             /* -1 if nal_type != 5 */

  int i_poc_lsb;
  int i_delta_poc_bottom;

  int i_delta_poc[2];
  int i_redundant_pic_cnt;

  int b_direct_spatial_mv_pred;

  int b_num_ref_idx_override;
  int i_num_ref_idx_l0_active;
  int i_num_ref_idx_l1_active;

  int b_ref_pic_list_reordering_l0;
  int b_ref_pic_list_reordering_l1;
  struct
  {
    int idc;
    int arg;
  } ref_pic_list_order[2][16];

  int i_cabac_init_idc;

  int i_qp;
  int i_qp_delta;
  int b_sp_for_swidth;
  int i_qs_delta;

  /* deblocking filter */
  int i_disable_deblocking_filter_idc;
  int i_alpha_c0_offset;
  int i_beta_offset;

} xavs_slice_header_t;

typedef struct xavs_lookahead_t
{
  volatile uint8_t b_exit_thread;
  uint8_t b_thread_active;
  uint8_t b_analyse_keyframe;
  uint8_t b_input_pending;
  int i_last_idr;
  int i_slicetype_length;
  xavs_frame_t *last_nonb;
  xavs_synch_frame_list_t ifbuf;
  xavs_synch_frame_list_t next;
  xavs_synch_frame_list_t ofbuf;
} xavs_lookahead_t;

/* From ffmpeg
 */
#define XAVS_SCAN8_SIZE (6*8)
#define XAVS_SCAN8_0 (4+1*8)

static const int xavs_scan8[16 + 2 * 4] = {
  /* Luma */
  4 + 1 * 8, 5 + 1 * 8, 4 + 2 * 8, 5 + 2 * 8,
  6 + 1 * 8, 7 + 1 * 8, 6 + 2 * 8, 7 + 2 * 8,
  4 + 3 * 8, 5 + 3 * 8, 4 + 4 * 8, 5 + 4 * 8,
  6 + 3 * 8, 7 + 3 * 8, 6 + 4 * 8, 7 + 4 * 8,

  /* Cb */
  1 + 1 * 8, 2 + 1 * 8,
  1 + 2 * 8, 2 + 2 * 8,

  /* Cr */
  1 + 4 * 8, 2 + 4 * 8,
  1 + 5 * 8, 2 + 5 * 8,
};

/*
   0 1 2 3 4 5 6 7
 0
 1   B B   L L L L
 2   B B   L L L L
 3         L L L L
 4   R R   L L L L
 5   R R
*/

typedef struct xavs_ratecontrol_t xavs_ratecontrol_t;
typedef struct xavs_vlc_table_t xavs_vlc_table_t;

struct xavs_t
{
  /* encoder parameters */
  xavs_param_t param;

  xavs_t *thread[XAVS_THREAD_MAX + 1];
  xavs_pthread_t thread_handle;
  int b_thread_active;
  int i_thread_phase;           /* which thread to use for the next frame */

  /* bitstream output */
  struct
  {
    int i_nal;
    xavs_nal_t nal[XAVS_NAL_MAX];
    int i_bitstream;            /* size of p_bitstream */
    uint8_t *p_bitstream;       /* will hold data for all nal */
    bs_t bs;
  } out;

  /* frame number/poc */
  int i_frame;

  int i_frame_offset;           /* decoding only */
  int i_frame_num;              /* decoding only */
  int i_thread_frames;          /* Number of different frames being encoded by threads;
                                 * 1 when sliced-threads is on. */

  int i_poc_msb;                /* decoding only */
  int i_poc_lsb;                /* decoding only */
  int i_poc;                    /* decoding only */

  int i_thread_num;             /* threads only */
  int i_nal_type;               /* threads only */
  int i_nal_ref_idc;            /* threads only */

  /* We use only one SPS and one PPS */
  xavs_sps_t sps_array[1];
  xavs_sps_t *sps;
  xavs_pps_t pps_array[1];
  xavs_pps_t *pps;
  int i_idr_pic_id;

  /*cqm 0 Intra Y; 1 inter Y; 2 Intra C; 3 Inter C; */
  /* quantization matrix for decoding, [cqm][qp][coef_y][coef_x] */
  int dequant8_mf[4][64][8][8]; /* [4][64][8][8] */
  /* quantization matrix for trellis, [cqm][qp][coef_y][coef_x] */
  int unquant8_mf[4][64][8][8]; /* [4][64][8][8] */
  /* quantization matrix for encoding [cqm][qp][coef] */
  int quant8_mf[4][64][64];     /* [4][64][64] */
  /* quantization encoding deadzone [cqm][qp][coef] */
  uint16_t quant8_bias[4][64][64];      /* [4][64][64] */

  uint32_t nr_residual_sum[2][64];
  uint32_t nr_offset[2][64];
  uint32_t nr_count[2];

  xavs_seq_header_t sqh;
  xavs_sequence_display_extention_t sde;
  xavs_copyright_extention_t cre;
  xavs_camera_parameters_extention_t cpe;
  xavs_user_data_t ud;
  xavs_picture_display_extention_t pde;
  xavs_i_pic_header_t ih;
  xavs_pb_pic_header_t pbh;

  /* Slice header */
  xavs_slice_header_t sh;

  struct
  {
    /* Frames to be encoded (whose types have been decided) */
    xavs_frame_t *current[XAVS_BFRAME_MAX + XAVS_LOOKAHEAD_MAX + XAVS_THREAD_MAX + 3];
    /* Temporary buffer (frames types not yet decided) */
    xavs_frame_t *next[XAVS_BFRAME_MAX + XAVS_LOOKAHEAD_MAX + XAVS_THREAD_MAX + 3];
    /* Unused frames */
    xavs_frame_t *unused[XAVS_BFRAME_MAX + XAVS_LOOKAHEAD_MAX + XAVS_THREAD_MAX + 3];
    /* For adaptive B decision */
    xavs_frame_t *last_nonb;

    /* frames used for reference +1 for decoding + sentinels */
    xavs_frame_t *reference[XAVS_BFRAME_MAX + XAVS_THREAD_MAX + 2 + 3];

    int i_last_idr;             /* Frame number of the last IDR */

    int i_input;                /* Number of input frames already accepted */

    int i_max_dpb;              /* Number of frames allocated in the decoded picture buffer */
    int i_max_ref0;
    int i_max_ref1;
    int i_delay;                /* Number of frames buffered for B reordering */
    int b_have_lowres;          /* Whether 1/2 resolution luma planes are being used */
  } frames;

  /* current frame being encoded */
  xavs_frame_t *fenc;

  /* frame being reconstructed */
  xavs_frame_t *fdec;

  /* references lists */
  int i_ref0;
  xavs_frame_t *fref0[16 + 3];  /* ref list 0 */
  int i_ref1;
  xavs_frame_t *fref1[16 + 3];  /* ref list 1 */
  int b_ref_reorder[2];



  /* Current MB DCT coeffs */
  struct
  {
    DECLARE_ALIGNED (int, luma16x16_dc[16], 16);
      DECLARE_ALIGNED (int, chroma_dc[2][4], 16);
      DECLARE_ALIGNED (int, luma8x8[4][64], 16);
      DECLARE_ALIGNED (int, chroma8x8[2][64], 16);
    union
    {
      DECLARE_ALIGNED (int, residual_ac[15], 16);
        DECLARE_ALIGNED (int, luma4x4[16], 16);
    } block[16 + 8];
  } dct;

  /* MB table and cache for current frame/mb */
  struct
  {
    int i_mb_count;             /* number of mbs in a frame */

    /* Strides */
    int i_mb_stride;
    int i_b8_stride;
    int i_b4_stride;

    /* Current index */
    int i_mb_x;
    int i_mb_y;
    int i_mb_xy;
    int i_b8_xy;
    int i_b4_xy;

    /* Search parameters */
    int i_me_method;
    int i_subpel_refine;
    int b_chroma_me;
    int b_trellis;
    int b_noise_reduction;

    /* Allowed qpel MV range to stay within the picture + emulated edge pixels */
    int mv_min[2];
    int mv_max[2];
    /* Subpel MV range for motion search.
     * same mv_min/max but includes levels' i_mv_range. */
    int mv_min_spel[2];
    int mv_max_spel[2];
    /* Fullpel MV range for motion search */
    int mv_min_fpel[2];
    int mv_max_fpel[2];

    /* neighboring MBs */
    unsigned int i_neighbour;
    unsigned int i_neighbour8[4];       /* neighbours of each 8x8 or 4x4 block that are available */
    unsigned int i_neighbour4[16];      /* at the time the block is coded */
    int i_mb_type_top;
    int i_mb_type_left;
    int i_mb_type_topleft;
    int i_mb_type_topright;

    /* mb table */
    int8_t *type;               /* mb type */
    int8_t *qp;                 /* mb qp */
    int16_t *cbp;               /* mb cbp: 0x0?: luma, 0x?0: chroma, 0x100: luma dc, 0x0200 and 0x0400: chroma dc  (all set for PCM) */
      int8_t (*intra4x4_pred_mode)[7];  /* intra4x4 pred mode. for non I4x4 set to I_PRED_4x4_DC(2) */
      uint8_t (*non_zero_count)[16 + 4 + 4];    /* nzc. for I_PCM set to 16 */
    int8_t *chroma_pred_mode;   /* chroma_pred_mode. cabac only. for non intra I_PRED_CHROMA_DC(0) */
      int16_t (*mv[2])[2];      /* mb mv. set to 0 for intra mb */
      int16_t (*mvd[2])[2];     /* mb mv difference with predict. set to 0 if intra. cabac only */
    int8_t *ref[2];             /* mb ref. set to -1 if non used (intra or Lx only) */
    int8_t list[352 * 288 * 4];
      int16_t (*mvr[2][16])[2]; /* 16x16 mv for each possible ref */
    int8_t *skipbp;             /* block pattern for SKIP or DIRECT (sub)mbs. B-frames + cabac only */
    int8_t *mb_transform_size;  /* transform_size_8x8_flag of each mb */

    /* current value */
    int i_type;
    int i_partition;
    int i_sub_partition[4];
    int b_transform_8x8;

    int i_cbp_luma;
    int i_cbp_chroma;

    int i_intra16x16_pred_mode;
    int i_chroma_pred_mode;

    struct
    {
      /* space for p_fenc and p_fdec */
#define FENC_STRIDE 16
#define FDEC_STRIDE 32
      DECLARE_ALIGNED (uint8_t, fenc_buf[24 * FENC_STRIDE], 16);
      DECLARE_ALIGNED (uint8_t, fdec_buf[27 * FDEC_STRIDE], 16);

      /* pointer over mb of the frame to be compressed */
      uint8_t *p_fenc[3];

      /* pointer over mb of the frame to be reconstrucated  */
      uint8_t *p_fdec[3];

      /* pointer over mb of the references */
      uint8_t *p_fref[2][16][4 + 2];    /* last: lN, lH, lV, lHV, cU, cV */
      uint16_t *p_integral[2][16];

      /* fref stride */
      int i_stride[3];
    } pic;

    /* cache */
    struct
    {
      /* real intra4x4_pred_mode if I_4X4 or I_8X8, I_PRED_4x4_DC if mb available, -1 if not */
      int intra4x4_pred_mode[XAVS_SCAN8_SIZE];

      /* i_non_zero_count if availble else 0x80 */
      int non_zero_count[XAVS_SCAN8_SIZE];

      /* -1 if unused, -2 if unavaible */
      int8_t ref[2][XAVS_SCAN8_SIZE];

      int8_t list[XAVS_SCAN8_SIZE];

      /* 0 if non avaible */
      int16_t mv[2][XAVS_SCAN8_SIZE][2];
      int16_t mvd[2][XAVS_SCAN8_SIZE][2];

      /* 1 if SKIP or DIRECT. set only for B-frames + CABAC */
      int8_t skip[XAVS_SCAN8_SIZE];

      int16_t direct_mv[2][XAVS_SCAN8_SIZE][2];
      int8_t direct_ref[2][XAVS_SCAN8_SIZE];

      /* number of neighbors (top and left) that used 8x8 dct */
      int i_neighbour_transform_size;
      int b_transform_8x8_allowed;
    } cache;

    /* */
    int i_qp;                   /* current qp */
    int i_chroma_qp;
    int i_last_qp;              /* last qp */
    int i_last_dqp;             /* last delta qp */
    int b_variable_qp;          /* whether qp is allowed to vary per macroblock */
    int b_lossless;
    int b_direct_auto_read;     /* take stats for --direct auto from the 2pass log */
    int b_direct_auto_write;    /* analyse direct modes, to use and/or save */

    /* B_direct and weighted prediction */
    int dist_scale_factor[16][16];
    int bipred_weight[16][16];
    /* maps fref1[0]'s ref indices into the current list0 */
    int map_col_to_list0_buf[2];        // for negative indices
    int map_col_to_list0[16];
  } mb;

  /* rate control encoding only */
  xavs_ratecontrol_t *rc;

  /* stats */
  struct
  {
    /* Current frame stats */
    struct
    {
      /* Headers bits (MV+Ref+MB Block Type */
      int i_mv_bits;
      /* Texture bits (Intra/Predicted) */
      int i_tex_bits;
      /* ? */
      int i_misc_bits;
      /* MB type counts */
      int i_mb_count[19];
      int i_mb_count_i;
      int i_mb_count_p;
      int i_mb_count_skip;
      int i_mb_count_8x8dct[2];
      int i_mb_count_size[7];
      int i_mb_count_ref[16];
      /* Estimated (SATD) cost as Intra/Predicted frame */
      /* XXX: both omit the cost of MBs coded as P_SKIP */
      int i_intra_cost;
      int i_inter_cost;
      /* Adaptive direct mv pred */
      int i_direct_score[2];
    } frame;

    /* Cummulated stats */

    /* per slice info */
    int i_slice_count[5];
    int64_t i_slice_size[5];
    int i_slice_qp[5];
    /* */
    int64_t i_sqe_global[5];
    float f_psnr_average[5];
    float f_psnr_mean_y[5];
    float f_psnr_mean_u[5];
    float f_psnr_mean_v[5];
    /* */
    int64_t i_mb_count[5][19];
    int64_t i_mb_count_8x8dct[2];
    int64_t i_mb_count_size[2][7];
    int64_t i_mb_count_ref[2][16];
    /* */
    int i_direct_score[2];
    int i_direct_frames[2];

  } stat;

  /* CPU functions dependants */
  xavs_predict_t predict_8x8c[4 + 3];
  xavs_predict_t predict_8x8[9 + 3];

  xavs_pixel_function_t pixf;
  xavs_mc_functions_t mc;
  xavs_dct_function_t dctf;
  xavs_csp_function_t csp;
  xavs_zigzag_function_t zigzagf;
  xavs_quant_function_t quantf;
  xavs_deblock_function_t loopf;

  /* vlc table for decoding purpose only */
  xavs_vlc_table_t *xavs_coeff_token_lookup[5];
  xavs_vlc_table_t *xavs_level_prefix_lookup;
  xavs_vlc_table_t *xavs_total_zeros_lookup[15];
  xavs_vlc_table_t *xavs_total_zeros_dc_lookup[3];
  xavs_vlc_table_t *xavs_run_before_lookup[7];

#if VISUALIZE
  struct visualize_t *visualize;
#endif

  xavs_lookahead_t *lookahead;
};

// included at the end because it needs xavs_t
#include "macroblock.h"

#endif
