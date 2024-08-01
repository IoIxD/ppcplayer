/*****************************************************************************
 * x264: h264 encoder
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
 * xavs: xavs encoder
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common/common.h"
#include "common/cpu.h"

#include "set.h"
#include "analyse.h"
#include "ratecontrol.h"
#include "macroblock.h"

#if VISUALIZE
#include "common/visualize.h"
#endif

//#define DEBUG_MB_TYPE
//#define DEBUG_BENCHMARK

#ifdef DEBUG_BENCHMARK
static int64_t i_mtime_encode_frame = 0;
static int64_t i_mtime_analyse = 0;
static int64_t i_mtime_encode = 0;
static int64_t i_mtime_write = 0;
static int64_t i_mtime_filter = 0;
#define TIMER_START( d ) \
    { \
        int64_t d##start = xavs_mdate();

#define TIMER_STOP( d ) \
        d += xavs_mdate() - d##start;\
    }
#else
#define TIMER_START( d )
#define TIMER_STOP( d )
#endif

#define NALU_OVERHEAD 5         // startcode + NAL type costs 5 bytes per frame

static void xavs_thread_sync_context( xavs_t *dst, xavs_t *src );
static void xavs_thread_sync_stat( xavs_t *dst, xavs_t *src );
static int xavs_encoder_get_frame_size( xavs_t *h, int start );

/****************************************************************************
 *
 ******************************* xavs libs **********************************
 *
 ****************************************************************************/
static float
xavs_psnr (int64_t i_sqe, int64_t i_size)
{
  double f_mse = (double) i_sqe / ((double) 65025.0 * (double) i_size);
  if (f_mse <= 0.0000000001)    /* Max 100dB */
    return 100;

  return (float) (-10.0 * log (f_mse) / log (10.0));
}

static void
xavs_frame_dump (xavs_t * h)
{
  FILE *f = fopen (h->param.psz_dump_yuv, "r+b");
  int i, y;
  if (!f)
    return;

  /* Write the frame in display order */
  fseek (f, h->fdec->i_frame * h->param.i_height * h->param.i_width * 3 / 2, SEEK_SET);
  for (i = 0; i < h->fdec->i_plane; i++)
    for (y = 0; y < h->param.i_height >> !!i; y++)
      fwrite (&h->fdec->plane[i][y * h->fdec->i_stride[i]], 1, h->param.i_width >> !!i, f);

  fclose (f);
}


/* Fill "default" values */
static void
xavs_slice_header_init (xavs_t * h, xavs_slice_header_t * sh, int i_qp)
{
  int i;

  sh->i_slice_start_code = 0x1;
  sh->i_slice_vertical_position = 0;    //one slice per frame now.
  sh->i_slice_vertical_position_extension = 0;
  sh->b_fixed_slice_qp = (h->param.rc.i_rc_method == XAVS_RC_CQP) ? 1 : 0;
  sh->i_slice_qp = i_qp;
  sh->b_slice_weighting_flag = 0;
  for (i = 0; i < 4; i++)

  {
    sh->i_luma_scale[i] = 1;
    sh->i_luma_shift[i] = 0;
    sh->i_chroma_scale[i] = 1;
    sh->i_chroma_shift[i] = 0;
  }
  sh->b_mb_weighting_flag = 0;

}

static void
xavs_slice_header_write (bs_t * s, xavs_slice_header_t * sh, xavs_seq_header_t * sqh, xavs_i_pic_header_t * ih, xavs_pb_pic_header_t * pbh)
{
  int fixed_picture_qp;

  bs_write (s, 8, sh->i_slice_vertical_position);
  if (sqh->i_vertical_size > 2800)
    bs_write (s, 3, sh->i_slice_vertical_position_extension);

  fixed_picture_qp = (sh->i_type == SLICE_TYPE_I) ? ih->b_fixed_picture_qp : pbh->b_fixed_picture_qp;

  if (fixed_picture_qp == 0)
  {
    bs_write1 (s, sh->b_fixed_slice_qp);
    bs_write (s, 6, sh->i_slice_qp);
  }

  if (!(sh->i_type == SLICE_TYPE_I))
  {
    bs_write1 (s, 0);           //sh->b_slice_weighting_flag); 
  }
}

/****************************************************************************
 *
 ****************************************************************************
 ****************************** External API*********************************
 ****************************************************************************
 *
 ****************************************************************************/

