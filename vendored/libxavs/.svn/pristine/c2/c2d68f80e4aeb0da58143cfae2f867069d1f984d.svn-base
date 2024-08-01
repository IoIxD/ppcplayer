%include "i386inc.asm"


SECTION_RODATA 
pb_01: times  16 db 0x01

SECTION .text
cglobal xavs_deblock_v_chroma_intra_mmxext
cglobal xavs_deblock_h_chroma_intra_mmxext
cglobal xavs_deblock_v_luma_intra_sse2
cglobal xavs_deblock_h_luma_intra_sse2

%macro LOAD_MASK_MMX 2
    movd     mm4, %1
    movd     mm5, %2
    pshufw   mm4, mm4, 0
    pshufw   mm5, mm5, 0
    packuswb mm4, mm4  ; 8x alpha-1
    packuswb mm5, mm5  ; 8x beta-1
%endmacro
%macro CHROMA_INTRA_P0 4  
     movq    %4, %1
     pxor    %4, %3
     pand    %4, [pb_01 GOT_ebx]  ; %4 = (p0^q1)&1 
     pavgb   %1, %3
     psubusb %1, %4
     pavgb   %1, %2       ; dst = avg(p1, avg(p0,q1) - ((p0^q1)&1))

%endmacro
%macro DIFF_GT_MMX  5
    movq    %5, %2
    movq    %4, %1
    psubusb %5, %1
    psubusb %4, %2
    por     %4, %5
    psubusb %4, %3
%endmacro

%macro DIFF_GT_MMX1 5              ; |p0-q0| > (alpha>>2+1)
    movq      %5, %2
    movq      %4, %1
    psubusb   %5, %1
    psubusb   %4, %2
    por       %4, %5
    pxor      %5, %5 
    punpcklbw %3, %5
    psrlw     %3, 2
    mov       bx, 1
    pinsrw    %5,ebx,0
    pshufw    %5, %5,0
    paddw     %3, %5
    packuswb  %3, %3 
    psubusb   %4, %3
%endmacro
%define PASS8ROWS(base, base3, stride, stride3)  [base], [base+stride], [base+stride*2], [base3],[base3+stride], [base3+stride*2], [base3+stride3], [base3+stride*4]

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
 
%macro SBUTTERFLY 4
    movq       %4, %2
    punpckl%1  %2, %3
    punpckh%1  %4, %3
%endmacro

; in: 8 rows of 8 (only the middle 6 pels are used) in %1..%8
; out: 6 rows of 8 in [%9+0*16] .. [%9+5*16]
%macro TRANSPOSE6x8_MEM 9
    movq  mm0, %1
    movq  mm1, %3
    movq  mm2, %5 
    movq  mm3, %7
    SBUTTERFLY bw, mm0, %2, mm4
    SBUTTERFLY bw, mm1, %4, mm5
    SBUTTERFLY bw, mm2, %6, mm6
    movq  [%9+0x10], mm5
    SBUTTERFLY bw, mm3, %8, mm7
    SBUTTERFLY wd, mm0, mm1, mm5  
    SBUTTERFLY wd, mm2, mm3, mm1
    punpckhdq mm0, mm2
    movq  [%9+0x00], mm0
    SBUTTERFLY wd, mm4, [%9+0x10], mm3
    SBUTTERFLY wd, mm6, mm7, mm2
    SBUTTERFLY dq, mm4, mm6, mm0
    SBUTTERFLY dq, mm5, mm1, mm7
    punpckldq mm3, mm2
    movq  [%9+0x10], mm5
    movq  [%9+0x20], mm7
    movq  [%9+0x30], mm4
    movq  [%9+0x40], mm0 
    movq  [%9+0x50], mm3
%endmacro

%macro LOAD_MASK_SSE2 2   
  movd       xmm4, %1  
  movd       xmm5, %2   
  pshuflw    xmm4, xmm4, 0  
  pshuflw    xmm5, xmm5, 0   
  punpcklqdq xmm4, xmm4   
  punpcklqdq xmm5, xmm5   
  packuswb   xmm4, xmm4       ; 16x alpha-1  
  packuswb   xmm5, xmm5       ; 16x beta-1    
