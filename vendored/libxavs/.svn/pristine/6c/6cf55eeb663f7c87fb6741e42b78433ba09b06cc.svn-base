/*****************************************************************************
 * set: h264 encoder (SPS and PPS init and write)
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
 * set: xavs encoder (SPS and PPS init and write)
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common/common.h"
#ifndef _MSC_VER
#include "config.h"
#endif

static void
transpose (uint8_t * buf, int w)
{
  int i, j;
  for (i = 0; i < w; i++)
    for (j = 0; j < i; j++)
      XCHG (uint8_t, buf[w * i + j], buf[w * j + i]);
}

static void
scaling_list_write (bs_t * s, xavs_pps_t * pps, int idx)
{
  const int len = idx < 4 ? 16 : 64;
  const int *zigzag = idx < 4 ? xavs_zigzag_scan4 : xavs_zigzag_scan8;
  const uint8_t *list = pps->scaling_list[idx];
  const uint8_t *def_list = (idx == CQM_4IC) ? pps->scaling_list[CQM_4IY] : (idx == CQM_4PC) ? pps->scaling_list[CQM_4PY] : xavs_cqm_jvt[idx];
  if (!memcmp (list, def_list, len))
    bs_write (s, 1, 0);         // scaling_list_present_flag
  else if (!memcmp (list, xavs_cqm_jvt[idx], len))
  {
    bs_write (s, 1, 1);         // scaling_list_present_flag
    bs_write_se (s, -8);        // use jvt list
  }
  else
  {
    int j, run;
    bs_write (s, 1, 1);         // scaling_list_present_flag

    // try run-length compression of trailing values
    for (run = len; run > 1; run--)
      if (list[zigzag[run - 1]] != list[zigzag[run - 2]])
        break;
    if (run < len && len - run < bs_size_se ((int8_t) - list[zigzag[run]]))
      run = len;

    for (j = 0; j < run; j++)
      bs_write_se (s, (int8_t) (list[zigzag[j]] - (j > 0 ? list[zigzag[j - 1]] : 8)));  // delta

    if (run < len)
      bs_write_se (s, (int8_t) - list[zigzag[run]]);
  }
}

void
xavs_sequence_init (xavs_seq_header_t * sqh, xavs_param_t * param)
{
  int i_frame_rate_code;

  sqh->i_video_sequence_start_code = 0xB0;
  sqh->i_profile_idc = 0x20;    //Jizhun profile
  sqh->i_level_idc = param->i_level_idc;
  sqh->b_progressive_sequence = 1;      //frame sequence

  sqh->i_horizontal_size = param->i_width;
  sqh->i_vertical_size = param->i_height;

  sqh->i_chroma_format = param->i_chroma_format;        // 4:2:0
  sqh->i_sample_precision = param->i_sample_precision;  // 8 bits per sample
  sqh->i_aspect_ratio = param->i_aspect_ratio;  // 1:1

  switch (param->i_fps_num / param->i_fps_den)
  {
  case 23:
    i_frame_rate_code = 1;
    break;
  case 24:
    i_frame_rate_code = 2;      //0010
    break;
  case 25:
    i_frame_rate_code = 3;      //0011
    break;
  case 29:
    i_frame_rate_code = 4;
    break;
  case 30:
    i_frame_rate_code = 5;      //0101
    break;
  case 50:
    i_frame_rate_code = 6;      //0110
    break;
  case 60:
    i_frame_rate_code = 8;      //1000
    break;
  default:
    if (param->i_fps_num == 24000 && param->i_fps_den == 1001)
      i_frame_rate_code = 1;    //0001
    else if (param->i_fps_num == 30000 && param->i_fps_den == 1001)
      i_frame_rate_code = 4;    //0100
    else if (param->i_fps_num == 60000 && param->i_fps_den == 1001)
      i_frame_rate_code = 7;    //0111
    else
      i_frame_rate_code = 9;    //1001
    break;
  }
  sqh->i_frame_rate_code = i_frame_rate_code;

  sqh->i_bit_rate_lower = param->rc.i_bitrate & (0x3FFFF);      // lower 18 bits of bitrate
  sqh->i_bit_rate_upper = param->rc.i_bitrate >> 18;    // bits upper to 18 bits
  sqh->b_low_delay = (param->i_bframe == 0);
  sqh->i_bbv_buffer_size = param->rc.i_vbv_buffer_size;

}

void
xavs_sequence_write (bs_t * s, xavs_seq_header_t * sqh)
{
  bs_write (s, 8, sqh->i_video_sequence_start_code);
  bs_write (s, 8, sqh->i_profile_idc);
  bs_write (s, 8, sqh->i_level_idc);
  bs_write1 (s, sqh->b_progressive_sequence);
  bs_write (s, 14, sqh->i_horizontal_size);
  bs_write (s, 14, sqh->i_vertical_size);
  bs_write (s, 2, sqh->i_chroma_format);
  bs_write (s, 3, sqh->i_sample_precision);
  bs_write (s, 4, sqh->i_aspect_ratio);
  bs_write (s, 4, sqh->i_frame_rate_code);
  bs_write (s, 18, 500);        //sqh->i_bit_rate_lower);
  bs_write1 (s, 1);             //marker bit
  bs_write (s, 12, 0);          //sqh->i_bit_rate_upper);
  bs_write1 (s, sqh->b_low_delay);
  bs_write1 (s, 1);
  bs_write (s, 18, 122800);     //sqh->i_bbv_buffer_size);
  bs_write (s, 3, 0);           //reserved bits
  //bs_rbsp_trailing( s );
}

void
xavs_sequence_end_write (bs_t * s)
{
  bs_write (s, 8, 0xB1);
}

void
xavs_i_picture_write (bs_t * s, xavs_i_pic_header_t * ih, xavs_seq_header_t * sqh)
{
  bs_write (s, 8, 0xB3);        //ih->i_i_picture_start_code);
  bs_write (s, 16, ih->i_bbv_delay);
  bs_write1 (s, ih->b_time_code_flag);
  if (ih->b_time_code_flag)
    bs_write (s, 24, ih->i_time_code);
  bs_write1 (s, 1);             //marker bit
  bs_write (s, 8, ih->i_picture_distance);
  if (sqh->b_low_delay)
    bs_write_ue (s, 1);         //ih->i_bbv_check_times);
  bs_write1 (s, ih->b_progressive_frame);
  if (!ih->b_progressive_frame)
    bs_write1 (s, ih->b_picture_structure);

  bs_write1 (s, 1);             //ih->b_top_field_first);
  bs_write1 (s, 0);             //ih->b_repeat_first_field);
  bs_write1 (s, ih->b_fixed_picture_qp);
  bs_write (s, 6, ih->i_picture_qp);
  //if(!ih->b_progressive_frame && !ih->b_picture_structure)
  //bs_write1( s, ih->b_skip_mode_flag);
  bs_write (s, 4, ih->i_reserved_bits);
  bs_write1 (s, ih->b_loop_filter_disable);
  if (!ih->b_loop_filter_disable)
    bs_write1 (s, ih->b_loop_filter_parameter_flag);
  if (ih->b_loop_filter_parameter_flag)
  {
    bs_write_se (s, ih->i_alpha_c_offset);
    bs_write_se (s, ih->i_beta_offset);
  }
  bs_rbsp_trailing (s);
}

void
xavs_pb_picture_write (bs_t * s, xavs_pb_pic_header_t * pbh, xavs_seq_header_t * sqh)
{
  bs_write (s, 8, pbh->i_pb_picture_start_code);
  bs_write (s, 16, pbh->i_bbv_delay);
  bs_write (s, 2, pbh->i_picture_coding_type);
  bs_write (s, 8, pbh->i_picture_distance);
  if (sqh->b_low_delay)
    bs_write_ue (s, 1);         //pbh->i_bbv_check_times);
  bs_write1 (s, 1);             //pbh->b_progressive_frame);
  /*if(!pbh->b_progressive_frame){
     bs_write1( s, pbh->b_picture_structure);
     if(!pbh->b_picture_structure)
     bs_write1( s, pbh->b_advanced_pred_mode_disable);
     } */
  bs_write1 (s, 0);             //pbh->b_top_field_first);
  bs_write1 (s, 0);             //pbh->b_repeat_first_field);
  bs_write1 (s, pbh->b_fixed_picture_qp);
  bs_write (s, 6, pbh->i_picture_qp);
  if (pbh->i_picture_coding_type == 1)
    bs_write1 (s, pbh->b_picture_reference_flag);
  //bs_write1( s, pbh->b_no_forward_reference_flag);
  bs_write (s, 4, 0);           //reserved bits
  bs_write1 (s, 1);             //pbh->b_skip_mode_flag);
  bs_write1 (s, pbh->b_loop_filter_disable);
  if (!pbh->b_loop_filter_disable)
    bs_write1 (s, pbh->b_loop_filter_parameter_flag);
  if (pbh->b_loop_filter_parameter_flag)
  {
    bs_write_se (s, pbh->i_alpha_c_offset);
    bs_write_se (s, pbh->i_beta_offset);
  }
  bs_rbsp_trailing (s);
}

