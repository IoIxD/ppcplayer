/*****************************************************************************
 * mc.c: h264 encoder library (Motion Compensation)
 *****************************************************************************
 * Copyright (C) 2003 Laurent Aimar
 * $Id: mc-c.c,v 1.5 2004/06/18 01:59:58 chenm001 Exp $
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>5
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common/common.h"


// NASM functions 
extern void xavs_pixel_avg_w4_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w8_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w16_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w16_sse2( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_weight_4x4_mmxext( uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_weight_w8_mmxext( uint8_t *, int, uint8_t *, int, int, int );
extern void xavs_pixel_avg_weight_w16_mmxext( uint8_t *, int, uint8_t *, int, int, int );
extern void xavs_mc_copy_w4_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w8_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w16_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w16_sse2( uint8_t *, int, uint8_t *, int, int );

extern void xavs_pixel_51315_w8_sse2( uint8_t *, int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_51315_w16_sse2( uint8_t *, int, uint8_t *, int, uint8_t *, int, int );

static const int hpel_table[16][4] = {
  {0, 0, 0, 0}, {0, -1, 0, 1}, {0, 0, 0, 0}, {0, 1, 0, -1},
  {-1, 0, 1, 0}, {0, 0, 0, 0}, {1, 0, -1, 0}, {0, 0, 0, 0},
  {0, 0, 0, 0}, {0, 1, 0, -1}, {0, 0, 0, 0}, {0, -1, 0, 1},
  {1, 0, -1, 0}, {0, 0, 0, 0}, {-1, 0, 1, 0}, {0, 0, 0, 0}
};

#define AVG(W,H) \
static void xavs_pixel_avg_ ## W ## x ## H ## _mmxext( uint8_t *dst, int i_dst, uint8_t *src, int i_src ) \
{ \
    xavs_pixel_avg_w ## W ## _mmxext( dst, i_dst, dst, i_dst, src, i_src, H ); \
}
AVG(16,16)
AVG(16,8)
AVG(8,16)
AVG(8,8)
AVG(8,4)
AVG(4,8)
AVG(4,4)
AVG(4,2)

#define AVG_WEIGHT(W,H) \
void xavs_pixel_avg_weight_ ## W ## x ## H ## _mmxext( uint8_t *dst, int i_dst, uint8_t *src, int i_src, int i_weight_dst ) \
{ \
    xavs_pixel_avg_weight_w ## W ## _mmxext( dst, i_dst, src, i_src, i_weight_dst, H ); \
}
AVG_WEIGHT(16,16)
AVG_WEIGHT(16,8)
AVG_WEIGHT(8,16)
AVG_WEIGHT(8,8)
AVG_WEIGHT(8,4)
/*
static void (* const xavs_pixel_avg_wtab_mmxext[5])( uint8_t *, int, uint8_t *, int, uint8_t *, int, int ) =
{
    NULL,
    xavs_pixel_avg_w4_mmxext,
    xavs_pixel_avg_w8_mmxext,
    NULL,
    xavs_pixel_avg_w16_mmxext
};*/
static void (* const xavs_pixel_avg_wtab1_mmxext[5])( uint8_t *, int, uint8_t *, int, uint8_t *, int, int ) =
{
    NULL,
    NULL,
    xavs_pixel_51315_w8_sse2,
    NULL,
    xavs_pixel_51315_w16_sse2
};
/*static void (* const xavs_pixel_avg_wtab2_mmxext[5])( uint8_t *, int, uint8_t *, int, uint8_t *, int, int ) =
{
    NULL,
    NULL,
    xavs_pixel_7_w8_sse2,
    NULL,
    xavs_pixel_7_w16_sse2
};*/
/*
static inline uint8_t xavs_clip_uint8( int x )
{
    return x&(~255) ? (-x)>>31 : x;
}
*/


