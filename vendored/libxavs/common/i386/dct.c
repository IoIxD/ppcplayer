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

#include "common.h"
/*
ifdef HAVE_MMXEXT
#   include "x86/dct.h"
#endif
#ifdef ARCH_PPC
#   include "ppc/dct.h"
#endif
*/
static inline void pixel_sub_wxh( int16_t *diff, int i_size,
                                  uint8_t *pix1, int i_pix1, uint8_t *pix2, int i_pix2 )
{
    int y, x;
    for( y = 0; y < i_size; y++ )
    {
        for( x = 0; x < i_size; x++ )
        {
            diff[x + y*i_size] = pix1[x] - pix2[x];
        }
        pix1 += i_pix1;
        pix2 += i_pix2;
    }
}
//dct_sse2Ê±ºòÓÃ
static inline void pixel_sub_8x8( int16_t dct[8][8], uint8_t *pix1,  uint8_t *pix2 )
{
    int y, x;
    for( y = 0; y < 8; y++ )
    {
        for( x = 0; x <8; x++ )
        {
            dct[x][y] = pix1[x] - pix2[x];
        }
        pix1 += FENC_STRIDE;
        pix2 += FDEC_STRIDE;
    }
}

/****************************************************************************
 * 8x8 transform:
 ****************************************************************************/
#define DCT8_Horizontal_1D {\
    const int s07 = SRC(0) + SRC(7);\
    const int s16 = SRC(1) + SRC(6);\
    const int s25 = SRC(2) + SRC(5);\
    const int s34 = SRC(3) + SRC(4);\
    const int a0 = s07 + s34;\
    const int a1 = s16 + s25;\
    const int a2 = s07 - s34;\
    const int a3 = s16 - s25;\
    const int d07 = SRC(0) - SRC(7);\
    const int d16 = SRC(1) - SRC(6);\
    const int d25 = SRC(2) - SRC(5);\
    const int d34 = SRC(3) - SRC(4);\
    const int a4 = ((d07 - d34)<<1) + d07;\
    const int a5 = ((d16 + d25)<<1) + d16;\
    const int a6 = ((d16 - d25)<<1) - d25;\
    const int a7 = ((d07 + d34)<<1) + d34;\
    DST(0) =  (a0 + a1)<<3;\
    DST(4) =  (a0 - a1)<<3;\
    DST(2) =  (a2*10) + (a3<<2);\
    DST(6) =  (a2<<2) - (a3*10);\
    DST(1) =  ((a4 + a5 + a7)  << 1) + a5;\
    DST(3) =  ((a4 - a5 + a6)  << 1) + a4;\
    DST(5) =  ((-a5 - a6 + a7) << 1) + a7;\
    DST(7) =  ((a4 - a6 - a7)  << 1) - a6; \
}
	
#define DCT8_Vertical_1D {\
    const int s07 = SRC(0) + SRC(7);\
    const int s16 = SRC(1) + SRC(6);\
    const int s25 = SRC(2) + SRC(5);\
    const int s34 = SRC(3) + SRC(4);\
    const int a0 = s07 + s34;\
    const int a1 = s16 + s25;\
    const int a2 = s07 - s34;\
    const int a3 = s16 - s25;\
    const int d07 = SRC(0) - SRC(7);\
    const int d16 = SRC(1) - SRC(6);\
    const int d25 = SRC(2) - SRC(5);\
    const int d34 = SRC(3) - SRC(4);\
    const int a4 = ((d07 - d34)<<1) + d07;\
    const int a5 = ((d16 + d25)<<1) + d16;\
    const int a6 = ((d16 - d25)<<1) - d25;\
    const int a7 = ((d07 + d34)<<1) + d34;\
    DST(0) =  (((a0 + a1)<<3) + (1<<4))>>5;\
    DST(4) =  (((a0 - a1)<<3) + (1<<4))>>5;\
    DST(2) =  ((a2*10) + (a3<<2) + (1<<4))>>5;\
    DST(6) =  ((a2<<2) - (a3*10) + (1<<4))>>5;\
    DST(1) =  (((a4 + a5 + a7)<<1) + a5 + (1<<4))>>5;\
    DST(3) =  (((a4 - a5 + a6)<<1) + a4 + (1<<4))>>5;\
    DST(5) =  (((a7 - a5 - a6)<<1) + a7 + (1<<4))>>5;\
    DST(7) =  (((a4 - a6 - a7)<<1) - a6 + (1<<4))>>5; \
}

