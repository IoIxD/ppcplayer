;*****************************************************************************
;* mc-a2.asm: xavs encoder library
;*****************************************************************************
;* Copyright (C) 2005 x264 project
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

;=============================================================================
; Macros and other preprocessor constants
;=============================================================================

%include "i386inc.asm"

;=============================================================================
; Read only data
;=============================================================================

SECTION_RODATA

ALIGN 16
mmx_dw_32:
    times 4 dw 32
mmx_dw_4:
    times 4 dw 4
mmx_dw_5:
    times 4 dw 5

%assign twidth  0
%assign theight 4
%assign tdstp1  8
%assign tdstp2  12
%assign tdst1   16
%assign tdst2   20
%assign tsrc    24
%assign tsrcp   28
%assign toffset 32
%assign tbuffer 36


;=============================================================================
; Macros
;=============================================================================

%macro LOAD_4 9
       movd  %1, %5
       movd  %2, %6
       movd  %3, %7
       movd  %4, %8
       punpcklbw %1, %9
       punpcklbw %2, %9
       punpcklbw %3, %9
       punpcklbw %4, %9
%endmacro

%macro FILT_4 5
      paddw  %1, %4
      paddw  %2, %3
      pmullw  %2, %5
      psubw   %2, %1
%endmacro


;=============================================================================
; Code
;=============================================================================

SECTION .text

cglobal xavs_horizontal_filter_mmxext
cglobal xavs_center_filter_mmxext

;-----------------------------------------------------------------------------
;
; void xavs_center_filter_mmxext( uint8_t *dst1, int i_dst1_stride,
;                                 uint8_t *dst2, int i_dst2_stride,
;                                  uint8_t *src, int i_src_stride,
;                                  int i_width, int i_height );
;
;-----------------------------------------------------------------------------

ALIGN 16
xavs_center_filter_mmxext :

    push        edi
    push        esi
    push        ebx
    push        ebp

    mov    edx,      [esp + 40]         ; src_stride
    lea    edx,      [edx + edx + 18 + tbuffer]
    sub    esp,      edx
    mov    [esp + toffset] ,edx
    
    mov    eax,      [esp + edx + 20]   ; dst1
    mov    [esp + tdst1]   ,eax
    
    mov    eax,      [esp + edx + 28]   ; dst2
    mov    [esp + tdst2]   ,eax
    
    mov    eax,      [esp + edx + 44]   ; width
    mov    [esp + twidth]  ,eax
    
    mov    eax,      [esp + edx + 48]   ; height
    mov    [esp + theight] ,eax
    
    mov    eax,      [esp + edx + 24]   ; dst1_stride
    mov    [esp + tdstp1]  ,eax
    
    mov    eax,      [esp + edx + 32]   ; dst2_stride
    mov    [esp + tdstp2]  ,eax

    mov    ecx,      [esp + edx + 40]   ; src_stride
    mov    [esp + tsrcp]   ,ecx
    mov    eax,      [esp + edx + 36]   ; src
    sub    eax,      ecx
    mov [esp+tsrc], eax     ;src-src_stride
    lea ebx,[ecx+ecx*2]     ;3*src_strid
    
    picpush   ebx
    picgetgot  ebx
    pxor mm0,mm0

loopcy:
      mov      edi,  [picesp + tdst1]
      lea      ebp,  [picesp + tbuffer]
      mov      esi,  [esp + tsrc]
      movq     mm7, [mmx_dw_5 GOT_ebx]
      movq     mm6, [mmx_dw_4 GOT_ebx]
      picpop    ebx
      LOAD_4  mm1, mm2, mm3, mm4, [esi],[esi+ecx], [esi+2*ecx],[esi+ebx],mm0
      FILT_4  mm1, mm2, mm3, mm4, mm7
      pshufw    mm1,mm2,0
      movq     [ebp+8], mm2
      movq     [ebp],   mm1
      paddw     mm2,  mm6
      psraw     mm2,  3
      packuswb  mm2,   mm2
      movd     [edi],  mm2
      mov     eax,  8
      add     esi,  4
