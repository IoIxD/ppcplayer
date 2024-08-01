/*****************************************************************************
 * set.h: h264 encoder
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
 * set.h: xavs encoder
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_SET_H_
#define _XAVS_SET_H_

enum profile_e
{
  PROFILE_JIZHUN   = 20,
  PROFILE_SHENZHAN = 24,
  PROFILE_YIDONG   = 32,
  PROFILE_JIAQIANG = 88,
  PROFILE_BASELINE = 64,
  PROFILE_MAIN = 77,
  PROFILE_HIGH = 100,
  PROFILE_HIGH10 = 110,
  PROFILE_HIGH422 = 122,
  PROFILE_HIGH444 = 144
};

enum cqm4_e
{
  CQM_4IY = 0,
  CQM_4PY = 1,
  CQM_4IC = 2,
  CQM_4PC = 3
};
enum cqm8_e
{
  CQM_8IY = 0,
  CQM_8PY = 1,
  CQM_8IC = 2,
  CQM_8PC = 3
};

typedef struct
{
  int i_id;

  int i_profile_idc;
  int i_level_idc;

  int b_constraint_set0;
  int b_constraint_set1;
  int b_constraint_set2;

  int i_log2_max_frame_num;

  int i_poc_type;
  /* poc 0 */
  int i_log2_max_poc_lsb;
  /* poc 1 */
  int b_delta_pic_order_always_zero;
  int i_offset_for_non_ref_pic;
  int i_offset_for_top_to_bottom_field;
  int i_num_ref_frames_in_poc_cycle;
  int i_offset_for_ref_frame[256];

  int i_num_ref_frames;
  int b_gaps_in_frame_num_value_allowed;
  int i_mb_width;
  int i_mb_height;
  int b_frame_mbs_only;
  int b_mb_adaptive_frame_field;
  int b_direct8x8_inference;

  int b_crop;
  struct
  {
    int i_left;
    int i_right;
    int i_top;
    int i_bottom;
  } crop;

  int b_vui;
  struct
  {
    int b_aspect_ratio_info_present;
    int i_sar_width;
    int i_sar_height;

    int b_overscan_info_present;
    int b_overscan_info;

    int b_signal_type_present;
    int i_vidformat;
    int b_fullrange;
    int b_color_description_present;
    int i_colorprim;
    int i_transfer;
    int i_colmatrix;

    int b_chroma_loc_info_present;
    int i_chroma_loc_top;
    int i_chroma_loc_bottom;

    int b_timing_info_present;
    int i_num_units_in_tick;
    int i_time_scale;
    int b_fixed_frame_rate;

    int b_bitstream_restriction;
    int b_motion_vectors_over_pic_boundaries;
    int i_max_bytes_per_pic_denom;
    int i_max_bits_per_mb_denom;
    int i_log2_max_mv_length_horizontal;
    int i_log2_max_mv_length_vertical;
    int i_num_reorder_frames;
    int i_max_dec_frame_buffering;

    /* FIXME to complete */
  } vui;

  int b_qpprime_y_zero_transform_bypass;

} xavs_sps_t;

typedef struct
{
  int i_id;
  int i_sps_id;

  int b_cabac;

  int b_pic_order;
  int i_num_slice_groups;

  int i_num_ref_idx_l0_active;
  int i_num_ref_idx_l1_active;

  int b_weighted_pred;
  int b_weighted_bipred;

  int i_pic_init_qp;
  int i_pic_init_qs;

  int i_chroma_qp_index_offset;

  int b_deblocking_filter_control;
  int b_constrained_intra_pred;
  int b_redundant_pic_cnt;

  int b_transform_8x8_mode;

  int i_cqm_preset;
  const uint8_t *scaling_list[6];       /* could be 8, but we don't allow separate Cb/Cr lists */

} xavs_pps_t;
typedef struct
{
  int i_video_sequence_start_code;

  int i_profile_idc;
  int i_level_idc;

  int b_progressive_sequence;

  int i_horizontal_size;
  int i_vertical_size;
  int i_chroma_format;
  int i_sample_precision;
  int i_aspect_ratio;
  int i_frame_rate_code;
  int i_bit_rate_lower;
  int i_bit_rate_upper;
  int b_low_delay;
  int i_bbv_buffer_size;
  int i_reserved_bits;

} xavs_seq_header_t;

typedef struct
{
  int i_extention_start_code;
  int i_extention_id;           //'0010'
  int i_video_format;
  int b_sample_range;
  int b_colour_description;
  int i_colour_primaries;
  int i_transfer_characteristics;
  int i_matrix_coefficients;
  int i_display_horizontal_size;
  int i_display_vertical_size;
  int i_reserved_bits;
} xavs_sequence_display_extention_t;