%endmacro

%macro DIFF_GT_SSE2  5    
  movdqa   %5, %2  
  movdqa   %4, %1   
  psubusb  %5, %1    
  psubusb  %4, %2   
  por      %4, %5   
  psubusb  %4, %3
%endmacro

%macro CHROMA_INTRA_P0_SSE2 4  
     movdqa  %4, %1
     pxor    %4, %3
     pand    %4, [pb_01 GOT_ebx]  ; %4 = (p0^q1)&1 
     pavgb   %1, %3
     psubusb %1, %4
     pavgb   %1, %2                ; dst = avg(p1, avg(p0,q1) - ((p0^q1)&1))
%endmacro

%macro DIFF_GT_SSE1 5              ; |p0-q0| >(alpha>>2+1)
    movdqa    %5, %2
    movdqa    %4, %1
    psubusb   %5, %1
    psubusb   %4, %2
    por       %4, %5
    movd      %3, edx
    pshuflw   %3, %3,0
    punpcklqdq %3,%3  
    psrlw     %3, 2 
    mov       ebx,1
    movd      %5, ebx
    pshuflw   %5, %5,0
    punpcklqdq %5,%5 
    paddw     %3, %5
    packuswb  %3, %3
    psubusb   %4, %3  
%endmacro   
    
;-----------------------------------------------------------------------------
;void xavs_deblock_h_luma_intra_sse2( uint8_t *pix, int stride, int alpha, int beta )
;-----------------------------------------------------------------------------
xavs_deblock_h_luma_intra_sse2:
    push  edi
    push  esi
    push  ebp
    mov   edi, [esp+16]       ;pix
    mov   esi, [esp+20]       ;stride
    mov   edx, [esp+24]       ;alpha
    mov   ecx, [esp+28]       ;beta 
    sub   edi, 4
    dec   edx
    dec   ecx
    mov   ebp, esi
    add   ebp, esi
    add   ebp, esi
    mov   eax, edi
    add   edi, ebp
    push  esp