extern void xavs_qpel1_filter_mmxext( uint8_t *dst,  int i_dst_stride,uint8_t *src1, int i_src1_stride,uint8_t *src2, int i_src2_stride,int i_width, int i_height,uint8_t *src3,uint8_t *src4);
static inline void pixel_avg( uint8_t *dst,  int i_dst_stride,
                              uint8_t *src1, int i_src1_stride,
                              uint8_t *src2, int i_src2_stride,
                              int i_width, int i_height,int qpel_idx,uint8_t *src3,uint8_t *src4)
{
    int x, y;
	//uint8_t *src3[1024];
	//uint8_t *src4[1024];
     if(qpel_idx==5||qpel_idx==13||qpel_idx==15||qpel_idx==7)
			{
				xavs_pixel_avg_wtab1_mmxext[i_width>>2](dst, i_dst_stride, src1, i_src1_stride,src2, i_src2_stride, i_height );
			}
     else 
		 {
                xavs_qpel1_filter_mmxext( dst,  i_dst_stride,src1,  i_src1_stride,src2, i_src2_stride,i_width, i_height,src3,src4 );
			     }
  
	
	
	
/*	for( y = 0; y < i_height; y++ )
    {
        for( x = 0; x < i_width; x++ )
        {
    
			if(qpel_idx==1)
				dst[x]=xavs_clip_uint8(((96*src2[x]+42*src2[x+1]-7*src2[x+2]-2*src2[x-1]-src2[x-2])+64)>>7);
			
			if(qpel_idx==3)
				dst[x]=xavs_clip_uint8(((96*src2[x]+42*src2[x-1]-7*src2[x-2]-2*src2[x+1]-src2[x+2])+64)>>7);
			if(qpel_idx==4)
				dst[x]=xavs_clip_uint8(((-src2[x-2*i_src2_stride]-2*src2[x-i_src2_stride]+96*src2[x]
				       +42*src2[x+i_src2_stride]-7*src2[x+2*i_src2_stride])+64)>>7);
			if(qpel_idx==12) 
				dst[x]=xavs_clip_uint8(((-7*src2[x-2*i_src2_stride]+42*src2[x-i_src2_stride]+96*src2[x]
				       -2*src2[x+i_src2_stride]-src2[x+2*i_src2_stride])+64)>>7);
			if(qpel_idx==9)
				dst[x]=xavs_clip_uint8(((-(-src2[x-2-i_src2_stride]+5*src2[x-2]+5*src2[x-2+i_src2_stride]-src2[x-2+2*i_src2_stride])
				       -2*(-src2[x-1-i_src2_stride]+5*src2[x-1]+5*src2[x-1+i_src2_stride]-src2[x-1+2*i_src2_stride])
                       +96*(-src2[x-i_src2_stride]+5*src2[x]+5*src2[x+i_src2_stride]-src2[x+2*i_src2_stride])
                       +42*(-src2[x+1-i_src2_stride]+5*src2[x+1]+5*src2[x+1+i_src2_stride]-src2[x+1+2*i_src2_stride])
                       -7*(-src2[x+2-i_src2_stride]+5*src2[x+2]+5*src2[x+2+i_src2_stride]-src2[x+2+2*i_src2_stride]))+512)>>10);
            if(qpel_idx==11)
				dst[x]=xavs_clip_uint8(((-7*(-src2[x-2-i_src2_stride]+5*src2[x-2]+5*src2[x-2+i_src2_stride]-src2[x-2+2*i_src2_stride])
				       +42*(-src2[x-1-i_src2_stride]+5*src2[x-1]+5*src2[x-1+i_src2_stride]-src2[x-1+2*i_src2_stride])
                       +96*(-src2[x-i_src2_stride]+5*src2[x]+5*src2[x+i_src2_stride]-src2[x+2*i_src2_stride])
                       -2*(-src2[x+1-i_src2_stride]+5*src2[x+1]+5*src2[x+1+i_src2_stride]-src2[x+1+2*i_src2_stride])
                       -1*(-src2[x+2-i_src2_stride]+5*src2[x+2]+5*src2[x+2+i_src2_stride]-src2[x+2+2*i_src2_stride]))+512)>>10);
            if(qpel_idx==6)
				dst[x]=xavs_clip_uint8(((-(-src2[x-1-2*i_src2_stride]+5*src2[x-2*i_src2_stride]+5*src2[x+1-2*i_src2_stride]-src2[x+2-2*i_src2_stride])
				       -2*(-src2[x-1-1*i_src2_stride]+5*src2[x-1*i_src2_stride]+5*src2[x+1-1*i_src2_stride]-src2[x+2-1*i_src2_stride])
					   +96*(-src2[x-1]+5*src2[x]+5*src2[x+1]-src2[x+2])
					   +42*(-src2[x-1+i_src2_stride]+5*src2[x+i_src2_stride]+5*src2[x+1+i_src2_stride]-src2[x+2+i_src2_stride])
					   -7*(-src2[x-1+2*i_src2_stride]+5*src2[x+2*i_src2_stride]+5*src2[x+1+2*i_src2_stride]-src2[x+2+2*i_src2_stride]))+512)>>10);
			if(qpel_idx==14)
				dst[x]=xavs_clip_uint8(((-7*(-src2[x-1-2*i_src2_stride]+5*src2[x-2*i_src2_stride]+5*src2[x+1-2*i_src2_stride]-src2[x+2-2*i_src2_stride])
				       +42*(-src2[x-1-1*i_src2_stride]+5*src2 [x-1*i_src2_stride]+5*src2[x+1-1*i_src2_stride]-src2[x+2-1*i_src2_stride])
                       +96*(-src2[x-1]+5*src2[x]+5*src2[x+1]-src2[x+2])
					   -2*(-src2[x-1+i_src2_stride]+5*src2[x+i_src2_stride]+5*src2[x+1+i_src2_stride]-src2[x+2+i_src2_stride])
					   -(-src2[x-1+2*i_src2_stride]+5*src2[x+2*i_src2_stride]+5*src2[x+1+2*i_src2_stride]-src2[x+2+2*i_src2_stride]))+512)>>10);
			if(qpel_idx==5)
				dst[x]=xavs_clip_uint8(((64*src2[x]-(-src2[x-1-1*i_src2_stride]+5*src2[x-1*i_src2_stride]+5*src2[x+1-1*i_src2_stride]-src2[x+2-1*i_src2_stride])
				       +5*(-src2[x-1]+5*src2[x]+5*src2[x+1]-src2[x+2])
					   +5*(-src2[x-1+1*i_src2_stride]+5*src2[x+1*i_src2_stride]+5*src2[x+1+1*i_src2_stride]-src2[x+2+1*i_src2_stride])
					   -1*(-src2[x-1+2*i_src2_stride]+5*src2[x+2*i_src2_stride]+5*src2[x+1+2*i_src2_stride]-src2[x+2+2*i_src2_stride]))+64)>>7);
			if(qpel_idx==7)
				dst[x]=xavs_clip_uint8(((64*src2[x]-(-src2[x-2-1*i_src2_stride]+5*src2[x-1-1*i_src2_stride]+5*src2[x-1*i_src2_stride]-src2[x+1-1*i_src2_stride])
				       +5*(-src2[x-2]+5*src2[x-1]+5*src2[x]-src2[x+1])
					   +5*(-src2[x-2+1*i_src2_stride]+5*src2[x-1+1*i_src2_stride]+5*src2[x+1*i_src2_stride]-src2[x+1+1*i_src2_stride])
					   -1*(-src2[x-2+2*i_src2_stride]+5*src2[x-1+2*i_src2_stride]+5*src2[x+2*i_src2_stride]-src2[x+1+2*i_src2_stride]))+64)>>7);
			if(qpel_idx==13)
				dst[x]=xavs_clip_uint8(((64*src2[x]-(-src2[x-1-2*i_src2_stride]+5*src2[x-2*i_src2_stride]+5*src2[x+1-2*i_src2_stride]-src2[x+2-2*i_src2_stride])
				       +5*(-src2[x-1-1*i_src2_stride]+5*src2[x-1*i_src2_stride]+5*src2[x+1-1*i_src2_stride]-src2[x+2-1*i_src2_stride])
					   +5*(-src2[x-1]+5*src2[x]+5*src2[x+1]-src2[x+2])
					   -1*(-src2[x-1+1*i_src2_stride]+5*src2[x+1*i_src2_stride]+5*src2[x+1+1*i_src2_stride]-src2[x+2+1*i_src2_stride]))+64)>>7);
			if(qpel_idx==15)
				dst[x]=xavs_clip_uint8(((64*src2[x]-(-src2[x-2-2*i_src2_stride]+5*src2[x-1-2*i_src2_stride]+5*src2[x-2*i_src2_stride]-src2[x+1-2*i_src2_stride])
				       +5*(-src2[x-2-1*i_src2_stride]+5*src2[x-1-1*i_src2_stride]+5*src2[x-1*i_src2_stride]-src2[x+1-1*i_src2_stride])
					   +5*(-src2[x-2]+5*src2[x-1]+5*src2[x]-src2[x+1])
					   -1*(-src2[x-2+1*i_src2_stride]+5*src2[x-1+1*i_src2_stride]+5*src2[x+1*i_src2_stride]-src2[x+1+1*i_src2_stride]))+64)>>7);
       
		}
        dst  += i_dst_stride;
        src1 += i_src1_stride;
        src2 += i_src2_stride;
    }*/
}