void
xavs_sps_init (xavs_sps_t * sps, int i_id, xavs_param_t * param)
{
  sps->i_id = i_id;

  sps->b_qpprime_y_zero_transform_bypass = param->rc.i_rc_method == XAVS_RC_CQP && param->rc.i_qp_constant == 0;
  if (sps->b_qpprime_y_zero_transform_bypass)
    sps->i_profile_idc = PROFILE_HIGH444;
  else if (param->analyse.b_transform_8x8 || param->i_cqm_preset != XAVS_CQM_FLAT)
    sps->i_profile_idc = PROFILE_HIGH;
  else if (param->b_cabac || param->i_bframe > 0)
    sps->i_profile_idc = PROFILE_MAIN;
  else
    sps->i_profile_idc = PROFILE_BASELINE;

  sps->i_profile_idc = PROFILE_JIZHUN;

  sps->i_level_idc = param->i_level_idc;

  sps->b_constraint_set0 = 0;
  sps->b_constraint_set1 = 0;
  sps->b_constraint_set2 = 0;

  sps->i_log2_max_frame_num = 4;        /* at least 4 */
  while ((1 << sps->i_log2_max_frame_num) <= param->i_keyint_max)
  {
    sps->i_log2_max_frame_num++;
  }
  sps->i_log2_max_frame_num++;  /* just in case */

  sps->i_poc_type = 0;
  if (sps->i_poc_type == 0)
  {
    sps->i_log2_max_poc_lsb = sps->i_log2_max_frame_num + 1;    /* max poc = 2*frame_num */
  }
  else if (sps->i_poc_type == 1)
  {
    int i;

    /* FIXME */
    sps->b_delta_pic_order_always_zero = 1;
    sps->i_offset_for_non_ref_pic = 0;
    sps->i_offset_for_top_to_bottom_field = 0;
    sps->i_num_ref_frames_in_poc_cycle = 0;

    for (i = 0; i < sps->i_num_ref_frames_in_poc_cycle; i++)
    {
      sps->i_offset_for_ref_frame[i] = 0;
    }
  }

  sps->b_vui = 1;

  sps->b_gaps_in_frame_num_value_allowed = 0;
  sps->i_mb_width = (param->i_width + 15) / 16;
  sps->i_mb_height = (param->i_height + 15) / 16;
  sps->b_frame_mbs_only = 1;
  sps->b_mb_adaptive_frame_field = 0;
  sps->b_direct8x8_inference = 0;
  if (sps->b_frame_mbs_only == 0 || !(param->analyse.inter & XAVS_ANALYSE_PSUB8x8))
  {
    sps->b_direct8x8_inference = 1;
  }

  sps->crop.i_left = 0;
  sps->crop.i_top = 0;
  sps->crop.i_right = (-param->i_width) & 15;
  sps->crop.i_bottom = (-param->i_height) & 15;
  sps->b_crop = sps->crop.i_left || sps->crop.i_top || sps->crop.i_right || sps->crop.i_bottom;

  sps->vui.b_aspect_ratio_info_present = 0;
  if (param->vui.i_sar_width > 0 && param->vui.i_sar_height > 0)
  {
    sps->vui.b_aspect_ratio_info_present = 1;
    sps->vui.i_sar_width = param->vui.i_sar_width;
    sps->vui.i_sar_height = param->vui.i_sar_height;
  }

  sps->vui.b_overscan_info_present = (param->vui.i_overscan ? 1 : 0);
  if (sps->vui.b_overscan_info_present)
    sps->vui.b_overscan_info = (param->vui.i_overscan == 2 ? 1 : 0);

  sps->vui.b_signal_type_present = 0;
  sps->vui.i_vidformat = (param->vui.i_vidformat <= 5 ? param->vui.i_vidformat : 5);
  sps->vui.b_fullrange = (param->vui.b_fullrange ? 1 : 0);
  sps->vui.b_color_description_present = 0;

  sps->vui.i_colorprim = (param->vui.i_colorprim <= 9 ? param->vui.i_colorprim : 2);
  sps->vui.i_transfer = (param->vui.i_transfer <= 11 ? param->vui.i_transfer : 2);
  sps->vui.i_colmatrix = (param->vui.i_colmatrix <= 9 ? param->vui.i_colmatrix : 2);
  if (sps->vui.i_colorprim != 2 || sps->vui.i_transfer != 2 || sps->vui.i_colmatrix != 2)
  {
    sps->vui.b_color_description_present = 1;
  }

  if (sps->vui.i_vidformat != 5 || sps->vui.b_fullrange || sps->vui.b_color_description_present)
  {
    sps->vui.b_signal_type_present = 1;
  }

  /* FIXME: not sufficient for interlaced video */
  sps->vui.b_chroma_loc_info_present = (param->vui.i_chroma_loc ? 1 : 0);
  if (sps->vui.b_chroma_loc_info_present)
  {
    sps->vui.i_chroma_loc_top = param->vui.i_chroma_loc;
    sps->vui.i_chroma_loc_bottom = param->vui.i_chroma_loc;
  }

  sps->vui.b_timing_info_present = 0;
  if (param->i_fps_num > 0 && param->i_fps_den > 0)
  {
    sps->vui.b_timing_info_present = 1;
    sps->vui.i_num_units_in_tick = param->i_fps_den;
    sps->vui.i_time_scale = param->i_fps_num * 2;
    sps->vui.b_fixed_frame_rate = 1;
  }

  sps->vui.i_num_reorder_frames = param->b_bframe_pyramid ? 2 : param->i_bframe ? 1 : 0;
  /* extra slot with pyramid so that we don't have to override the
   * order of forgetting old pictures */
  sps->vui.i_max_dec_frame_buffering = 
  sps->i_num_ref_frames = XAVS_MIN (16, param->i_frame_reference 
                                        + sps->vui.i_num_reorder_frames + param->b_bframe_pyramid);

  sps->vui.b_bitstream_restriction = 1;
  if (sps->vui.b_bitstream_restriction)
  {
    sps->vui.b_motion_vectors_over_pic_boundaries = 1;
    sps->vui.i_max_bytes_per_pic_denom = 0;
    sps->vui.i_max_bits_per_mb_denom = 0;
    sps->vui.i_log2_max_mv_length_horizontal = sps->vui.i_log2_max_mv_length_vertical = (int) (log (param->analyse.i_mv_range * 4 - 1) / log (2)) + 1;
  }
}