extern void xavs_sub8x8_dct8_sse2( int16_t dct[8][8],       uint8_t *pix1, uint8_t *pix2, int16_t *tmp);
extern void xavs_sub16x16_dct8_sse2( int16_t dct[4][8][8],  uint8_t *pix1, uint8_t *pix2, int16_t *tmp);

static void sub8x8_dct8( int16_t dct[8][8], uint8_t *pix1, uint8_t *pix2 )
{
    int i;   
#ifdef HAVE_SSE2
if(1)
  {
	int16_t tmp[8][8];
	xavs_sub8x8_dct8_sse2( (int16_t*) dct[8][8],  pix1, pix2 ,tmp);
  }

  #else

  {
   pixel_sub_wxh( (int16_t*)dct, 8, pix1, FENC_STRIDE, pix2, FDEC_STRIDE );
 
   #define SRC(x) dct[i][x]
   #define DST(x) dct[i][x]
     for( i = 0; i < 8; i++ )
        DCT8_Horizontal_1D
   #undef SRC
   #undef DST

   #define SRC(x) dct[x][i]
   #define DST(x) dct[x][i]
     for( i = 0; i < 8; i++ )
        DCT8_Vertical_1D
   #undef SRC
   #undef DST
  }

#endif


}

static void sub16x16_dct8( int16_t dct[4][8][8], uint8_t *pix1, uint8_t *pix2 )
{
    sub8x8_dct8( dct[0], &pix1[0],               &pix2[0] );
    sub8x8_dct8( dct[1], &pix1[8],               &pix2[8] );
    sub8x8_dct8( dct[2], &pix1[8*FENC_STRIDE+0], &pix2[8*FDEC_STRIDE+0] );
    sub8x8_dct8( dct[3], &pix1[8*FENC_STRIDE+8], &pix2[8*FDEC_STRIDE+8] );
}

#define IDCT8_Horizontal_1D {\
     int a0 = SRC(0);\
     int a1 = SRC(4);\
     int a2 = SRC(2);\
     int a3 = SRC(6);\
     int a4 = SRC(1);\
     int a5 = SRC(3);\
     int a6 = SRC(5);\
     int a7 = SRC(7);\
     int b0 = ((a4 - a7)<<1) + a4;\
     int b1 = ((a5 + a6)<<1) + a5;\
     int b2 = ((a5 - a6)<<1) - a6;\
     int b3 = ((a4 + a7)<<1) + a7;\
     int b4 = ((b0  + b1 + b3)<<1) + b1;\
     int b5 = ((b0  - b1 + b2)<<1) + b0;\
     int b6 = ((-b1 - b2 + b3)<<1) + b3;\
     int b7 = ((b0  - b2 - b3)<<1) - b2;\
     int t0 = (a2*10)+(a3<<2);\
     int t1 = (a0 + a1)<<3;\
     a3 = (a2<<2)-(a3*10);\
     a1 = (a0 - a1)<<3;	 \
     a2 = t0;\
     a0 = t1;\
     b0=a0 + a2;\
     b1=a1 + a3;\
     b2=a1 - a3;\
     b3=a0 - a2;\
     a0 = ((b0+b4)+(1<<2));\
     a1 = ((b1+b5)+(1<<2));\
     a2 = ((b2+b6)+(1<<2));\
     a3 = ((b3+b7)+(1<<2));\
     a7 = ((b0-b4)+(1<<2));\
     a6 = ((b1-b5)+(1<<2));\
     a5 = ((b2-b6)+(1<<2));\
     a4 = ((b3-b7)+(1<<2));\
     a0  = ((a0 < -32768)? -32768: ((a0 > 32767)? 32767 : a0))>>3; \
     a1  = ((a1 < -32768)? -32768: ((a1 > 32767)? 32767 : a1))>>3; \
     a2  = ((a2 < -32768)? -32768: ((a2 > 32767)? 32767 : a2))>>3; \
     a3  = ((a3 < -32768)? -32768: ((a3 > 32767)? 32767 : a3))>>3; \
     a7  = ((a7 < -32768)? -32768: ((a7 > 32767)? 32767 : a7))>>3; \
     a6  = ((a6 < -32768)? -32768: ((a6 > 32767)? 32767 : a6))>>3; \
     a5  = ((a5 < -32768)? -32768: ((a5 > 32767)? 32767 : a5))>>3; \
     a4  = ((a4 < -32768)? -32768: ((a4 > 32767)? 32767 : a4))>>3; \
     DST(0,a0);\
     DST(1,a1);\
     DST(2,a2);\
     DST(3,a3);\
     DST(7,a7);\
     DST(6,a6);\
     DST(5,a5);\
     DST(4,a4);\
}
	