typedef struct
{
  int i_extention_start_code;
  int i_extention_id;           //'0100'
  int b_copyright_flag;
  int i_copyright_id;
  int b_original_or_copy;
  int i_reserved_bits;
  int i_copyright_number_1;
  int i_copyright_number_2;
  int i_copyright_number_3;

} xavs_copyright_extention_t;

typedef struct
{
  int i_extention_start_code;
  int i_extention_id;           //'1011'
  int i_camera_id;
  int i_height_of_image_device;
  int i_focal_length;
  int i_f_number;
  int i_vertical_angle_of_view;
  int i_camera_position_x_upper;
  int i_camera_position_x_lower;
  int i_camera_position_y_upper;
  int i_camera_position_y_lower;
  int i_camera_position_z_upper;
  int i_camera_position_z_lower;
  int i_camera_direction_x;
  int i_camera_direction_y;
  int i_camera_direction_z;
  int i_image_plane_vertical_x;
  int i_image_plane_vertical_y;
  int i_image_plane_vertical_z;
  int i_reserved_bits;
} xavs_camera_parameters_extention_t;

typedef struct
{
  int i_user_data_start_code;
  char *psz_userdata;

} xavs_user_data_t;

typedef struct
{
  int i_i_picture_start_code;
  int i_bbv_delay;
  int b_time_code_flag;
  int i_time_code;
  int i_picture_distance;
  int i_bbv_check_times;
  int b_progressive_frame;
  int b_picture_structure;
  int b_top_field_first;
  int b_repeat_first_field;
  int b_fixed_picture_qp;
  int i_picture_qp;
  int b_skip_mode_flag;
  int i_reserved_bits;
  int b_loop_filter_disable;
  int b_loop_filter_parameter_flag;
  int i_alpha_c_offset;
  int i_beta_offset;
} xavs_i_pic_header_t;

typedef struct
{
  int i_pb_picture_start_code;
  int i_bbv_delay;
  int i_picture_coding_type;
  int i_picture_distance;
  int i_bbv_check_times;
  int b_progressive_frame;
  int b_picture_structure;
  int b_advanced_pred_mode_disable;
  int b_top_field_first;
  int b_repeat_first_field;
  int b_fixed_picture_qp;
  int i_picture_qp;
  int b_picture_reference_flag;
  int b_no_forward_reference_flag;
  int b_skip_mode_flag;
  int b_loop_filter_disable;
  int b_loop_filter_parameter_flag;
  int i_alpha_c_offset;
  int i_beta_offset;
} xavs_pb_pic_header_t;

typedef struct
{
  int i_extention_start_code;
  int i_extention_code;         //'0111'
  int i_frame_centre_horizontal_offset[3];
  int i_frame_centre_vertical_offset[3];

} xavs_picture_display_extention_t;

/* default quant matrices */
static const uint8_t xavs_cqm_jvt4i[16] = {
  6, 13, 20, 28,
  13, 20, 28, 32,
  20, 28, 32, 37,
  28, 32, 37, 42
};
static const uint8_t xavs_cqm_jvt4p[16] = {
  10, 14, 20, 24,
  14, 20, 24, 27,
  20, 24, 27, 30,
  24, 27, 30, 34
};
static const uint8_t xavs_cqm_jvt8i[64] = {
  6, 10, 13, 16, 18, 23, 25, 27,
  10, 11, 16, 18, 23, 25, 27, 29,
  13, 16, 18, 23, 25, 27, 29, 31,
  16, 18, 23, 25, 27, 29, 31, 33,
  18, 23, 25, 27, 29, 31, 33, 36,
  23, 25, 27, 29, 31, 33, 36, 38,
  25, 27, 29, 31, 33, 36, 38, 40,
  27, 29, 31, 33, 36, 38, 40, 42
};
static const uint8_t xavs_cqm_jvt8p[64] = {
  9, 13, 15, 17, 19, 21, 22, 24,
  13, 13, 17, 19, 21, 22, 24, 25,
  15, 17, 19, 21, 22, 24, 25, 27,
  17, 19, 21, 22, 24, 25, 27, 28,
  19, 21, 22, 24, 25, 27, 28, 30,
  21, 22, 24, 25, 27, 28, 30, 32,
  22, 24, 25, 27, 28, 30, 32, 33,
  24, 25, 27, 28, 30, 32, 33, 35
};
static const uint8_t xavs_cqm_flat16[64] = {
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16,
  16, 16, 16, 16, 16, 16, 16, 16
};
static const uint8_t *const xavs_cqm_jvt[6] = {
  xavs_cqm_jvt4i, xavs_cqm_jvt4p,
  xavs_cqm_jvt4i, xavs_cqm_jvt4p,
  xavs_cqm_jvt8i, xavs_cqm_jvt8p
};

int xavs_cqm_init (xavs_t * h);
void xavs_cqm_delete (xavs_t * h);
int xavs_cqm_parse_file (xavs_t * h, const char *filename);

#endif
