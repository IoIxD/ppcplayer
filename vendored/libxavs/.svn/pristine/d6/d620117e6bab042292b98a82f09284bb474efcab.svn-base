/*****************************************************************************
 * predict.h: xavs encoder library
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#include "common/common.h"

#ifdef HAVE_MMXEXT
#   include "predict.h"
#endif

extern void predict_8x8c_h_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8c_dc_128_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8c_dc_left_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8c_dc_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8c_v_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8c_dc_top_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8c_p_core_mmxext( src, i00, b, c );
extern void predict_8x8_h_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8_v_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8_dc_128_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8_dc_top_mmxext( uint8_t *src, int i_neighbor );
extern void predict_8x8_dc_left_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8_dc_mmxext( uint8_t *src,int i_neighbor );
extern void predict_8x8_ddr_sse2( uint8_t *src,int i_neighbor );
extern void predict_8x8_ddl_sse2( uint8_t *src,int i_neighbor );
/****************************************************************************
 * 8x8 prediction for intra chroma block
 ****************************************************************************/

	static void predict_8x8c_p( uint8_t *src, int i_neighbor )
{
    int a, b, c, i;
    int H = 0;
    int V = 0;
    int i00;

    for( i = 1; i <= 4; i++ )
    {
        H += i * ( src[3+i - FDEC_STRIDE] - src[3-i - FDEC_STRIDE] );
        V += i * ( src[(3+i)*FDEC_STRIDE -1] - src[(3-i)*FDEC_STRIDE -1] );
    }

    a = 16 * ( src[7*FDEC_STRIDE -1] + src[7 - FDEC_STRIDE] );
    b = ( 17 * H + 16 ) >> 5;
    c = ( 17 * V + 16 ) >> 5;
    i00 = a -3*b -3*c + 16;

    predict_8x8c_p_core_mmxext( src, i00, b, c );
}

void
xavs_predict_8x8_init_mmxext (xavs_predict_t pf[12])
{
  pf[I_PRED_8x8_V] = predict_8x8_v_mmxext;
  pf[I_PRED_8x8_H] = predict_8x8_h_mmxext;
  pf[I_PRED_8x8_DC] = predict_8x8_dc_mmxext;
  pf[I_PRED_8x8_DC_LEFT] = predict_8x8_dc_left_mmxext;
  pf[I_PRED_8x8_DC_TOP] = predict_8x8_dc_top_mmxext;
  pf[I_PRED_8x8_DC_128] = predict_8x8_dc_128_mmxext;
}
 void xavs_predict_8x8c_init_mmxext( xavs_predict_t pf[7] )
  {
   pf[I_PRED_CHROMA_V] = predict_8x8c_v_mmxext;
   pf[I_PRED_CHROMA_H] = predict_8x8c_h_mmxext;
   pf[I_PRED_CHROMA_P] = predict_8x8c_p;
   pf[I_PRED_CHROMA_DC] = predict_8x8c_dc_mmxext;
   pf[I_PRED_CHROMA_DC_LEFT] = predict_8x8c_dc_left_mmxext;
   pf[I_PRED_CHROMA_DC_TOP] = predict_8x8c_dc_top_mmxext;
   pf[I_PRED_CHROMA_DC_128] = predict_8x8c_dc_128_mmxext;
  }

 void xavs_predict_8x8_init_sse2(xavs_predict_t pf[12])
 {
	pf[I_PRED_8x8_DDL] = predict_8x8_ddl_sse2;
	pf[I_PRED_8x8_DDR] = predict_8x8_ddr_sse2;
 }
