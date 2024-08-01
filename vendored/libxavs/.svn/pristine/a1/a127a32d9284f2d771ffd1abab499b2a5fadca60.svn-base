/*****************************************************************************
 * dct.c: xavs encoder library
 *****************************************************************************
 * Copyright (C)  2009 xavs project
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

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include <stdlib.h>
#include <stdarg.h>

#include "dct.h"
#include "common/common.h"


/*
void x264_sub8x8_dct_mmx( int16_t dct[4][4][4], uint8_t *pix1, uint8_t *pix2 )
{
    x264_sub4x4_dct_mmx( dct[0], &pix1[0], &pix2[0] );
    x264_sub4x4_dct_mmx( dct[1], &pix1[4], &pix2[4] );
    x264_sub4x4_dct_mmx( dct[2], &pix1[4*FENC_STRIDE+0], &pix2[4*FDEC_STRIDE+0] );
    x264_sub4x4_dct_mmx( dct[3], &pix1[4*FENC_STRIDE+4], &pix2[4*FDEC_STRIDE+4] );
}

void x264_sub16x16_dct_mmx( int16_t dct[16][4][4], uint8_t *pix1, uint8_t *pix2 )
{
    x264_sub8x8_dct_mmx( &dct[ 0], &pix1[0], &pix2[0] );
    x264_sub8x8_dct_mmx( &dct[ 4], &pix1[8], &pix2[8] );
    x264_sub8x8_dct_mmx( &dct[ 8], &pix1[8*FENC_STRIDE+0], &pix2[8*FDEC_STRIDE+0] );
    x264_sub8x8_dct_mmx( &dct[12], &pix1[8*FENC_STRIDE+8], &pix2[8*FDEC_STRIDE+8] );
}
*/


/****************************************************************************
 * addXxX_idct:
 ****************************************************************************/

/*
void x264_add8x8_idct_mmx( uint8_t *p_dst, int16_t dct[4][4][4] )
{
    x264_add4x4_idct_mmx( p_dst,                   dct[0] );
    x264_add4x4_idct_mmx( &p_dst[4],               dct[1] );
    x264_add4x4_idct_mmx( &p_dst[4*FDEC_STRIDE+0], dct[2] );
    x264_add4x4_idct_mmx( &p_dst[4*FDEC_STRIDE+4], dct[3] );
}

void x264_add16x16_idct_mmx( uint8_t *p_dst, int16_t dct[16][4][4] )
{
    x264_add8x8_idct_mmx( &p_dst[0],               &dct[0] );
    x264_add8x8_idct_mmx( &p_dst[8],               &dct[4] );
    x264_add8x8_idct_mmx( &p_dst[8*FDEC_STRIDE],   &dct[8] );
    x264_add8x8_idct_mmx( &p_dst[8*FDEC_STRIDE+8], &dct[12] );                      idctµÄmmx²¿·Ö
}
*/
/***********************
 * dct8/idct8 functions
 ***********************/

extern void xavs_sub8x8_dct8_sse2( int16_t dct[8][8],       uint8_t *pix1, uint8_t *pix2 ,int16_t *tmp);
extern void xavs_sub16x16_dct8_sse2( int16_t dct[4][8][8],  uint8_t *pix1, uint8_t *pix2 ,int16_t *tmp);

void xavs_sub16x16_dct8_sse2( int16_t dct[4][8][8],  uint8_t *pix1, uint8_t *pix2 , int16_t *tmp)
{
    xavs_sub8x8_dct8_sse2( dct[0],  &pix1[0],               &pix2[0],              &tmp[0]);
    xavs_sub8x8_dct8_sse2( dct[1],  &pix1[8],               &pix2[8],              &tmp[1]);
    xavs_sub8x8_dct8_sse2( dct[2],  &pix1[8*FENC_STRIDE+0], &pix2[8*FDEC_STRIDE+0],&tmp[2]);
    xavs_sub8x8_dct8_sse2( dct[3],  &pix1[8*FENC_STRIDE+8], &pix2[8*FDEC_STRIDE+8],&tmp[3]);
}
/*static inline void pixel_sub_8x8( int16_t dct[8][8], uint8_t *pix1,  uint8_t *pix2 )
{
    int y, x;
    for( y = 0; y < 8; y++ )
    {
        for( x = 0; x <8; x++ )
        {
            diff[x + y*8] = pix1[x] - pix2[x];
        }
        pix1 += i_pix1;
        pix2 += i_pix2;
    }
}*/

