/*****************************************************************************
 * common.c: h264 library
 *****************************************************************************
 * Copyright (C) 2003-2008 x264 project
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Laurent Aimar <fenrir@via.ecp.fr>
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
 * common.c: xavs library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "common.h"
#include "cpu.h"

static void xavs_log_default (void *, int, const char *, va_list);

/****************************************************************************
 * xavs_param_default:
 ****************************************************************************/
void
xavs_param_default (xavs_param_t * param)
{
  /* */
  memset (param, 0, sizeof (xavs_param_t));

  /* CPU autodetect */
  param->cpu = xavs_cpu_detect ();
  param->i_threads = 1;
  param->b_sliced_threads = 0;//1 slice parallel, 0 frame parallel

  /* Video properties */
  param->i_csp = XAVS_CSP_I420;
  param->i_width = 0;
  param->i_height = 0;
  param->vui.i_sar_width = 0;
  param->vui.i_sar_height = 0;
  param->vui.i_overscan = 0;    /* undef */
  param->vui.i_vidformat = 5;   /* undef */
  param->vui.b_fullrange = 0;   /* off */
  param->vui.i_colorprim = 2;   /* undef */
  param->vui.i_transfer = 2;    /* undef */
  param->vui.i_colmatrix = 2;   /* undef */
  param->vui.i_chroma_loc = 0;  /* left center */
  param->i_fps_num = 25;
  param->i_fps_den = 1;
  param->i_level_idc = 64;      /* as close to "unrestricted" as we can get */

  /* Encoder parameters */
  param->i_frame_reference = 2;
  param->i_keyint_max = 250;
  param->i_keyint_min = 25;
  param->i_bframe = 4;
  param->i_scenecut_threshold = 40;
  param->i_bframe_adaptive = 1; //enable adaptive B frame
  param->i_bframe_bias = 0;
  param->analyse.b_bframe_rdo = 1;

  param->b_deblocking_filter = 1;
  param->i_deblocking_filter_alphac0 = 0;
  param->i_deblocking_filter_beta = 0;


  param->rc.i_rc_method = 2;    /* 0:XAVS_RC_CQP; 1:XAVS_RC_CRF; 2:XAVS_RC_ABR */
  param->rc.i_bitrate = 750;
  param->rc.f_rate_tolerance = 1.0;
  param->rc.i_vbv_max_bitrate = 1500;
  param->rc.i_vbv_buffer_size = 2000;
  param->rc.f_vbv_buffer_init = 0.9;
  param->rc.i_qp_constant = 26;
  param->rc.f_rf_constant = 26;
  param->rc.i_qp_min = 18;
  param->rc.i_qp_max = 60;
  param->rc.i_qp_step = 4;
  param->rc.f_ip_factor = (float) 1.4;
  param->rc.f_pb_factor = (float) 1.3;
  param->rc.i_aq_mode = XAVS_AQ_VARIANCE;
  param->rc.f_aq_strength = 1.0;
  param->rc.i_lookahead = 0;

  param->rc.b_stat_write = 0;
  param->rc.psz_stat_out = "xavs_2pass.log";
  param->rc.b_stat_read = 0;
  param->rc.psz_stat_in = "xavs_2pass.log";
  param->rc.f_qcompress = (float) 0.6;
  param->rc.f_qblur = (float) 0.5;
  param->rc.f_complexity_blur = 20;
  param->rc.i_zones = 0;

  /* Log */
  param->pf_log = xavs_log_default;
  param->p_log_private = NULL;
  param->i_log_level = XAVS_LOG_INFO;

  /* */
  param->analyse.intra = XAVS_ANALYSE_I8x8;
  param->analyse.inter = XAVS_ANALYSE_I8x8 | XAVS_ANALYSE_PSUB16x16 | XAVS_ANALYSE_BSUB16x16;
  param->analyse.i_direct_mv_pred = XAVS_DIRECT_PRED_SPATIAL;
  param->analyse.i_me_method = XAVS_ME_UMH;
  param->analyse.i_me_range = 32;
  param->analyse.i_subpel_refine = 7;
  param->analyse.b_chroma_me = 1;
  param->analyse.i_mv_range = -1;
  param->analyse.i_chroma_qp_offset = 0;
  param->analyse.b_fast_pskip = 1;
  param->analyse.b_transform_8x8 = 1;
  param->analyse.i_trellis = 1;
  param->analyse.i_luma_deadzone[0] = 21;       /*intra */
  param->analyse.i_luma_deadzone[1] = 11;       /*inter */
  param->analyse.b_psnr = 0;
  param->analyse.b_ssim = 0;
  param->analyse.b_skip_mode = 1;       //run

  param->i_chroma_format = 1;   /* 1: 4:2:0, 2: 4:2:2 */
  param->i_sample_precision = 1;        /* 1: 8 bits per sample */
  param->i_aspect_ratio = 1;    /* '0001':1/1, '0010':4/3, '0011': 16/9, '0100':2.21/ 1 */

  param->i_cqm_preset = XAVS_CQM_FLAT;
  memset (param->cqm_4iy, 16, 16);
  memset (param->cqm_4ic, 16, 16);
  memset (param->cqm_4py, 16, 16);
  memset (param->cqm_4pc, 16, 16);
  memset (param->cqm_8iy, 16, 64);
  memset (param->cqm_8py, 16, 64);

  param->b_repeat_headers = 1;
  param->b_aud = 0;
}