static int
xavs_validate_parameters (xavs_t * h)
{
  if (h->param.i_width <= 0 || h->param.i_height <= 0)
  {
    xavs_log (h, XAVS_LOG_ERROR, "invalid width x height (%dx%d)\n", h->param.i_width, h->param.i_height);
    return -1;
  }

  if (h->param.i_width % 2 || h->param.i_height % 2)
  {
    xavs_log (h, XAVS_LOG_ERROR, "width or height not divisible by 2 (%dx%d)\n", h->param.i_width, h->param.i_height);
    return -1;
  }
  if (h->param.i_csp != XAVS_CSP_I420)
  {
    xavs_log (h, XAVS_LOG_ERROR, "invalid CSP (only I420 supported)\n");
    return -1;
  }

  h->param.i_threads = xavs_clip3 (h->param.i_threads, 1, XAVS_THREAD_MAX);
  h->param.i_threads = XAVS_MIN (h->param.i_threads, (h->param.i_height + 15) / 16);

//#ifndef HAVE_PTHREAD
//  if (h->param.i_threads > 1)
//  {
//    xavs_log (h, XAVS_LOG_WARNING, "not compiled with pthread support!\n");
//    xavs_log (h, XAVS_LOG_WARNING, "multislicing anyway, but you won't see any speed gain.\n");
//  }
//  else
//    h->param.b_sliced_threads = 0;
//#endif

  h->i_thread_frames = h->param.b_sliced_threads ? 1 : h->param.i_threads;
  if(h->i_thread_frames > 1) 
     xavs_log (h, XAVS_LOG_INFO, "frame-level multithread encoding\n");
  else if (h->param.b_sliced_threads!=0 && h->param.i_threads>1 )
     xavs_log (h, XAVS_LOG_INFO, "slice-level multithread encoding\n");

  if (h->param.rc.i_rc_method < 0 || h->param.rc.i_rc_method > 2)
  {
    xavs_log (h, XAVS_LOG_ERROR, "no ratecontrol method specified\n");
    return -1;
  }
  h->param.rc.f_rf_constant = xavs_clip3f (h->param.rc.f_rf_constant, 0, 63);
  h->param.rc.i_qp_constant = xavs_clip3 (h->param.rc.i_qp_constant, 0, 63);
  if (h->param.rc.i_rc_method == XAVS_RC_CRF)
  {
    h->param.rc.i_qp_constant = h->param.rc.f_rf_constant;
    h->param.rc.i_bitrate = 0;
  }
  if ((h->param.rc.i_rc_method == XAVS_RC_CQP || h->param.rc.i_rc_method == XAVS_RC_CRF) && h->param.rc.i_qp_constant == 0)
  {
    h->mb.b_lossless = 1;
    h->param.i_cqm_preset = XAVS_CQM_FLAT;
    h->param.psz_cqm_file = NULL;
    h->param.rc.i_rc_method = XAVS_RC_CQP;
    h->param.rc.f_ip_factor = 1;
    h->param.rc.f_pb_factor = 1;
    h->param.analyse.b_psnr = 0;
    h->param.analyse.i_chroma_qp_offset = 0;
    h->param.analyse.i_trellis = 0;
    h->param.analyse.b_fast_pskip = 0;
    h->param.analyse.i_noise_reduction = 0;
    h->param.analyse.f_psy_rd = 0;
    h->param.i_bframe = 0;
  }
  if (h->param.rc.i_rc_method == XAVS_RC_CQP)
  {
    float qp_p = h->param.rc.i_qp_constant;
    float qp_i = qp_p - 6 * log (h->param.rc.f_ip_factor) / log (2);
    float qp_b = qp_p + 6 * log (h->param.rc.f_pb_factor) / log (2);
    h->param.rc.i_qp_min = xavs_clip3 ((int) (XAVS_MIN3 (qp_p, qp_i, qp_b)), 0, 63);
    h->param.rc.i_qp_max = xavs_clip3 ((int) (XAVS_MAX3 (qp_p, qp_i, qp_b) + .999), 0, 63);
    h->param.rc.i_aq_mode = 0;
    h->param.rc.b_mb_tree = 0;
  }
  h->param.rc.i_qp_max = xavs_clip3 (h->param.rc.i_qp_max, 0, 63);
  h->param.rc.i_qp_min = xavs_clip3 (h->param.rc.i_qp_min, 0, h->param.rc.i_qp_max);

  if ((h->param.i_width % 16 || h->param.i_height % 16) && h->param.i_height != 1080 && !h->mb.b_lossless)
  {
    // There's nothing special about 1080 in that the warning still applies to it,
    // but chances are the user can't help it if his content is already 1080p,
    // so there's no point in warning in that case.
    xavs_log (h, XAVS_LOG_WARNING, "width or height not divisible by 16 (%dx%d), compression will suffer.\n", h->param.i_width, h->param.i_height);
  }

  h->param.i_frame_reference = xavs_clip3 (h->param.i_frame_reference, 1, 2);
  if (h->param.i_keyint_max <= 0)
    h->param.i_keyint_max = 1;
  h->param.i_keyint_min = xavs_clip3 (h->param.i_keyint_min, 1, h->param.i_keyint_max / 2 + 1);
  h->param.rc.i_lookahead = xavs_clip3 (h->param.rc.i_lookahead, 0, XAVS_LOOKAHEAD_MAX);
  {
      int maxrate = XAVS_MAX( h->param.rc.i_vbv_max_bitrate, h->param.rc.i_bitrate );
      float bufsize = maxrate ? (float)h->param.rc.i_vbv_buffer_size / maxrate : 0;
      float fps = h->param.i_fps_num > 0 && h->param.i_fps_den > 0 ? (float) h->param.i_fps_num / h->param.i_fps_den : 25.0;
      h->param.rc.i_lookahead = XAVS_MIN( h->param.rc.i_lookahead, XAVS_MAX( h->param.i_keyint_max, bufsize*fps ) );
  }

  if( h->param.rc.b_stat_read )
      h->param.rc.i_lookahead = 0;
#ifdef HAVE_PTHREAD
  if( h->param.i_sync_lookahead < 0 )
      h->param.i_sync_lookahead = h->param.i_bframe + 1;
  h->param.i_sync_lookahead = XAVS_MIN( h->param.i_sync_lookahead, XAVS_LOOKAHEAD_MAX );
  if( h->param.rc.b_stat_read )
      h->param.i_sync_lookahead = 0;
#else
  h->param.i_sync_lookahead = 0;
#endif

  h->param.i_bframe = xavs_clip3 (h->param.i_bframe, 0, XAVS_BFRAME_MAX);
  h->param.i_bframe_bias = xavs_clip3 (h->param.i_bframe_bias, -90, 100);
  h->param.b_bframe_pyramid = h->param.b_bframe_pyramid && h->param.i_bframe > 1;
  h->param.i_bframe_adaptive = h->param.i_bframe_adaptive && h->param.i_bframe > 0;
  h->mb.b_direct_auto_write = h->param.analyse.i_direct_mv_pred == XAVS_DIRECT_PRED_AUTO && h->param.i_bframe && (h->param.rc.b_stat_write || !h->param.rc.b_stat_read);

  h->param.i_deblocking_filter_alphac0 = xavs_clip3 (h->param.i_deblocking_filter_alphac0, -6, 6);
  h->param.i_deblocking_filter_beta = xavs_clip3 (h->param.i_deblocking_filter_beta, -6, 6);

  h->param.i_cabac_init_idc = xavs_clip3 (h->param.i_cabac_init_idc, 0, 2);

  if (h->param.i_cqm_preset < XAVS_CQM_FLAT || h->param.i_cqm_preset > XAVS_CQM_CUSTOM)
    h->param.i_cqm_preset = XAVS_CQM_FLAT;

  if (h->param.analyse.i_me_method < XAVS_ME_DIA || h->param.analyse.i_me_method > XAVS_ME_ESA)
    h->param.analyse.i_me_method = XAVS_ME_HEX;
  if (h->param.analyse.i_me_range < 4)
    h->param.analyse.i_me_range = 4;
  if (h->param.analyse.i_me_range > 16 && h->param.analyse.i_me_method <= XAVS_ME_HEX)
    h->param.analyse.i_me_range = 16;
  h->param.analyse.i_subpel_refine = xavs_clip3 (h->param.analyse.i_subpel_refine, 1, 7);
  h->param.analyse.b_bframe_rdo = h->param.analyse.b_bframe_rdo && h->param.analyse.i_subpel_refine >= 6;
  h->param.analyse.b_mixed_references = h->param.analyse.b_mixed_references && h->param.i_frame_reference > 1;
  h->param.analyse.inter &= XAVS_ANALYSE_PSUB16x16 | XAVS_ANALYSE_PSUB8x8 | XAVS_ANALYSE_BSUB16x16 | XAVS_ANALYSE_I4x4 | XAVS_ANALYSE_I8x8;
  h->param.analyse.intra &= XAVS_ANALYSE_I4x4 | XAVS_ANALYSE_I8x8;
  if (!(h->param.analyse.inter & XAVS_ANALYSE_PSUB16x16))
    h->param.analyse.inter &= ~XAVS_ANALYSE_PSUB8x8;
  if (!h->param.analyse.b_transform_8x8)
  {
    h->param.analyse.inter &= ~XAVS_ANALYSE_I8x8;
    h->param.analyse.intra &= ~XAVS_ANALYSE_I8x8;
  }
  h->param.analyse.i_chroma_qp_offset = xavs_clip3 (h->param.analyse.i_chroma_qp_offset, -12, 12);

  h->param.rc.i_aq_mode = xavs_clip3 (h->param.rc.i_aq_mode, 0, 2);
  if (h->param.rc.f_aq_strength == 0)
    h->param.rc.i_aq_mode = 0;

  h->param.analyse.i_trellis = 0;

  h->param.analyse.i_trellis = xavs_clip3 (h->param.analyse.i_trellis, 0, 2);

  h->param.analyse.i_noise_reduction = xavs_clip3 (h->param.analyse.i_noise_reduction, 0, 1 << 16);

  {
    const xavs_level_t *l = xavs_levels;
    while (l->level_idc != 0 && l->level_idc != h->param.i_level_idc)
      l++;
    if (l->level_idc == 0)
    {
      xavs_log (h, XAVS_LOG_ERROR, "invalid level_idc: %d\n", h->param.i_level_idc);
      return -1;
    }
    if (h->param.analyse.i_mv_range <= 0)
      h->param.analyse.i_mv_range = (int) l->mv_range;
    else
      h->param.analyse.i_mv_range = xavs_clip3 (h->param.analyse.i_mv_range, 32, 2048);
  }

  if (h->param.rc.f_qblur < 0)
    h->param.rc.f_qblur = 0;
  if (h->param.rc.f_complexity_blur < 0)
    h->param.rc.f_complexity_blur = 0;

  /* ensure the booleans are 0 or 1 so they can be used in math */
#define BOOLIFY(x) h->param.x = !!h->param.x
  BOOLIFY (b_cabac);
  BOOLIFY (b_deblocking_filter);
  BOOLIFY (analyse.b_transform_8x8);
  BOOLIFY (analyse.b_weighted_bipred);
  BOOLIFY (analyse.b_bidir_me);
  BOOLIFY (analyse.b_chroma_me);
  BOOLIFY (analyse.b_fast_pskip);
  BOOLIFY (rc.b_stat_write);
  BOOLIFY (rc.b_stat_read);
#undef BOOLIFY

  return 0;
}

/****************************************************************************
 * xavs_encoder_open:
 ****************************************************************************/
