

BITS 32

%include "i386inc.asm"

SECTION_RODATA
dw_4:  times 8 dw 4


SECTION .text
cglobal xavs_deblock_v_chroma_mmxext
cglobal xavs_deblock_h_chroma_mmxext


; out: %4 = |%1-%2|>%3
; clobbers: %5
%macro DIFF_GT_SSE 5
    movaps    %5, %2
    movaps    %4, %1
    psubusw   %5, %1
    psubusw   %4, %2
    por       %4, %5
    psubusw   %4, %3
%endmacro

; expands to [base],...,[base+7*stride]
%define PASS8ROWS(base, base3, stride, stride3) \
    [base], [base+stride], [base+stride*2], [base3], \
    [base3+stride], [base3+stride*2], [base3+stride3], [base3+stride*4]

; in: 8 rows of 4 bytes in %1..%8
; out: 4 rows of 8 bytes in mm0..mm3
%macro TRANSPOSE4x8_LOAD 8
    movd       xmm0, %1
    movd       xmm2, %2
    movd       xmm1, %3
    movd       xmm3, %4
    punpcklbw  xmm0, xmm2
    punpcklbw  xmm1, xmm3
    punpcklwd  xmm0, xmm1

    movd       xmm4, %5
    movd       xmm6, %6
    movd       xmm5, %7
    movd       xmm7, %8
    punpcklbw  xmm4, xmm6
    punpcklbw  xmm5, xmm7
    punpcklwd  xmm4, xmm5
   

    movaps     xmm2, xmm0 
    punpckldq  xmm0, xmm4
    punpckhdq  xmm2, xmm4
    movhlps    xmm1, xmm0
    movhlps    xmm3, xmm2 

%endmacro

; in: 4 rows of 8 bytes in mm0..mm3
; out: 8 rows of 4 bytes in %1..%8
%macro TRANSPOSE8x4_STORE 8
    movq       mm4, mm0
    movq       mm5, mm1
    movq       mm6, mm2
    punpckhdq  mm4, mm4
    punpckhdq  mm5, mm5
    punpckhdq  mm6, mm6

    punpcklbw  mm0, mm1
    punpcklbw  mm2, mm3
    movq       mm1, mm0
    punpcklwd  mm0, mm2
    punpckhwd  mm1, mm2
    movd       %1,  mm0
    punpckhdq  mm0, mm0
    movd       %2,  mm0
    movd       %3,  mm1
    punpckhdq  mm1, mm1
    movd       %4,  mm1

    punpckhdq  mm3, mm3
    punpcklbw  mm4, mm5
    punpcklbw  mm6, mm3
    movq       mm5, mm4
    punpcklwd  mm4, mm6
    punpckhwd  mm5, mm6
    movd       %5,  mm4
    punpckhdq  mm4, mm4
    movd       %6,  mm4
    movd       %7,  mm5
    punpckhdq  mm5, mm5
    movd       %8,  mm5
%endmacro

ALIGN 16
;-----------------------------------------------------------------------------
;   void xavs_deblock_v_chroma_mmxext( uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0 )
;-----------------------------------------------------------------------------
xavs_deblock_v_chroma_mmxext:
    push  edi
    push  esi
    mov   edi, [esp+12] ; pix
    mov   esi, [esp+16] ; stride
    mov   edx, [esp+20] ; alpha
    mov   ecx, [esp+24] ; beta
    dec   edx
    dec   ecx
    mov   eax, edi
    sub   eax, esi
    sub   eax, esi
    push  ebx
    mov   ebx,[esp+32]  ;tc0
    
    pxor   xmm4, xmm4
    movq  xmm0, [eax]
    movq  xmm1, [eax+esi]
    movq  xmm2, [edi]
    movq  xmm3, [edi+esi]
    punpcklbw  xmm0, xmm4
    punpcklbw  xmm1, xmm4
    punpcklbw  xmm2, xmm4
    punpcklbw  xmm3, xmm4
    
    movd     xmm4, edx
    movd     xmm5, ecx
    pshuflw  xmm4, xmm4, 0
    pshuflw  xmm5, xmm5, 0
    movlhps  xmm4, xmm4  ; 8x alpha-1
    movlhps  xmm5, xmm5  ; 8x beta-1

    DIFF_GT_SSE  xmm1, xmm2, xmm4, xmm7, xmm6 ; |p0-q0| > alpha-1
    DIFF_GT_SSE  xmm0, xmm1, xmm5, xmm4, xmm6 ; |p1-p0| > beta-1
    por      xmm7, xmm4
    DIFF_GT_SSE  xmm3, xmm2, xmm5, xmm4, xmm6 ; |q1-q0| > beta-1
    por      xmm7, xmm4
    pxor     xmm6, xmm6
    pcmpeqw  xmm7, xmm6
    
    movd       xmm6, [ebx]
    punpcklbw  xmm6, xmm6
    punpcklbw  xmm6, xmm6
    pxor       xmm5, xmm5
    movaps     xmm4, xmm6 
    pcmpgtb    xmm4, xmm5
    punpcklbw  xmm4, xmm4
    punpcklbw  xmm6, xmm5
    pand       xmm6, xmm4      
    pand       xmm6, xmm7
   
    
    
    ; in: xmm0=p1 xmm1=p0 xmm2=q0 xmm3=q1
    movaps     xmm4, xmm2
    psubw      xmm4, xmm1
    movdqa     xmm5, xmm4
    paddw      xmm4, xmm5
    paddw      xmm4, xmm5
    
    psubw      xmm0, xmm3
    paddw      xmm0, [dw_4 GOT_ebx]
    paddw      xmm4,xmm0
   
    
    psraw      xmm4, 3
    movaps     xmm5, xmm4
    pxor       xmm7, xmm7
    pcmpgtw    xmm7, xmm4
    pxor       xmm4, xmm7
    psubw      xmm4, xmm7
   
    pcmpgtw    xmm4, xmm6
    
    pxor       xmm6, xmm7
    psubw      xmm6, xmm7
    
    pand       xmm6, xmm4
    pandn      xmm4, xmm5
    por        xmm4, xmm6
    paddw      xmm1, xmm4
    psubw      xmm2, xmm4
    
    packuswb   xmm1,xmm2
    movlps     [eax+esi],xmm1
    movhps     [edi],xmm1
    
    pop        ebx
    pop        esi
    pop        edi
    ret
    

