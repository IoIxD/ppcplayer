BITS 32

;=============================================================================
; Macros and other preprocessor constants
;=============================================================================

%include "i386inc.asm"

;=============================================================================
; Macros and other preprocessor constants
;=============================================================================

SECTION_RODATA

ALIGN 16
mmx_dw_8:
    times 8 dw 8
mmx_dw_7:
    times 8 dw 7

    
        
;=============================================================================
; Code
;=============================================================================

SECTION .text
cglobal xavs_pixel_51315_w8_sse2
cglobal xavs_pixel_51315_w16_sse2
cglobal xavs_qpel1_filter_mmxext

ALIGN 16
;-----------------------------------------------------------------------------
;
; static inline void pixel_avg( uint8_t *dst,  int i_dst_stride,
;                              uint8_t *src1, int i_src1_stride,
;                              uint8_t *src2, int i_src2_stride,
;                              int i_width, int i_height,int qpel_idx;
;
;-----------------------------------------------------------------------------
xavs_pixel_51315_w8_sse2:
    push        ebp
    push        ebx
    push        esi
    push        edi

    mov         edi, [esp+20]       ; dst
    mov         ebx, [esp+28]       ; src1
    mov         ecx, [esp+36]       ; src2
    mov         esi, [esp+24]       ; i_dst_stride
    mov         eax, [esp+32]       ; i_src1_stride
    mov         edx, [esp+40]       ; i_src2_stride
    mov         ebp, [esp+44]       ; i_height
ALIGN 4
.height_loop1    
    movq        mm0, [ebx]
    movq        mm1, [ecx]
    pavgb       mm0,mm1
    movq        [edi], mm0
    
    lea         ebx, [ebx+eax]
    lea         ecx, [ecx+edx]
    lea         edi, [edi+esi]
    dec         ebp
    jne         .height_loop1

    pop         edi
    pop         esi
    pop         ebx
    pop         ebp
    ret
    
    
ALIGN 16
;-----------------------------------------------------------------------------
;
; static inline void pixel_avg( uint8_t *dst,  int i_dst_stride,
;                              uint8_t *src1, int i_src1_stride,
;                              uint8_t *src2, int i_src2_stride,
;                              int i_width, int i_height,int qpel_idx;
;
;-----------------------------------------------------------------------------
xavs_pixel_51315_w16_sse2:
    push        ebp
    push        ebx
    push        esi
    push        edi

    mov         edi, [esp+20]       ; dst
    mov         ebx, [esp+28]       ; src1
    mov         ecx, [esp+36]       ; src2
    mov         esi, [esp+24]       ; i_dst_stride
    mov         eax, [esp+32]       ; i_src1_stride
    mov         edx, [esp+40]       ; i_src2_stride
    mov         ebp, [esp+44]       ; i_height
ALIGN 4
.height_loop2    
    movdqu      xmm0, [ebx]
    movdqu      xmm1, [ecx]
    pavgb       xmm0, xmm1
    movdqu      [edi], xmm0

    
    lea         ebx, [ebx+eax]
    lea         ecx, [ecx+edx]
    lea         edi, [edi+esi]
    dec         ebp
    jne         .height_loop2

    pop         edi
    pop         esi
    pop         ebx
    pop         ebp
    ret


ALIGN 16
;-----------------------------------------------------------------------------
;
; static inline void pixel_avg( uint8_t *dst,  int i_dst_stride,
;                              uint8_t *src1, int i_src1_stride,
;                              uint8_t *src2, int i_src2_stride,
;                              int i_width, int i_height,uint8_t *src3,uint8_t *src4);
;
;-----------------------------------------------------------------------------
xavs_qpel1_filter_mmxext:
    push        ebp
    push        ebx
    push        esi
    push        edi

    mov         edi, [esp+20];[esp+32]       ; dst
    mov         ebx, [esp+28];[esp+40]       ; src1
    mov         ecx, [esp+36];[esp+48]       ; src2
    mov         eax, [esp+52];[esp+64]       ; src3
    mov         edx, [esp+56];[esp+68]       ; src4
    mov         ebp, [esp+48];[esp+60]       ; i_height

    pxor        xmm0, xmm0
    
loophy11:

    xor        esi,esi
ALIGN 4
loophx11:
    movq        xmm1, [ebx+esi] ;src1
    movq        xmm2, [ecx+esi] ;src2    
    movq        xmm3, [eax+esi] ;src3
    movq        xmm4, [edx+esi] ;src4
    punpcklbw   xmm1, xmm0
    punpcklbw   xmm2, xmm0
    punpcklbw   xmm3, xmm0
    punpcklbw   xmm4, xmm0
    paddw       xmm1, xmm2
    pmullw      xmm1,[mmx_dw_7 GOT_ebx]  
   ; movdqa      xmm5,xmm1
   ; psllw       xmm1, 3
   ; psubw       xmm1, xmm5
    paddw       xmm3, xmm4
    paddw       xmm3, [mmx_dw_8 GOT_ebx]
    paddw       xmm1, xmm3
    psraw       xmm1, 4
    packuswb    xmm1, xmm1
    movq        [edi+esi],xmm1
    add         esi,8 
    cmp         esi,    [esp+44];[esp + 56]           ; width
    jnz         loophx11
    
    
    add         eax,    [esp+32];[esp + 44]           ; src_pitch
    add         ebx,    [esp+32];[esp + 44]
    add         ecx,    [esp+40];[esp + 52]
    add         edx,    [esp+40];[esp + 52]
    add         edi,    [esp+24];[esp + 36]           ; dst_pitch
    
    dec         ebp
    jnz         loophy11
    
    pop         edi
    pop         esi
    pop         ebx
    pop         ebp
            
    ret