xavs_t *
xavs_encoder_open (xavs_param_t * param)
{
  int i_slicetype_length;
  xavs_t *h = xavs_malloc (sizeof (xavs_t));
  int i;

  memset (h, 0, sizeof (xavs_t));

  /* Create a copy of param */
  memcpy (&h->param, param, sizeof (xavs_param_t));

  if (xavs_validate_parameters (h) < 0)
  {
    xavs_free (h);
    return NULL;
  }

  if (h->param.psz_cqm_file)
    if (xavs_cqm_parse_file (h, h->param.psz_cqm_file) < 0)
    {
      xavs_free (h);
      return NULL;
    }

  if (h->param.rc.psz_stat_out)
    h->param.rc.psz_stat_out = strdup (h->param.rc.psz_stat_out);
  if (h->param.rc.psz_stat_in)
    h->param.rc.psz_stat_in = strdup (h->param.rc.psz_stat_in);

  /* VUI */
  if (h->param.vui.i_sar_width > 0 && h->param.vui.i_sar_height > 0)
  {
    int i_w = param->vui.i_sar_width;
    int i_h = param->vui.i_sar_height;

    xavs_reduce_fraction (&i_w, &i_h);

    while (i_w > 65535 || i_h > 65535)
    {
      i_w /= 2;
      i_h /= 2;
    }

    h->param.vui.i_sar_width = 0;
    h->param.vui.i_sar_height = 0;
    if (i_w == 0 || i_h == 0)
    {
      xavs_log (h, XAVS_LOG_ERROR, "cannot create valid sample aspect ratio\n");
    }
    else if (i_w == i_h)
    {
      xavs_log (h, XAVS_LOG_INFO, "no need for a SAR\n");
    }
    else
    {
      xavs_log (h, XAVS_LOG_INFO, "using SAR=%d/%d\n", i_w, i_h);
      h->param.vui.i_sar_width = i_w;
      h->param.vui.i_sar_height = i_h;
    }
  }

  xavs_reduce_fraction (&h->param.i_fps_num, &h->param.i_fps_den);

  /* Init xavs_t */
  h->out.i_nal = 0;
  h->out.i_bitstream = (int) XAVS_MAX (1000000, h->param.i_width * h->param.i_height * 1.7 * (h->param.rc.i_rc_method ? pow (0.5, h->param.rc.i_qp_min) : pow (0.5, h->param.rc.i_qp_constant) * XAVS_MAX (1, h->param.rc.f_ip_factor)));
  h->out.p_bitstream = xavs_malloc (h->out.i_bitstream);

  h->i_frame = 0;
  h->i_frame_num = 0;
  h->i_idr_pic_id = 0;

  /*init the sequence header */
  xavs_sequence_init (&h->sqh, &h->param);

  h->sps = &h->sps_array[0];
  xavs_sps_init (h->sps, 0, &h->param);

  h->pps = &h->pps_array[0];
  xavs_pps_init (h->pps, 0, &h->param, h->sps);

  xavs_validate_levels (h);

  xavs_cqm_init (h);

  h->mb.i_mb_count = h->sps->i_mb_width * h->sps->i_mb_height;

  /* Init frames. */
  if( h->param.i_bframe_adaptive == XAVS_B_ADAPT_TRELLIS && !h->param.rc.b_stat_read )
      h->frames.i_delay = XAVS_MAX(h->param.i_bframe,3)*4;
  else
      h->frames.i_delay = h->param.i_bframe;
  if( h->param.rc.i_vbv_buffer_size )
      h->frames.i_delay = XAVS_MAX( h->frames.i_delay, h->param.rc.i_lookahead );
  i_slicetype_length = h->frames.i_delay;
  h->frames.i_delay += h->i_thread_frames - 1;
  h->frames.i_delay += h->param.i_sync_lookahead;

  h->frames.i_max_ref0 = h->param.i_frame_reference;
  h->frames.i_max_ref1 = h->sps->vui.i_num_reorder_frames;
  h->frames.i_max_dpb = h->sps->vui.i_max_dec_frame_buffering;
  h->frames.b_have_lowres = !h->param.rc.b_stat_read && (h->param.rc.i_rc_method == XAVS_RC_ABR || h->param.rc.i_rc_method == XAVS_RC_CRF || h->param.i_bframe_adaptive || h->param.i_scenecut_threshold);
  h->frames.b_have_lowres |= (h->param.rc.b_stat_read && h->param.rc.i_vbv_buffer_size > 0);

  for (i = 0; i < sizeof(h->frames.current)/sizeof(xavs_frame_t *); i++)
  {
    h->frames.current[i] = NULL;
    h->frames.next[i] = NULL;
    h->frames.unused[i] = NULL;
  }
/*
  for (i = 0; i < 1 + h->frames.i_delay + h->frames.i_max_dpb; i++)
  {
    h->frames.unused[i] = xavs_frame_new (h);
  }
  for (i = 0; i <= h->frames.i_max_dpb; i++)
  {
    h->frames.reference[i] = NULL;
  }
*/
  h->frames.i_last_idr = -h->param.i_keyint_max;
  h->frames.i_input = 0;
  h->frames.last_nonb = NULL;

  h->i_ref0 = 0;
  h->i_ref1 = 0;

  h->fdec = xavs_frame_get_unused(h);
  xavs_macroblock_cache_init(h);

  /* init CPU functions */
  xavs_predict_8x8c_init (h->param.cpu, h->predict_8x8c);
  xavs_predict_8x8_init (h->param.cpu, h->predict_8x8);

  xavs_pixel_init (h->param.cpu, &h->pixf);
  xavs_dct_init (h->param.cpu, &h->dctf);
  xavs_mc_init (h->param.cpu, &h->mc);
  xavs_csp_init (h->param.cpu, h->param.i_csp, &h->csp);
  xavs_quant_init (h, h->param.cpu, &h->quantf);
  xavs_deblock_init (h->param.cpu, &h->loopf);

  memcpy (h->pixf.mbcmp, (h->mb.b_lossless || h->param.analyse.i_subpel_refine <= 1) ? h->pixf.sad : h->pixf.satd, sizeof (h->pixf.mbcmp));

  /* allocate the memory for each thread in the rate_control */
  h->i_thread_num = 0;
  h->thread[0] = h;
  for (i = 1; i < param->i_threads + !!h->param.i_sync_lookahead; i++)
  {
      h->thread[i] = xavs_malloc (sizeof (xavs_t));
  }

  for (i = 1; i < param->i_threads + !!h->param.i_sync_lookahead; i++)
  {
    *h->thread[i] = *h;
    if (!h->param.b_sliced_threads) {
      h->thread[i]->fdec = xavs_frame_get_unused(h);
      xavs_macroblock_cache_init(h->thread[i]);
      h->thread[i]->out.p_bitstream = xavs_malloc (h->out.i_bitstream);
    } 
    else
      h->thread[i]->fdec = h->thread[0]->fdec;

  }

  if (xavs_lookahead_init(h, i_slicetype_length))
    goto fail;

  /* rate control */
  if (xavs_ratecontrol_new (h) < 0)
    return NULL;

  if (h->param.psz_dump_yuv)
  {
    /* create or truncate the reconstructed video file */
    FILE *f = fopen (h->param.psz_dump_yuv, "w");
    if (f)
      fclose (f);
    else
    {
      xavs_log (h, XAVS_LOG_ERROR, "can't write to %s\n");
      goto fail;
    }
  }

  xavs_log (h, XAVS_LOG_INFO, "using cpu capabilities %s%s%s%s%s%s\n",
            param->cpu & XAVS_CPU_MMX ? "MMX " : "", param->cpu & XAVS_CPU_MMXEXT ? "MMXEXT " : "", param->cpu & XAVS_CPU_SSE ? "SSE " : "", param->cpu & XAVS_CPU_SSE2 ? "SSE2 " : "", param->cpu & XAVS_CPU_3DNOW ? "3DNow! " : "", param->cpu & XAVS_CPU_ALTIVEC ? "Altivec " : "");

  xavs_log( h, XAVS_LOG_INFO, "profile %s, level %d.%d\n",
            h->sps->i_profile_idc == PROFILE_JIZHUN ? "JiZhun" :
	    h->sps->i_profile_idc == PROFILE_YIDONG ? "Mobile" :
	    h->sps->i_profile_idc == PROFILE_SHENZHAN? "ShenZhan": "JiaQiang", 
	    h->sps->i_level_idc/10, h->sps->i_level_idc%10 );

  return h;

fail:
  xavs_free (h);
  return NULL;
}

/****************************************************************************
 * xavs_encoder_reconfig:
 ****************************************************************************/
int
xavs_encoder_reconfig (xavs_t * h, xavs_param_t * param)
{
  h->param.i_bframe_bias = param->i_bframe_bias;
  h->param.i_deblocking_filter_alphac0 = param->i_deblocking_filter_alphac0;
  h->param.i_deblocking_filter_beta = param->i_deblocking_filter_beta;
  h->param.analyse.i_me_method = param->analyse.i_me_method;
  h->param.analyse.i_me_range = param->analyse.i_me_range;
  h->param.analyse.i_subpel_refine = param->analyse.i_subpel_refine;
  h->param.analyse.i_trellis = param->analyse.i_trellis;
  h->param.analyse.intra = param->analyse.intra;
  h->param.analyse.inter = param->analyse.inter;

  memcpy (h->pixf.mbcmp, (h->mb.b_lossless || h->param.analyse.i_subpel_refine <= 1) ? h->pixf.sad : h->pixf.satd, sizeof (h->pixf.mbcmp));

  return xavs_validate_parameters (h);
}

/* internal usage */
static void
xavs_nal_start (xavs_t * h, int i_type, int i_ref_idc)
{
  xavs_nal_t *nal = &h->out.nal[h->out.i_nal];

  nal->i_ref_idc = i_ref_idc;
  nal->i_type = i_type;

  bs_align_0 (&h->out.bs);      /* not needed */

  nal->i_payload = 0;
  nal->p_payload = &h->out.p_bitstream[bs_pos (&h->out.bs) / 8];
}
static void
xavs_nal_end (xavs_t * h)
{
  xavs_nal_t *nal = &h->out.nal[h->out.i_nal];

  bs_align_0 (&h->out.bs);      /* not needed */

  nal->i_payload = (int) (&h->out.p_bitstream[bs_pos (&h->out.bs) / 8] - nal->p_payload);

  h->out.i_nal++;
}

/****************************************************************************
 * xavs_encoder_headers:
 ****************************************************************************/
int
xavs_encoder_headers (xavs_t * h, xavs_nal_t ** pp_nal, int *pi_nal)
{
  /* init bitstream context */
  h->out.i_nal = 0;
  bs_init (&h->out.bs, h->out.p_bitstream, h->out.i_bitstream);

  /* Put SPS and PPS */
  if (h->i_frame == 0)
  {
    /* identify ourself */
    xavs_nal_start (h, NAL_SEI, NAL_PRIORITY_DISPOSABLE);
    xavs_sei_version_write (h, &h->out.bs);
    xavs_nal_end (h);

    /* generate sequence parameters */
    xavs_nal_start (h, NAL_SPS, NAL_PRIORITY_HIGHEST);
    xavs_sps_write (&h->out.bs, h->sps);
    xavs_nal_end (h);

    /* generate picture parameters */
    xavs_nal_start (h, NAL_PPS, NAL_PRIORITY_HIGHEST);
    xavs_pps_write (&h->out.bs, h->pps);
    xavs_nal_end (h);
  }
  /* now set output */
  *pi_nal = h->out.i_nal;
  *pp_nal = &h->out.nal[0];

  return 0;
}

static inline void
xavs_frame_sort (xavs_frame_t * list[XAVS_BFRAME_MAX + 1], int b_dts)
{
  int i, b_ok;
  do
  {
    b_ok = 1;
    for (i = 0; list[i + 1]; i++)
    {
      int dtype = list[i]->i_type - list[i + 1]->i_type;
      int dtime = list[i]->i_frame - list[i + 1]->i_frame;
      int swap = b_dts ? dtype > 0 || (dtype == 0 && dtime > 0) : dtime > 0;
      if (swap)
      {
        XCHG (xavs_frame_t *, list[i], list[i + 1]);
        b_ok = 0;
      }
    }
  }
  while (!b_ok);
}

#define xavs_frame_sort_dts(list) xavs_frame_sort(list, 1)
#define xavs_frame_sort_pts(list) xavs_frame_sort(list, 0)