static void (* const xavs_mc_copy_wtab_mmx[5])( uint8_t *, int, uint8_t *, int, int )=
{
    NULL,
    xavs_mc_copy_w4_mmx,
    xavs_mc_copy_w8_mmx,
    NULL,
    xavs_mc_copy_w16_mmx
};

//static const int hpel_ref0[16] = {0,1,1,1,0,1,1,1,2,3,3,3,0,1,1,1};
//static const int hpel_ref1[16] = {0,0,0,0,2,2,3,2,2,2,3,2,2,2,3,2};
static const int hpel_ref0[16] = {0,1,1,1,2,0,1,3,2,2,3,2,2,0,1,0};
static const int hpel_ref1[16] = {0,0,0,0,0,3,3,0,2,3,3,3,0,3,3,3};

void mc_luma_mmx( uint8_t *src[4], int i_src_stride,
                     uint8_t *dst,    int i_dst_stride,
                     int mvx,int mvy,
                     int i_width, int i_height )
{
     int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
	
    uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride;

   

    if( qpel_idx & 5 ) //qpel interpolation needed 
    {
		//uint8_t *src2 = src[0] + offset + ((mvx&3) == 3)+((mvy&3) == 3) * i_src_stride;
		
		
		
		uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
		uint8_t *src3 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride + hpel_table[qpel_idx][0]* i_src_stride + hpel_table[qpel_idx][1];
		uint8_t *src4 =  src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3) + hpel_table[qpel_idx][2]* i_src_stride+ hpel_table[qpel_idx][3];
		
		if(qpel_idx == 15)
	   {
           uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset +  i_src_stride+ 1;
		   uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset;
	   }
      
        pixel_avg( dst, i_dst_stride, src1, i_src_stride,
                   src2, i_src_stride, i_width, i_height,qpel_idx,src3,src4 );
    }
    else
    {
        xavs_mc_copy_wtab_mmx[i_width>>2](
                dst, i_dst_stride, src1, i_src_stride, i_height );
    }
}