%define pix1_tmp esp-160
    TRANSPOSE6x8_MEM  PASS8ROWS(eax, edi, esi, ebp), pix1_tmp
    lea    eax, [eax+esi*8] 
    lea    edi, [edi+esi*8]
    TRANSPOSE6x8_MEM  PASS8ROWS(eax, edi, esi, ebp), pix1_tmp+8
    movdqu  xmm0,[pix1_tmp+0x10]
    movdqu  xmm1,[pix1_tmp+0x20]   
    movdqu  xmm2,[pix1_tmp+0x30]
    movdqu  xmm3,[pix1_tmp+0x40]   
    
    LOAD_MASK_SSE2 edx, ecx
    movdqu  [esp-64],xmm5
    DIFF_GT_SSE2  xmm1, xmm2, xmm4, xmm7, xmm6               ; |p0-q0| > alpha-1
    DIFF_GT_SSE2  xmm0, xmm1, xmm5, xmm2, xmm6               ; |p1-p0| > beta-1
    por      xmm7, xmm2
    movdqu   xmm2, [pix1_tmp+0x30] 
    DIFF_GT_SSE2  xmm3, xmm2, xmm5, xmm1, xmm6               ; |q1-q0| > beta-1  
    por      xmm7, xmm1  
    pxor     xmm6, xmm6  
    pcmpeqb  xmm7, xmm6
    movdqu   [esp-32],xmm7
    movdqu   xmm1, [pix1_tmp+0x20] 
    add      edx,1
    DIFF_GT_SSE1  xmm1, xmm2, xmm4, xmm0, xmm6               ;|p0-q0| >(alpha>>2+1)
    movdqu   [esp-16],xmm0
    movdqu   xmm2,[pix1_tmp+0x00]
    DIFF_GT_SSE2  xmm2, xmm1, xmm5, xmm4, xmm6              ;|p2-p0| >beta-1
    por     xmm4,xmm0
    pxor    xmm6,xmm6
    pcmpeqb xmm4,xmm6    
    pand    xmm4,xmm7 
    movdqu  [esp-48],xmm4 
    movdqu  xmm0,[pix1_tmp+0x10] 
    movdqu  xmm2,[pix1_tmp+0x30]
    CHROMA_INTRA_P0_SSE2  xmm1, xmm0, xmm2 ,xmm6             ;p1=1,p0=0
    movdqa  xmm3, xmm1
    movdqu  xmm5,[pix1_tmp+0x20]
    CHROMA_INTRA_P0_SSE2  xmm0,xmm5, xmm2 ,xmm6              ;p0=1
    pand    xmm3, xmm4
    movdqu  xmm6,[pix1_tmp+0x10]
    pandn   xmm4,xmm6
    por     xmm3,xmm4
    movdqu  xmm4,[esp-48]
    pand    xmm0, xmm4 
    pandn   xmm4, xmm1
    pand    xmm4, xmm7
    ;movdqu  xmm6,[pix1_tmp+0x20]
    pandn   xmm7, xmm5
    por     xmm4, xmm7
    movdqa  xmm1, xmm4
    por     xmm1, xmm0
    movdqu  xmm5,[esp-64]
    movdqu  xmm0,[pix1_tmp+0x50]
    DIFF_GT_SSE2  xmm0, xmm2, xmm5, xmm4,xmm6         ;|q2-q0| >beta-1
    movdqu  xmm6,[esp-16]
    por     xmm4,xmm6
    pxor    xmm6,xmm6
    pcmpeqb xmm4,xmm6 
    movdqu  xmm6,[esp-32]
    pand    xmm4,xmm6
    movdqu [esp-48],xmm4
    movdqa  xmm5,xmm3
    movdqu  xmm3,[pix1_tmp+0x40]
    movdqu  xmm4,[pix1_tmp+0x20]
    CHROMA_INTRA_P0_SSE2  xmm2, xmm3,xmm4,xmm6        ;q0=0,q1=1   
    movdqa  xmm0, xmm2
    movdqu  xmm7,[pix1_tmp+0x30]
    CHROMA_INTRA_P0_SSE2  xmm3,xmm7,xmm4,xmm6        ;q0=1
    movdqu  xmm7,[esp-32]
    movdqu  xmm4,[esp-48]
    pand    xmm0, xmm4
    movdqu  xmm6,[pix1_tmp+0x40]
    pandn   xmm4,xmm6
    por     xmm0,xmm4
    movdqu  xmm4,[esp-48]
    pand    xmm3, xmm4
    pandn   xmm4, xmm2
    pand    xmm4, xmm7 
    movdqu  xmm6,[pix1_tmp+0x30]
    pandn   xmm7, xmm6
    por     xmm4, xmm7
    movdqa  xmm2, xmm4
    por     xmm2, xmm3  

    movdqu    [pix1_tmp+0x10],xmm5
    movdqu    [pix1_tmp+0x20],xmm1
    movdqu    [pix1_tmp+0x30],xmm2
    movdqu    [pix1_tmp+0x40],xmm0 
    pop   esp
    mov   edi,[esp+16]       ;pix
    sub   edi, 2
    lea   ebp,[esi+2*esi]
    mov   eax, edi
    add   edi, ebp
    sub   esp, 4
    movq    mm0, [pix1_tmp+0x10]
    movq    mm1, [pix1_tmp+0x20]
    movq    mm2, [pix1_tmp+0x30]
    movq    mm3, [pix1_tmp+0x40]
    add   esp, 4
    TRANSPOSE8x4_STORE  PASS8ROWS(eax, edi, esi, ebp)
    lea    eax, [eax+esi*8]
    lea    edi, [edi+esi*8]
    sub    esp, 4  
    movq   mm0, [pix1_tmp+0x18]
    movq   mm1, [pix1_tmp+0x28]
    movq   mm2, [pix1_tmp+0x38]
    movq   mm3, [pix1_tmp+0x48]
    add    esp, 4
    TRANSPOSE8x4_STORE  PASS8ROWS(eax, edi, esi, ebp)
    pop    ebp
    pop    esi
    pop    edi
    ret