static inline void
xavs_reference_build_list (xavs_t * h, int i_poc, int i_slice_type)
{
  int i;
  int b_ok;

  /* build ref list 0/1 */
  h->i_ref0 = 0;
  h->i_ref1 = 0;

  for (i = 0; h->frames.reference[i]; i++)
  {
    if (h->frames.reference[i]->i_poc >= 0)
    {
      if (h->frames.reference[i]->i_poc < i_poc)
      {
        h->fref0[h->i_ref0++] = h->frames.reference[i];
      }
      else if (h->frames.reference[i]->i_poc > i_poc)
      {
        h->fref1[h->i_ref1++] = h->frames.reference[i];
      }
    }
  }

  /* Order ref0 from higher to lower poc */
  do
  {
    b_ok = 1;
    for (i = 0; i < h->i_ref0 - 1; i++)
    {
      if (h->fref0[i]->i_poc < h->fref0[i + 1]->i_poc)
      {
        XCHG (xavs_frame_t *, h->fref0[i], h->fref0[i + 1]);
        b_ok = 0;
        break;
      }
    }
  }
  while (!b_ok);
  /* Order ref1 from lower to higher poc (bubble sort) for B-frame */
  do
  {
    b_ok = 1;
    for (i = 0; i < h->i_ref1 - 1; i++)
    {
      if (h->fref1[i]->i_poc > h->fref1[i + 1]->i_poc)
      {
        XCHG (xavs_frame_t *, h->fref1[i], h->fref1[i + 1]);
        b_ok = 0;
        break;
      }
    }
  }
  while (!b_ok);

  /* In the standard, a P-frame's ref list is sorted by frame_num.
   * We use POC, but check whether explicit reordering is needed */
  h->b_ref_reorder[0] = h->b_ref_reorder[1] = 0;
  if (i_slice_type == SLICE_TYPE_P)
  {
    for (i = 0; i < h->i_ref0 - 1; i++)
      if (h->fref0[i]->i_frame_num < h->fref0[i + 1]->i_frame_num)
      {
        h->b_ref_reorder[0] = 1;
        break;
      }
  }

  h->i_ref1 = (i_slice_type == SLICE_TYPE_B) ? 1 : XAVS_MIN (h->i_ref1, h->frames.i_max_ref1);
  h->i_ref0 = XAVS_MIN (h->i_ref0, h->frames.i_max_ref0);
  h->i_ref0 = (i_slice_type == SLICE_TYPE_B) ? 1 : XAVS_MIN (h->i_ref0, 16 - h->i_ref1);
}

static inline int 
xavs_update_reference_list (xavs_t *h)
{
  if( !h->fdec->b_kept_as_ref )
  {
      if( h->i_thread_frames > 1 )
      {
          xavs_frame_put_unused(h, h->fdec);
          h->fdec = xavs_frame_get_unused(h);
          if( !h->fdec )
              return -1;
      }
      return 0;
  }

  /* move frame in the buffer */
  assert(h->frames.reference[h->sps->i_num_ref_frames] == NULL);
  xavs_frame_put( h->frames.reference, h->fdec );
  if( h->frames.reference[h->sps->i_num_ref_frames] )
      xavs_frame_put_unused(h, xavs_frame_get(h->frames.reference));
  assert(h->frames.reference[h->sps->i_num_ref_frames] == NULL);
  h->fdec = xavs_frame_get_unused(h);
  if( !h->fdec )
      return -1;
  return 0;
}

static inline void
xavs_reference_update (xavs_t * h)
{
  int i;

  /* expand border */
  xavs_frame_expand_border (h->fdec);

  /* create filtered images */
  xavs_frame_filter (h->param.cpu, h->fdec);

  /* expand border of filtered images */
  xavs_frame_expand_border_filtered (h->fdec);

  /* move lowres copy of the image to the ref frame */
  for (i = 0; i < 4; i++)
    XCHG (uint8_t *, h->fdec->lowres[i], h->fenc->lowres[i]);
}

static inline void
xavs_reference_reset (xavs_t * h)
{
  while (h->frames.reference[0])
    xavs_frame_put_unused(h, xavs_frame_get(h->frames.reference));

  h->fdec->i_poc = h->fenc->i_poc = 0;
}

static void
xavs_i_pic_header_init (xavs_t * h, xavs_i_pic_header_t * ih, int i_qp)
{
  ih->i_i_picture_start_code = 0xB3;
  ih->i_bbv_delay = 0xFFFF;     //
  ih->b_time_code_flag = 0;     // if there's time code in this picture header
  ih->i_time_code = 0;          //need to change

  ih->i_picture_distance = (h->fenc->i_poc / 2) % 256;  //h->fenc->i_frame % 256;

  ih->i_bbv_check_times = 0;
  ih->b_progressive_frame = 1;
  if (!ih->b_progressive_frame)
    ih->b_picture_structure = 1;
  else
    ih->b_picture_structure = 0;

  ih->b_top_field_first = 1;
  ih->b_repeat_first_field = 0;
  ih->b_fixed_picture_qp = (h->param.rc.i_rc_method == XAVS_RC_CQP) ? 1 : 0;
  ih->i_picture_qp = i_qp;

  ih->b_skip_mode_flag = h->param.analyse.b_skip_mode;

  ih->i_reserved_bits = 0;
  ih->b_loop_filter_disable = !h->param.b_deblocking_filter;
  ih->b_loop_filter_parameter_flag = 0; //'1' there are alpha_c_offset & beta_offset parameters in streams 
  //'0' there are no alpha_c_offset & beta_offset parameters in streams 
  if (ih->b_loop_filter_parameter_flag)
  {
    ih->i_alpha_c_offset = h->param.i_deblocking_filter_alphac0;
    ih->i_beta_offset = h->param.i_deblocking_filter_beta;
  }
}

static inline void
xavs_pb_pic_header_init (xavs_t * h, xavs_pb_pic_header_t * pbh, int i_qp, int i_slice_type)
{
  pbh->i_pb_picture_start_code = 0xB6;
  pbh->i_bbv_delay = 0xFFFF;
  pbh->i_picture_coding_type = (i_slice_type == SLICE_TYPE_P) ? 1 : 2;
  pbh->i_picture_distance = (h->fenc->i_poc / 2) % 256; //h->fenc->i_frame % 256;
  pbh->i_bbv_check_times = 0;
  pbh->b_progressive_frame = 1;
  pbh->b_picture_structure = 1;
  pbh->b_advanced_pred_mode_disable = 1;
  pbh->b_top_field_first = 1;
  pbh->b_repeat_first_field = 0;
  pbh->b_fixed_picture_qp = (h->param.rc.i_rc_method == XAVS_RC_CQP) ? 1 : 0;
  pbh->i_picture_qp = i_qp;
  pbh->b_picture_reference_flag = (h->i_ref0 > 1) ? 0 : 1;
  pbh->b_no_forward_reference_flag = 0;
  pbh->b_skip_mode_flag = h->param.analyse.b_skip_mode;
  pbh->b_loop_filter_disable = !h->param.b_deblocking_filter;
  pbh->b_loop_filter_parameter_flag = 1;        //'1' there are alpha_c_offset & beta_offset parameters in streams 
  //'0' there are no alpha_c_offset & beta_offset parameters in streams 
  if (pbh->b_loop_filter_parameter_flag)
  {
    pbh->i_alpha_c_offset = h->param.i_deblocking_filter_alphac0;
    pbh->i_beta_offset = h->param.i_deblocking_filter_beta;
  }
}

static void
xavs_slice_init (xavs_t * h, int i_slice_type, int i_qp)
{
  xavs_slice_header_init (h, &h->sh, i_qp);

  h->sh.i_type = i_slice_type;
  h->sh.i_first_mb = 0;
  h->sh.i_last_mb = h->mb.i_mb_count;
  h->sh.i_frame_num = h->i_frame_num;
  h->sh.i_qp = i_qp;

  /* If effective qp <= 15, deblocking would have no effect anyway */
  h->sh.i_disable_deblocking_filter_idc = !h->param.b_deblocking_filter;
  h->sh.i_alpha_c0_offset = h->param.i_deblocking_filter_alphac0;
  h->sh.i_beta_offset = h->param.i_deblocking_filter_beta;

  h->sh.i_num_ref_idx_l0_active = h->i_ref0 <= 0 ? 1 : h->i_ref0;
  h->sh.i_num_ref_idx_l1_active = h->i_ref1 <= 0 ? 1 : h->i_ref1;

  h->fdec->i_frame_num = h->sh.i_frame_num;

  xavs_macroblock_slice_init (h, i_slice_type);
}

static int
xavs_slice_write (xavs_t * h)
{
  int i_skip;
  int mb_xy;
  int i;

  /* init stats */
  memset (&h->stat.frame, 0, sizeof (h->stat.frame));

  /* Slice */
  xavs_nal_start (h, h->i_nal_type, h->i_nal_ref_idc);

  /* Slice header */
  xavs_slice_header_write (&h->out.bs, &h->sh, &h->sqh, &h->ih, &h->pbh);

  h->mb.i_last_qp = h->sh.i_qp;
  h->mb.i_last_dqp = 0;

  for (mb_xy = h->sh.i_first_mb, i_skip = 0; mb_xy < h->sh.i_last_mb; mb_xy++)
  {
    const int i_mb_y = mb_xy / h->sps->i_mb_width;
    const int i_mb_x = mb_xy % h->sps->i_mb_width;

    int mb_spos = bs_pos (&h->out.bs);

    /* load cache */
    xavs_macroblock_cache_load (h, i_mb_x, i_mb_y);

    /* analyse parameters
     * Slice I: choose I_8x8 submodes
     * Slice P: choose between using P mode or intra (8x8)
     * */
    TIMER_START (i_mtime_analyse);
    xavs_macroblock_analyse (h);
    TIMER_STOP (i_mtime_analyse);

    /* encode this macrobock -> be carefull it can change the mb type to P_SKIP if needed */
    TIMER_START (i_mtime_encode);
    xavs_macroblock_encode (h);
    TIMER_STOP (i_mtime_encode);

    TIMER_START (i_mtime_write);

    if (IS_SKIP (h->mb.i_type))
      i_skip++;
    else
    {
      if (h->sh.i_type != SLICE_TYPE_I)
      {
        bs_write_ue (&h->out.bs, i_skip);       /* skip run */
        i_skip = 0;
      }
      xavs_macroblock_write_cavlc (h, &h->out.bs);
    }

    TIMER_STOP (i_mtime_write);

#if VISUALIZE
    if (h->param.b_visualize)
      xavs_visualize_mb (h);
#endif

    /* save cache */
    xavs_macroblock_cache_save (h);

    /* accumulate mb stats */
    h->stat.frame.i_mb_count[h->mb.i_type]++;
    if (!IS_SKIP (h->mb.i_type) && !IS_INTRA (h->mb.i_type) && !IS_DIRECT (h->mb.i_type))
    {
      if (h->mb.i_partition != D_8x8)
        h->stat.frame.i_mb_count_size[xavs_mb_partition_pixel_table[h->mb.i_partition]] += 4;
      else
        for (i = 0; i < 4; i++)
          h->stat.frame.i_mb_count_size[xavs_mb_partition_pixel_table[h->mb.i_sub_partition[i]]]++;
      if (h->param.i_frame_reference > 1)
      {
        for (i = 0; i < 4; i++)
        {
          int i_ref = h->mb.cache.ref[0][xavs_scan8[4 * i]];
          if (i_ref >= 0)
            h->stat.frame.i_mb_count_ref[i_ref]++;
        }
      }
    }

    if (h->mb.i_cbp_luma && !IS_INTRA (h->mb.i_type))
    {
      h->stat.frame.i_mb_count_8x8dct[0]++;
      h->stat.frame.i_mb_count_8x8dct[1]++;     //= h->mb.b_transform_8x8;
    }

    xavs_ratecontrol_mb (h, bs_pos (&h->out.bs) - mb_spos);
  }

  if (i_skip > 0)
  {
    bs_write_ue (&h->out.bs, i_skip);   /* last skip run */
  }

  /* rbsp_slice_trailing_bits */
  bs_rbsp_trailing (&h->out.bs);

  xavs_nal_end (h);

  /* Compute misc bits */
  h->stat.frame.i_misc_bits = bs_pos (&h->out.bs) + NALU_OVERHEAD * 8 - h->stat.frame.i_tex_bits - h->stat.frame.i_mv_bits;

  return 0;
}