ALIGN 16
;-----------------------------------------------------------------------------
;   void xavs_deblock_h_chroma_mmxext( uint8_t *pix, int stride, int alpha, int beta, int8_t *tc0 )
;-----------------------------------------------------------------------------
xavs_deblock_h_chroma_mmxext:
   
    push  edi
    push  esi
    push  ebp
    mov   edi, [esp+16]
    mov   esi, [esp+20]
    mov   edx, [esp+24]
    mov   ecx, [esp+28]
    dec   edx
    dec   ecx
    sub   edi, 2
    mov   ebp, esi
    add   ebp, esi
    add   ebp, esi
    mov   eax, edi
    add   edi, ebp

    push  ebx
    mov   ebx, [esp+36] ; tc0
    sub   esp, 16

    TRANSPOSE4x8_LOAD  PASS8ROWS(eax, edi, esi, ebp)
    movq  [esp+8], xmm0
    movq  [esp+0], xmm3
    
       
    pxor       xmm4, xmm4
    punpcklbw  xmm0, xmm4
    punpcklbw  xmm1, xmm4
    punpcklbw  xmm2, xmm4
    punpcklbw  xmm3, xmm4

    movd     xmm4, edx
    movd     xmm5, ecx
    pshuflw  xmm4, xmm4, 0
    pshuflw  xmm5, xmm5, 0
    movlhps  xmm4, xmm4  ; 8x alpha-1
    movlhps  xmm5, xmm5  ; 8x beta-1

    DIFF_GT_SSE  xmm1, xmm2, xmm4, xmm7, xmm6 ; |p0-q0| > alpha-1
    DIFF_GT_SSE  xmm0, xmm1, xmm5, xmm4, xmm6 ; |p1-p0| > beta-1
    por      xmm7, xmm4
    DIFF_GT_SSE  xmm3, xmm2, xmm5, xmm4, xmm6 ; |q1-q0| > beta-1
    por      xmm7, xmm4
    pxor     xmm6, xmm6
    pcmpeqw  xmm7, xmm6

    
    movd       xmm6, [ebx]
    punpcklbw  xmm6, xmm6
    punpcklbw  xmm6, xmm6
    pxor       xmm5, xmm5
    movaps     xmm4, xmm6 
    pcmpgtb    xmm4, xmm5
    punpcklbw  xmm4, xmm4
    punpcklbw  xmm6, xmm5
    pand       xmm6, xmm4      
    pand       xmm6, xmm7
    picgetgot  ebx
     
    ; in: xmm0=p1 xmm1=p0 xmm2=q0 xmm3=q1
    movaps     xmm4, xmm2
    psubw      xmm4, xmm1
    movdqa     xmm5, xmm4
    paddw      xmm4, xmm5
    paddw      xmm4, xmm5
    psubw      xmm0, xmm3
    paddw      xmm0, [dw_4 GOT_ebx]
    paddw      xmm4,xmm0
    psraw      xmm4, 3
    movaps     xmm5, xmm4
    pxor       xmm7, xmm7
    pcmpgtw    xmm7, xmm4
    pxor       xmm4, xmm7
    psubw      xmm4, xmm7
    pcmpgtw    xmm4, xmm6
    pxor       xmm6, xmm7
    psubw      xmm6, xmm7
    pand       xmm6, xmm4
    pandn      xmm4, xmm5
    por        xmm4, xmm6
    paddw      xmm1, xmm4
    psubw      xmm2, xmm4
    packuswb   xmm1,xmm2
   
    movdq2q    mm1, xmm1
    psrldq     xmm1,8
    movdq2q    mm2,xmm1
    
    movq  mm0, [esp+8]
    movq  mm3, [esp+0]
    TRANSPOSE8x4_STORE PASS8ROWS(eax, edi, esi, ebp)

    add  esp, 16
    pop  ebx
    pop  ebp
    pop  esi
    pop  edi
    ret


    
    
    
    
   
    
    
    
    