#define IDCT8_Vertical_1D {\
     int a0 = SRC(0);\
     int a1 = SRC(4);\
     int a2 = SRC(2);\
     int a3 = SRC(6);\
     int a4 = SRC(1);\
     int a5 = SRC(3);\
     int a6 = SRC(5);\
     int a7 = SRC(7);\
     int b0 = ((a4 - a7)<<1) + a4;\
     int b1 = ((a5 + a6)<<1) + a5;\
     int b2 = ((a5 - a6)<<1) - a6;\
     int b3 = ((a4 + a7)<<1) + a7;\
     int b4 = ((b0  + b1 + b3)<<1) + b1;\
     int b5 = ((b0  - b1 + b2)<<1) + b0;\
     int b6 = ((-b1 - b2 + b3)<<1) + b3;\
     int b7 = ((b0  - b2 - b3)<<1) - b2;\
     int t0 = (a2*10)+(a3<<2);\
     int t1 = (a0 + a1)<<3;\
     a3 = (a2<<2)-(a3*10);\
     a1 = (a0 - a1)<<3;	 \
     a2 = t0;\
     a0 = t1;\
     b0=a0 + a2;\
     b1=a1 + a3;\
     b2=a1 - a3;\
     b3=a0 - a2;\
     a0 = ((b0+b4)+ 64);\
     a1 = ((b1+b5)+ 64);\
     a2 = ((b2+b6)+ 64);\
     a3 = ((b3+b7)+ 64);\
     a7 = ((b0-b4)+ 64);\
     a6 = ((b1-b5)+ 64);\
     a5 = ((b2-b6)+ 64);\
     a4 = ((b3-b7)+ 64);\
     a0 = ((a0 < -32768)? -32768: ((a0 > 32767)? 32767 : a0))>>7; \
     a1 = ((a1 < -32768)? -32768: ((a1 > 32767)? 32767 : a1))>>7; \
     a2 = ((a2 < -32768)? -32768: ((a2 > 32767)? 32767 : a2))>>7;\
     a3 = ((a3 < -32768)? -32768: ((a3 > 32767)? 32767 : a3))>>7;\
     a7 = ((a7 < -32768)? -32768: ((a7 > 32767)? 32767 : a7))>>7;\
     a6 = ((a6 < -32768)? -32768: ((a6 > 32767)? 32767 : a6))>>7;\
     a5 = ((a5 < -32768)? -32768: ((a5 > 32767)? 32767 : a5))>>7;\
     a4 = ((a4 < -32768)? -32768: ((a4 > 32767)? 32767 : a4))>>7;\
     DST(0,a0);\
     DST(1,a1);\
     DST(2,a2);\
     DST(3,a3);\
     DST(7,a7);\
     DST(6,a6);\
     DST(5,a5);\
     DST(4,a4);\
}