static inline int
xavs_slices_write (xavs_t * h)
{
  int i_frame_size = 0;

#if VISUALIZE
  if (h->param.b_visualize)
    xavs_visualize_init (h);
#endif

  if (!h->param.b_sliced_threads)
  {
    //xavs_ratecontrol_threads_start( h );
    if (h->i_thread_frames > 1) {
      if (xavs_pthread_create(&h->thread_handle, NULL, (void *)xavs_slice_write, (void *)h))
        return -1;
      h->b_thread_active = 1;
    }
    else {
      xavs_slice_write (h);
      i_frame_size = h->out.nal[h->out.i_nal - 1].i_payload;
    }
  }
  else
  {
    int i_nal = h->out.i_nal;
    int i_bs_size = h->out.i_bitstream / h->param.i_threads;
    int i;
    /* duplicate contexts */
    for (i = 0; i < h->param.i_threads; i++)
    {
      xavs_t *t = h->thread[i];
      if (i > 0)
      {
        memcpy (t, h, sizeof (xavs_t));
        t->out.p_bitstream += i * i_bs_size;
        bs_init (&t->out.bs, t->out.p_bitstream, i_bs_size);
        t->i_thread_num = i;
      }
      t->sh.i_first_mb = (i * h->sps->i_mb_height / h->param.i_threads) * h->sps->i_mb_width;
      t->sh.i_last_mb = ((i + 1) * h->sps->i_mb_height / h->param.i_threads) * h->sps->i_mb_width;
      t->sh.i_slice_vertical_position=i * h->sps->i_mb_height / h->param.i_threads;
      t->out.i_nal = i_nal + i;
    }
    //xavs_ratecontrol_threads_start( h );

    /* dispatch */
#ifdef HAVE_PTHREAD
    {
      pthread_t handles[XAVS_THREAD_MAX];
      for (i = 0; i < h->param.i_threads; i++)
        pthread_create (&handles[i], NULL, (void *) xavs_slice_write, (void *) h->thread[i]);
      for (i = 0; i < h->param.i_threads; i++)
        pthread_join (handles[i], NULL);
    }
#else
    for (i = 0; i < h->param.i_threads; i++)
      xavs_slice_write (h->thread[i]);
#endif

    /* merge contexts */
    i_frame_size = h->out.nal[i_nal].i_payload;
    for (i = 1; i < h->param.i_threads; i++)
    {
      int j;
      xavs_t *t = h->thread[i];
      h->out.nal[i_nal + i] = t->out.nal[i_nal + i];
      i_frame_size += t->out.nal[i_nal + i].i_payload;
      // all entries in stat.frame are ints
      for (j = 0; j < sizeof (h->stat.frame) / sizeof (int); j++)
        ((int *) &h->stat.frame)[j] += ((int *) &t->stat.frame)[j];
    }
    h->out.i_nal = i_nal + h->param.i_threads;
  }

#if VISUALIZE
  if (h->param.b_visualize)
  {
    xavs_visualize_show (h);
    xavs_visualize_close (h);
  }
#endif

  return i_frame_size;
}

/****************************************************************************
 *  XXX: i_poc   : is the poc of the current given picture
 *       i_frame : is the number of the frame being coded
 *  ex:  type frame poc
 *       I      0   2*0
 *       P      1   2*3
 *       B      2   2*1
 *       B      3   2*2
 *       P      4   2*6
 *       B      5   2*4
 *       B      6   2*5
 ****************************************************************************/