uint8_t *get_ref_mmx( uint8_t *src[4], int i_src_stride,
                      uint8_t *dst,   int *i_dst_stride,
                      int mvx,int mvy,
                      int i_width, int i_height )
{
    int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
    
	uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride;
	
	

    if( qpel_idx & 5 ) // qpel interpolation needed 
    {
       
		//uint8_t *src2 = src[0] + offset + ((mvx&3) == 3)+((mvy&3) == 3) * i_src_stride;//wei

		
		uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
		uint8_t *src3 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride + hpel_table[qpel_idx][0]* i_src_stride + hpel_table[qpel_idx][1];
		uint8_t *src4 =  src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3) + hpel_table[qpel_idx][2]* i_src_stride+ hpel_table[qpel_idx][3];
		
	   
		if(qpel_idx == 15)
	   {
		uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset +  i_src_stride+ 1;
        uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset;
	   }
		pixel_avg( dst, *i_dst_stride, src1, i_src_stride,
                   src2, i_src_stride, i_width, i_height,qpel_idx,src3,src4 );
        /*xavs_pixel_avg_wtab_mmxext[i_width>>2](
                dst, *i_dst_stride, src1, i_src_stride,
                src2, i_src_stride, i_height );*/
        return dst;

    }
    else
    {
        *i_dst_stride = i_src_stride;
        return src1;
    }
}
/*void get_ref_mmx( uint8_t *src[4], int i_src_stride,
                         uint8_t *dst,    int * i_dst_stride,
                         int mvx,int mvy,
                         int i_width, int i_height )
{
    int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
    uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset+ ((mvy&3) == 3) * i_src_stride;
   if(qpel_idx == 15) 
	{
      uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset +  i_src_stride+ 1;
	}
    if( qpel_idx & 5 ) // qpel interpolation needed 
    {
        //uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
		uint8_t *src2 = src[0] + offset + ((mvx&3) == 3)+((mvy&3) == 3) * i_src_stride;

        if(qpel_idx==5||qpel_idx==13||qpel_idx==7)
		{ uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
		}
		if(qpel_idx == 15)
	   {
        uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset;
	   }
         pixel_avg( dst, *i_dst_stride, src1, i_src_stride,
                   src2, i_src_stride, i_width, i_height,qpel_idx );

        return dst;
    }
    else
    {
        *i_dst_stride = i_src_stride;
        return src1;
    }
}*/