static void add8x8_idct8( uint8_t *dst, int16_t dct[8][8] )
{
    int i;
#ifdef HAVE_SSE2
    
   if(1)
   {
	    int16_t tmp[8][8];
      
        xavs_add8x8_idct8_sse2  ( dst, (int16_t*) dct[8][8] ,   tmp);
	
   }

#else

  {
   #define SRC(x)     dct[i][x]
   #define DST(x,rhs) dct[i][x] = (rhs)
     for( i = 0; i < 8; i++ )
        IDCT8_Horizontal_1D
   #undef SRC
   #undef DST

   #define SRC(x)     dct[x][i]
   #define DST(x,rhs) dst[i + x*FDEC_STRIDE] = xavs_clip_uint8( dst[i + x*FDEC_STRIDE] + (rhs) );
     for( i = 0; i < 8; i++ )
       IDCT8_Vertical_1D
   #undef SRC
   #undef DST
  }

#endif
}

static void add16x16_idct8( uint8_t *dst, int16_t dct[4][8][8] )
{
    add8x8_idct8( &dst[0],               dct[0] );
    add8x8_idct8( &dst[8],               dct[1] );
    add8x8_idct8( &dst[8*FDEC_STRIDE+0], dct[2] );
    add8x8_idct8( &dst[8*FDEC_STRIDE+8], dct[3] );
}


/****************************************************************************
 * xavs_dct_init:
 ****************************************************************************/
//extern void xavs_sub8x8_dct8_sse2( int16_t dct[8][8],  uint8_t *pix1, uint8_t *pix2 );
//extern void xavs_sub16x16_dct8_sse2( int16_t dct[4][8][8], uint8_t *pix1, uint8_t *pix2 );

void xavs_dct_init( int cpu, xavs_dct_function_t *dctf )
{
    //dctf->pixel_sub_8x8 = pixel_sub_8x8;
    dctf->sub8x8_dct8   = sub8x8_dct8;
    dctf->add8x8_idct8  = add8x8_idct8;
    
    dctf->sub16x16_dct8  = sub16x16_dct8;
    dctf->add16x16_idct8 = add16x16_idct8;


/*#ifdef HAVE_MMXEXT
    if( cpu&xavs_CPU_MMX )
    {

#ifndef ARCH_X86_64
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_mmx;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_mmx;

        dctf->add8x8_idct8  = xavs_add8x8_idct8_mmx;
        dctf->add16x16_idct8= xavs_add16x16_idct8_mmx;
#endif
    }
    if( cpu&XAVS_CPU_SSE2 )
    {
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_sse2;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_sse2;
        dctf->add8x8_idct8  = xavs_add8x8_idct8_sse2;
        dctf->add16x16_idct8= xavs_add16x16_idct8_sse2;
    }

    if( cpu&XAVS_CPU_SSSE3 )
    {
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_ssse3;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_ssse3;
    }
#endif

#if defined(HAVE_SSE2) && defined(ARCH_X86_64)
    if( cpu&xavs_CPU_SSE2 )
    {
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_sse2;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_sse2;

        dctf->add8x8_idct8  = xavs_add8x8_idct8_sse2;
        dctf->add16x16_idct8= xavs_add16x16_idct8_sse2;
    }
#endif
*/
/* #ifdef HAVE_SSE2
    
if( cpu&XAVS_CPU_SSE2 )
 {
	   
       dctf->sub8x8_dct8   = xavs_sub8x8_dct8_sse2;
       dctf->sub16x16_dct8 = xavs_sub16x16_dct8_sse2;
        //dctf->add8x8_idct8  = xavs_add8x8_idct8_sse2;
        //dctf->add16x16_idct8= xavs_add16x16_idct8_sse2;
 }

    /*if( cpu&XAVS_CPU_SSSE3 )
    {
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_ssse3;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_ssse3;
    }
 #endif*/

/*#if defined(HAVE_SSE2) && defined(ARCH_X86_64)
    if( cpu&XAVS_CPU_SSE2 )
    {
        dctf->sub8x8_dct8   = xavs_sub8x8_dct8_sse2;
        dctf->sub16x16_dct8 = xavs_sub16x16_dct8_sse2;

        dctf->add8x8_idct8  = xavs_add8x8_idct8_sse2;
        dctf->add16x16_idct8= xavs_add16x16_idct8_sse2;
    }
#endif*/
}