/****************************************************************************
 * xavs_log:
 ****************************************************************************/
void
xavs_log (xavs_t * h, int i_level, const char *psz_fmt, ...)
{
  if (i_level <= h->param.i_log_level)
  {
    va_list arg;
    va_start (arg, psz_fmt);
    h->param.pf_log (h->param.p_log_private, i_level, psz_fmt, arg);
    va_end (arg);
  }
}

static void
xavs_log_default (void *p_unused, int i_level, const char *psz_fmt, va_list arg)
{
  char *psz_prefix;
  switch (i_level)
  {
  case XAVS_LOG_ERROR:
    psz_prefix = "error";
    break;
  case XAVS_LOG_WARNING:
    psz_prefix = "warning";
    break;
  case XAVS_LOG_INFO:
    psz_prefix = "info";
    break;
  case XAVS_LOG_DEBUG:
    psz_prefix = "debug";
    break;
  default:
    psz_prefix = "unknown";
    break;
  }
  fprintf (stderr, "xavs [%s]: ", psz_prefix);
  vfprintf (stderr, psz_fmt, arg);
}

/****************************************************************************
 * xavs_picture_alloc:
 ****************************************************************************/
int
xavs_picture_alloc (xavs_picture_t * pic, int i_csp, int i_width, int i_height)
{
  pic->i_type = XAVS_TYPE_AUTO;
  pic->i_qpplus1 = 0;
  pic->img.i_csp = i_csp;
  switch (i_csp & XAVS_CSP_MASK)
  {
  case XAVS_CSP_I420:
  case XAVS_CSP_YV12:
    pic->img.i_plane = 3;
    pic->img.plane[0] = xavs_malloc (3 * i_width * i_height / 2);
    if (!pic->img.plane[0])
      return -1;
    pic->img.plane[1] = pic->img.plane[0] + i_width * i_height;
    pic->img.plane[2] = pic->img.plane[1] + i_width * i_height / 4;
    pic->img.i_stride[0] = i_width;
    pic->img.i_stride[1] = i_width / 2;
    pic->img.i_stride[2] = i_width / 2;
    break;

  case XAVS_CSP_I422:
    pic->img.i_plane = 3;
    pic->img.plane[0] = xavs_malloc (2 * i_width * i_height);
    if (!pic->img.plane[0])
      return -1;
    pic->img.plane[1] = pic->img.plane[0] + i_width * i_height;
    pic->img.plane[2] = pic->img.plane[1] + i_width * i_height / 2;
    pic->img.i_stride[0] = i_width;
    pic->img.i_stride[1] = i_width / 2;
    pic->img.i_stride[2] = i_width / 2;
    break;

  case XAVS_CSP_I444:
    pic->img.i_plane = 3;
    pic->img.plane[0] = xavs_malloc (3 * i_width * i_height);
    if (!pic->img.plane[0])
      return -1;
    pic->img.plane[1] = pic->img.plane[0] + i_width * i_height;
    pic->img.plane[2] = pic->img.plane[1] + i_width * i_height;
    pic->img.i_stride[0] = i_width;
    pic->img.i_stride[1] = i_width;
    pic->img.i_stride[2] = i_width;
    break;

  case XAVS_CSP_YUYV:
    pic->img.i_plane = 1;
    pic->img.plane[0] = xavs_malloc (2 * i_width * i_height);
    if (!pic->img.plane[0])
      return -1;
    pic->img.i_stride[0] = 2 * i_width;
    break;

  case XAVS_CSP_RGB:
  case XAVS_CSP_BGR:
    pic->img.i_plane = 1;
    pic->img.plane[0] = xavs_malloc (3 * i_width * i_height);
    if (!pic->img.plane[0])
      return -1;
    pic->img.i_stride[0] = 3 * i_width;
    break;

  case XAVS_CSP_BGRA:
    pic->img.i_plane = 1;
    pic->img.plane[0] = xavs_malloc (4 * i_width * i_height);
    if (!pic->img.plane[0])
      return -1;
    pic->img.i_stride[0] = 4 * i_width;
    break;

  default:
    fprintf (stderr, "invalid CSP\n");
    pic->img.i_plane = 0;
    if (!pic->img.plane[0])
      return -1;
  }
  return 0;
}