int
xavs_encoder_encode (xavs_t * h, xavs_nal_t ** pp_nal, int *pi_nal, 
                     xavs_picture_t * pic_in, xavs_picture_t * pic_out)
{
  xavs_t *thread_current, *thread_prev, *thread_oldest;
  xavs_frame_t *frame_psnr;   /* just to keep the current decoded frame for psnr calculation */
  int i_nal_type;
  int i_nal_ref_idc;
  int i_slice_type;
  int i_frame_size;

  int i;

  int i_global_qp;

  char psz_message[80];

  if (h->i_thread_frames > 1 && !h->param.b_sliced_threads)
  {
    thread_prev     = h->thread[h->i_thread_phase];
    h->i_thread_phase = (h->i_thread_phase + 1) % h->i_thread_frames;
    thread_current  = h->thread[h->i_thread_phase];
    thread_oldest   = h->thread[(h->i_thread_phase + 1) % h->i_thread_frames];
    xavs_thread_sync_context(thread_current, thread_prev);
    xavs_thread_sync_ratecontrol(thread_current, thread_prev, thread_oldest);

    h = thread_current;
  } 
  else 
  {
    thread_current = thread_oldest = h;
  }

  if (xavs_update_reference_list(h)) {
    xavs_log (h, XAVS_LOG_DEBUG, "xavs_update_reference_list failed\n");
    return -1;
  }

  h->fdec->i_lines_completed = -1;

  /* no data out */
  *pi_nal = 0;
  *pp_nal = NULL;

  /* ------------------- Setup new frame from picture -------------------- */
  TIMER_START (i_mtime_encode_frame);
  if (pic_in != NULL)
  {
    /* 1: Copy the picture to a frame and move it to a buffer */
    xavs_frame_t *fenc = xavs_frame_get_unused(h);
    if (fenc == NULL)
        return -1;

    fenc->b_last_minigop_bframe = 0;
    xavs_frame_copy_picture (h, fenc, pic_in);

    if (h->param.i_width % 16 || h->param.i_height % 16)
      xavs_frame_expand_border_mod16( h, fenc );

    fenc->i_frame = h->frames.i_input++;

    if( h->frames.b_have_lowres )
       xavs_frame_init_lowres(h->param.cpu, fenc);

    /* 2: Place the frame into the queue for its slice type decision */
    xavs_lookahead_put_frame( h, fenc );

    if( h->frames.i_input <= h->frames.i_delay - h->i_thread_frames )
    {
      /* Nothing yet to encode, waiting for filling of buffers */
      pic_out->i_type = XAVS_TYPE_AUTO;
      return 0;
    }
  }
  else
  {
    /* signal kills for lookahead thread */
    xavs_pthread_mutex_lock( &h->lookahead->ifbuf.mutex );
    h->lookahead->b_exit_thread = 1;
    xavs_pthread_cond_broadcast( &h->lookahead->ifbuf.cv_fill );
    xavs_pthread_mutex_unlock( &h->lookahead->ifbuf.mutex );
  }

  /* 3: The picture is analyzed in the lookahead */
  if (h->frames.current[0] == NULL)
  {
      xavs_lookahead_get_frames( h );
  }

  if (h->frames.current[0] == NULL && xavs_lookahead_is_empty( h ) ) 
  {
      goto encoder_frame_end;
  }
  TIMER_STOP (i_mtime_encode_frame);

  /* ------------------- Get frame to be encoded ------------------------- */
  /* 4: get picture to encode */
  h->fenc = xavs_frame_get (h->frames.current);
  if (h->fenc == NULL)
  {
    /* Nothing yet to encode (ex: waiting for I/P with B frames) */
    /* waiting for filling bframe buffer */
    pic_out->i_type = XAVS_TYPE_AUTO;
    xavs_log (h, XAVS_LOG_DEBUG, "xavs_frame_get failed\n");
    return -1;
  }

do_encode:

  if (h->fenc->i_type == XAVS_TYPE_IDR)
  {
    h->frames.i_last_idr = h->fenc->i_frame;
    h->i_frame_num = 0;
  }

  /* ------------------- Setup frame context ----------------------------- */
  /* 5: Init data dependant of frame type */
  TIMER_START (i_mtime_encode_frame);
  if (h->fenc->i_type == XAVS_TYPE_IDR)
  {
    /* reset ref pictures */
    xavs_reference_reset (h);

    i_nal_type = NAL_SLICE_IDR;
    i_nal_ref_idc = NAL_PRIORITY_HIGHEST;
    i_slice_type = SLICE_TYPE_I;
  }
  else if (h->fenc->i_type == XAVS_TYPE_I)
  {
    i_nal_type = NAL_SLICE;
    i_nal_ref_idc = NAL_PRIORITY_HIGH;  /* Not completely true but for now it is (as all I/P are kept as ref) */
    i_slice_type = SLICE_TYPE_I;
  }
  else if (h->fenc->i_type == XAVS_TYPE_P)
  {
    i_nal_type = NAL_SLICE;
    i_nal_ref_idc = NAL_PRIORITY_HIGH;  /* Not completely true but for now it is (as all I/P are kept as ref) */
    i_slice_type = SLICE_TYPE_P;
  }
  else                          /* B frame */
  {
    i_nal_type = NAL_SLICE;
    i_nal_ref_idc = NAL_PRIORITY_DISPOSABLE;
    i_slice_type = SLICE_TYPE_B;
  }

  h->fdec->i_poc = h->fenc->i_poc = 2 * (h->fenc->i_frame - h->frames.i_last_idr);
  h->fdec->i_type = h->fenc->i_type;
  h->fdec->i_frame = h->fenc->i_frame;

  if (i_slice_type != SLICE_TYPE_B)
    h->fenc->b_kept_as_ref = h->fdec->b_kept_as_ref = 1;
  else
    h->fenc->b_kept_as_ref = h->fdec->b_kept_as_ref = 0;

  /* ------------------- Init                ----------------------------- */
  /* build ref list 0/1 */
  xavs_reference_build_list (h, h->fdec->i_poc, i_slice_type);

  /* Init the rate control */
  xavs_ratecontrol_start (h, i_slice_type, h->fenc->i_qpplus1);
  i_global_qp = xavs_ratecontrol_qp (h);

  pic_out->i_qpplus1 = h->fdec->i_qpplus1 = i_global_qp + 1;

  if (i_slice_type == SLICE_TYPE_B)
    xavs_macroblock_bipred_init (h);

  if (h->fenc->b_kept_as_ref)
    h->i_frame_num++;

  /* ------------------------ Create picture header  ----------------------- */
  if (i_slice_type == SLICE_TYPE_I)
  {
    xavs_i_pic_header_init (h, &h->ih, i_global_qp);
    h->sh.b_picture_fixed_qp = h->ih.b_fixed_picture_qp;
  }
  else
  {
    xavs_pb_pic_header_init (h, &h->pbh, i_global_qp, i_slice_type);
    h->sh.b_picture_fixed_qp = h->pbh.b_fixed_picture_qp;
  }

  /* ------------------------ Create slice header  ----------------------- */
  xavs_slice_init (h, /*i_nal_type, */ i_slice_type, i_global_qp);


  /* ---------------------- Write the bitstream -------------------------- */
  /* Init bitstream context */
  h->out.i_nal = 0;
  bs_init (&h->out.bs, h->out.p_bitstream, h->out.i_bitstream);

  h->i_nal_type = i_nal_type;
  h->i_nal_ref_idc = i_nal_ref_idc;

  if (i_nal_type == NAL_SLICE_IDR && h->param.b_repeat_headers)
  {
    /* generate sequence parameters */
    xavs_nal_start (h, NAL_SPS, NAL_PRIORITY_HIGHEST);

    xavs_sequence_write (&h->out.bs, &h->sqh);
    xavs_nal_end (h);
  }

  /* Write frame */
  xavs_nal_start (h, NAL_PPS, NAL_PRIORITY_HIGHEST);
  if (i_slice_type == SLICE_TYPE_I)
  {
    xavs_i_picture_write (&h->out.bs, &h->ih, &h->sqh);
  }
  else                          //pb
  {
    xavs_pb_picture_write (&h->out.bs, &h->pbh, &h->sqh);
  }
  xavs_nal_end (h);

  /* Write frame */
  xavs_slices_write (h);

encoder_frame_end:
  h = thread_oldest;
  if( h->b_thread_active )
  {
      int ret = 0;
      xavs_pthread_join( h->thread_handle, (void *)&ret );
      h->b_thread_active = 0;
      if (ret)
          return ret;
  }
  if( !h->out.i_nal )
  {
      pic_out->i_type = XAVS_TYPE_AUTO;
      return 0;
  }

  /* restore CPU state (before using float again) */
  xavs_cpu_restore (h->param.cpu);

  i_frame_size = xavs_encoder_get_frame_size(h, 0);
  i_slice_type = h->sh.i_type;
  i_nal_type = h->i_nal_type;
  i_nal_ref_idc = h->i_nal_ref_idc;
  if (i_slice_type == SLICE_TYPE_P && !h->param.rc.b_stat_read && h->param.i_scenecut_threshold >= 0)
  {
    const int *mbs = h->stat.frame.i_mb_count;
    int i_mb_i = mbs[I_16x16] + mbs[I_8x8] + mbs[I_4x4];
    int i_mb_p = mbs[P_L0] + mbs[P_8x8];
    int i_mb_s = mbs[P_SKIP];
    int i_mb = h->sps->i_mb_width * h->sps->i_mb_height;
    int64_t i_inter_cost = h->stat.frame.i_inter_cost;
    int64_t i_intra_cost = h->stat.frame.i_intra_cost;

    float f_bias;
    int i_gop_size = h->fenc->i_frame - h->frames.i_last_idr;
    float f_thresh_max = (float) ((float) h->param.i_scenecut_threshold / 100.0);
    /* magic numbers pulled out of thin air */
    float f_thresh_min = f_thresh_max * h->param.i_keyint_min / (h->param.i_keyint_max * 4);
    if (h->param.i_keyint_min == h->param.i_keyint_max)
      f_thresh_min = f_thresh_max;

    /* macroblock_analyse() doesn't further analyse skipped mbs,
     * so we have to guess their cost */
    if (i_mb_s < i_mb)
      i_intra_cost = i_intra_cost * i_mb / (i_mb - i_mb_s);

    if (i_gop_size < h->param.i_keyint_min / 4)
      f_bias = f_thresh_min / 4;
    else if (i_gop_size <= h->param.i_keyint_min)
      f_bias = f_thresh_min * i_gop_size / h->param.i_keyint_min;
    else
    {
      f_bias = f_thresh_min + (f_thresh_max - f_thresh_min) * (i_gop_size - h->param.i_keyint_min) / (h->param.i_keyint_max - h->param.i_keyint_min);
    }
    f_bias = (float) XAVS_MIN (f_bias, 1.0);

    /* Bad P will be reencoded as I */
    if (i_mb_s < i_mb && i_inter_cost >= (1.0 - f_bias) * i_intra_cost
        && !h->param.i_sync_lookahead)
    {
      int b;

      xavs_log(h, XAVS_LOG_DEBUG, "scene cut at %d Icost:%.0f Pcost:%.0f ratio:%.3f bias=%.3f lastIDR:%d (I:%d P:%d S:%d)\n", h->fenc->i_frame, (double) i_intra_cost, (double) i_inter_cost, (double) i_inter_cost / i_intra_cost, f_bias, i_gop_size, i_mb_i, i_mb_p, i_mb_s);

      /* Restore frame num */
      h->i_frame_num--;

      for (b = 0; h->frames.current[b] && IS_XAVS_TYPE_B (h->frames.current[b]->i_type); b++);
      if (b > 0)
      {
        /* If using B-frames, force GOP to be closed.
         * Even if this frame is going to be I and not IDR, forcing a
         * P-frame before the scenecut will probably help compression.
         * 
         * We don't yet know exactly which frame is the scene cut, so
         * we can't assign an I-frame. Instead, change the previous
         * B-frame to P, and rearrange coding order. */

        if (h->param.i_bframe_adaptive || b > 1)
          h->fenc->i_type = XAVS_TYPE_AUTO;
        xavs_frame_sort_pts (h->frames.current);
        xavs_lookahead_put_frame(h, h->fenc);
        h->fenc = h->frames.current[b - 1];
        h->frames.current[b - 1] = NULL;
        h->fenc->i_type = XAVS_TYPE_P;
        xavs_frame_sort_dts (h->frames.current);
      }
      /* Do IDR if needed */
      else if (i_gop_size >= h->param.i_keyint_min)
      {
        xavs_frame_t *tmp;

        /* Reset */
        h->i_frame_num = 0;

        /* Reinit field of fenc */
        h->fenc->i_type = XAVS_TYPE_IDR;
        h->fenc->i_poc = 0;

        /* Put enqueued frames back in the pool */
        while ((tmp = xavs_frame_get (h->frames.current)) != NULL)
          xavs_lookahead_put_frame(h, tmp);
        xavs_frame_sort_pts (h->frames.next);
      }
      else
      {
        h->fenc->i_type = XAVS_TYPE_I;
      }
      goto do_encode;
    }
  }

  /* End bitstream, set output  */
  *pi_nal = h->out.i_nal;
  *pp_nal = h->out.nal;
  h->out.i_nal = 0;

  /* Set output picture properties */
  if (i_slice_type == SLICE_TYPE_I)
    pic_out->i_type = i_nal_type == NAL_SLICE_IDR ? XAVS_TYPE_IDR : XAVS_TYPE_I;
  else if (i_slice_type == SLICE_TYPE_P)
    pic_out->i_type = XAVS_TYPE_P;
  else
    pic_out->i_type = XAVS_TYPE_B;
  pic_out->i_pts = h->fenc->i_pts;

  pic_out->img.i_plane = h->fdec->i_plane;
  for (i = 0; i < 3; i++)
  {
    pic_out->img.i_stride[i] = h->fdec->i_stride[i];
    pic_out->img.plane[i] = h->fdec->plane[i];
  }

  /* ---------------------- Update encoder state ------------------------- */

  /* update rc */
  xavs_cpu_restore (h->param.cpu);
  xavs_ratecontrol_end (h, i_frame_size * 8);

  /* apply deblocking filter to the current decoded picture */
  if (!h->sh.i_disable_deblocking_filter_idc)
  {
    TIMER_START (i_mtime_filter);
    xavs_frame_deblocking_filter (h, h->sh.i_type);
    TIMER_STOP (i_mtime_filter);
  }

  /* Dump reconstructed frame */
  if (h->param.psz_dump_yuv)
    xavs_frame_dump (h);

  /* handle references */
  if (i_nal_ref_idc != NAL_PRIORITY_DISPOSABLE)
    xavs_reference_update (h);

  /* increase frame count */
  h->i_frame++;

  /* restore CPU state (before using float again) */
  xavs_cpu_restore (h->param.cpu);

  xavs_noise_reduction_update (h);

  TIMER_STOP (i_mtime_encode_frame);

  /* ---------------------- Compute/Print statistics --------------------- */
  xavs_thread_sync_stat(h, h->thread[0]);

  /* Slice stat */
  h->stat.i_slice_count[i_slice_type]++;
  h->stat.i_slice_size[i_slice_type] += i_frame_size + NALU_OVERHEAD;
  h->stat.i_slice_qp[i_slice_type] += h->fdec->f_qp_avg_aq;

  for (i = 0; i < 19; i++)
    h->stat.i_mb_count[h->sh.i_type][i] += h->stat.frame.i_mb_count[i];
  for (i = 0; i < 2; i++)
    h->stat.i_mb_count_8x8dct[i] += h->stat.frame.i_mb_count_8x8dct[i];
  if (h->sh.i_type != SLICE_TYPE_I)
  {
    for (i = 0; i < 7; i++)
      h->stat.i_mb_count_size[h->sh.i_type][i] += h->stat.frame.i_mb_count_size[i];
    for (i = 0; i < 16; i++)
      h->stat.i_mb_count_ref[h->sh.i_type][i] += h->stat.frame.i_mb_count_ref[i];
  }
  if (i_slice_type == SLICE_TYPE_B)
  {
    h->stat.i_direct_frames[h->sh.b_direct_spatial_mv_pred]++;
    if (h->mb.b_direct_auto_write)
    {
      if (h->stat.i_direct_score[0] + h->stat.i_direct_score[1] > h->mb.i_mb_count)
      {
        for (i = 0; i < 2; i++)
          h->stat.i_direct_score[i] = h->stat.i_direct_score[i] * 9 / 10;
      }
      for (i = 0; i < 2; i++)
        h->stat.i_direct_score[i] += h->stat.frame.i_direct_score[i];
    }
  }


  if (h->param.analyse.b_psnr)
  {
    int64_t i_sqe_y, i_sqe_u, i_sqe_v;
    frame_psnr = h->fdec;

    /* PSNR */
    i_sqe_y = xavs_pixel_ssd_wxh (&h->pixf, frame_psnr->plane[0],frame_psnr->i_stride[0],
		                            h->fenc->plane[0], h->fenc->i_stride[0],
					    h->param.i_width, h->param.i_height);
    i_sqe_u = xavs_pixel_ssd_wxh (&h->pixf, frame_psnr->plane[1], frame_psnr->i_stride[1],
		                            h->fenc->plane[1], h->fenc->i_stride[1],
					    h->param.i_width / 2, h->param.i_height / 2);
    i_sqe_v = xavs_pixel_ssd_wxh (&h->pixf, frame_psnr->plane[2], frame_psnr->i_stride[2],
		                            h->fenc->plane[2], h->fenc->i_stride[2],
					    h->param.i_width / 2, h->param.i_height / 2);
    xavs_cpu_restore (h->param.cpu);

    h->stat.i_sqe_global[i_slice_type] += i_sqe_y + i_sqe_u + i_sqe_v;
    h->stat.f_psnr_average[i_slice_type] += xavs_psnr (i_sqe_y + i_sqe_u + i_sqe_v, 3 * h->param.i_width * h->param.i_height / 2);
    h->stat.f_psnr_mean_y[i_slice_type] += xavs_psnr (i_sqe_y, h->param.i_width * h->param.i_height);
    h->stat.f_psnr_mean_u[i_slice_type] += xavs_psnr (i_sqe_u, h->param.i_width * h->param.i_height / 4);
    h->stat.f_psnr_mean_v[i_slice_type] += xavs_psnr (i_sqe_v, h->param.i_width * h->param.i_height / 4);

    snprintf (psz_message, 80, " PSNR Y:%2.2f U:%2.2f V:%2.2f",
              xavs_psnr (i_sqe_y, h->param.i_width * h->param.i_height),
	      xavs_psnr (i_sqe_u, h->param.i_width * h->param.i_height / 4),
	      xavs_psnr (i_sqe_v, h->param.i_width * h->param.i_height / 4));
    psz_message[79] = '\0';
  }
  else
  {
    psz_message[0] = '\0';
  }

  xavs_thread_sync_stat(h->thread[0], h);
  xavs_thread_sync_stat(thread_current, h);

  xavs_log (h, XAVS_LOG_DEBUG,
                "frame=%4d QP=%i NAL=%d Slice:%c Poc:%-3d I:%-4d P:%-4d SKIP:%-4d size=%d bytes%s\n",
            h->i_frame - 1, 
	    h->fdec->f_qp_avg_aq, 
	    i_nal_ref_idc, 
	    i_slice_type == SLICE_TYPE_I ? 'I' : (i_slice_type == SLICE_TYPE_P ? 'P' : 'B'), 
	    h->fdec->i_poc, 
	    h->stat.frame.i_mb_count_i, 
	    h->stat.frame.i_mb_count_p, 
	    h->stat.frame.i_mb_count_skip, 
	    i_frame_size, 
	    psz_message);

  xavs_frame_put_unused(h, h->fenc);

#ifdef DEBUG_MB_TYPE
  {
    static const char mb_chars[] = { 'i', 'i', 'I', 'C', 'P', '8', 'S',
      'D', '<', 'X', 'B', 'X', '>', 'B', 'B', 'B', 'B', '8', 'S'
    };
    int mb_xy;
    for (mb_xy = 0; mb_xy < h->sps->i_mb_width * h->sps->i_mb_height; mb_xy++)
    {
      if (h->mb.type[mb_xy] < 19 && h->mb.type[mb_xy] >= 0)
        fprintf (stderr, "%c ", mb_chars[h->mb.type[mb_xy]]);
      else
        fprintf (stderr, "? ");

      if ((mb_xy + 1) % h->sps->i_mb_width == 0)
        fprintf (stderr, "\n");
    }
  }
#endif

  return 0;
}