// gcc pessimizes multi-dimensional arrays here, even with constant indices
#define ZIG(i,y,x) level[i] = dct[0][y*8+x];
#define ZIGZAG8_FRAME\
    ZIG( 0,0,0) ZIG( 1,0,1) ZIG( 2,1,0) ZIG( 3,2,0)\
    ZIG( 4,1,1) ZIG( 5,0,2) ZIG( 6,0,3) ZIG( 7,1,2)\
    ZIG( 8,2,1) ZIG( 9,3,0) ZIG(10,4,0) ZIG(11,3,1)\
    ZIG(12,2,2) ZIG(13,1,3) ZIG(14,0,4) ZIG(15,0,5)\
    ZIG(16,1,4) ZIG(17,2,3) ZIG(18,3,2) ZIG(19,4,1)\
    ZIG(20,5,0) ZIG(21,6,0) ZIG(22,5,1) ZIG(23,4,2)\
    ZIG(24,3,3) ZIG(25,2,4) ZIG(26,1,5) ZIG(27,0,6)\
    ZIG(28,0,7) ZIG(29,1,6) ZIG(30,2,5) ZIG(31,3,4)\
    ZIG(32,4,3) ZIG(33,5,2) ZIG(34,6,1) ZIG(35,7,0)\
    ZIG(36,7,1) ZIG(37,6,2) ZIG(38,5,3) ZIG(39,4,4)\
    ZIG(40,3,5) ZIG(41,2,6) ZIG(42,1,7) ZIG(43,2,7)\
    ZIG(44,3,6) ZIG(45,4,5) ZIG(46,5,4) ZIG(47,6,3)\
    ZIG(48,7,2) ZIG(49,7,3) ZIG(50,6,4) ZIG(51,5,5)\
    ZIG(52,4,6) ZIG(53,3,7) ZIG(54,4,7) ZIG(55,5,6)\
    ZIG(56,6,5) ZIG(57,7,4) ZIG(58,7,5) ZIG(59,6,6)\
    ZIG(60,5,7) ZIG(61,6,7) ZIG(62,7,6) ZIG(63,7,7)

#define ZIGZAG8_FIELD\
    ZIG( 0,0,0) ZIG( 1,1,0) ZIG( 2,2,0) ZIG( 3,0,1)\
    ZIG( 4,3,0) ZIG( 5,4,0) ZIG( 6,1,1) ZIG( 7,2,1)\
    ZIG( 8,5,0) ZIG( 9,6,0) ZIG(10,3,1) ZIG(11,0,2)\
    ZIG(12,1,2) ZIG(13,7,0) ZIG(14,4,1) ZIG(15,2,2)\
    ZIG(16,0,3) ZIG(17,5,1) ZIG(18,6,1) ZIG(19,3,2)\
    ZIG(20,1,3) ZIG(21,2,3) ZIG(22,0,4) ZIG(23,7,1)\
    ZIG(24,4,2) ZIG(25,1,4) ZIG(26,5,2) ZIG(27,3,3)\
    ZIG(28,2,4) ZIG(29,6,2) ZIG(30,4,3) ZIG(31,3,4)\
    ZIG(32,0,5) ZIG(33,1,5) ZIG(34,7,2) ZIG(35,5,3)\
    ZIG(36,4,4) ZIG(37,2,5) ZIG(38,0,6) ZIG(39,3,5)\
    ZIG(40,6,3) ZIG(41,5,4) ZIG(42,1,6) ZIG(43,2,6)\
    ZIG(44,4,5) ZIG(45,7,3) ZIG(46,6,4) ZIG(47,3,6)\
    ZIG(48,5,5) ZIG(49,7,4) ZIG(50,4,6) ZIG(51,6,5)\
    ZIG(52,5,6) ZIG(53,7,5) ZIG(54,6,6) ZIG(55,0,7)\
    ZIG(56,7,6) ZIG(57,1,7) ZIG(58,2,7) ZIG(59,3,7)\
    ZIG(60,4,7) ZIG(61,5,7) ZIG(62,6,7) ZIG(63,7,7)


