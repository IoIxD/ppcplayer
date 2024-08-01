;*****************************************************************************
;* quant-a.asm: xavs encoder library
;*****************************************************************************
;* Copyright (C) 2005 x264 project
;*
;* Authors: Alex Izvorski <aizvorksi@gmail.com>
;*          Christian Heine <sennindemokrit@gmx.net>
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation; either version 2 of the License, or
;* (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program; if not, write to the Free Software
;* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
;*****************************************************************************

BITS 32

%include "i386inc.asm"

SECTION_RODATA
pd_1:  times 4 dd 1

;=============================================================================
; Code
;=============================================================================

SECTION .text
cglobal xavs_quant_8x8_sse2
cglobal xavs_dequant_8x8_sse2

;=============================================================================
; quant
;=============================================================================

ALIGN 16
;-----------------------------------------------------------------------------
;   void __cdecl xavs_quant_8x8_sse( int16_t dct[8][8],
;        int mf[64], uint16_t bias[64], int qp,&nz );
;-----------------------------------------------------------------------------

xavs_quant_8x8_sse2:
    push        edi
    push        ebx
    push        ecx
    push        edx
      
    ;mov         eax, [esp+28]; qp
    mov         eax, [esp+32]; qp
  
    ;mov         ebx, [esp+36]
    mov         ebx, [esp+40]
    movd        xmm6,[ebx+eax*4]        ;qptable
     
    
    ;mov         eax, [esp+16]  ; &dct[0][0]
    ;mov         ecx, [esp+20] ; &mf[0] 
    
    mov         eax, [esp+20]  ; &dct[0][0]
    mov         ecx, [esp+24] ; &mf[0] 
    
    punpcklwd   xmm6, xmm6
    punpcklwd   xmm6, xmm6
    punpcklwd   xmm6, xmm6        ;qptable in each dword
    
    mov         edi, [esp+28] ; bias[0]
    
    mov         edx, [esp+36]
    pxor        xmm3, xmm3
  
%rep 8
    ;movdqu      xmm5, [ecx] ;mf-4
    
    movdqu      xmm7,[edi]
    pxor        xmm2, xmm2
    punpcklwd   xmm7, xmm2

   
    movdqu      xmm0, [eax]   ; load dct coeffs
    
    pxor        xmm4, xmm4
    pcmpgtw     xmm4, xmm0    ; sign(coeff)
    pxor        xmm0, xmm4
    psubw       xmm0, xmm4    ; abs(coeff)


    pshufw      mm5, [ecx], 10110001b
    paddw       mm5, [ecx+8]
    pshufw      mm5, mm5, 10001101b
    pshufw      mm1, [ecx+16], 10110001b
    paddw       mm1, [ecx+24]
    pshufw      mm1, mm1, 10001101b
    movq2dq     xmm1,mm1
    pxor        xmm5,xmm5
    movlhps     xmm5,xmm1
    movq2dq     xmm1,mm5
    por         xmm5,xmm1
   
    
    movdqu      xmm2,xmm0
    pmulhuw     xmm2,xmm5
    pmullw      xmm0,xmm5
    

    movdqu      xmm1, xmm0
    punpcklwd   xmm0, xmm2
    punpckhwd   xmm1, xmm2


    picpush     ebx
    picgetgot   ebx
    movdqa      xmm5, [pd_1 GOT_ebx]
    picpop      ebx
    pslld       xmm5,18
    paddd       xmm0,xmm5
    psrad       xmm0,19
    paddd       xmm1,xmm5
    psrad       xmm1,19

    packssdw    xmm0,xmm1
    movdqu      xmm1,xmm0
    pmullw      xmm0,xmm6
    pmulhuw     xmm1,xmm6    
    movdqu      xmm2,xmm0
    punpcklwd   xmm0,xmm1
    punpckhwd   xmm2,xmm1

    paddd       xmm0,xmm7     ; round with f
    paddd       xmm2, xmm7
    psrad       xmm0, 15
    psrad       xmm2, 15
    
    packssdw    xmm0, xmm2    ; pack
    pxor        xmm0, xmm4    ; restore sign
    psubw       xmm0, xmm4
    movdqu      [eax], xmm0     ; store
   
    add         ecx, byte 32
    add         eax, byte 16
    add         edi, byte 16
    
    por         xmm3, xmm0
%endrep
    movhlps     xmm2,xmm3
    por         xmm2,xmm3
    movdq2q     mm3, xmm2
    
    movq        mm2,mm3
    psrlq       mm2,32
    por         mm3,mm2
    movd        [edx],mm3
    
   pop     edx
   pop     ecx
   pop     ebx
   pop     edi
    ret

;=============================================================================
; dequant
;=============================================================================


ALIGN 16
;-----------------------------------------------------------------------------
;   void __cdecl xavs_dequant_8x8_mmx( int16_t dct[8][8],
;       int dequant_mf[64][8][8], int i_qp);
;-----------------------------------------------------------------------------

xavs_dequant_8x8_sse2:
    
    push        edx
    push        ebx
    push        ecx
    
    mov         eax, [esp+16] ; dct
    mov         edx, [esp+24] ; i_qp
    
   ; mov         ebx, 0x4ba558; &dequant_shifttable[0] 
    mov         ebx, [esp+28]  
    movd        xmm5,[ebx+edx*2]       ;shift_bits = dequant_shifttable[i_qp]
    pshuflw     xmm5,xmm5,11111100b

   
    mov         ecx, [esp+20]   ;&dequant_mf[0]
    shl         edx, 8
    ;lea         ecx,[ecx+edx]
    movd        xmm3,[ecx+edx]
    punpcklwd   xmm3,xmm3
    punpcklwd   xmm3,xmm3
    punpcklwd   xmm3,xmm3
    
    picpush     ebx
    picgetgot   ebx
    movdqu      xmm6, [pd_1 GOT_ebx]
    picpop      ebx
    pslld       xmm6, xmm5          
    psrld       xmm6,1         ;f
    

%rep 8
    movdqu      xmm0, [eax]
    pxor        xmm4, xmm4
    pcmpgtw     xmm4, xmm0    ; sign(coeff)
    pxor        xmm0, xmm4
    psubw       xmm0, xmm4    ; abs(coeff)
    
    movdqu      xmm2,xmm0
    pmulhuw     xmm2,xmm3
    pmullw      xmm0,xmm3

    movdqu      xmm1, xmm0
    punpcklwd   xmm0, xmm2
    punpckhwd   xmm1, xmm2
    
    movdqu      xmm7, xmm4
    punpcklwd   xmm4, xmm4
    punpckhwd   xmm7, xmm7
    
    pxor        xmm0, xmm4    ; restore sign
    psubd       xmm0, xmm4
    pxor        xmm1, xmm7    ; restore sign
    psubd       xmm1, xmm7
    
    paddd       xmm0, xmm6
    paddd       xmm1, xmm6
   
    psrad       xmm0, xmm5
    psrad       xmm1, xmm5
    
    
    

    packssdw    xmm0,xmm1
    movdqu      [eax],xmm0

    add     eax , byte 16
  
%endrep
     pop  ecx
     pop  ebx
     pop  edx
     ret