void
xavs_sps_write (bs_t * s, xavs_sps_t * sps)
{
  bs_write (s, 8, sps->i_profile_idc);
  bs_write (s, 1, sps->b_constraint_set0);
  bs_write (s, 1, sps->b_constraint_set1);
  bs_write (s, 1, sps->b_constraint_set2);

  bs_write (s, 5, 0);           /* reserved */

  bs_write (s, 8, sps->i_level_idc);

  bs_write_ue (s, sps->i_id);

  if (sps->i_profile_idc >= PROFILE_HIGH)
  {
    bs_write_ue (s, 1);         // chroma_format_idc = 4:2:0
    bs_write_ue (s, 0);         // bit_depth_luma_minus8
    bs_write_ue (s, 0);         // bit_depth_chroma_minus8
    bs_write (s, 1, sps->b_qpprime_y_zero_transform_bypass);
    bs_write (s, 1, 0);         // seq_scaling_matrix_present_flag
  }

  bs_write_ue (s, sps->i_log2_max_frame_num - 4);
  bs_write_ue (s, sps->i_poc_type);
  if (sps->i_poc_type == 0)
  {
    bs_write_ue (s, sps->i_log2_max_poc_lsb - 4);
  }
  else if (sps->i_poc_type == 1)
  {
    int i;

    bs_write (s, 1, sps->b_delta_pic_order_always_zero);
    bs_write_se (s, sps->i_offset_for_non_ref_pic);
    bs_write_se (s, sps->i_offset_for_top_to_bottom_field);
    bs_write_ue (s, sps->i_num_ref_frames_in_poc_cycle);

    for (i = 0; i < sps->i_num_ref_frames_in_poc_cycle; i++)
    {
      bs_write_se (s, sps->i_offset_for_ref_frame[i]);
    }
  }
  bs_write_ue (s, sps->i_num_ref_frames);
  bs_write (s, 1, sps->b_gaps_in_frame_num_value_allowed);
  bs_write_ue (s, sps->i_mb_width - 1);
  bs_write_ue (s, sps->i_mb_height - 1);
  bs_write (s, 1, sps->b_frame_mbs_only);
  if (!sps->b_frame_mbs_only)
  {
    bs_write (s, 1, sps->b_mb_adaptive_frame_field);
  }
  bs_write (s, 1, sps->b_direct8x8_inference);

  bs_write (s, 1, sps->b_crop);
  if (sps->b_crop)
  {
    bs_write_ue (s, sps->crop.i_left / 2);
    bs_write_ue (s, sps->crop.i_right / 2);
    bs_write_ue (s, sps->crop.i_top / 2);
    bs_write_ue (s, sps->crop.i_bottom / 2);
  }

  bs_write (s, 1, sps->b_vui);
  if (sps->b_vui)
  {
    bs_write1 (s, sps->vui.b_aspect_ratio_info_present);
    if (sps->vui.b_aspect_ratio_info_present)
    {
      int i;
      static const struct
      {
        int w, h;
        int sar;
      } sar[] =
      {
        {
        1, 1, 1},
        {
        12, 11, 2},
        {
        10, 11, 3},
        {
        16, 11, 4},
        {
        40, 33, 5},
        {
        24, 11, 6},
        {
        20, 11, 7},
        {
        32, 11, 8},
        {
        80, 33, 9},
        {
        18, 11, 10},
        {
        15, 11, 11},
        {
        64, 33, 12},
        {
        160, 99, 13},
        {
        0, 0, -1}
      };
      for (i = 0; sar[i].sar != -1; i++)
      {
        if (sar[i].w == sps->vui.i_sar_width && sar[i].h == sps->vui.i_sar_height)
          break;
      }
      if (sar[i].sar != -1)
      {
        bs_write (s, 8, sar[i].sar);
      }
      else
      {
        bs_write (s, 8, 255);   /* aspect_ratio_idc (extented) */
        bs_write (s, 16, sps->vui.i_sar_width);
        bs_write (s, 16, sps->vui.i_sar_height);
      }
    }

    bs_write1 (s, sps->vui.b_overscan_info_present);
    if (sps->vui.b_overscan_info_present)
      bs_write1 (s, sps->vui.b_overscan_info);

    bs_write1 (s, sps->vui.b_signal_type_present);
    if (sps->vui.b_signal_type_present)
    {
      bs_write (s, 3, sps->vui.i_vidformat);
      bs_write1 (s, sps->vui.b_fullrange);
      bs_write1 (s, sps->vui.b_color_description_present);
      if (sps->vui.b_color_description_present)
      {
        bs_write (s, 8, sps->vui.i_colorprim);
        bs_write (s, 8, sps->vui.i_transfer);
        bs_write (s, 8, sps->vui.i_colmatrix);
      }
    }

    bs_write1 (s, sps->vui.b_chroma_loc_info_present);
    if (sps->vui.b_chroma_loc_info_present)
    {
      bs_write_ue (s, sps->vui.i_chroma_loc_top);
      bs_write_ue (s, sps->vui.i_chroma_loc_bottom);
    }

    bs_write1 (s, sps->vui.b_timing_info_present);
    if (sps->vui.b_timing_info_present)
    {
      bs_write (s, 32, sps->vui.i_num_units_in_tick);
      bs_write (s, 32, sps->vui.i_time_scale);
      bs_write1 (s, sps->vui.b_fixed_frame_rate);
    }

    bs_write1 (s, 0);           /* nal_hrd_parameters_present_flag */
    bs_write1 (s, 0);           /* vcl_hrd_parameters_present_flag */
    bs_write1 (s, 0);           /* pic_struct_present_flag */
    bs_write1 (s, sps->vui.b_bitstream_restriction);
    if (sps->vui.b_bitstream_restriction)
    {
      bs_write1 (s, sps->vui.b_motion_vectors_over_pic_boundaries);
      bs_write_ue (s, sps->vui.i_max_bytes_per_pic_denom);
      bs_write_ue (s, sps->vui.i_max_bits_per_mb_denom);
      bs_write_ue (s, sps->vui.i_log2_max_mv_length_horizontal);
      bs_write_ue (s, sps->vui.i_log2_max_mv_length_vertical);
      bs_write_ue (s, sps->vui.i_num_reorder_frames);
      bs_write_ue (s, sps->vui.i_max_dec_frame_buffering);
    }
  }

  bs_rbsp_trailing (s);
}