loopcx1:                                    
      LOAD_4  mm1, mm2, mm3, mm4, [esi],[esi+ecx], [esi+2*ecx],[esi+ebx],mm0
      FILT_4   mm1, mm2, mm3, mm4,mm7
      movq  [ebp+2*eax],mm2
      paddw mm2,mm6
      psraw   mm2, 3
      packuswb mm2, mm2
      movd  [edi+eax-4],mm2
      add   esi,4
      add   eax,4
      cmp   eax,[esp+twidth]
      jnz   loopcx1

      LOAD_4  mm1, mm2, mm3, mm4, [esi],[esi+ecx], [esi+2*ecx],[esi+ebx],mm0
      FILT_4  mm1, mm2, mm3, mm4, mm7
      pshufw    mm1,mm2,7
      movq     [ebp+2*eax],  mm2
      movq     [ebp+2*eax+8],  mm1
      paddw     mm2,  mm6
      psraw     mm2,  3
      packuswb  mm2,   mm2
      movd     [edi+eax-4],  mm2
     
      mov         esi,    [esp + tsrc]
      add         esi,    ecx
      mov         [esp + tsrc],  esi

      add         edi,    [esp + tdstp1]
      mov         [esp + tdst1], edi

      mov         edi,    [esp + tdst2]
      mov         eax,    [esp + twidth]
      sub         eax,    4

      picpush     ebx
      picgetgot   ebx     


loopcx2:
      movq      mm1,    [picesp + 2 * eax + 2  + 4 + tbuffer]
      movq      mm2,    [picesp + 2 * eax + 4  + 4 + tbuffer]
      movq      mm3,    [picesp + 2 * eax + 6  + 4 + tbuffer]
      movq      mm4,    [picesp + 2 * eax + 8  + 4 + tbuffer]
      
      paddw    mm1, mm4
      paddw    mm2, mm3
      pmullw   mm2, mm7
      psubw    mm2, mm1

      paddw     mm2, [mmx_dw_32 GOT_ebx]
      psraw     mm2, 6
      packuswb  mm2,   mm2
      movd      [edi + eax], mm2

      sub       eax,    4
      jge       loopcx2

      add       edi,    [picesp + tdstp2]
      mov       [picesp + tdst2], edi

      dec        dword [picesp + theight]
      jnz        loopcy

      picpop     ebx

      add        esp,    [esp + toffset]

      pop         ebp
      pop         ebx
      pop         esi
      pop         edi

      ret


;-----------------------------------------------------------------------------
;
; void xavs_horizontal_filter_mmxext( uint8_t *dst, int i_dst_stride,
;                                     uint8_t *src, int i_src_stride,
;                                     int i_width, int i_height );
;
;-----------------------------------------------------------------------------

ALIGN 16
xavs_horizontal_filter_mmxext :
    push edi
    push esi
    mov     edi, [esp+12]  ;dst
    mov     esi, [esp+20]  ;src
    pxor     mm0, mm0
    picpush   ebx
    picgetgot  ebx
    movq     mm7, [mmx_dw_5 GOT_ebx]
    mov      ecx, [esp+32]    ;height
    sub       esi, 1
loophy:
      xor       eax, eax
loophx:
      
      
      LOAD_4  mm1, mm2, mm3, mm4, [esi+eax], [esi+eax+1], [esi+eax+2], [esi+eax+3], mm0
      FILT_4   mm1, mm2, mm3, mm4,mm7
      paddw  mm2, [mmx_dw_4 GOT_ebx]
      psraw   mm2, 3

      LOAD_4  mm5, mm6, mm3, mm4, [esi+eax+4], [esi+eax+5], [esi+eax+6], [esi+eax+7], mm0
      FILT_4   mm5, mm6, mm3, mm4,mm7
      paddw  mm6, [mmx_dw_4 GOT_ebx]
      psraw   mm6, 3

      packuswb mm2, mm6
      movq    [edi+eax], mm2
      add  eax, 8
      cmp  eax, [esp+28]    ;width
      jnz   loophx
      add  esi, [esp+24]     ;src_pitch
      add  edi, [esp+16]     ;dst_pitch
      dec  ecx
      jnz   loophy
      picpop    ebx
      pop   esi
      pop   edi
      
      ret
