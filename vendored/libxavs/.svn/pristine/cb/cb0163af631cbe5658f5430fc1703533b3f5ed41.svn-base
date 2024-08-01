/*****************************************************************************
 * csp.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009 xavs project
 *
 * Authors: 
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

/*****************************************************************************
 * csp.h: xavs encoder library
 *****************************************************************************
 * Copyright (C) 2009~2010 xavs project
 * Authors: Jianwen Chen <jianwen.chen.video@gmail.com>
 * This code is modified on x264 project and will follow the license of x264
 *****************************************************************************/

#ifndef _XAVS_CSP_H_
#define _XAVS_CSP_H_

typedef struct
{
  void (*i420) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*i422) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*i444) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*yv12) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*yuyv) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*rgb) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*bgr) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
  void (*bgra) (xavs_frame_t *, xavs_image_t *, int i_width, int i_height);
} xavs_csp_function_t;


void xavs_csp_init (int cpu, int i_csp, xavs_csp_function_t * pf);

#endif