void
xavs_pps_init (xavs_pps_t * pps, int i_id, xavs_param_t * param, xavs_sps_t * sps)
{
  int i, j;

  pps->i_id = i_id;
  pps->i_sps_id = sps->i_id;
  pps->b_cabac = param->b_cabac;

  pps->b_pic_order = 0;
  pps->i_num_slice_groups = 1;

  pps->i_num_ref_idx_l0_active = 1;
  pps->i_num_ref_idx_l1_active = 1;

  pps->b_weighted_pred = 0;
  pps->b_weighted_bipred = param->analyse.b_weighted_bipred ? 2 : 0;

  pps->i_pic_init_qp = param->rc.i_rc_method == XAVS_RC_ABR ? 30 : param->rc.i_qp_constant;
  pps->i_pic_init_qs = 26;

  pps->i_chroma_qp_index_offset = param->analyse.i_chroma_qp_offset;
  pps->b_deblocking_filter_control = 1;
  pps->b_constrained_intra_pred = 0;
  pps->b_redundant_pic_cnt = 0;

  pps->b_transform_8x8_mode = param->analyse.b_transform_8x8 ? 1 : 0;

  pps->i_cqm_preset = param->i_cqm_preset;
  switch (pps->i_cqm_preset)
  {
  case XAVS_CQM_FLAT:
    for (i = 0; i < 6; i++)
      pps->scaling_list[i] = xavs_cqm_flat16;
    break;
  case XAVS_CQM_JVT:
    for (i = 0; i < 6; i++)
      pps->scaling_list[i] = xavs_cqm_jvt[i];
    break;
  case XAVS_CQM_CUSTOM:
    /* match the transposed DCT & zigzag */
    transpose (param->cqm_4iy, 4);
    transpose (param->cqm_4ic, 4);
    transpose (param->cqm_4py, 4);
    transpose (param->cqm_4pc, 4);
    transpose (param->cqm_8iy, 8);
    transpose (param->cqm_8py, 8);
    pps->scaling_list[CQM_4IY] = param->cqm_4iy;
    pps->scaling_list[CQM_4IC] = param->cqm_4ic;
    pps->scaling_list[CQM_4PY] = param->cqm_4py;
    pps->scaling_list[CQM_4PC] = param->cqm_4pc;
    pps->scaling_list[CQM_8IY + 4] = param->cqm_8iy;
    pps->scaling_list[CQM_8PY + 4] = param->cqm_8py;
    for (i = 0; i < 6; i++)
      for (j = 0; j < (i < 4 ? 16 : 64); j++)
        if (pps->scaling_list[i][j] == 0)
          pps->scaling_list[i] = xavs_cqm_jvt[i];
    break;
  }
}