ALIGN 16    
;-----------------------------------------------------------------------------
;void xavs_deblock_v_luma_intra _sse2( uint8_t *pix, int stride, int alpha, int beta )
;-----------------------------------------------------------------------------
xavs_deblock_v_luma_intra_sse2:
    picpush ebx
    picgetgot ebx
    push    edi
    push    esi
    mov     edi, [esp+12] ; pix
    mov     esi, [esp+16] ; stride
    mov     edx, [esp+20] ; alpha
    mov     ecx, [esp+24] ; beta
    dec     edx
    dec     ecx
    mov     eax, edi
    sub     eax, esi    
    sub     eax, esi
    sub     eax, esi 
    sub     esp, 32
    
    movdqu    xmm0, [eax+esi]   ; p1
    movdqu    xmm1, [eax+2*esi] ; p0
    movdqu    xmm2, [edi]       ; q0
    movdqu    xmm3, [edi+esi]   ; q1
    
    LOAD_MASK_SSE2 edx, ecx
    DIFF_GT_SSE2  xmm1, xmm2, xmm4, xmm7, xmm6               ; |p0-q0| > alpha-1
    DIFF_GT_SSE2  xmm0, xmm1, xmm5, xmm2, xmm6               ; |p1-p0| > beta-1
    por      xmm7, xmm2
    movdqu   xmm2, [edi] 
    DIFF_GT_SSE2  xmm3, xmm2, xmm5, xmm1, xmm6               ; |q1-q0| > beta-1  
    por      xmm7, xmm1  
    pxor     xmm6, xmm6  
    pcmpeqb  xmm7, xmm6
    movdqu   [esp-24],xmm7
    movdqu   xmm1, [eax+2*esi] 
    add      edx,1
    DIFF_GT_SSE1  xmm1, xmm2, xmm4, xmm0, xmm6               ;|p0-q0| >(alpha>>2+1)
    movdqu   [esp-8],xmm0
    DIFF_GT_SSE2  [eax], xmm1, xmm5, xmm4, xmm6              ;|p2-p0| >beta-1
    por     xmm4,xmm0
    pxor    xmm6,xmm6
    pcmpeqb xmm4,xmm6    
    pand    xmm4,xmm7
    movdqu  [esp-40],xmm4 
    movdqu  xmm0,[eax+esi] 
    CHROMA_INTRA_P0_SSE2  xmm1, xmm0, xmm2 ,xmm6             ;p1=1,p0=0
    movdqa  xmm3, xmm1
    CHROMA_INTRA_P0_SSE2  xmm0,[eax+2*esi], xmm2 ,xmm6       ;p0=1
    pand    xmm3, xmm4
    pandn   xmm4,[eax+esi]
    por     xmm3,xmm4
    movdqu  xmm4,[esp-40]
    pand    xmm0, xmm4 
    pandn   xmm4, xmm1
    pand    xmm4, xmm7
    pandn   xmm7,[eax+2*esi]
    por     xmm4, xmm7
    movdqa  xmm1, xmm4
    por     xmm1, xmm0
    DIFF_GT_SSE2  [edi+2*esi], xmm2, xmm5, xmm4,xmm6         ;|q2-q0| >beta-1
    movdqu  xmm6,[esp-8]
    por     xmm4,xmm6
    pxor    xmm6,xmm6
    pcmpeqb xmm4,xmm6 
    movdqu  xmm6,[esp-24]
    pand    xmm4,xmm6
    movdqu [esp-40],xmm4
    movdqa  xmm5,xmm3
    movdqu  xmm3,[edi+esi]
    CHROMA_INTRA_P0_SSE2  xmm2, xmm3,[eax+2*esi],xmm6        ;q0=0,q1=1   
    movdqa  xmm0, xmm2
    CHROMA_INTRA_P0_SSE2  xmm3,[edi],[eax+2*esi],xmm6        ;q0=1
    movdqu  xmm7,[esp-24]
    pand    xmm0, xmm4
    pandn   xmm4,[edi+esi]
    por     xmm0,xmm4
    movdqu  xmm4,[esp-40]
    pand    xmm3, xmm4
    pandn   xmm4, xmm2
    pand    xmm4, xmm7 
    pandn   xmm7,[edi]
    por     xmm4, xmm7
    movdqa  xmm2, xmm4
    por     xmm2, xmm3  
     
    movdqu    [eax+esi],   xmm5
    movdqu    [eax+2*esi], xmm1
    movdqu    [edi],       xmm2
    movdqu    [edi+esi],   xmm0
    
    add     esp,32
    pop     esi
    pop     edi
    picpop  ebx
    ret