/****************************************************************************
 * xavs_picture_clean:
 ****************************************************************************/
void
xavs_picture_clean (xavs_picture_t * pic)
{
  xavs_free (pic->img.plane[0]);

  /* just to be safe */
  memset (pic, 0, sizeof (xavs_picture_t));
}

/****************************************************************************
 * xavs_nal_encode:
 ****************************************************************************/
int
xavs_nal_encode (void *p_data, int *pi_data, int b_annexeb, xavs_nal_t * nal)
{
  uint8_t *dst = p_data;
  uint8_t *src = nal->p_payload;
  uint8_t *end = &nal->p_payload[nal->i_payload];

  if (b_annexeb)
  {
    *dst++ = 0x00;
    *dst++ = 0x00;
    *dst++ = 0x01;
  }

  while (src < end)
  {
    *dst++ = *src++;
  }
  *pi_data = (int) (dst - (uint8_t *) p_data);

  return *pi_data;
}

/****************************************************************************
 * xavs_nal_decode:
 ****************************************************************************/
int
xavs_nal_decode (xavs_nal_t * nal, void *p_data, int i_data)
{
  uint8_t *src = p_data;
  uint8_t *end = &src[i_data];
  uint8_t *dst = nal->p_payload;

  nal->i_type = src[0] & 0x1f;
  nal->i_ref_idc = (src[0] >> 5) & 0x03;

  src++;

  while (src < end)
  {
    if (src < end - 3 && src[0] == 0x00 && src[1] == 0x00 && src[2] == 0x03)
    {
      *dst++ = 0x00;
      *dst++ = 0x00;

      src += 3;
      continue;
    }
    *dst++ = *src++;
  }

  nal->i_payload = (int) (dst - (uint8_t *) p_data);
  return 0;
}



/****************************************************************************
 * xavs_malloc:
 ****************************************************************************/
void *
xavs_malloc (int i_size)
{
  uint8_t *align_buf = NULL;
#ifdef SYS_MACOSX
  /* Mac OS X always returns 16 bytes aligned memory */
  align_buf = malloc (i_size);
#elif defined( HAVE_MALLOC_H )
  align_buf = memalign (16, i_size);
#else
  uint8_t *buf = malloc (i_size + 15 + sizeof (void **) + sizeof (int));
  if (buf)
  {
    align_buf = buf + 15 + sizeof (void **) + sizeof (int);
    align_buf -= (intptr_t) align_buf & 15;
    *((void **) (align_buf - sizeof (void **))) = buf;
    *((int *) (align_buf - sizeof (void **) - sizeof (int))) = i_size;
  }
#endif
  if (!align_buf)
    xavs_log (NULL, XAVS_LOG_ERROR, "malloc of size %d failed\n", i_size);
  return align_buf;
}

/****************************************************************************
 * xavs_free:
 ****************************************************************************/
void
xavs_free (void *p)
{
  if (p)
  {
#if defined( HAVE_MALLOC_H ) || defined( SYS_MACOSX )
    free (p);
#else
    free (*(((void **) p) - 1));
#endif
  }
}

/****************************************************************************
 * xavs_realloc:
 ****************************************************************************/
void *
xavs_realloc (void *p, int i_size)
{
#ifdef HAVE_MALLOC_H
  return realloc (p, i_size);
#else
  int i_old_size = 0;
  uint8_t *p_new;
  if (p)
  {
    i_old_size = *((int *) ((uint8_t *) p) - sizeof (void **) - sizeof (int));
  }
  p_new = xavs_malloc (i_size);
  if (i_old_size > 0 && i_size > 0)
  {
    memcpy (p_new, p, (i_old_size < i_size) ? i_old_size : i_size);
  }
  xavs_free (p);
  return p_new;
#endif
}

/****************************************************************************
 * xavs_reduce_fraction:
 ****************************************************************************/
void
xavs_reduce_fraction (int *n, int *d)
{
  int a = *n;
  int b = *d;
  int c;
  if (!a || !b)
    return;
  c = a % b;
  while (c)
  {
    a = b;
    b = c;
    c = a % b;
  }
  *n /= b;
  *d /= b;
}

/****************************************************************************
 * xavs_slurp_file:
 ****************************************************************************/