/*void xavs_dct_init_sse2( int cpu, xavs_dct_function_t *dctf )
{
    
    dctf->sub8x8_dct8   = xavs_sub8x8_dct8_sse2;
   // dctf->add8x8_idct8  = add8x8_idct8;
    
    dctf->sub16x16_dct8  = xavs_sub16x16_dct8_sse2;
    //dctf->add16x16_idct8 = add16x16_idct8;
}*/

extern void xavs_add8x8_idct8_sse2  (uint8_t *dst, int16_t dct[8][8],   int16_t *tmp);
extern void xavs_add16x16_idct8_sse2(uint8_t *dst, int16_t dct[4][8][8],int16_t *tmp);

void xavs_add16x16_idct8_sse2( uint8_t *p_dst, int16_t dct[4][8][8],int16_t *tmp )
{
    xavs_add8x8_idct8_sse2( p_dst,                 dct[0] , tmp[0]);
    xavs_add8x8_idct8_sse2( p_dst+8,               dct[1] , tmp[1]);
    xavs_add8x8_idct8_sse2( p_dst+8*FDEC_STRIDE,   dct[2] , tmp[2]);
    xavs_add8x8_idct8_sse2( p_dst+8*FDEC_STRIDE+8, dct[3] , tmp[3]);                 
}

//#else // ARCH_X86

/*
void x264_pixel_sub_8x8_mmx( int16_t *diff, uint8_t *pix1, uint8_t *pix2 );
void x264_pixel_add_8x8_mmx( uint8_t *pix, uint16_t *diff );
void x264_transpose_8x8_mmx( int16_t src[8][8] );
void x264_ydct8_mmx( int16_t dct[8][8] );
void x264_yidct8_mmx( int16_t dct[8][8] );
*/
//lsp060308
//inline 
/*
void x264_sub8x8_dct8_mmx( int16_t dct[8][8], uint8_t *pix1, uint8_t *pix2 )
{
    x264_pixel_sub_8x8_mmx( (int16_t *)dct, pix1, pix2 );
    x264_ydct8_mmx( dct );
    x264_transpose_8x8_mmx( dct );
    x264_ydct8_mmx( dct );
}

void x264_sub16x16_dct8_mmx( int16_t dct[4][8][8], uint8_t *pix1, uint8_t *pix2 )
{
    x264_sub8x8_dct8_mmx( dct[0], pix1,                 pix2 );
    x264_sub8x8_dct8_mmx( dct[1], pix1+8,               pix2+8 );
    x264_sub8x8_dct8_mmx( dct[2], pix1+8*FENC_STRIDE,   pix2+8*FDEC_STRIDE );
    x264_sub8x8_dct8_mmx( dct[3], pix1+8*FENC_STRIDE+8, pix2+8*FDEC_STRIDE+8 );
}

//lsp060308
//inline 
void x264_add8x8_idct8_mmx( uint8_t *dst, int16_t dct[8][8] )
{
    dct[0][0] += 32;
    x264_yidct8_mmx( dct );
    x264_transpose_8x8_mmx( dct );
    x264_yidct8_mmx( dct );
    x264_pixel_add_8x8_mmx( dst, (uint16_t *)dct ); // including >>6 at the end
}

void x264_add16x16_idct8_mmx( uint8_t *dst, int16_t dct[4][8][8] )
{
    x264_add8x8_idct8_mmx( dst,                 dct[0] );
    x264_add8x8_idct8_mmx( dst+8,               dct[1] );
    x264_add8x8_idct8_mmx( dst+8*FDEC_STRIDE,   dct[2] );
    x264_add8x8_idct8_mmx( dst+8*FDEC_STRIDE+8, dct[3] );
}
*/
//#endif