ALIGN 16
;-----------------------------------------------------------------------------
;void xavs_deblock_v_chroma_intra_mmxext( uint8_t *pix, int stride, int alpha, int beta )  
;-----------------------------------------------------------------------------
xavs_deblock_v_chroma_intra_mmxext:
    picpush ebx
    picgetgot ebx
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
    sub   eax, esi

    movq  mm0, [eax+esi]                           ;p1
    movq  mm1, [eax+2*esi]                         ;p0
    movq  mm2, [edi]                               ;q0
    movq  mm3, [edi+esi]                           ;q1
    
    LOAD_MASK_MMX edx, ecx
    DIFF_GT_MMX  mm1, mm2, mm4, mm7, mm6           ;|p0-q0| >alpha-1
    DIFF_GT_MMX  mm0, mm1, mm5, mm2, mm6           ;|p1-p0| > beta-1
    por     mm7, mm2
    movq    mm2, [edi]
    DIFF_GT_MMX  mm3, mm2, mm5, mm1, mm6           ;|q1-q0| > beta-1
    por     mm7, mm1
    pxor    mm6, mm6
    pcmpeqb mm7, mm6
    movq    [esp-16],mm7
    movq  mm1, [eax+2*esi] 
    add     edx,1
    LOAD_MASK_MMX edx, ecx
    DIFF_GT_MMX1  mm1, mm2, mm4, mm0, mm6          ;|p0-q0| >(alpha>>2+1)
    movq    [esp-8],mm0
    DIFF_GT_MMX  [eax], mm1, mm5, mm4, mm6         ;|p2-p0| >beta-1
    por     mm4,mm0
    pxor    mm6,mm6
    pcmpeqb mm4,mm6 
    pand    mm4,mm7
    movq    mm0,[eax+esi]
    CHROMA_INTRA_P0  mm1, mm0, mm2 ,mm6             ;p0 0
    CHROMA_INTRA_P0  mm0,[eax+2*esi], mm2 ,mm6      ;p0 1
    pand   mm0, mm4 
    pandn  mm4, mm1
    pand   mm4, mm7
    pandn  mm7,[eax+2*esi]
    por    mm4, mm7
    movq   mm1, mm4
    por    mm1, mm0
    DIFF_GT_MMX  [edi+2*esi], mm2, mm5, mm4, mm6   ;|q2-q0| >beta-1
    por     mm4,[esp-8]
    pxor    mm6,mm6
    pcmpeqb mm4,mm6 
    pand    mm4,[esp-16]
    CHROMA_INTRA_P0  mm3, mm2,[eax+2*esi],mm6       ;q0 1
    CHROMA_INTRA_P0  mm2,[edi+esi],[eax+2*esi],mm6  ;q0 0
    movq   mm7,[esp-16]
    pand   mm3, mm4
    pandn  mm4, mm2
    pand   mm4, mm7 
    pandn  mm7,[edi]
    por    mm4, mm7
    movq   mm2, mm4
    por    mm2, mm3
 
    movq  [eax+2*esi], mm1
    movq  [edi], mm2
    
    picpop ebx
    pop  esi
    pop  edi 
    ret