void
xavs_pps_write (bs_t * s, xavs_pps_t * pps)
{
  bs_write_ue (s, pps->i_id);
  bs_write_ue (s, pps->i_sps_id);

  bs_write (s, 1, pps->b_cabac);
  bs_write (s, 1, pps->b_pic_order);
  bs_write_ue (s, pps->i_num_slice_groups - 1);

  bs_write_ue (s, pps->i_num_ref_idx_l0_active - 1);
  bs_write_ue (s, pps->i_num_ref_idx_l1_active - 1);
  bs_write (s, 1, pps->b_weighted_pred);
  bs_write (s, 2, pps->b_weighted_bipred);

  bs_write_se (s, pps->i_pic_init_qp - 26);
  bs_write_se (s, pps->i_pic_init_qs - 26);
  bs_write_se (s, pps->i_chroma_qp_index_offset);

  bs_write (s, 1, pps->b_deblocking_filter_control);
  bs_write (s, 1, pps->b_constrained_intra_pred);
  bs_write (s, 1, pps->b_redundant_pic_cnt);

  if (pps->b_transform_8x8_mode || pps->i_cqm_preset != XAVS_CQM_FLAT)
  {
    bs_write (s, 1, pps->b_transform_8x8_mode);
    bs_write (s, 1, (pps->i_cqm_preset != XAVS_CQM_FLAT));
    if (pps->i_cqm_preset != XAVS_CQM_FLAT)
    {
      scaling_list_write (s, pps, CQM_4IY);
      scaling_list_write (s, pps, CQM_4IC);
      bs_write (s, 1, 0);       // Cr = Cb
      scaling_list_write (s, pps, CQM_4PY);
      scaling_list_write (s, pps, CQM_4PC);
      bs_write (s, 1, 0);       // Cr = Cb
      if (pps->b_transform_8x8_mode)
      {
        scaling_list_write (s, pps, CQM_8IY + 4);
        scaling_list_write (s, pps, CQM_8PY + 4);
      }
    }
    bs_write_se (s, pps->i_chroma_qp_index_offset);
  }

  bs_rbsp_trailing (s);
}