static void zigzag_scan_8x8_frame( int16_t level[64], int16_t dct[8][8] )
{
    ZIGZAG8_FRAME
}

static void zigzag_scan_8x8_field( int16_t level[64], int16_t dct[8][8] )
{
    ZIGZAG8_FIELD
}


#undef ZIG
#define ZIG(i,y,x) {\
    int oe = x+y*FENC_STRIDE;\
    int od = x+y*FDEC_STRIDE;\
    level[i] = p_src[oe] - p_dst[od];\
    nz |= level[i];\
}
#define COPY8x8\
    *(uint64_t*)(p_dst+0*FDEC_STRIDE) = *(uint64_t*)(p_src+0*FENC_STRIDE);\
    *(uint64_t*)(p_dst+1*FDEC_STRIDE) = *(uint64_t*)(p_src+1*FENC_STRIDE);\
    *(uint64_t*)(p_dst+2*FDEC_STRIDE) = *(uint64_t*)(p_src+2*FENC_STRIDE);\
    *(uint64_t*)(p_dst+3*FDEC_STRIDE) = *(uint64_t*)(p_src+3*FENC_STRIDE);\
    *(uint64_t*)(p_dst+4*FDEC_STRIDE) = *(uint64_t*)(p_src+4*FENC_STRIDE);\
    *(uint64_t*)(p_dst+5*FDEC_STRIDE) = *(uint64_t*)(p_src+5*FENC_STRIDE);\
    *(uint64_t*)(p_dst+6*FDEC_STRIDE) = *(uint64_t*)(p_src+6*FENC_STRIDE);\
    *(uint64_t*)(p_dst+7*FDEC_STRIDE) = *(uint64_t*)(p_src+7*FENC_STRIDE);


static int zigzag_sub_8x8_frame( int16_t level[64], const uint8_t *p_src, uint8_t *p_dst )
{
    int nz = 0;
    ZIGZAG8_FRAME
    COPY8x8
    return !!nz;
}

static int zigzag_sub_8x8_field( int16_t level[64], const uint8_t *p_src, uint8_t *p_dst )
{
    int nz = 0;
    ZIGZAG8_FIELD
    COPY8x8
    return !!nz;
}
#undef ZIG

void xavs_zigzag_init( int cpu, xavs_zigzag_function_t *pf, int b_interlaced )
{
    if( b_interlaced )
    {
        pf->scan_8x8   = zigzag_scan_8x8_field;
        pf->sub_8x8    = zigzag_sub_8x8_field;
    }
    else
    {
        pf->scan_8x8   = zigzag_scan_8x8_frame;
        pf->sub_8x8    = zigzag_sub_8x8_frame;
#ifdef HAVE_MMX
        if( cpu&XAVS_CPU_MMXEXT )
            pf->scan_8x8 = xavs_zigzag_scan_8x8_frame_mmxext;
        if( cpu&XAVS_CPU_SSE2_IS_FAST )
            pf->scan_8x8 = xavs_zigzag_scan_8x8_frame_sse2;
        if( cpu&XAVS_CPU_SSSE3 )
        {
            pf->scan_8x8 = xavs_zigzag_scan_8x8_frame_ssse3;
        }
#endif
    }
}