char *
xavs_slurp_file (const char *filename)
{
  int b_error = 0;
  int i_size;
  char *buf;
  FILE *fh = fopen (filename, "rb");
  if (!fh)
    return NULL;
  b_error |= fseek (fh, 0, SEEK_END) < 0;
  b_error |= (i_size = ftell (fh)) <= 0;
  b_error |= fseek (fh, 0, SEEK_SET) < 0;
  if (b_error)
    return NULL;
  buf = xavs_malloc (i_size + 2);
  if (buf == NULL)
    return NULL;
  b_error |= fread (buf, 1, i_size, fh) != i_size;
  if (buf[i_size - 1] != '\n')
    buf[i_size++] = '\n';
  buf[i_size] = 0;
  fclose (fh);
  if (b_error)
  {
    xavs_free (buf);
    return NULL;
  }
  return buf;
}

/****************************************************************************
 * xavs_param2string:
 ****************************************************************************/
char *
xavs_param2string (xavs_param_t * p, int b_res)
{
  char *buf = xavs_malloc (1000);
  char *s = buf;

  if (b_res)
  {
    s += sprintf (s, "%dx%d ", p->i_width, p->i_height);
    s += sprintf (s, "fps=%d/%d ", p->i_fps_num, p->i_fps_den);
  }

  s += sprintf (s, " ref=%d", p->i_frame_reference);
  s += sprintf (s, " deblock=%d:%d:%d", p->b_deblocking_filter, p->i_deblocking_filter_alphac0, p->i_deblocking_filter_beta);
  s += sprintf (s, " analyse=%#x:%#x", p->analyse.intra, p->analyse.inter);
  s += sprintf (s, " me=%s", xavs_motion_est_names[p->analyse.i_me_method]);
  s += sprintf (s, " subme=%d", p->analyse.i_subpel_refine);
  s += sprintf (s, " brdo=%d", p->analyse.b_bframe_rdo);
  s += sprintf (s, " mixed_ref=%d", p->analyse.b_mixed_references);
  s += sprintf (s, " me_range=%d", p->analyse.i_me_range);
  s += sprintf (s, " chroma_me=%d", p->analyse.b_chroma_me);
  s += sprintf (s, " trellis=%d", p->analyse.i_trellis);
  s += sprintf (s, " 8x8dct=%d", p->analyse.b_transform_8x8);
  s += sprintf (s, " cqm=%d", p->i_cqm_preset);
  s += sprintf (s, " chroma_qp_offset=%d", p->analyse.i_chroma_qp_offset);
  s += sprintf (s, " slices=%d", p->i_threads);
  s += sprintf (s, " nr=%d", p->analyse.i_noise_reduction);

  s += sprintf (s, " bframes=%d", p->i_bframe);
  if (p->i_bframe)
  {
    s += sprintf (s, "  b_adapt=%d b_bias=%d direct=%d wpredb=%d bime=%d", p->i_bframe_adaptive, p->i_bframe_bias, p->analyse.i_direct_mv_pred, p->analyse.b_weighted_bipred, p->analyse.b_bidir_me);
  }

  s += sprintf (s, " keyint=%d keyint_min=%d scenecut=%d", p->i_keyint_max, p->i_keyint_min, p->i_scenecut_threshold);

  s += sprintf (s, " rc=%s", p->rc.b_stat_read && p->rc.i_rc_method ? "2pass" : p->rc.i_rc_method ? p->rc.i_vbv_buffer_size ? "cbr" : "abr" : p->rc.f_rf_constant ? "crf" : "cqp");
  if (p->rc.i_rc_method || p->rc.f_rf_constant)
  {
    if (p->rc.f_rf_constant)
      s += sprintf (s, " crf=%.1f", p->rc.f_rf_constant);
    else
      s += sprintf (s, " bitrate=%d ratetol=%.1f", p->rc.i_bitrate, p->rc.f_rate_tolerance);
    s += sprintf (s, "qcomp=%.2f qpmin=%d qpmax=%d qpstep=%d", p->rc.f_qcompress, p->rc.i_qp_min, p->rc.i_qp_max, p->rc.i_qp_step);
    if (p->rc.b_stat_read)
      s += sprintf (s, " cplxblur=%.1f qblur=%.1f", p->rc.f_complexity_blur, p->rc.f_qblur);
    if (p->rc.i_vbv_buffer_size)
      s += sprintf (s, " vbv_maxrate=%d vbv_bufsize=%d", p->rc.i_vbv_max_bitrate, p->rc.i_vbv_buffer_size);
  }
  else
    s += sprintf (s, " qp=%d", p->rc.i_qp_constant);
  if (p->rc.i_rc_method || p->rc.i_qp_constant != 0)
  {
    s += sprintf (s, " ip_ratio=%.2f", p->rc.f_ip_factor);
    if (p->i_bframe)
      s += sprintf (s, " pb_ratio=%.2f", p->rc.f_pb_factor);
    if (p->rc.i_zones)
      s += sprintf (s, " zones");
  }

  return buf;
}