/*
void mc_luma_mmx( uint8_t *src[4], int i_src_stride,
                  uint8_t *dst,    int i_dst_stride,
                  int mvx,int mvy,
                  int i_width, int i_height )
{
    int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
    uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride;

    if( qpel_idx & 5 )  //qpel interpolation needed 
    {
        uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
        xavs_pixel_avg_wtab_mmxext[i_width>>2](
                dst, i_dst_stride, src1, i_src_stride,
                src2, i_src_stride, i_height );
    }
    else
    {
        xavs_mc_copy_wtab_mmx[i_width>>2](
                dst, i_dst_stride, src1, i_src_stride, i_height );
    }
}
*/



void xavs_mc_mmxext_init( xavs_mc_functions_t *pf )
{
 
    pf->mc_luma   = mc_luma_mmx;
   pf->get_ref   = get_ref_mmx;
    pf->avg[PIXEL_16x16] = xavs_pixel_avg_16x16_mmxext;
    pf->avg[PIXEL_16x8]  = xavs_pixel_avg_16x8_mmxext;
    pf->avg[PIXEL_8x16]  = xavs_pixel_avg_8x16_mmxext;
    pf->avg[PIXEL_8x8]   = xavs_pixel_avg_8x8_mmxext;
    pf->avg[PIXEL_8x4]   = xavs_pixel_avg_8x4_mmxext;
    pf->avg[PIXEL_4x8]   = xavs_pixel_avg_4x8_mmxext;
    pf->avg[PIXEL_4x4]   = xavs_pixel_avg_4x4_mmxext;
    pf->avg[PIXEL_4x2]   = xavs_pixel_avg_4x2_mmxext;
    
    pf->avg_weight[PIXEL_16x16] = xavs_pixel_avg_weight_16x16_mmxext;
    pf->avg_weight[PIXEL_16x8]  = xavs_pixel_avg_weight_16x8_mmxext;
    pf->avg_weight[PIXEL_8x16]  = xavs_pixel_avg_weight_8x16_mmxext;
    pf->avg_weight[PIXEL_8x8]   = xavs_pixel_avg_weight_8x8_mmxext;
    pf->avg_weight[PIXEL_8x4]   = xavs_pixel_avg_weight_8x4_mmxext;
    pf->avg_weight[PIXEL_4x4]   = xavs_pixel_avg_weight_4x4_mmxext;
    pf->copy[PIXEL_16x16] = xavs_mc_copy_w16_mmx;
    pf->copy[PIXEL_8x8]   = xavs_mc_copy_w8_mmx;
    pf->copy[PIXEL_4x4]   = xavs_mc_copy_w4_mmx;
}

