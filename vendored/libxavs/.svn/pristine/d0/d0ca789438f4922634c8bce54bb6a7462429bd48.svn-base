/*****************************************************************************
 * frame.h: h264 encoder library
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
 * frame.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_FRAME_H_
#define _XAVS_FRAME_H_

//#include <inttypes.h>  //lsp060308

typedef struct
{
  /* */
  int i_poc;
  int i_type;
  int i_qpplus1;
  int64_t i_pts;
  int i_frame;                  /* Presentation frame number */
  int i_frame_num;              /* Coded frame number */
  int b_kept_as_ref;
  int b_last_minigop_bframe;    /* the last b frame of a sequence of b frame */
  int i_reference_count;        /* number of threads using this frame */
  int i_bframes;                /* number of bframes following this nonb in coded order */
  float f_qp_avg_rc;            /* QPs as decided by ratecontrol */
  float f_qp_avg_aq;            /* QPs as decided by AQ in addition to ratecontrol */

  /* YUV buffer */
  int i_plane;
  int i_stride[4];
  int i_lines[4];
  int i_stride_lowres;
  int i_lines_lowres;
  uint8_t *plane[4];
  uint8_t *filtered[4];         /* plane[0], H, V, HV */
  uint8_t *lowres[4];           /* half-size copy of input frame: Orig, H, V, HV */
  uint16_t *integral;

  /* for unrestricted mv we allocate more data than needed
   * allocated data are stored in buffer */
  void *buffer[12];

  /* motion data */
  int8_t *mb_type;
    int16_t (*mv[2])[2];
  int8_t *ref[2];
  int i_ref[2];
  int ref_poc[2][16];

  /* for adaptive B-frame decision.
   * contains the SATD cost of the lowres frame encoded in various modes
   * FIXME: how big an array do we need? */
  int i_cost_est[XAVS_BFRAME_MAX + 2][XAVS_BFRAME_MAX + 2];
  int i_satd;                   // the i_cost_est of the selected frametype
  int i_intra_mbs[XAVS_BFRAME_MAX + 2];
  int *i_row_satds[XAVS_BFRAME_MAX + 2][XAVS_BFRAME_MAX + 2];
  int *i_row_satd;
  int *i_row_bits;
  int *i_row_qp;
  float *f_qp_offset;

  /* threading */
  int i_lines_completed; /* in pixels */
  int i_lines_weighted; /* FIXME: this only supports weighting of one reference frame */

  xavs_pthread_mutex_t mutex;
  xavs_pthread_cond_t  cv;

} xavs_frame_t;

/* synchronized frame list */
typedef struct
{
  xavs_frame_t **list;
  int i_max_size;
  int i_size;
  xavs_pthread_mutex_t mutex;
  xavs_pthread_cond_t cv_fill;  /* event signaling that the list became fuller */
  xavs_pthread_cond_t cv_empty; /* event signaling that the list became emptier */
} xavs_synch_frame_list_t;


typedef void (*xavs_deblock_inter_t) (uint8_t * pix, int stride, int alpha, int beta, int8_t * tc0);
typedef void (*xavs_deblock_intra_t) (uint8_t * pix, int stride, int alpha, int beta);
typedef struct
{
  xavs_deblock_inter_t deblock_v_luma;
  xavs_deblock_inter_t deblock_h_luma;
  xavs_deblock_inter_t deblock_v_chroma;
  xavs_deblock_inter_t deblock_h_chroma;
  xavs_deblock_intra_t deblock_v_luma_intra;
  xavs_deblock_intra_t deblock_h_luma_intra;
  xavs_deblock_intra_t deblock_v_chroma_intra;
  xavs_deblock_intra_t deblock_h_chroma_intra;
} xavs_deblock_function_t;

xavs_frame_t *xavs_frame_new (xavs_t * h);
void xavs_frame_delete (xavs_frame_t * frame);

void xavs_frame_put (xavs_frame_t ** list, xavs_frame_t * frame);
void xavs_frame_push (xavs_frame_t ** list, xavs_frame_t * frame);
xavs_frame_t * xavs_frame_get (xavs_frame_t ** list);
xavs_frame_t * xavs_frame_get_unused (xavs_t *h);
void xavs_frame_put_unused (xavs_t *h, xavs_frame_t *frame);

void xavs_frame_copy_picture (xavs_t * h, xavs_frame_t * dst, xavs_picture_t * src);

void xavs_frame_expand_border (xavs_frame_t * frame);
void xavs_frame_expand_border_filtered (xavs_frame_t * frame);
void xavs_frame_expand_border_lowres (xavs_frame_t * frame);
void xavs_frame_expand_border_mod16 (xavs_t * h, xavs_frame_t * frame);

void xavs_frame_deblocking_filter (xavs_t * h, int i_slice_type);

void xavs_frame_filter (int cpu, xavs_frame_t * frame);
void xavs_frame_init_lowres (int cpu, xavs_frame_t * frame);

void xavs_deblock_init (int cpu, xavs_deblock_function_t * pf);

int  xavs_synch_frame_list_init( xavs_synch_frame_list_t *slist, int nelem );
void xavs_synch_frame_list_delete( xavs_synch_frame_list_t *slist );
void xavs_synch_frame_list_push( xavs_synch_frame_list_t *slist, xavs_frame_t *frame );

#endif