/*
#define ZIG(i,y,x) level[i] = dct[y][x];
void scan_zigzag_8x8full( int level[64], int16_t dct[8][8] )
{
    ZIG( 0,0,0) ZIG( 1,0,1) ZIG( 2,1,0) ZIG( 3,2,0)
    ZIG( 4,1,1) ZIG( 5,0,2) ZIG( 6,0,3) ZIG( 7,1,2)
    ZIG( 8,2,1) ZIG( 9,3,0) ZIG(10,4,0) ZIG(11,3,1)
    ZIG(12,2,2) ZIG(13,1,3) ZIG(14,0,4) ZIG(15,0,5)
    ZIG(16,1,4) ZIG(17,2,3) ZIG(18,3,2) ZIG(19,4,1)
    ZIG(20,5,0) ZIG(21,6,0) ZIG(22,5,1) ZIG(23,4,2)
    ZIG(24,3,3) ZIG(25,2,4) ZIG(26,1,5) ZIG(27,0,6)
    ZIG(28,0,7) ZIG(29,1,6) ZIG(30,2,5) ZIG(31,3,4)
    ZIG(32,4,3) ZIG(33,5,2) ZIG(34,6,1) ZIG(35,7,0)
    ZIG(36,7,1) ZIG(37,6,2) ZIG(38,5,3) ZIG(39,4,4)
    ZIG(40,3,5) ZIG(41,2,6) ZIG(42,1,7) ZIG(43,2,7)
    ZIG(44,3,6) ZIG(45,4,5) ZIG(46,5,4) ZIG(47,6,3)
    ZIG(48,7,2) ZIG(49,7,3) ZIG(50,6,4) ZIG(51,5,5)
    ZIG(52,4,6) ZIG(53,3,7) ZIG(54,4,7) ZIG(55,5,6)
    ZIG(56,6,5) ZIG(57,7,4) ZIG(58,7,5) ZIG(59,6,6)
    ZIG(60,5,7) ZIG(61,6,7) ZIG(62,7,6) ZIG(63,7,7)
}
#undef ZIG
 */

#define ZIG(i,y,x) level[i] = dct[y][x];
void scan_zigzag_8x8full( int level[64], int16_t dct[8][8] )
#ifdef HAVE_SSE2
    {
    ZIG( 0,0,0) ZIG( 1,1,0) ZIG( 2,0,1) ZIG( 3,0,2)
    ZIG( 4,1,1) ZIG( 5,2,0) ZIG( 6,3,0) ZIG( 7,2,1)
    ZIG( 8,1,2) ZIG( 9,0,3) ZIG(10,0,4) ZIG(11,1,3)
    ZIG(12,2,2) ZIG(13,3,1) ZIG(14,4,0) ZIG(15,5,0)
    ZIG(16,4,1) ZIG(17,3,2) ZIG(18,2,3) ZIG(19,1,4)
    ZIG(20,0,5) ZIG(21,0,6) ZIG(22,1,5) ZIG(23,2,4)
    ZIG(24,3,3) ZIG(25,4,2) ZIG(26,5,1) ZIG(27,6,0)
    ZIG(28,7,0) ZIG(29,6,1) ZIG(30,5,2) ZIG(31,4,3)
    ZIG(32,3,4) ZIG(33,2,5) ZIG(34,1,6) ZIG(35,0,7)
    ZIG(36,1,7) ZIG(37,2,6) ZIG(38,3,5) ZIG(39,4,4)
    ZIG(40,5,3) ZIG(41,6,2) ZIG(42,7,1) ZIG(43,7,2)
    ZIG(44,6,3) ZIG(45,5,4) ZIG(46,4,5) ZIG(47,3,6)
    ZIG(48,2,7) ZIG(49,3,7) ZIG(50,4,6) ZIG(51,5,5)
    ZIG(52,6,4) ZIG(53,7,3) ZIG(54,7,4) ZIG(55,6,5)
    ZIG(56,5,6) ZIG(57,4,7) ZIG(58,5,7) ZIG(59,6,6)
    ZIG(60,7,5) ZIG(61,7,6) ZIG(62,6,7) ZIG(63,7,7)
    }