/****************************************************************************
 * xavs_encoder_close:
 ****************************************************************************/
void
xavs_encoder_close (xavs_t * h)
{
#ifdef DEBUG_BENCHMARK
  int64_t i_mtime_total = i_mtime_analyse + i_mtime_encode + i_mtime_write + i_mtime_filter + 1;
#endif
  int64_t i_yuv_size = 3 * h->param.i_width * h->param.i_height / 2;
  int i;

  xavs_lookahead_delete(h);

#ifdef DEBUG_BENCHMARK
  xavs_log (h, XAVS_LOG_INFO,
            "analyse=%d(%lldms) encode=%d(%lldms) write=%d(%lldms) filter=%d(%lldms)\n",
            (int) (100 * i_mtime_analyse / i_mtime_total), i_mtime_analyse / 1000,
            (int) (100 * i_mtime_encode / i_mtime_total), i_mtime_encode / 1000, (int) (100 * i_mtime_write / i_mtime_total), i_mtime_write / 1000, (int) (100 * i_mtime_filter / i_mtime_total), i_mtime_filter / 1000);
#endif

  /* Slices used and PSNR */
  for (i = 0; i < 5; i++)
  {
    static const int slice_order[] = { SLICE_TYPE_I, SLICE_TYPE_SI, SLICE_TYPE_P, SLICE_TYPE_SP, SLICE_TYPE_B };
    static const char *slice_name[] = { "P", "B", "I", "SP", "SI" };
    int i_slice = slice_order[i];

    if (h->stat.i_slice_count[i_slice] > 0)
    {
      const int i_count = h->stat.i_slice_count[i_slice];
      if (h->param.analyse.b_psnr)
      {
        xavs_log (h, XAVS_LOG_INFO,
                  "slice %s:%-5d Avg QP:%5.2f  size:%6.0f  PSNR Mean Y:%5.2f U:%5.2f V:%5.2f Avg:%5.2f Global:%5.2f\n",
                  slice_name[i_slice],
                  i_count,
                  (double) h->stat.i_slice_qp[i_slice] / i_count,
                  (double) h->stat.i_slice_size[i_slice] / i_count,
                  h->stat.f_psnr_mean_y[i_slice] / i_count,
		  h->stat.f_psnr_mean_u[i_slice] / i_count,
		  h->stat.f_psnr_mean_v[i_slice] / i_count,
		  h->stat.f_psnr_average[i_slice] / i_count,
		  xavs_psnr (h->stat.i_sqe_global[i_slice],
                  i_count * i_yuv_size));
      }
      else
      {
        xavs_log (h, XAVS_LOG_INFO, 
                  "slice %s:%-5d Avg QP:%5.2f  size:%6.0f\n",
                  slice_name[i_slice],
		  i_count,
		  (double) h->stat.i_slice_qp[i_slice] / i_count,
		  (double) h->stat.i_slice_size[i_slice] / i_count);
      }
    }
  }

  /* MB types used */
  if (h->stat.i_slice_count[SLICE_TYPE_I] > 0)
  {
    const int64_t *i_mb_count = h->stat.i_mb_count[SLICE_TYPE_I];
    const double i_count = h->stat.i_slice_count[SLICE_TYPE_I] * h->mb.i_mb_count / 100.0;
    xavs_log (h, XAVS_LOG_INFO, "mb I  I16..4: %4.1f%% %4.1f%% %4.1f%%\n", i_mb_count[I_16x16] / i_count, i_mb_count[I_8x8] / i_count, i_mb_count[I_4x4] / i_count);
  }
  if (h->stat.i_slice_count[SLICE_TYPE_P] > 0)
  {
    const int64_t *i_mb_count = h->stat.i_mb_count[SLICE_TYPE_P];
    const int64_t *i_mb_size = h->stat.i_mb_count_size[SLICE_TYPE_P];
    const double i_count = h->stat.i_slice_count[SLICE_TYPE_P] * h->mb.i_mb_count / 100.0;
    xavs_log (h, XAVS_LOG_INFO,
              "mb P  I16..4: %4.1f%% %4.1f%% %4.1f%%  P16..4: %4.1f%% %4.1f%% %4.1f%% %4.1f%% %4.1f%%    skip:%4.1f%%\n",
              i_mb_count[I_16x16] / i_count,
              i_mb_count[I_8x8] / i_count,
              i_mb_count[I_4x4] / i_count,
              i_mb_size[PIXEL_16x16] / (i_count * 4),
              (i_mb_size[PIXEL_16x8] + i_mb_size[PIXEL_8x16]) / (i_count * 4), i_mb_size[PIXEL_8x8] / (i_count * 4), (i_mb_size[PIXEL_8x4] + i_mb_size[PIXEL_4x8]) / (i_count * 4), i_mb_size[PIXEL_4x4] / (i_count * 4), i_mb_count[P_SKIP] / i_count);
  }
  if (h->stat.i_slice_count[SLICE_TYPE_B] > 0)
  {
    const int64_t *i_mb_count = h->stat.i_mb_count[SLICE_TYPE_B];
    const int64_t *i_mb_size = h->stat.i_mb_count_size[SLICE_TYPE_B];
    const double i_count = h->stat.i_slice_count[SLICE_TYPE_B] * h->mb.i_mb_count / 100.0;
    xavs_log (h, XAVS_LOG_INFO,
              "mb B  I16..4: %4.1f%% %4.1f%% %4.1f%%  B16..8: %4.1f%% %4.1f%% %4.1f%%  direct:%4.1f%%  skip:%4.1f%%\n",
              i_mb_count[I_16x16] / i_count,
              i_mb_count[I_8x8] / i_count, i_mb_count[I_4x4] / i_count, i_mb_size[PIXEL_16x16] / (i_count * 4), (i_mb_size[PIXEL_16x8] + i_mb_size[PIXEL_8x16]) / (i_count * 4), i_mb_size[PIXEL_8x8] / (i_count * 4), i_mb_count[B_DIRECT] / i_count, i_mb_count[B_SKIP] / i_count);
  }

  xavs_ratecontrol_summary (h);

  if (h->stat.i_slice_count[SLICE_TYPE_I] + h->stat.i_slice_count[SLICE_TYPE_P] + h->stat.i_slice_count[SLICE_TYPE_B] > 0)
  {
    const int i_count = h->stat.i_slice_count[SLICE_TYPE_I] + h->stat.i_slice_count[SLICE_TYPE_P] + h->stat.i_slice_count[SLICE_TYPE_B];
    float fps = (float) h->param.i_fps_num / h->param.i_fps_den;
#define SUM3(p) (p[SLICE_TYPE_I] + p[SLICE_TYPE_P] + p[SLICE_TYPE_B])
#define SUM3b(p,o) (p[SLICE_TYPE_I][o] + p[SLICE_TYPE_P][o] + p[SLICE_TYPE_B][o])
    float f_bitrate = fps * SUM3 (h->stat.i_slice_size) / i_count / 125;

    if (h->param.analyse.b_transform_8x8)
    {
      int64_t i_i8x8 = SUM3b (h->stat.i_mb_count, I_8x8);
      int64_t i_intra = i_i8x8 + SUM3b (h->stat.i_mb_count, I_4x4) + SUM3b (h->stat.i_mb_count, I_16x16);
      xavs_log (h, XAVS_LOG_INFO, "8x8 transform  intra:%.1f%%  inter:%.1f%%\n", 100. * i_i8x8 / i_intra, 100. * h->stat.i_mb_count_8x8dct[1] / h->stat.i_mb_count_8x8dct[0]);
    }

    if (h->param.analyse.i_direct_mv_pred == XAVS_DIRECT_PRED_AUTO && h->stat.i_slice_count[SLICE_TYPE_B])
    {
      xavs_log (h, XAVS_LOG_INFO, "direct mvs  spatial:%.1f%%  temporal:%.1f%%\n", h->stat.i_direct_frames[1] * 100. / h->stat.i_slice_count[SLICE_TYPE_B], h->stat.i_direct_frames[0] * 100. / h->stat.i_slice_count[SLICE_TYPE_B]);
    }

    if (h->param.i_frame_reference > 1)
    {
      int i_slice;
      for (i_slice = 0; i_slice < 2; i_slice++)
      {
        char buf[200];
        char *p = buf;
        int64_t i_den = 0;
        int i_max = 0;
        for (i = 0; i < h->param.i_frame_reference; i++)
          if (h->stat.i_mb_count_ref[i_slice][i])
          {
            i_den += h->stat.i_mb_count_ref[i_slice][i];
            i_max = i;
          }
        if (i_max == 0)
          continue;
        for (i = 0; i <= i_max; i++)
          p += sprintf (p, " %4.1f%%", 100. * h->stat.i_mb_count_ref[i_slice][i] / i_den);
        xavs_log (h, XAVS_LOG_INFO, "ref %c %s\n", i_slice == SLICE_TYPE_P ? 'P' : 'B', buf);
      }
    }

    if (h->param.analyse.b_psnr)
    {
      xavs_log (h, XAVS_LOG_INFO,
                "PSNR Mean Y:%6.3f U:%6.3f V:%6.3f Avg:%6.3f Global:%6.3f kb/s:%.2f\n",
                SUM3 (h->stat.f_psnr_mean_y) / i_count,
                SUM3 (h->stat.f_psnr_mean_u) / i_count,
                SUM3 (h->stat.f_psnr_mean_v) / i_count,
                SUM3 (h->stat.f_psnr_average) / i_count,
		xavs_psnr (SUM3 (h->stat.i_sqe_global), i_count * i_yuv_size),
		f_bitrate);
    }      
    else
      xavs_log (h, XAVS_LOG_INFO, "kb/s:%.1f\n", f_bitrate);
  }

  /* frames */
  for (i = 0; i < XAVS_BFRAME_MAX + 3; i++)
  {
    if (h->frames.current[i])
      xavs_frame_delete (h->frames.current[i]);
    if (h->frames.next[i])
      xavs_frame_delete (h->frames.next[i]);
    if (h->frames.unused[i])
      xavs_frame_delete (h->frames.unused[i]);
  }
  /* ref frames */
  for (i = 0; i < h->frames.i_max_dpb; i++)
  {
    xavs_frame_delete (h->frames.reference[i]);
  }

  /* rc */
  xavs_ratecontrol_delete (h);

  /* param */
  if (h->param.rc.psz_stat_out)
    free (h->param.rc.psz_stat_out);
  if (h->param.rc.psz_stat_in)
    free (h->param.rc.psz_stat_in);

  xavs_free (h->out.p_bitstream);
  for (i = 1; i < h->param.i_threads; i++)
  { 
    if (!h->param.b_sliced_threads)
    {
      xavs_macroblock_cache_end (h->thread[i]);
      xavs_free (h->thread[i]->out.p_bitstream);
    }
    xavs_free (h->thread[i]);
  }
  xavs_free (h);
}