ALIGN 16
;-----------------------------------------------------------------------------
;void xavs_deblock_h_chroma_intra_mmxext( uint8_t *pix, int stride, int alpha, int beta )
;-----------------------------------------------------------------------------
xavs_deblock_h_chroma_intra_mmxext:
    push  edi
    push  esi
    push  ebp
    mov   edi, [esp+16]  ;pix
    mov   esi, [esp+20]  ;STRIDE
    mov   edx, [esp+24]  ;alpha
    mov   ecx, [esp+28]  ;beta 
    sub   edi, 4
    dec   edx
    dec   ecx
    mov   ebp, esi
    add   ebp, esi
    add   ebp, esi
    mov   eax, edi
    add   edi, ebp
    sub   esp, 96
 %define pix_tmp esp    
    TRANSPOSE6x8_MEM  PASS8ROWS(eax, edi, esi, ebp),pix_tmp 
    movq  mm0,[pix_tmp+0x10]
    movq  mm1,[pix_tmp+0x20]   
    movq  mm2,[pix_tmp+0x30]
    movq  mm3,[pix_tmp+0x40]   
     
    LOAD_MASK_MMX edx, ecx
    DIFF_GT_MMX  mm1, mm2, mm4, mm7, mm6           ;|p0-q0| >alpha-1
    DIFF_GT_MMX  mm0, mm1, mm5, mm2, mm6           ;|p1-p0| > beta-1
    por     mm7, mm2
    movq    mm2, [pix_tmp+0x30]
    DIFF_GT_MMX  mm3, mm2, mm5, mm1, mm6           ;|q1-q0| > beta-1
    por     mm7, mm1
    pxor    mm6, mm6
    pcmpeqb mm7, mm6
    movq    [esp-8],mm7
    movq    mm1, [pix_tmp+0x20]
    add     edx,1
    LOAD_MASK_MMX edx, ecx
    DIFF_GT_MMX1  mm1, mm2, mm4, mm0, mm6          ;|p0-q0| >(alpha>>2+1)
    movq    [esp-16],mm0
    DIFF_GT_MMX  [pix_tmp+0x00], mm1, mm5, mm4, mm6         ;|p2-p0| >beta-1
    por     mm4,mm0
    pxor    mm6,mm6
    pcmpeqb mm4,mm6 
    pand    mm4,mm7
    movq    mm0,[pix_tmp+0x10]
    CHROMA_INTRA_P0  mm1, mm0, mm2 ,mm6                ;p0 0
    CHROMA_INTRA_P0  mm0,[pix_tmp+0x20], mm2 ,mm6      ;p0 1
    pand   mm0, mm4 
    pandn  mm4, mm1
    pand   mm4, mm7
    pandn  mm7,[pix_tmp+0x20] 
    por    mm4, mm7
    movq   mm1, mm4
    por    mm1, mm0
    DIFF_GT_MMX [pix_tmp+0x50], mm2, mm5, mm4, mm6   ;|q2-q0| >beta-1
    por     mm4,[esp-16]
    pxor    mm6,mm6
    pcmpeqb mm4,mm6 
    pand    mm4,[esp-8]
    CHROMA_INTRA_P0  mm3, mm2,[pix_tmp+0x20],mm6            ;q0 1
    CHROMA_INTRA_P0  mm2,[pix_tmp+0x40],[pix_tmp+0x20],mm6  ;q0 0
    movq   mm7,[esp-8]
    pand   mm3, mm4
    pandn  mm4, mm2
    pand   mm4, mm7
    pandn  mm7,[pix_tmp+0x30]
    por    mm4, mm7
    movq   mm2, mm4
    por    mm2, mm3
    
    movq  mm0,[pix_tmp+0x10]
    movq  mm3,[pix_tmp+0x40]
    
    add   esp, 96
    mov   edi,[esp+16]
    sub   edi, 2
    mov   ebp, esi
    add   ebp, esi
    add   ebp, esi
    mov   eax, edi
    add   edi, ebp
    TRANSPOSE8x4_STORE  PASS8ROWS(eax, edi, esi, ebp)
    pop  ebp 
    pop  esi
    pop  edi
    ret


