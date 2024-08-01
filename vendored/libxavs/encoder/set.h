/*****************************************************************************
 * set.h: h264 encoder
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
 * set.h: xavs encoder
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_ENC_SET_H_
#define _XAVS_ENC_SET_H_

void xavs_sps_init (xavs_sps_t * sps, int i_id, xavs_param_t * param);
void xavs_sps_write (bs_t * s, xavs_sps_t * sps);
void xavs_pps_init (xavs_pps_t * pps, int i_id, xavs_param_t * param, xavs_sps_t * sps);
void xavs_pps_write (bs_t * s, xavs_pps_t * pps);
void xavs_sei_version_write (xavs_t * h, bs_t * s);
void xavs_validate_levels (xavs_t * h);

void xavs_sequence_init (xavs_seq_header_t * sqh, xavs_param_t * param);
void xavs_sequence_write (bs_t * s, xavs_seq_header_t * sqh);
void xavs_i_picture_write (bs_t * s, xavs_i_pic_header_t * ih, xavs_seq_header_t * sqh);
void xavs_pb_picture_write (bs_t * s, xavs_pb_pic_header_t * pbh, xavs_seq_header_t * sqh);

#endif