/****************************************************************************
 * xavs_encoder_delayed_frames:
 ****************************************************************************/
int
xavs_encoder_delayed_frames (xavs_t * h)
{
  int delayed_frames = 0;
  int i;
  for (i = 0; i < h->param.i_threads; i++)
    delayed_frames += h->thread[i]->b_thread_active;
  h = h->thread[h->i_thread_phase % h->param.i_threads];
  for (i = 0; h->frames.current[i]; i++)
    delayed_frames++;
  xavs_pthread_mutex_lock (&h->lookahead->ofbuf.mutex);
  xavs_pthread_mutex_lock (&h->lookahead->ifbuf.mutex);
  xavs_pthread_mutex_lock (&h->lookahead->next.mutex);
  delayed_frames += h->lookahead->ifbuf.i_size + h->lookahead->next.i_size + h->lookahead->ofbuf.i_size;
  xavs_pthread_mutex_unlock (&h->lookahead->next.mutex);
  xavs_pthread_mutex_unlock (&h->lookahead->ifbuf.mutex);
  xavs_pthread_mutex_unlock (&h->lookahead->ofbuf.mutex);
  return delayed_frames;
}


static void xavs_thread_sync_context( xavs_t *dst, xavs_t *src )
{
    xavs_frame_t **f;
    if( dst == src )
        return;

    // reference counting
    for( f = src->frames.reference; *f; f++ )
        (*f)->i_reference_count++;
    for( f = dst->frames.reference; *f; f++ )
        xavs_frame_put_unused(src, *f);
    src->fdec->i_reference_count++;
    xavs_frame_put_unused(src, dst->fdec);
    dst->fdec = NULL;
        
    // copy everything except the per-thread pointers and the constants.
    memcpy(&dst->i_frame, 
           &src->i_frame, 
           offsetof(xavs_t, mb.type) - offsetof(xavs_t, i_frame));
    dst->param = src->param;
    dst->stat = src->stat;
}

static void xavs_thread_sync_stat( xavs_t *dst, xavs_t *src )
{
    if( dst == src )
        return;

    memcpy(&dst->stat.i_slice_count, 
           &src->stat.i_slice_count, 
           sizeof(dst->stat) - sizeof(dst->stat.frame));
}

static int xavs_encoder_get_frame_size( xavs_t *h, int start )
{
    int sz = 0;
    for (; start < h->out.i_nal; ++start)
        sz += h->out.nal[start].i_payload;

    return sz;
}