void xavs_mc_sse2_init( xavs_mc_functions_t *pf )
{
    // todo: use sse2 
   pf->mc_luma   = mc_luma_mmx;
   pf->get_ref   = get_ref_mmx;
}


/*#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common/common.h"

// NASM functions 
extern void xavs_pixel_avg_w4_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w8_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w16_mmxext( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_w16_sse2( uint8_t *,  int, uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_weight_4x4_mmxext( uint8_t *, int, uint8_t *, int, int );
extern void xavs_pixel_avg_weight_w8_mmxext( uint8_t *, int, uint8_t *, int, int, int );
extern void xavs_pixel_avg_weight_w16_mmxext( uint8_t *, int, uint8_t *, int, int, int );
extern void xavs_mc_copy_w4_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w8_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w16_mmx( uint8_t *, int, uint8_t *, int, int );
extern void xavs_mc_copy_w16_sse2( uint8_t *, int, uint8_t *, int, int );

#define AVG(W,H) \
static void xavs_pixel_avg_ ## W ## x ## H ## _mmxext( uint8_t *dst, int i_dst, uint8_t *src, int i_src ) \
{ \
    xavs_pixel_avg_w ## W ## _mmxext( dst, i_dst, dst, i_dst, src, i_src, H ); \
}
AVG(16,16)
AVG(16,8)
AVG(8,16)
AVG(8,8)
AVG(8,4)
AVG(4,8)
AVG(4,4)
AVG(4,2)

#define AVG_WEIGHT(W,H) \
void xavs_pixel_avg_weight_ ## W ## x ## H ## _mmxext( uint8_t *dst, int i_dst, uint8_t *src, int i_src, int i_weight_dst ) \
{ \
    xavs_pixel_avg_weight_w ## W ## _mmxext( dst, i_dst, src, i_src, i_weight_dst, H ); \
}
AVG_WEIGHT(16,16)
AVG_WEIGHT(16,8)
AVG_WEIGHT(8,16)
AVG_WEIGHT(8,8)
AVG_WEIGHT(8,4)

static void (* const xavs_pixel_avg_wtab_mmxext[5])( uint8_t *, int, uint8_t *, int, uint8_t *, int, int ) =
{
    NULL,
    xavs_pixel_avg_w4_mmxext,
    xavs_pixel_avg_w8_mmxext,
    NULL,
    xavs_pixel_avg_w16_mmxext
};
static void (* const xavs_mc_copy_wtab_mmx[5])( uint8_t *, int, uint8_t *, int, int ) =
{
    NULL,
    xavs_mc_copy_w4_mmx,
    xavs_mc_copy_w8_mmx,
    NULL,
    xavs_mc_copy_w16_mmx
};
static const int hpel_ref0[16] = {0,1,1,1,0,1,1,1,2,3,3,3,0,1,1,1};
static const int hpel_ref1[16] = {0,0,0,0,2,2,3,2,2,2,3,2,2,2,3,2};

void mc_luma_mmx( uint8_t *src[4], int i_src_stride,
                  uint8_t *dst,    int i_dst_stride,
                  int mvx,int mvy,
                  int i_width, int i_height )
{
    int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
    uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride;

    if( qpel_idx & 5 ) // qpel interpolation needed 
    {
        uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
        xavs_pixel_avg_wtab_mmxext[i_width>>2](
                dst, i_dst_stride, src1, i_src_stride,
                src2, i_src_stride, i_height );
    }
    else
    {
        xavs_mc_copy_wtab_mmx[i_width>>2](
                dst, i_dst_stride, src1, i_src_stride, i_height );
    }
}

uint8_t *get_ref_mmx( uint8_t *src[4], int i_src_stride,
                      uint8_t *dst,   int *i_dst_stride,
                      int mvx,int mvy,
                      int i_width, int i_height )
{
    int qpel_idx = ((mvy&3)<<2) + (mvx&3);
    int offset = (mvy>>2)*i_src_stride + (mvx>>2);
    uint8_t *src1 = src[hpel_ref0[qpel_idx]] + offset + ((mvy&3) == 3) * i_src_stride;

    if( qpel_idx & 5 ) // qpel interpolation needed 
    {
        uint8_t *src2 = src[hpel_ref1[qpel_idx]] + offset + ((mvx&3) == 3);
        xavs_pixel_avg_wtab_mmxext[i_width>>2](
                dst, *i_dst_stride, src1, i_src_stride,
                src2, i_src_stride, i_height );
        return dst;

    }
    else
    {
        *i_dst_stride = i_src_stride;
        return src1;
    }
}


void xavs_mc_mmxext_init( xavs_mc_functions_t *pf )
{
    pf->mc_luma   = mc_luma_mmx;
    pf->get_ref   = get_ref_mmx;

    pf->avg[PIXEL_16x16] = xavs_pixel_avg_16x16_mmxext;
    pf->avg[PIXEL_16x8]  = xavs_pixel_avg_16x8_mmxext;
    pf->avg[PIXEL_8x16]  = xavs_pixel_avg_8x16_mmxext;
    pf->avg[PIXEL_8x8]   = xavs_pixel_avg_8x8_mmxext;
    pf->avg[PIXEL_8x4]   = xavs_pixel_avg_8x4_mmxext;
    pf->avg[PIXEL_4x8]   = xavs_pixel_avg_4x8_mmxext;
    pf->avg[PIXEL_4x4]   = xavs_pixel_avg_4x4_mmxext;
    pf->avg[PIXEL_4x2]   = xavs_pixel_avg_4x2_mmxext;
    
    pf->avg_weight[PIXEL_16x16] = xavs_pixel_avg_weight_16x16_mmxext;
    pf->avg_weight[PIXEL_16x8]  = xavs_pixel_avg_weight_16x8_mmxext;
    pf->avg_weight[PIXEL_8x16]  = xavs_pixel_avg_weight_8x16_mmxext;
    pf->avg_weight[PIXEL_8x8]   = xavs_pixel_avg_weight_8x8_mmxext;
    pf->avg_weight[PIXEL_8x4]   = xavs_pixel_avg_weight_8x4_mmxext;
    pf->avg_weight[PIXEL_4x4]   = xavs_pixel_avg_weight_4x4_mmxext;
    // avg_weight_4x8 is rare and 4x2 is not used

    pf->copy[PIXEL_16x16] = xavs_mc_copy_w16_mmx;
    pf->copy[PIXEL_8x8]   = xavs_mc_copy_w8_mmx;
    pf->copy[PIXEL_4x4]   = xavs_mc_copy_w4_mmx;
}
void xavs_mc_sse2_init( xavs_mc_functions_t *pf )
{
    // todo: use sse2 
    pf->mc_luma   = mc_luma_mmx;
    pf->get_ref   = get_ref_mmx;
}
*/