void
xavs_sei_version_write (xavs_t * h, bs_t * s)
{
  int i;
  // random ID number generated according to ISO-11578
  const uint8_t uuid[16] = {
    0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48, 0xb7,
    0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef
  };
  char version[1200];
  int length;
  char *opts = xavs_param2string (&h->param, 0);

  sprintf (version, "xavs -  %d%s - AVS video codec - " "Copyleft 2009 - http://xavs.sourceforge.net - options: %s", XAVS_BUILD, XAVS_VERSION, opts);
  xavs_free (opts);
  length = (int) strlen (version) + 1 + 16;

  bs_write (s, 8, 0x5);         // payload_type = user_data_unregistered
  // payload_size
  for (i = 0; i <= length - 255; i += 255)
    bs_write (s, 8, 255);
  bs_write (s, 8, length - i);

  for (i = 0; i < 16; i++)
    bs_write (s, 8, uuid[i]);
  for (i = 0; i < length - 16; i++)
    bs_write (s, 8, version[i]);

  bs_rbsp_trailing (s);
}

const xavs_level_t xavs_levels[] = {
  //level 2.0
  {16, 352, 288, 30, 2534400, 1000000, 122880, 396, 11880, -128, 127.75, 0, 0, -2048, 2047.75, 0, 1, 3200, 4224},
  //level 4.0
  {32, 720, 576, 30, 10368000, 10000000, 1228800, 1620, 186000, -256, 255.75, -128, 127.75, -2048, 2047.75, 0, 0, 3200, 4224},
  //level 4.2
  {34, 720, 576, 30, 10368000, 15000000, 1851392, 1620, 186000, -256, 255.75, -128, 127.75, -2048, 2047.75, 1, 0, 3200, 4224},
  //level 6.0
  {64, 1920, 1152, 60, 62668800, 20000000, 2457600, 8160, 244800, -512, 511.75, -256, 255.75, -2048, 2047.75, 0, 0, 3200, 4224},
  //level 6.2
  {66, 1920, 1152, 60, 62668800, 20000000, 3686400, 8160, 244800, -512, 511.75, -256, 255.75, -2048, 2047.75, 0, 0, 3200, 4224},

  {0}
};