#else
    {
    ZIG( 0,0,0) ZIG( 1,0,1) ZIG( 2,1,0) ZIG( 3,2,0)
    ZIG( 4,1,1) ZIG( 5,0,2) ZIG( 6,0,3) ZIG( 7,1,2)
    ZIG( 8,2,1) ZIG( 9,3,0) ZIG(10,4,0) ZIG(11,3,1)
    ZIG(12,2,2) ZIG(13,1,3) ZIG(14,0,4) ZIG(15,0,5)
    ZIG(16,1,4) ZIG(17,2,3) ZIG(18,3,2) ZIG(19,4,1)
    ZIG(20,5,0) ZIG(21,6,0) ZIG(22,5,1) ZIG(23,4,2)
    ZIG(24,3,3) ZIG(25,2,4) ZIG(26,1,5) ZIG(27,0,6)
    ZIG(28,0,7) ZIG(29,1,6) ZIG(30,2,5) ZIG(31,3,4)
    ZIG(32,4,3) ZIG(33,5,2) ZIG(34,6,1) ZIG(35,7,0)
    ZIG(36,7,1) ZIG(37,6,2) ZIG(38,5,3) ZIG(39,4,4)
    ZIG(40,3,5) ZIG(41,2,6) ZIG(42,1,7) ZIG(43,2,7)
    ZIG(44,3,6) ZIG(45,4,5) ZIG(46,5,4) ZIG(47,6,3)
    ZIG(48,7,2) ZIG(49,7,3) ZIG(50,6,4) ZIG(51,5,5)
    ZIG(52,4,6) ZIG(53,3,7) ZIG(54,4,7) ZIG(55,5,6)
    ZIG(56,6,5) ZIG(57,7,4) ZIG(58,7,5) ZIG(59,6,6)
    ZIG(60,5,7) ZIG(61,6,7) ZIG(62,7,6) ZIG(63,7,7)
    }
#endif
#undef ZIG

#define ZIG(i,y,x) {\
    int oe = x+y*FENC_STRIDE;\
    int od = x+y*FDEC_STRIDE;\
    level[i] = p_src[oe] - p_dst[od];\
    p_dst[od] = p_src[oe];\
}

void sub_zigzag_8x8full( int level[64], const uint8_t *p_src, uint8_t *p_dst )
{
    ZIG( 0,0,0) ZIG( 1,0,1) ZIG( 2,1,0) ZIG( 3,2,0)
    ZIG( 4,1,1) ZIG( 5,0,2) ZIG( 6,0,3) ZIG( 7,1,2)
    ZIG( 8,2,1) ZIG( 9,3,0) ZIG(10,4,0) ZIG(11,3,1)
    ZIG(12,2,2) ZIG(13,1,3) ZIG(14,0,4) ZIG(15,0,5)
    ZIG(16,1,4) ZIG(17,2,3) ZIG(18,3,2) ZIG(19,4,1)
    ZIG(20,5,0) ZIG(21,6,0) ZIG(22,5,1) ZIG(23,4,2)
    ZIG(24,3,3) ZIG(25,2,4) ZIG(26,1,5) ZIG(27,0,6)
    ZIG(28,0,7) ZIG(29,1,6) ZIG(30,2,5) ZIG(31,3,4)
    ZIG(32,4,3) ZIG(33,5,2) ZIG(34,6,1) ZIG(35,7,0)
    ZIG(36,7,1) ZIG(37,6,2) ZIG(38,5,3) ZIG(39,4,4)
    ZIG(40,3,5) ZIG(41,2,6) ZIG(42,1,7) ZIG(43,2,7)
    ZIG(44,3,6) ZIG(45,4,5) ZIG(46,5,4) ZIG(47,6,3)
    ZIG(48,7,2) ZIG(49,7,3) ZIG(50,6,4) ZIG(51,5,5)
    ZIG(52,4,6) ZIG(53,3,7) ZIG(54,4,7) ZIG(55,5,6)
    ZIG(56,6,5) ZIG(57,7,4) ZIG(58,7,5) ZIG(59,6,6)
    ZIG(60,5,7) ZIG(61,6,7) ZIG(62,7,6) ZIG(63,7,7)

}

#undef ZIG