void
xavs_validate_levels (xavs_t * h)
{
  int mbs;

  const xavs_level_t *l = xavs_levels;
  while (l->level_idc != 0 && l->level_idc != h->param.i_level_idc)
    l++;

  mbs = h->sps->i_mb_width * h->sps->i_mb_height;
  if (l->frame_size < mbs || l->frame_size * 8 < h->sps->i_mb_width * h->sps->i_mb_width || l->frame_size * 8 < h->sps->i_mb_height * h->sps->i_mb_height)
    xavs_log (h, XAVS_LOG_WARNING, "frame MB size (%dx%d) > level limit (%d)\n", h->sps->i_mb_width, h->sps->i_mb_height, l->frame_size);

#define CHECK( name, limit, val ) \
    if( (val) > (limit) ) \
        xavs_log( h, XAVS_LOG_WARNING, name " (%d) > level limit (%d)\n", (int)(val), (limit) );

  // CHECK( "DPB size", l->dpb, mbs * 384 * h->sps->i_num_ref_frames );
  CHECK ("VBV bitrate", l->bitrate, h->param.rc.i_vbv_max_bitrate);
  CHECK ("VBV buffer", l->cpb, h->param.rc.i_vbv_buffer_size);
  CHECK ("MV range", l->mv_range, h->param.analyse.i_mv_range);

  if (h->param.i_fps_den > 0)
    CHECK ("MB rate", l->mbps, (int64_t) mbs * h->param.i_fps_num / h->param.i_fps_den);

  /* TODO check the rest of the limits */
}
