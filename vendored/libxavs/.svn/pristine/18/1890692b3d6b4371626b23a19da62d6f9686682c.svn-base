%include "i386inc.asm"

SECTION .text

cglobal xavs_add8x8_idct8_sse2
;============================================
; Macros and other preprocessor constants
;============================================

%macro XMM_SUMSUB_BA 2
    paddd   %1, %2
    paddd   %2, %2
    psubd   %2, %1
%endmacro

%macro XMM_EXTEND_CBA 3
    movdqu      %2, %1
    pxor        %3, %3 
    pcmpgtw     %3, %1                  ; sign(coeff)      
    punpcklwd   %2, %3
    punpckhwd   %1, %3
%endmacro

xavs_add8x8_idct8_sse2:

;*****************************************************************
    push eax
    push ebx
    
    ;mov eax,edx
    mov ecx,[esp+12]
    mov eax,[esp+16]
    mov ebx,[esp+20]

;***********************************************************
;extend
    ;mov eax,edx
    movdqu      xmm0, [eax+0*16] 
    XMM_EXTEND_CBA  xmm0, xmm1, xmm2 
    movdqu      [ebx+0*16], xmm0
    movdqu      [eax+0*16], xmm1

    movdqu      xmm3, [eax+1*16] 
    XMM_EXTEND_CBA  xmm3, xmm4, xmm5 
    movdqu      [ebx+1*16], xmm3
    movdqu      [eax+1*16], xmm4

    movdqu      xmm6, [eax+2*16] 
    XMM_EXTEND_CBA  xmm6, xmm7, xmm0 
    movdqu      [ebx+2*16], xmm6
    movdqu      [eax+2*16], xmm7

    movdqu      xmm1, [eax+3*16] 
    XMM_EXTEND_CBA  xmm1, xmm2, xmm3 
    movdqu      [ebx+3*16], xmm1
    movdqu      [eax+3*16], xmm2

    movdqu      xmm4, [eax+4*16] 
    XMM_EXTEND_CBA  xmm4, xmm5, xmm6 
    movdqu      [ebx+4*16], xmm4
    movdqu      [eax+4*16], xmm5

    movdqu      xmm7, [eax+5*16] 
    XMM_EXTEND_CBA  xmm7, xmm0, xmm1 
    movdqu      [ebx+5*16], xmm7
    movdqu      [eax+5*16], xmm0

    movdqu      xmm2, [eax+6*16] 
    XMM_EXTEND_CBA  xmm2, xmm3, xmm4 
    movdqu      [ebx+6*16], xmm2
    movdqu      [eax+6*16], xmm3

    movdqu      xmm5, [eax+7*16] 
    XMM_EXTEND_CBA  xmm5, xmm6, xmm7 
    movdqu      [ebx+7*16], xmm5
    movdqu      [eax+7*16], xmm6

;***************************************************************************
;part1
    movdqu      xmm1, [eax+1*16]    ; xmm1 = d1
    movdqu      xmm3, [eax+3*16]    ; xmm3 = d3
    movdqu      xmm5, [eax+5*16]    ; xmm5 = d5
    movdqu      xmm7, [eax+7*16]    ; xmm7 = d7

    movdqu      xmm0, xmm1          ; xmm0 = d1
    movdqu      xmm2, xmm3          ; xmm2 = d3
    movdqu      xmm4, xmm5          ; xmm4 = d5
    movdqu      xmm6, xmm7          ; xmm6 = d7

    XMM_SUMSUB_BA xmm7, xmm1        ; xmm7=d1+d7 xmm1=d1-d7
    XMM_SUMSUB_BA xmm5, xmm3        ; xmm5=d3+d5 xmm3=d3-d5
    pslld       xmm1, 1
    pslld       xmm3, 1
    pslld       xmm5, 1
    pslld       xmm7, 1
    paddd       xmm1, xmm0          ;xmm1=b[0]
    paddd       xmm5, xmm2          ;xmm5=b[1]
    psubd       xmm3, xmm4          ;xmm3=b[2]
    paddd       xmm7, xmm6          ;xmm7=b[3]

    movdqu      xmm0, xmm1          ; xmm0 = b[0]
    movdqu      xmm2, xmm5          ; xmm2 = b[1]
    movdqu      xmm4, xmm3          ; xmm4 = b[2]
    movdqu      xmm6, xmm7          ; xmm6 = b[3]

    psubd       xmm1, xmm3
    psubd       xmm1, xmm7
    pslld       xmm1, 1
    psubd       xmm1, xmm3          ;xmm1=b[7]

    psubd       xmm7, xmm3
    psubd       xmm7, xmm5
    pslld       xmm7, 1
    paddd       xmm7, xmm6          ;xmm7=b[6]

    psubd       xmm3, xmm5
    paddd       xmm3, xmm0
    pslld       xmm3, 1
    paddd       xmm3, xmm0          ;xmm3=b[5]

    paddd       xmm5, xmm6
    paddd       xmm5, xmm0
    pslld       xmm5, 1
    paddd       xmm5, xmm2          ;xmm5=b[4]
   
    movdqu      xmm2, [eax+2*16]    ; xmm2 = d2
    movdqu      xmm6, [eax+6*16]    ; xmm6 = d6
    movdqu      xmm0, xmm2          ; xmm0 = d2
    movdqu      xmm4, xmm6          ; xmm4 = d6

    pslld       xmm2, 1
    pslld       xmm0, 3
    paddd       xmm0, xmm2
    pslld       xmm6, 2
    paddd       xmm6, xmm0          ; xmm6=tmp[2] 
   
    movdqu      xmm0, xmm4          ; xmm0 = d6
    pslld       xmm4, 1
    pslld       xmm0, 3
    paddd       xmm4, xmm0
    pslld       xmm2, 1
    psubd       xmm2, xmm4          ; xmm2=tmp[3]
  
    movdqu      xmm0, [eax+0*16]    ; xmm0 = d0
    movdqu      xmm4, [eax+4*16]    ; xmm4 = d4
    XMM_SUMSUB_BA xmm4, xmm0        ; xmm4=d0+d4 xmm0=d0-d4
    pslld       xmm0, 3             ; xmm0=tmp[1]
    pslld       xmm4, 3             ; xmm4=tmp[0]
     
    XMM_SUMSUB_BA xmm6, xmm4        ; xmm6=b[0] xmm4=b[3]
    XMM_SUMSUB_BA xmm2, xmm0        ; xmm2=b[1] xmm0=b[2]

    XMM_SUMSUB_BA xmm5, xmm6        ; xmm5=dst[0] xmm6=dst[7]
    XMM_SUMSUB_BA xmm3, xmm2        ; xmm3=dst[1] xmm2=dst[6]
    XMM_SUMSUB_BA xmm7, xmm0        ; xmm7=dst[2] xmm0=dst[5]
    XMM_SUMSUB_BA xmm1, xmm4        ; xmm1=dst[3] xmm4=dst[4]


    movdqu      [eax+0*16], xmm5
    movdqu      [eax+1*16], xmm3

    push ecx
    mov  ecx,04h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=04h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=04h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,3
    paddd  xmm1,xmm5
    psrad  xmm1,3
    paddd  xmm2,xmm5
    psrad  xmm2,3
    paddd  xmm4,xmm5
    psrad  xmm4,3
    paddd  xmm6,xmm5
    psrad  xmm6,3
    paddd  xmm7,xmm5
    psrad  xmm7,3

    movdqu      [eax+2*16], xmm7
    movdqu      [eax+3*16], xmm1
    movdqu      [eax+4*16], xmm4
    movdqu      [eax+5*16], xmm0
    movdqu      [eax+6*16], xmm2
    movdqu      [eax+7*16], xmm6
    movdqu      xmm0, [eax+0*16]
    movdqu      xmm1, [eax+1*16]
    paddd       xmm0,xmm5
    psrad       xmm0,3
    paddd       xmm1,xmm5
    psrad       xmm1,3
    movdqu      [eax+0*16], xmm0
    movdqu      [eax+1*16], xmm1
    
    
;********************************************************************
;part2
    movdqu      xmm1, [ebx+1*16]    ; xmm1 = d1
    movdqu      xmm3, [ebx+3*16]    ; xmm3 = d3
    movdqu      xmm5, [ebx+5*16]    ; xmm5 = d5
    movdqu      xmm7, [ebx+7*16]    ; xmm7 = d7

    movdqu      xmm0, xmm1          ; xmm0 = d1
    movdqu      xmm2, xmm3          ; xmm2 = d3
    movdqu      xmm4, xmm5          ; xmm4 = d5
    movdqu      xmm6, xmm7          ; xmm6 = d7

    XMM_SUMSUB_BA xmm7, xmm1        ; xmm7=d1+d7 xmm1=d1-d7
    XMM_SUMSUB_BA xmm5, xmm3        ; xmm5=d3+d5 xmm3=d3-d5
    pslld       xmm1, 1
    pslld       xmm3, 1
    pslld       xmm5, 1
    pslld       xmm7, 1
    paddd       xmm1, xmm0          ;xmm1=b[0]
    paddd       xmm5, xmm2          ;xmm5=b[1]
    psubd       xmm3, xmm4          ;xmm3=b[2]
    paddd       xmm7, xmm6          ;xmm7=b[3]

    movdqu      xmm0, xmm1          ; xmm0 = b[0]
    movdqu      xmm2, xmm5          ; xmm2 = b[1]
    movdqu      xmm4, xmm3          ; xmm4 = b[2]
    movdqu      xmm6, xmm7          ; xmm6 = b[3]

    psubd       xmm1, xmm3
    psubd       xmm1, xmm7
    pslld       xmm1, 1
    psubd       xmm1, xmm3          ;xmm1=b[7]

    psubd       xmm7, xmm3
    psubd       xmm7, xmm5
    pslld       xmm7, 1
    paddd       xmm7, xmm6          ;xmm7=b[6]

    psubd       xmm3, xmm5
    paddd       xmm3, xmm0
    pslld       xmm3, 1
    paddd       xmm3, xmm0          ;xmm3=b[5]

    paddd       xmm5, xmm6
    paddd       xmm5, xmm0
    pslld       xmm5, 1
    paddd       xmm5, xmm2          ;xmm5=b[4]
   
    movdqu      xmm2, [ebx+2*16]    ; xmm2 = d2
    movdqu      xmm6, [ebx+6*16]    ; xmm6 = d6
    movdqu      xmm0, xmm2          ; xmm0 = d2
    movdqu      xmm4, xmm6          ; xmm4 = d6

    pslld       xmm2, 1
    pslld       xmm0, 3
    paddd       xmm0, xmm2
    pslld       xmm6, 2
    paddd       xmm6, xmm0          ; xmm6=tmp[2] 
   
    movdqu      xmm0, xmm4          ; xmm0 = d6
    pslld       xmm4, 1
    pslld       xmm0, 3
    paddd       xmm4, xmm0
    pslld       xmm2, 1
    psubd       xmm2, xmm4          ; xmm2=tmp[3]
  
    movdqu      xmm0, [ebx+0*16]    ; xmm0 = d0
    movdqu      xmm4, [ebx+4*16]    ; xmm4 = d4
    XMM_SUMSUB_BA xmm4, xmm0        ; xmm4=d0+d4 xmm0=d0-d4
    pslld       xmm0, 3             ; xmm0=tmp[1]
    pslld       xmm4, 3             ; xmm4=tmp[0]
     
    XMM_SUMSUB_BA xmm6, xmm4        ; xmm6=b[0] xmm4=b[3]
    XMM_SUMSUB_BA xmm2, xmm0        ; xmm2=b[1] xmm0=b[2]

    XMM_SUMSUB_BA xmm5, xmm6        ; xmm5=dst[0] xmm6=dst[7]
    XMM_SUMSUB_BA xmm3, xmm2        ; xmm3=dst[1] xmm2=dst[6]
    XMM_SUMSUB_BA xmm7, xmm0        ; xmm7=dst[2] xmm0=dst[5]
    XMM_SUMSUB_BA xmm1, xmm4        ; xmm1=dst[3] xmm4=dst[4]

    movdqu      [ebx+0*16], xmm5
    movdqu      [ebx+1*16], xmm3
    push ecx
    mov  ecx,04h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=04h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=04h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,3
    paddd  xmm1,xmm5
    psrad  xmm1,3
    paddd  xmm2,xmm5
    psrad  xmm2,3
    paddd  xmm4,xmm5
    psrad  xmm4,3
    paddd  xmm6,xmm5
    psrad  xmm6,3
    paddd  xmm7,xmm5
    psrad  xmm7,3

    movdqu      [ebx+2*16], xmm7
    movdqu      [ebx+3*16], xmm1
    movdqu      [ebx+4*16], xmm4
    movdqu      [ebx+5*16], xmm0
    movdqu      [ebx+6*16], xmm2
    movdqu      [ebx+7*16], xmm6
    movdqu      xmm0, [ebx+0*16]
    movdqu      xmm1, [ebx+1*16]
    paddd       xmm0,xmm5
    psrad       xmm0,3
    paddd       xmm1,xmm5
    psrad       xmm1,3
    movdqu      [ebx+0*16], xmm0
    movdqu      [ebx+1*16], xmm1
    
    
;****************************************
    movdqu      xmm0,  [eax+0*16]  
    movdqu      xmm2,  [eax+1*16]
    movdqu      xmm4,  [eax+2*16]
    movdqu      xmm6,  [eax+3*16]
 
    movdqu      xmm1,  [ebx+0*16]  
    movdqu      xmm3,  [ebx+1*16]
    movdqu      xmm5,  [ebx+2*16]
    movdqu      xmm7,  [ebx+3*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+0*16], xmm0  
    movdqu      [eax+1*16], xmm2
    movdqu      [eax+2*16], xmm4
    movdqu      [eax+3*16], xmm6
    
    movdqu      xmm0,  [eax+4*16]  
    movdqu      xmm2,  [eax+5*16]
    movdqu      xmm4,  [eax+6*16]
    movdqu      xmm6,  [eax+7*16]
 
    movdqu      xmm1,  [ebx+4*16]  
    movdqu      xmm3,  [ebx+5*16]
    movdqu      xmm5,  [ebx+6*16]
    movdqu      xmm7,  [ebx+7*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+4*16], xmm0  
    movdqu      [eax+5*16], xmm2
    movdqu      [eax+6*16], xmm4
    movdqu      [eax+7*16], xmm6
   
   
   ;**************************************************************
;tranpose8x8

    movdqu xmm0, [eax + 0]  ;    xmm0:a7a6a5a4a3a2a1a0
    movdqu xmm1, [eax + 16] ;    xmm1:b7b6b5b4b3b2b1b0
    movdqu xmm2, [eax + 32] ;    xmm2:c7c6c5c4c3c2c1c0
    movdqu xmm3, [eax + 48] ;    xmm3:d7d6d5d4d3d2d1d0
   
    pxor   xmm4,xmm4       
    paddw  xmm4,xmm0 
    punpcklwd xmm0,xmm1     ;    xmm0:b3a3b2a2b1a1b0a0
    punpckhwd xmm4,xmm1     ;    xmm4:b7a7b6a6b5a5b4a4
   
    pxor   xmm5,xmm5
    paddw  xmm5,xmm2 
    punpcklwd xmm2,xmm3     ;    xmm2:d3c3d2c2d1c1d0c0
    punpckhwd xmm5,xmm3     ;    xmm5:d7c7d6c6d5c5d4c4

    pxor   xmm6,xmm6
    paddw  xmm6,xmm0 
    punpckldq xmm0,xmm2     ;    xmm0:d1c1b1a1d0c0b0a0
    punpckhdq xmm6,xmm2     ;    xmm6:d3c3b3a3d2c2b2a2

    pxor   xmm7,xmm7
    paddw  xmm7,xmm4 
    punpckldq xmm4,xmm5     ;    xmm4:d5c5b5a5d4c4b4a4
    punpckhdq xmm7,xmm5     ;    xmm7:d7c7b7a7d6c6b6a6

    movdqu  [eax + 0],xmm0  ;    d1c1b1a1d0c0b0a0
    movdqu  [eax + 16],xmm6 ;    d3c3b3a3d2c2b2a2
    movdqu  [eax + 32],xmm4 ;    d5c5b5a5d4c4b4a4
    movdqu  [eax + 48],xmm7 ;    d7c7b7a7d6c6b6a6

    
    movdqu xmm0, [eax + 64] ;    xmm0:e7e6e5e4e3e2e1e0
    movdqu xmm1, [eax + 80] ;    xmm1:f7f6f5f4f3f2f1f0
    movdqu xmm2, [eax + 96] ;    xmm2:g7g6g5g4g3g2g1g0 
    movdqu xmm3, [eax + 112];    xmm3:h7h6h5h4h3h2h1h0
   
    pxor   xmm4,xmm4       
    paddw  xmm4,xmm0 
    punpcklwd xmm0,xmm1     ;    xmm0:f3e3f2e2f1e1f0e0
    punpckhwd xmm4,xmm1     ;    xmm4:f7e7f6e6f5e5f4e4
   
    pxor   xmm5,xmm5
    paddw  xmm5,xmm2 
    punpcklwd xmm2,xmm3     ;    xmm2:h3g3h2g2h1g1h0g0
    punpckhwd xmm5,xmm3     ;    xmm5:h7g7h6g6h5g5h4g4

    pxor   xmm6,xmm6
    paddw  xmm6,xmm0 
    punpckldq xmm0,xmm2     ;    xmm0:h1g1f1e1h0g0f0e0
    punpckhdq xmm6,xmm2     ;    xmm6:h3g3f3e3h2g2f2e2

    pxor   xmm7,xmm7
    paddw  xmm7,xmm4 
    punpckldq xmm4,xmm5     ;    xmm4:h5g5f5e5h4g4f4e4
    punpckhdq xmm7,xmm5     ;    xmm7:h7g7f7e7h6g6f6e6

   
    movdqu  xmm1,[eax + 0]  ;    xmm1:d1c1b1a1d0c0b0a0  
    pxor   xmm5,xmm5       
    paddw  xmm5,xmm1
    punpcklqdq xmm1,xmm0    ;    xmm1:h0g0f0e0d0c0b0a0
    punpckhqdq xmm5,xmm0    ;    xmm5:h1g1f1e1d1c1b1a1
    movdqu  [eax + 0],xmm1  ;    
    
    movdqu xmm2,[eax + 16]  ;    xmm2:d3c3b3a3d2c2b2a2 
    movdqu  [eax + 16],xmm5 ;
    pxor   xmm3,xmm3       
    paddw  xmm3,xmm2
    punpcklqdq xmm2,xmm6    ;    xmm2:h2g2f2e2d2c2b2a2
    punpckhqdq xmm3,xmm6    ;    xmm3:h3g3f3e3d3c3b3a3

    movdqu xmm5,[eax + 32]  ;    xmm5:d5c5b5a5d4c4b4a4
    movdqu  [eax + 32],xmm2 ;
    pxor   xmm1,xmm1      
    paddw  xmm1,xmm5
    punpcklqdq xmm5,xmm4    ;    xmm5:h4g4f4e4d4c4b4a4
    punpckhqdq xmm1,xmm4    ;    xmm1:h5g5f5e5d5c5b5a5
    movdqu  [eax + 64],xmm5 ;    
    movdqu  [eax + 80],xmm1 ;    

    movdqu  xmm6,[eax + 48] ;    xmm6:d7c7b7a7d6c6b6a6 
    movdqu  [eax + 48],xmm3 ;
    pxor   xmm2,xmm2       
    paddw  xmm2,xmm6
    punpcklqdq xmm2,xmm7    ;    xmm2:h6g6f6e6d6c6b6a6
    punpckhqdq xmm6,xmm7    ;    xmm6:h7g7f7e7d7c7b7a7
    movdqu  [eax + 112],xmm6;    
    movdqu  [eax + 96],xmm2 ; 
 
;***********************************************************
;extend

   movdqu      xmm0, [eax+0*16] 
    XMM_EXTEND_CBA  xmm0, xmm1, xmm2 
    movdqu      [ebx+0*16], xmm0
    movdqu      [eax+0*16], xmm1

    movdqu      xmm3, [eax+1*16] 
    XMM_EXTEND_CBA  xmm3, xmm4, xmm5 
    movdqu      [ebx+1*16], xmm3
    movdqu      [eax+1*16], xmm4

    movdqu      xmm6, [eax+2*16] 
    XMM_EXTEND_CBA  xmm6, xmm7, xmm0 
    movdqu      [ebx+2*16], xmm6
    movdqu      [eax+2*16], xmm7

    movdqu      xmm1, [eax+3*16] 
    XMM_EXTEND_CBA  xmm1, xmm2, xmm3 
    movdqu      [ebx+3*16], xmm1
    movdqu      [eax+3*16], xmm2

    movdqu      xmm4, [eax+4*16] 
    XMM_EXTEND_CBA  xmm4, xmm5, xmm6 
    movdqu      [ebx+4*16], xmm4
    movdqu      [eax+4*16], xmm5

    movdqu      xmm7, [eax+5*16] 
    XMM_EXTEND_CBA  xmm7, xmm0, xmm1 
    movdqu      [ebx+5*16], xmm7
    movdqu      [eax+5*16], xmm0

    movdqu      xmm2, [eax+6*16] 
    XMM_EXTEND_CBA  xmm2, xmm3, xmm4 
    movdqu      [ebx+6*16], xmm2
    movdqu      [eax+6*16], xmm3

    movdqu      xmm5, [eax+7*16] 
    XMM_EXTEND_CBA  xmm5, xmm6, xmm7 
    movdqu      [ebx+7*16], xmm5
    movdqu      [eax+7*16], xmm6
    
    
;************************************************************
;part1
    movdqu      xmm1, [eax+1*16]    ; xmm1 = d1
    movdqu      xmm3, [eax+3*16]    ; xmm3 = d3
    movdqu      xmm5, [eax+5*16]    ; xmm5 = d5
    movdqu      xmm7, [eax+7*16]    ; xmm7 = d7

    movdqu      xmm0, xmm1          ; xmm0 = d1
    movdqu      xmm2, xmm3          ; xmm2 = d3
    movdqu      xmm4, xmm5          ; xmm4 = d5
    movdqu      xmm6, xmm7          ; xmm6 = d7

    XMM_SUMSUB_BA xmm7, xmm1        ; xmm7=d1+d7 xmm1=d1-d7
    XMM_SUMSUB_BA xmm5, xmm3        ; xmm5=d3+d5 xmm3=d3-d5
    pslld       xmm1, 1
    pslld       xmm3, 1
    pslld       xmm5, 1
    pslld       xmm7, 1
    paddd       xmm1, xmm0          ;xmm1=b[0]
    paddd       xmm5, xmm2          ;xmm5=b[1]
    psubd       xmm3, xmm4          ;xmm3=b[2]
    paddd       xmm7, xmm6          ;xmm7=b[3]

    movdqu      xmm0, xmm1          ; xmm0 = b[0]
    movdqu      xmm2, xmm5          ; xmm2 = b[1]
    movdqu      xmm4, xmm3          ; xmm4 = b[2]
    movdqu      xmm6, xmm7          ; xmm6 = b[3]

    psubd       xmm1, xmm3
    psubd       xmm1, xmm7
    pslld       xmm1, 1
    psubd       xmm1, xmm3          ;xmm1=b[7]

    psubd       xmm7, xmm3
    psubd       xmm7, xmm5
    pslld       xmm7, 1
    paddd       xmm7, xmm6          ;xmm7=b[6]

    psubd       xmm3, xmm5
    paddd       xmm3, xmm0
    pslld       xmm3, 1
    paddd       xmm3, xmm0          ;xmm3=b[5]

    paddd       xmm5, xmm6
    paddd       xmm5, xmm0
    pslld       xmm5, 1
    paddd       xmm5, xmm2          ;xmm5=b[4]
   
    movdqu      xmm2, [eax+2*16]    ; xmm2 = d2
    movdqu      xmm6, [eax+6*16]    ; xmm6 = d6
    movdqu      xmm0, xmm2          ; xmm0 = d2
    movdqu      xmm4, xmm6          ; xmm4 = d6

    pslld       xmm2, 1
    pslld       xmm0, 3
    paddd       xmm0, xmm2
    pslld       xmm6, 2
    paddd       xmm6, xmm0          ; xmm6=tmp[2] 
   
    movdqu      xmm0, xmm4          ; xmm0 = d6
    pslld       xmm4, 1
    pslld       xmm0, 3
    paddd       xmm4, xmm0
    pslld       xmm2, 1
    psubd       xmm2, xmm4          ; xmm2=tmp[3]
  
    movdqu      xmm0, [eax+0*16]    ; xmm0 = d0
    movdqu      xmm4, [eax+4*16]    ; xmm4 = d4
    XMM_SUMSUB_BA xmm4, xmm0        ; xmm4=d0+d4 xmm0=d0-d4
    pslld       xmm0, 3             ; xmm0=tmp[1]
    pslld       xmm4, 3             ; xmm4=tmp[0]
     
    XMM_SUMSUB_BA xmm6, xmm4        ; xmm6=b[0] xmm4=b[3]
    XMM_SUMSUB_BA xmm2, xmm0        ; xmm2=b[1] xmm0=b[2]

    XMM_SUMSUB_BA xmm5, xmm6        ; xmm5=dst[0] xmm6=dst[7]
    XMM_SUMSUB_BA xmm3, xmm2        ; xmm3=dst[1] xmm2=dst[6]
    XMM_SUMSUB_BA xmm7, xmm0        ; xmm7=dst[2] xmm0=dst[5]
    XMM_SUMSUB_BA xmm1, xmm4        ; xmm1=dst[3] xmm4=dst[4]
    
    movdqu      [eax+0*16], xmm5
    movdqu      [eax+1*16], xmm3
    push ecx
    mov  ecx,40h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=40h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=40h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,7
    paddd  xmm1,xmm5
    psrad  xmm1,7
    paddd  xmm2,xmm5
    psrad  xmm2,7
    paddd  xmm4,xmm5
    psrad  xmm4,7
    paddd  xmm6,xmm5
    psrad  xmm6,7
    paddd  xmm7,xmm5
    psrad  xmm7,7

    movdqu      [eax+2*16], xmm7
    movdqu      [eax+3*16], xmm1
    movdqu      [eax+4*16], xmm4
    movdqu      [eax+5*16], xmm0
    movdqu      [eax+6*16], xmm2
    movdqu      [eax+7*16], xmm6
    movdqu      xmm0, [eax+0*16]
    movdqu      xmm1, [eax+1*16]
    paddd       xmm0,xmm5
    psrad       xmm0,7
    paddd       xmm1,xmm5
    psrad       xmm1,7
    movdqu      [eax+0*16], xmm0
    movdqu      [eax+1*16], xmm1
;********************************************************************
;part2
    movdqu      xmm1, [ebx+1*16]    ; xmm1 = d1
    movdqu      xmm3, [ebx+3*16]    ; xmm3 = d3
    movdqu      xmm5, [ebx+5*16]    ; xmm5 = d5
    movdqu      xmm7, [ebx+7*16]    ; xmm7 = d7

    movdqu      xmm0, xmm1          ; xmm0 = d1
    movdqu      xmm2, xmm3          ; xmm2 = d3
    movdqu      xmm4, xmm5          ; xmm4 = d5
    movdqu      xmm6, xmm7          ; xmm6 = d7

    XMM_SUMSUB_BA xmm7, xmm1        ; xmm7=d1+d7 xmm1=d1-d7
    XMM_SUMSUB_BA xmm5, xmm3        ; xmm5=d3+d5 xmm3=d3-d5
    pslld       xmm1, 1
    pslld       xmm3, 1
    pslld       xmm5, 1
    pslld       xmm7, 1
    paddd       xmm1, xmm0          ;xmm1=b[0]
    paddd       xmm5, xmm2          ;xmm5=b[1]
    psubd       xmm3, xmm4          ;xmm3=b[2]
    paddd       xmm7, xmm6          ;xmm7=b[3]

    movdqu      xmm0, xmm1          ; xmm0 = b[0]
    movdqu      xmm2, xmm5          ; xmm2 = b[1]
    movdqu      xmm4, xmm3          ; xmm4 = b[2]
    movdqu      xmm6, xmm7          ; xmm6 = b[3]

    psubd       xmm1, xmm3
    psubd       xmm1, xmm7
    pslld       xmm1, 1
    psubd       xmm1, xmm3          ;xmm1=b[7]

    psubd       xmm7, xmm3
    psubd       xmm7, xmm5
    pslld       xmm7, 1
    paddd       xmm7, xmm6          ;xmm7=b[6]

    psubd       xmm3, xmm5
    paddd       xmm3, xmm0
    pslld       xmm3, 1
    paddd       xmm3, xmm0          ;xmm3=b[5]

    paddd       xmm5, xmm6
    paddd       xmm5, xmm0
    pslld       xmm5, 1
    paddd       xmm5, xmm2          ;xmm5=b[4]
   
    movdqu      xmm2, [ebx+2*16]    ; xmm2 = d2
    movdqu      xmm6, [ebx+6*16]    ; xmm6 = d6
    movdqu      xmm0, xmm2          ; xmm0 = d2
    movdqu      xmm4, xmm6          ; xmm4 = d6

    pslld       xmm2, 1
    pslld       xmm0, 3
    paddd       xmm0, xmm2
    pslld       xmm6, 2
    paddd       xmm6, xmm0          ; xmm6=tmp[2] 
   
    movdqu      xmm0, xmm4          ; xmm0 = d6
    pslld       xmm4, 1
    pslld       xmm0, 3
    paddd       xmm4, xmm0
    pslld       xmm2, 1
    psubd       xmm2, xmm4          ; xmm2=tmp[3]
  
    movdqu      xmm0, [ebx+0*16]    ; xmm0 = d0
    movdqu      xmm4, [ebx+4*16]    ; xmm4 = d4
    XMM_SUMSUB_BA xmm4, xmm0        ; xmm4=d0+d4 xmm0=d0-d4
    pslld       xmm0, 3             ; xmm0=tmp[1]
    pslld       xmm4, 3             ; xmm4=tmp[0]
     
    XMM_SUMSUB_BA xmm6, xmm4        ; xmm6=b[0] xmm4=b[3]
    XMM_SUMSUB_BA xmm2, xmm0        ; xmm2=b[1] xmm0=b[2]

    XMM_SUMSUB_BA xmm5, xmm6        ; xmm5=dst[0] xmm6=dst[7]
    XMM_SUMSUB_BA xmm3, xmm2        ; xmm3=dst[1] xmm2=dst[6]
    XMM_SUMSUB_BA xmm7, xmm0        ; xmm7=dst[2] xmm0=dst[5]
    XMM_SUMSUB_BA xmm1, xmm4        ; xmm1=dst[3] xmm4=dst[4]

    movdqu      [ebx+0*16], xmm5
    movdqu      [ebx+1*16], xmm3
    push ecx
    mov  ecx,40h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=40h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=40h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,7
    paddd  xmm1,xmm5
    psrad  xmm1,7
    paddd  xmm2,xmm5
    psrad  xmm2,7
    paddd  xmm4,xmm5
    psrad  xmm4,7
    paddd  xmm6,xmm5
    psrad  xmm6,7
    paddd  xmm7,xmm5
    psrad  xmm7,7

    movdqu      [ebx+2*16], xmm7
    movdqu      [ebx+3*16], xmm1
    movdqu      [ebx+4*16], xmm4
    movdqu      [ebx+5*16], xmm0
    movdqu      [ebx+6*16], xmm2
    movdqu      [ebx+7*16], xmm6
    movdqu      xmm0, [ebx+0*16]
    movdqu      xmm1, [ebx+1*16]
    paddd       xmm0,xmm5
    psrad       xmm0,7
    paddd       xmm1,xmm5
    psrad       xmm1,7
    movdqu      [ebx+0*16], xmm0
    movdqu      [ebx+1*16], xmm1

;*************************************************************
    movdqu      xmm0,  [eax+0*16]  
    movdqu      xmm2,  [eax+1*16]
    movdqu      xmm4,  [eax+2*16]
    movdqu      xmm6,  [eax+3*16]
 
    movdqu      xmm1,  [ebx+0*16]  
    movdqu      xmm3,  [ebx+1*16]
    movdqu      xmm5,  [ebx+2*16]
    movdqu      xmm7,  [ebx+3*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+0*16], xmm0  
    movdqu      [eax+1*16], xmm2
    movdqu      [eax+2*16], xmm4
    movdqu      [eax+3*16], xmm6
    
    movdqu      xmm0,  [eax+4*16]  
    movdqu      xmm2,  [eax+5*16]
    movdqu      xmm4,  [eax+6*16]
    movdqu      xmm6,  [eax+7*16]
 
    movdqu      xmm1,  [ebx+4*16]  
    movdqu      xmm3,  [ebx+5*16]
    movdqu      xmm5,  [ebx+6*16]
    movdqu      xmm7,  [ebx+7*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+4*16], xmm0  
    movdqu      [eax+5*16], xmm2
    movdqu      [eax+6*16], xmm4
    movdqu      [eax+7*16], xmm6
   
;***************************************************************
    pxor xmm0,xmm0

    movdqu xmm4,[ecx+4*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+4*16]
    paddw xmm4,xmm1 
    packuswb xmm4,xmm2
    movdqu [ecx+4*FDEC_STRIDE], xmm4  
    
    movdqu xmm4,[ecx+5*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+5*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+5*FDEC_STRIDE], xmm4 
    
    movdqu xmm4,[ecx+6*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+6*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+6*FDEC_STRIDE], xmm4  
    
    movdqu xmm4,[ecx+7*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+7*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+7*FDEC_STRIDE], xmm4  
    
    movdqu xmm4,[ecx+0*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+0*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+0*FDEC_STRIDE], xmm4  
    
    movdqu xmm4,[ecx+1*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+1*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+1*FDEC_STRIDE], xmm4 
    
    movdqu xmm4,[ecx+2*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+2*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+2*FDEC_STRIDE], xmm4
    
    movdqu xmm4,[ecx+3*FDEC_STRIDE]
    movdqu xmm2,xmm4
    punpcklbw  xmm4,xmm0
    punpckhbw  xmm2,xmm0
    movdqu xmm1,[eax+3*16]
    paddw xmm4,xmm1
    packuswb xmm4,xmm2
    movdqu [ecx+3*FDEC_STRIDE], xmm4    
    
    pop ebx
    pop eax
    ret

 ;*****************************************************************************************  
   cglobal xavs_sub8x8_dct8_sse2

%macro XMM_EXTEND_CBA 3
    movdqu      %2, %1
    pxor        %3, %3 
    pcmpgtw     %3, %1                  ; sign(coeff)       
    punpcklwd   %2, %3
    punpckhwd   %1, %3
%endmacro

xavs_sub8x8_dct8_sse2:
;h transform 
     
    push  ebx
    push  edx

    mov   ecx, [esp+12]; dct     
    mov   eax, [esp+16]; pix1    
    mov   edx, [esp+20]; pix2 
    mov   ebx, [esp+24]; tmp
    pxor xmm0,xmm0

    movq xmm4,[eax+4*FENC_STRIDE]
    punpcklbw  xmm4,xmm0
    movq xmm1,[edx+4*FDEC_STRIDE]
    punpcklbw  xmm1,xmm0
    psubw xmm4,xmm1
    movdqu [ecx+64], xmm4  
    
    movq xmm5,[eax+5*FENC_STRIDE]
    punpcklbw  xmm5,xmm0
    movq xmm2,[edx+5*FDEC_STRIDE]
    punpcklbw  xmm2,xmm0
    psubw xmm5,xmm2
    movdqu [ecx+80], xmm5

    movq xmm6,[eax+6*FENC_STRIDE]
    punpcklbw  xmm6,xmm0
    movq xmm3,[edx+6*FDEC_STRIDE]
    punpcklbw  xmm3,xmm0
    psubw xmm6,xmm3
    movdqu [ecx+96], xmm6


    movq xmm7,[eax+7*FENC_STRIDE]
    punpcklbw  xmm7,xmm0
    movq xmm4,[edx+7*FDEC_STRIDE]
    punpcklbw  xmm4,xmm0
    psubw xmm7,xmm4
    movdqu [ecx+112], xmm7


    pxor xmm6,xmm6

    movq xmm0,[eax+0*FENC_STRIDE]
    punpcklbw  xmm0,xmm6
    movq xmm4,[edx+0*FDEC_STRIDE]
    punpcklbw  xmm4,xmm6
    psubw xmm0,xmm4
    movdqu [ecx], xmm0

    movq xmm1,[eax+1*FENC_STRIDE]
    punpcklbw  xmm1,xmm6
    movq xmm4,[edx+1*FDEC_STRIDE]
    punpcklbw  xmm4,xmm6
    psubw xmm1,xmm4
    movdqu [ecx+16], xmm1

    movq xmm2,[eax+2*FENC_STRIDE]
    punpcklbw  xmm2,xmm6
    movq xmm4,[edx+2*FDEC_STRIDE]
    punpcklbw  xmm4,xmm6
    psubw xmm2,xmm4
    movdqu [ecx+32], xmm2


    movq xmm3,[eax+3*FENC_STRIDE]
    punpcklbw  xmm3,xmm6
    movq xmm4,[edx+3*FDEC_STRIDE]
    punpcklbw  xmm4,xmm6
    psubw xmm3,xmm4
    movdqu [ecx+48], xmm3


    mov eax,ecx
    mov ebx,[esp+24]
    movdqu xmm0, [eax + 0]  ;    
    movdqu xmm3, [eax + 48] ;   
    movdqu xmm4, [eax + 64] ;    
    movdqu xmm7, [eax + 112] ;   
    
    pxor   xmm1,xmm1
    paddw  xmm1,xmm0   
    paddw  xmm0,xmm7;    xmm0=s07=_mm_add_epi16(SRC_SSE[0], SRC_SSE[7]);
    psubw  xmm1,xmm7;    xmm1=d07=_mm_sub_epi16(SRC_SSE[0], SRC_SSE[7]);
    
    pxor   xmm2,xmm2
    paddw  xmm2,xmm3
    paddw  xmm3,xmm4;    xmm3=s34 = _mm_add_epi16(SRC_SSE[3], SRC_SSE[4]);
    psubw  xmm2,xmm4;    xmm2=d34 = _mm_sub_epi16(SRC_SSE[3], SRC_SSE[4]);
    
    pxor   xmm7,xmm7
    paddw  xmm7,xmm0
    paddw  xmm0,xmm3;    xmm0=a0=_mm_add_epi16(s07, s34);
    psubw  xmm7,xmm3;    xmm7=a2=_mm_sub_epi16(s07, s34); 
    pxor   xmm3,xmm3
    paddw  xmm3,xmm1
    psubw  xmm1,xmm2;    xmm1=_mm_sub_epi16(d07, d34)
    psllw  xmm1,1;       xmm1=_mm_slli_epi16 (_mm_sub_epi16(d07, d34), 1);
    paddw  xmm1,xmm3;    xmm1=a4=_mm_add_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d07, d34), 1), d07);

    paddw  xmm3,xmm2;    xmm3=_mm_add_epi16(d07, d34);
    psllw  xmm3,1;       xmm3=_mm_slli_epi16 (_mm_add_epi16(d07, d34), 1);
    paddw  xmm3,xmm2;    xmm3=a7=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d07, d34), 1), d34);
      
    movdqu [eax + 64],xmm1;      a4
    movdqu [eax + 112],xmm3;     a7
    
    movdqu xmm1, [eax + 16] ;    xmm1:SRC_SSE[1]
    movdqu xmm2, [eax + 32] ;    xmm2:SRC_SSE[2]
    movdqu xmm5, [eax + 80] ;    xmm5:SRC_SSE[5]
    movdqu xmm6, [eax + 96] ;    xmm6:SRC_SSE[6]
    
    pxor   xmm4,xmm4
    paddw  xmm4,xmm1   
    paddw  xmm1,xmm6;    xmm1=s16 =_mm_add_epi16(SRC_SSE[1], SRC_SSE[6]);
    psubw  xmm4,xmm6;    xmm4=d16 = _mm_sub_epi16(SRC_SSE[1], SRC_SSE[6]);

    pxor   xmm3,xmm3
    paddw  xmm3,xmm2
    paddw  xmm2,xmm5;    xmm2=s25 = _mm_add_epi16(SRC_SSE[2], SRC_SSE[5]);
    psubw  xmm3,xmm5;    xmm3=d25 = _mm_sub_epi16(SRC_SSE[2], SRC_SSE[5]);

    pxor   xmm5,xmm5
    paddw  xmm5,xmm1
    paddw  xmm1,xmm2;    xmm1=a1 = _mm_add_epi16(s16, s25);
    psubw  xmm5,xmm2;    xmm5=a3 = _mm_sub_epi16(s16, s25);
    pxor   xmm2,xmm2
    paddw  xmm2,xmm4
    paddw  xmm4,xmm3;    xmm4= _mm_add_epi16(d16, d25);
    psllw  xmm4,1;       xmm4=_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1);
    paddw  xmm4,xmm2;    xmm4=a5=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1), d16);
    
    psubw  xmm2,xmm3;    xmm2=_mm_sub_epi16(d16, d25)
    psllw  xmm2,1;       xmm2=_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1);
    psubw  xmm2,xmm3;    xmm2=a6=_mm_sub_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1), d25);
 
    movdqu [eax + 80],xmm2 ;      a6
    movdqu [eax + 96],xmm4;       a5 

    pxor   xmm2,xmm2
    paddw  xmm2,xmm0
    paddw  xmm0,xmm1;   xmm0=_mm_add_epi16(a0,a1);
    psllw  xmm0,3;      xmm0= DST_SSE[0] = _mm_slli_epi16(_mm_add_epi16(a0,a1), 3);
    psubw  xmm2,xmm1;   xmm2= _mm_sub_epi16(d16, d25);
    psllw  xmm2,3;      xmm2= DST_SSE[4] = _mm_slli_epi16(_mm_sub_epi16(a0,a1), 3);

    pxor   xmm4,xmm4
    paddw  xmm4,xmm7
    psllw  xmm7,3;      xmm7= _mm_slli_epi16(a2,3);
    psllw  xmm4,1;      xmm4= _mm_slli_epi16(a2,1);
    paddw  xmm7,xmm4;   xmm7= _mm_add_epi16( _mm_slli_epi16(a2,3), _mm_slli_epi16(a2,1));

    pxor   xmm3,xmm3   
    paddw  xmm3,xmm5
    psllw  xmm5,2;      xmm5=_mm_slli_epi16(a3,2);
    paddw  xmm7,xmm5;   xmm7=DST_SSE[2];
    
    psllw  xmm4,1;      xmm4=_mm_slli_epi16(a2,2);
    psllw  xmm3,1;      xmm3=_mm_slli_epi16(a3,1);
    psllw  xmm5,1;      xmm5=_mm_slli_epi16(a3,2);
    paddw  xmm3,xmm5;   xmm3= _mm_add_epi16( _mm_slli_epi16(a3,3), _mm_slli_epi16(a3,1));
    psubw  xmm4,xmm3;   xmm4=DST_SSE[6];


    movdqu [eax+0],xmm0;    DST_SSE[0]
    movdqu [eax+16],xmm7;   DST_SSE[2]
    movdqu [eax+32],xmm2;   DST_SSE[4]
    movdqu [eax+48],xmm4;   DST_SSE[6]

    movdqu xmm1,[eax + 64];    a4
    movdqu xmm3,[eax + 112];   a7
    movdqu xmm2,[eax + 80];    a6
    movdqu xmm4,[eax + 96];    a5 

    pxor   xmm0,xmm0   
    paddw  xmm0,xmm1
    psubw  xmm1,xmm4;   xmm1=_mm_sub_epi16(a4,a5);
    paddw  xmm1,xmm2;   xmm1=_mm_add_epi16( _mm_sub_epi16(a4,a5),a6);
    psllw  xmm1,1;      xmm1=_mm_slli_epi16( _mm_add_epi16( _mm_sub_epi16(a4,a5),a6),1);
    paddw  xmm1,xmm0;   xmm1=DST_SSE[3];
   
    paddw  xmm0,xmm4;   xmm0=_mm_add_epi16(a4,a5);
    paddw  xmm0,xmm3;   xmm0=_mm_add_epi16( _mm_add_epi16(a4,a5),a7);   
    psllw  xmm0,1;      xmm0=_mm_slli_epi16( _mm_add_epi16( _mm_add_epi16(a4,a5),a7),1);
    paddw  xmm0,xmm4;   xmm0=DST_SSE[1];
   
    pxor   xmm7,xmm7   
    paddw  xmm7,xmm3
    psubw  xmm3,xmm4;   xmm3=_mm_sub_epi16(a7,a5);
    psubw  xmm3,xmm2;   xmm3=_mm_sub_epi16( _mm_sub_epi16(a7,a5),a6);
    psllw  xmm3,1;      xmm3=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a7,a5),a6),1);
    paddw  xmm3,xmm7;   xmm3=DST_SSE[5];

    movdqu xmm6,[eax + 64];    a4
    psubw  xmm6,xmm7;   xmm6=_mm_sub_epi16(a4,a7);
    psubw  xmm6,xmm2;   xmm6=_mm_sub_epi16( _mm_sub_epi16(a4,a7),a6);
    psllw  xmm6,1;      xmm6=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a4,a7),a6),1);
    psubw  xmm6,xmm2;   xmm6=DST_SSE[7]

    movdqu xmm7,[eax+16]
    movdqu xmm2,xmm7;        xmm2=DST_SSE[2]
    movdqu [eax+16],xmm0;    DST_SSE[1]

    movdqu xmm4,[eax+32];    xmm4=DST_SSE[4]
    movdqu [eax+32],xmm2;    DST_SSE[2]

    movdqu xmm0 ,[eax+48];    xmm0=DST_SSE[6]
    movdqu [eax+48],xmm1;    DST_SSE[3]
   
    movdqu [eax+64],xmm4;    DST_SSE[4]
    movdqu [eax+80],xmm3;    DST_SSE[5]
    movdqu [eax+96],xmm0;    DST_SSE[6]
    movdqu [eax+112],xmm6;   DST_SSE[7]

;****************************************
;tranpose8x8

    movdqu xmm0, [eax + 0]  ;    xmm0:a7a6a5a4a3a2a1a0
    movdqu xmm1, [eax + 16] ;    xmm1:b7b6b5b4b3b2b1b0
    movdqu xmm2, [eax + 32] ;    xmm2:c7c6c5c4c3c2c1c0
    movdqu xmm3, [eax + 48] ;    xmm3:d7d6d5d4d3d2d1d0
   
    pxor   xmm4,xmm4       
    paddw  xmm4,xmm0 
    punpcklwd xmm0,xmm1     ;    xmm0:b3a3b2a2b1a1b0a0
    punpckhwd xmm4,xmm1     ;    xmm4:b7a7b6a6b5a5b4a4
   
    pxor   xmm5,xmm5
    paddw  xmm5,xmm2 
    punpcklwd xmm2,xmm3     ;    xmm2:d3c3d2c2d1c1d0c0
    punpckhwd xmm5,xmm3     ;    xmm5:d7c7d6c6d5c5d4c4

    pxor   xmm6,xmm6
    paddw  xmm6,xmm0 
    punpckldq xmm0,xmm2     ;    xmm0:d1c1b1a1d0c0b0a0
    punpckhdq xmm6,xmm2     ;    xmm6:d3c3b3a3d2c2b2a2

    pxor   xmm7,xmm7
    paddw  xmm7,xmm4 
    punpckldq xmm4,xmm5     ;    xmm4:d5c5b5a5d4c4b4a4
    punpckhdq xmm7,xmm5     ;    xmm7:d7c7b7a7d6c6b6a6

    movdqu  [eax + 0],xmm0  ;    d1c1b1a1d0c0b0a0
    movdqu  [eax + 16],xmm6 ;    d3c3b3a3d2c2b2a2
    movdqu  [eax + 32],xmm4 ;    d5c5b5a5d4c4b4a4
    movdqu  [eax + 48],xmm7 ;    d7c7b7a7d6c6b6a6

    
    movdqu xmm0, [eax + 64] ;    xmm0:e7e6e5e4e3e2e1e0
    movdqu xmm1, [eax + 80] ;    xmm1:f7f6f5f4f3f2f1f0
    movdqu xmm2, [eax + 96] ;    xmm2:g7g6g5g4g3g2g1g0 
    movdqu xmm3, [eax + 112];    xmm3:h7h6h5h4h3h2h1h0
   
    pxor   xmm4,xmm4       
    paddw  xmm4,xmm0 
    punpcklwd xmm0,xmm1     ;    xmm0:f3e3f2e2f1e1f0e0
    punpckhwd xmm4,xmm1     ;    xmm4:f7e7f6e6f5e5f4e4
   
    pxor   xmm5,xmm5
    paddw  xmm5,xmm2 
    punpcklwd xmm2,xmm3     ;    xmm2:h3g3h2g2h1g1h0g0
    punpckhwd xmm5,xmm3     ;    xmm5:h7g7h6g6h5g5h4g4

    pxor   xmm6,xmm6
    paddw  xmm6,xmm0 
    punpckldq xmm0,xmm2     ;    xmm0:h1g1f1e1h0g0f0e0
    punpckhdq xmm6,xmm2     ;    xmm6:h3g3f3e3h2g2f2e2

    pxor   xmm7,xmm7
    paddw  xmm7,xmm4 
    punpckldq xmm4,xmm5     ;    xmm4:h5g5f5e5h4g4f4e4
    punpckhdq xmm7,xmm5     ;    xmm7:h7g7f7e7h6g6f6e6

   
    movdqu  xmm1,[eax + 0]  ;    xmm1:d1c1b1a1d0c0b0a0  
    pxor   xmm5,xmm5       
    paddw  xmm5,xmm1
    punpcklqdq xmm1,xmm0    ;    xmm1:h0g0f0e0d0c0b0a0
    punpckhqdq xmm5,xmm0    ;    xmm5:h1g1f1e1d1c1b1a1
    movdqu  [eax + 0],xmm1  ;    
    
    movdqu xmm2,[eax + 16]  ;    xmm2:d3c3b3a3d2c2b2a2 
    movdqu  [eax + 16],xmm5 ;
    pxor   xmm3,xmm3       
    paddw  xmm3,xmm2
    punpcklqdq xmm2,xmm6    ;    xmm2:h2g2f2e2d2c2b2a2
    punpckhqdq xmm3,xmm6    ;    xmm3:h3g3f3e3d3c3b3a3

    movdqu xmm5,[eax + 32]  ;    xmm5:d5c5b5a5d4c4b4a4
    movdqu  [eax + 32],xmm2 ;
    pxor   xmm1,xmm1      
    paddw  xmm1,xmm5
    punpcklqdq xmm5,xmm4    ;    xmm5:h4g4f4e4d4c4b4a4
    punpckhqdq xmm1,xmm4    ;    xmm1:h5g5f5e5d5c5b5a5
    movdqu  [eax + 64],xmm5 ;    
    movdqu  [eax + 80],xmm1 ;    

    movdqu  xmm6,[eax + 48] ;    xmm6:d7c7b7a7d6c6b6a6 
    movdqu  [eax + 48],xmm3 ;
    pxor   xmm2,xmm2       
    paddw  xmm2,xmm6
    punpcklqdq xmm2,xmm7    ;    xmm2:h6g6f6e6d6c6b6a6
    punpckhqdq xmm6,xmm7    ;    xmm6:h7g7f7e7d7c7b7a7
    movdqu  [eax + 112],xmm6;    
    movdqu  [eax + 96],xmm2 ; 
    
    
 ;*****************************************************************
 ;EXTEND
 
    movdqu      xmm0, [eax+0*16]
    movdqu      xmm1, xmm0
    pxor        xmm2, xmm2
    pcmpgtw     xmm2, xmm0                  
    movdqu      [eax+0*16], xmm2
    punpcklwd   xmm1, xmm2
    punpckhwd   xmm0, xmm2
    movdqu      [ebx+0*16], xmm0
    movdqu      [eax+0*16], xmm1

    movdqu      xmm3, [eax+1*16] 
    XMM_EXTEND_CBA  xmm3, xmm4, xmm5 
    movdqu      [ebx+1*16], xmm3
    movdqu      [eax+1*16], xmm4

    movdqu      xmm6, [eax+2*16] 
    XMM_EXTEND_CBA  xmm6, xmm7, xmm0 
    movdqu      [ebx+2*16], xmm6
    movdqu      [eax+2*16], xmm7

    movdqu      xmm1, [eax+3*16] 
    XMM_EXTEND_CBA  xmm1, xmm2, xmm3 
    movdqu      [ebx+3*16], xmm1
    movdqu      [eax+3*16], xmm2

    movdqu      xmm4, [eax+4*16] 
    XMM_EXTEND_CBA  xmm4, xmm5, xmm6 
    movdqu      [ebx+4*16], xmm4
    movdqu      [eax+4*16], xmm5

    movdqu      xmm7, [eax+5*16] 
    XMM_EXTEND_CBA  xmm7, xmm0, xmm1 
    movdqu      [ebx+5*16], xmm7
    movdqu      [eax+5*16], xmm0

    movdqu      xmm2, [eax+6*16] 
    XMM_EXTEND_CBA  xmm2, xmm3, xmm4 
    movdqu      [ebx+6*16], xmm2
    movdqu      [eax+6*16], xmm3

    movdqu      xmm5, [eax+7*16] 
    XMM_EXTEND_CBA  xmm5, xmm6, xmm7 
    movdqu      [ebx+7*16], xmm5
    movdqu      [eax+7*16], xmm6
    
;**********************************************************************    
;PART 1

    movdqu xmm0, [eax + 0]  ;    
    movdqu xmm3, [eax + 48] ;   
    movdqu xmm4, [eax + 64] ;    
    movdqu xmm7, [eax + 112] ;   
    
    
    pxor   xmm1,xmm1
    paddd  xmm1,xmm0   
    paddd  xmm0,xmm7;    xmm0=s07=_mm_add_epi16(SRC_SSE[0], SRC_SSE[7]);
    psubd  xmm1,xmm7;    xmm1=d07=_mm_sub_epi16(SRC_SSE[0], SRC_SSE[7]);
    
    pxor   xmm2,xmm2
    paddd  xmm2,xmm3
    paddd  xmm3,xmm4;    xmm3=s34 = _mm_add_epi16(SRC_SSE[3], SRC_SSE[4]);
    psubd  xmm2,xmm4;    xmm2=d34 = _mm_sub_epi16(SRC_SSE[3], SRC_SSE[4]);

    pxor   xmm7,xmm7
    paddd  xmm7,xmm0
    paddd  xmm0,xmm3;    xmm0=a0=_mm_add_epi16(s07, s34);
    psubd  xmm7,xmm3;    xmm7=a2=_mm_sub_epi16(s07, s34); 
    pxor   xmm3,xmm3
    paddd  xmm3,xmm1
    psubd  xmm1,xmm2;    xmm1=_mm_sub_epi16(d07, d34)
    pslld  xmm1,1;       xmm1=_mm_slli_epi16 (_mm_sub_epi16(d07, d34), 1);
    paddd  xmm1,xmm3;    xmm1=a4=_mm_add_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d07, d34), 1), d07);

    paddd  xmm3,xmm2;    xmm3=_mm_add_epi16(d07, d34);
    pslld  xmm3,1;       xmm3=_mm_slli_epi16 (_mm_add_epi16(d07, d34), 1);
    paddd  xmm3,xmm2;    xmm3=a7=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d07, d34), 1), d34);
    
    ;movdqu [eax + 0],xmm0;     a0   
    ;movdqu [eax + 48],xmm7;    a2   
    movdqu [eax + 64],xmm1;      a4
    movdqu [eax + 112],xmm3;     a7


    movdqu xmm1, [eax + 16] ;    xmm1:SRC_SSE[1]
    movdqu xmm2, [eax + 32] ;    xmm2:SRC_SSE[2]
    movdqu xmm5, [eax + 80] ;    xmm5:SRC_SSE[5]
    movdqu xmm6, [eax + 96] ;    xmm6:SRC_SSE[6]
    
    pxor   xmm4,xmm4
    paddd  xmm4,xmm1   
    paddd  xmm1,xmm6;    xmm1=s16 =_mm_add_epi16(SRC_SSE[1], SRC_SSE[6]);
    psubd  xmm4,xmm6;    xmm4=d16 = _mm_sub_epi16(SRC_SSE[1], SRC_SSE[6]);

    pxor   xmm3,xmm3
    paddd  xmm3,xmm2
    paddd  xmm2,xmm5;    xmm2=s25 = _mm_add_epi16(SRC_SSE[2], SRC_SSE[5]);
    psubd  xmm3,xmm5;    xmm3=d25 = _mm_sub_epi16(SRC_SSE[2], SRC_SSE[5]);

    pxor   xmm5,xmm5
    paddd  xmm5,xmm1
    paddd  xmm1,xmm2;    xmm1=a1 = _mm_add_epi16(s16, s25);
    psubd  xmm5,xmm2;    xmm5=a3 = _mm_sub_epi16(s16, s25);
    pxor   xmm2,xmm2
    paddd  xmm2,xmm4
    paddd  xmm4,xmm3;    xmm4= _mm_add_epi16(d16, d25);
    pslld  xmm4,1;       xmm4=_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1);
    paddd  xmm4,xmm2;    xmm4=a5=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1), d16);
    
    psubd  xmm2,xmm3;    xmm2=_mm_sub_epi16(d16, d25)
    pslld  xmm2,1;       xmm2=_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1);
    psubd  xmm2,xmm3;    xmm2=a6=_mm_sub_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1), d25);

    movdqu [eax + 80],xmm2 ;      a6
    movdqu [eax + 96],xmm4;       a5 

    pxor   xmm2,xmm2
    paddd  xmm2,xmm0
    paddd  xmm0,xmm1;   xmm0=_mm_add_epi16(a0,a1);
    pslld  xmm0,3;      xmm0= DST_SSE[0] = _mm_slli_epi16(_mm_add_epi16(a0,a1), 3);
    psubd  xmm2,xmm1;   xmm2= _mm_sub_epi16(d16, d25);
    pslld  xmm2,3;      xmm2= DST_SSE[4] = _mm_slli_epi16(_mm_sub_epi16(a0,a1), 3);

    pxor   xmm4,xmm4
    paddd  xmm4,xmm7
    pslld  xmm7,3;      xmm7= _mm_slli_epi16(a2,3);
    pslld  xmm4,1;      xmm4= _mm_slli_epi16(a2,1);
    paddd  xmm7,xmm4;   xmm7= _mm_add_epi16( _mm_slli_epi16(a2,3), _mm_slli_epi16(a2,1));

    pxor   xmm3,xmm3   
    paddd  xmm3,xmm5
    pslld  xmm5,2;      xmm5=_mm_slli_epi16(a3,2);
    paddd  xmm7,xmm5;   xmm7=DST_SSE[2];
    
    pslld  xmm4,1;      xmm4=_mm_slli_epi16(a2,2);
    pslld  xmm3,1;      xmm3=_mm_slli_epi16(a3,1);
    pslld  xmm5,1;      xmm5=_mm_slli_epi16(a3,2);
    paddd  xmm3,xmm5;   xmm3= _mm_add_epi16( _mm_slli_epi16(a3,3), _mm_slli_epi16(a3,1));
    psubd  xmm4,xmm3;   xmm4=DST_SSE[6];
    
    push ecx
    mov  ecx,10h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=10h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=10h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,5
    paddd  xmm7,xmm5
    psrad  xmm7,5
    paddd  xmm2,xmm5
    psrad  xmm2,5
    paddd  xmm4,xmm5
    psrad  xmm4,5

    movdqu [eax+0],xmm0;    DST_SSE[0]
    movdqu [eax+16],xmm7;   DST_SSE[2]
    movdqu [eax+32],xmm2;   DST_SSE[4]
    movdqu [eax+48],xmm4;   DST_SSE[6]

    movdqu xmm1,[eax + 64];    a4
    movdqu xmm3,[eax + 112];   a7
    movdqu xmm2,[eax + 80];    a6
    movdqu xmm4,[eax + 96];    a5 

    pxor   xmm0,xmm0   
    paddd  xmm0,xmm1
    psubd  xmm1,xmm4;   xmm1=_mm_sub_epi16(a4,a5);
    paddd  xmm1,xmm2;   xmm1=_mm_add_epi16( _mm_sub_epi16(a4,a5),a6);
    pslld  xmm1,1;      xmm1=_mm_slli_epi16( _mm_add_epi16( _mm_sub_epi16(a4,a5),a6),1);
    paddd  xmm1,xmm0;   xmm1=DST_SSE[3];
   
    paddd  xmm0,xmm4;   xmm0=_mm_add_epi16(a4,a5);
    paddd  xmm0,xmm3;   xmm0=_mm_add_epi16( _mm_add_epi16(a4,a5),a7);   
    pslld  xmm0,1;      xmm0=_mm_slli_epi16( _mm_add_epi16( _mm_add_epi16(a4,a5),a7),1);
    paddd  xmm0,xmm4;   xmm0=DST_SSE[1];
   
    pxor   xmm7,xmm7   
    paddd  xmm7,xmm3
    psubd  xmm3,xmm4;   xmm3=_mm_sub_epi16(a7,a5);
    psubd  xmm3,xmm2;   xmm3=_mm_sub_epi16( _mm_sub_epi16(a7,a5),a6);
    pslld  xmm3,1;      xmm3=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a7,a5),a6),1);
    paddd  xmm3,xmm7;   xmm3=DST_SSE[5];

    movdqu xmm6,[eax + 64];    a4
    psubd  xmm6,xmm7;   xmm6=_mm_sub_epi16(a4,a7);
    psubd  xmm6,xmm2;   xmm6=_mm_sub_epi16( _mm_sub_epi16(a4,a7),a6);
    pslld  xmm6,1;      xmm6=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a4,a7),a6),1);
    psubd  xmm6,xmm2;   xmm6=DST_SSE[7]

    push ecx
    mov  ecx,10h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=10h
    pop  ecx

    movdqu xmm4,xmm5;   xmm4=10h
    psllq xmm5,32;
    paddd xmm5,xmm4
    movdqu xmm4,xmm5;
    punpcklqdq xmm5,xmm4;
    
    paddd  xmm1,xmm5
    psrad  xmm1,5
    paddd  xmm3,xmm5
    psrad  xmm3,5
    paddd  xmm0,xmm5
    psrad  xmm0,5
    paddd  xmm6,xmm5
    psrad  xmm6,5 
    
    movdqu xmm7,[eax+16]
    movdqu xmm2,xmm7;        xmm2=DST_SSE[2]
    movdqu [eax+16],xmm0;    DST_SSE[1]

    movdqu xmm4,[eax+32];    xmm4=DST_SSE[4]
    movdqu [eax+32],xmm2;    DST_SSE[2]

    movdqu xmm0 ,[eax+48];    xmm0=DST_SSE[6]
    movdqu [eax+48],xmm1;    DST_SSE[3]
   
    
    movdqu [eax+64],xmm4;    DST_SSE[4]
    movdqu [eax+80],xmm3;    DST_SSE[5]
    movdqu [eax+96],xmm0;    DST_SSE[6]
    movdqu [eax+112],xmm6;   DST_SSE[7]

 
;**************************************************************************** 
;part2 
    movdqu xmm0, [ebx + 0]  ;    
    movdqu xmm3, [ebx + 48] ;   
    movdqu xmm4, [ebx + 64] ;    
    movdqu xmm7, [ebx + 112] ;   
    
    
    pxor   xmm1,xmm1
    paddd  xmm1,xmm0   
    paddd  xmm0,xmm7;    xmm0=s07=_mm_add_epi16(SRC_SSE[0], SRC_SSE[7]);
    psubd  xmm1,xmm7;    xmm1=d07=_mm_sub_epi16(SRC_SSE[0], SRC_SSE[7]);
    
    pxor   xmm2,xmm2
    paddd  xmm2,xmm3
    paddd  xmm3,xmm4;    xmm3=s34 = _mm_add_epi16(SRC_SSE[3], SRC_SSE[4]);
    psubd  xmm2,xmm4;    xmm2=d34 = _mm_sub_epi16(SRC_SSE[3], SRC_SSE[4]);

    pxor   xmm7,xmm7
    paddd  xmm7,xmm0
    paddd  xmm0,xmm3;    xmm0=a0=_mm_add_epi16(s07, s34);
    psubd  xmm7,xmm3;    xmm7=a2=_mm_sub_epi16(s07, s34); 
    pxor   xmm3,xmm3
    paddd  xmm3,xmm1
    psubd  xmm1,xmm2;    xmm1=_mm_sub_epi16(d07, d34)
    pslld  xmm1,1;       xmm1=_mm_slli_epi16 (_mm_sub_epi16(d07, d34), 1);
    paddd  xmm1,xmm3;    xmm1=a4=_mm_add_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d07, d34), 1), d07);

    paddd  xmm3,xmm2;    xmm3=_mm_add_epi16(d07, d34);
    pslld  xmm3,1;       xmm3=_mm_slli_epi16 (_mm_add_epi16(d07, d34), 1);
    paddd  xmm3,xmm2;    xmm3=a7=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d07, d34), 1), d34);
    
     
    movdqu [ebx + 64],xmm1;      a4
    movdqu [ebx + 112],xmm3;     a7


    movdqu xmm1, [ebx + 16] ;    xmm1:SRC_SSE[1]
    movdqu xmm2, [ebx + 32] ;    xmm2:SRC_SSE[2]
    movdqu xmm5, [ebx + 80] ;    xmm5:SRC_SSE[5]
    movdqu xmm6, [ebx + 96] ;    xmm6:SRC_SSE[6]
    
    pxor   xmm4,xmm4
    paddd  xmm4,xmm1   
    paddd  xmm1,xmm6;    xmm1=s16 =_mm_add_epi16(SRC_SSE[1], SRC_SSE[6]);
    psubd  xmm4,xmm6;    xmm4=d16 = _mm_sub_epi16(SRC_SSE[1], SRC_SSE[6]);

    pxor   xmm3,xmm3
    paddd  xmm3,xmm2
    paddd  xmm2,xmm5;    xmm2=s25 = _mm_add_epi16(SRC_SSE[2], SRC_SSE[5]);
    psubd  xmm3,xmm5;    xmm3=d25 = _mm_sub_epi16(SRC_SSE[2], SRC_SSE[5]);

    pxor   xmm5,xmm5
    paddd  xmm5,xmm1
    paddd  xmm1,xmm2;    xmm1=a1 = _mm_add_epi16(s16, s25);
    psubd  xmm5,xmm2;    xmm5=a3 = _mm_sub_epi16(s16, s25);
    pxor   xmm2,xmm2
    paddd  xmm2,xmm4
    paddd  xmm4,xmm3;    xmm4= _mm_add_epi16(d16, d25);
    pslld  xmm4,1;       xmm4=_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1);
    paddd  xmm4,xmm2;    xmm4=a5=_mm_add_epi16(_mm_slli_epi16 ( _mm_add_epi16(d16, d25), 1), d16);
    
    psubd  xmm2,xmm3;    xmm2=_mm_sub_epi16(d16, d25)
    pslld  xmm2,1;       xmm2=_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1);
    psubd  xmm2,xmm3;    xmm2=a6=_mm_sub_epi16(_mm_slli_epi16 ( _mm_sub_epi16(d16, d25), 1), d25);
 
    movdqu [ebx + 80],xmm2 ;      a6
    movdqu [ebx + 96],xmm4;       a5 

    pxor   xmm2,xmm2
    paddd  xmm2,xmm0
    paddd  xmm0,xmm1;   xmm0=_mm_add_epi16(a0,a1);
    pslld  xmm0,3;      xmm0= DST_SSE[0] = _mm_slli_epi16(_mm_add_epi16(a0,a1), 3);
    psubd  xmm2,xmm1;   xmm2= _mm_sub_epi16(d16, d25);
    pslld  xmm2,3;      xmm2= DST_SSE[4] = _mm_slli_epi16(_mm_sub_epi16(a0,a1), 3);

    pxor   xmm4,xmm4
    paddd  xmm4,xmm7
    pslld  xmm7,3;      xmm7= _mm_slli_epi16(a2,3);
    pslld  xmm4,1;      xmm4= _mm_slli_epi16(a2,1);
    paddd  xmm7,xmm4;   xmm7= _mm_add_epi16( _mm_slli_epi16(a2,3), _mm_slli_epi16(a2,1));

    pxor   xmm3,xmm3   
    paddd  xmm3,xmm5
    pslld  xmm5,2;      xmm5=_mm_slli_epi16(a3,2);
    paddd  xmm7,xmm5;   xmm7=DST_SSE[2];
    
    pslld  xmm4,1;      xmm4=_mm_slli_epi16(a2,2);
    pslld  xmm3,1;      xmm3=_mm_slli_epi16(a3,1);
    pslld  xmm5,1;      xmm5=_mm_slli_epi16(a3,2);
    paddd  xmm3,xmm5;   xmm3= _mm_add_epi16( _mm_slli_epi16(a3,3), _mm_slli_epi16(a3,1));
    psubd  xmm4,xmm3;   xmm4=DST_SSE[6];

    push ecx
    mov  ecx,10h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=10h
    pop  ecx

    movdqu xmm3,xmm5;   xmm3=10h
    psllq xmm5,32;
    paddd xmm5,xmm3
    movdqu xmm3,xmm5;
    punpcklqdq xmm5,xmm3;
    
    paddd  xmm0,xmm5
    psrad  xmm0,5
    paddd  xmm7,xmm5
    psrad  xmm7,5
    paddd  xmm2,xmm5
    psrad  xmm2,5
    paddd  xmm4,xmm5
    psrad  xmm4,5

    movdqu [ebx+0],xmm0;    DST_SSE[0]
    movdqu [ebx+16],xmm7;   DST_SSE[2]
    movdqu [ebx+32],xmm2;   DST_SSE[4]
    movdqu [ebx+48],xmm4;   DST_SSE[6]

    movdqu xmm1,[ebx + 64];    a4
    movdqu xmm3,[ebx + 112];   a7
    movdqu xmm2,[ebx + 80];    a6
    movdqu xmm4,[ebx + 96];    a5 

    pxor   xmm0,xmm0   
    paddd  xmm0,xmm1
    psubd  xmm1,xmm4;   xmm1=_mm_sub_epi16(a4,a5);
    paddd  xmm1,xmm2;   xmm1=_mm_add_epi16( _mm_sub_epi16(a4,a5),a6);
    pslld  xmm1,1;      xmm1=_mm_slli_epi16( _mm_add_epi16( _mm_sub_epi16(a4,a5),a6),1);
    paddd  xmm1,xmm0;   xmm1=DST_SSE[3];
   
    paddd  xmm0,xmm4;   xmm0=_mm_add_epi16(a4,a5);
    paddd  xmm0,xmm3;   xmm0=_mm_add_epi16( _mm_add_epi16(a4,a5),a7);   
    pslld  xmm0,1;      xmm0=_mm_slli_epi16( _mm_add_epi16( _mm_add_epi16(a4,a5),a7),1);
    paddd  xmm0,xmm4;   xmm0=DST_SSE[1];
   
    pxor   xmm7,xmm7   
    paddd  xmm7,xmm3
    psubd  xmm3,xmm4;   xmm3=_mm_sub_epi16(a7,a5);
    psubd  xmm3,xmm2;   xmm3=_mm_sub_epi16( _mm_sub_epi16(a7,a5),a6);
    pslld  xmm3,1;      xmm3=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a7,a5),a6),1);
    paddd  xmm3,xmm7;   xmm3=DST_SSE[5];

    movdqu xmm6,[ebx + 64];    a4
    psubd  xmm6,xmm7;   xmm6=_mm_sub_epi16(a4,a7);
    psubd  xmm6,xmm2;   xmm6=_mm_sub_epi16( _mm_sub_epi16(a4,a7),a6);
    pslld  xmm6,1;      xmm6=_mm_slli_epi16( _mm_sub_epi16( _mm_sub_epi16(a4,a7),a6),1);
    psubd  xmm6,xmm2;   xmm6=DST_SSE[7]

    push ecx
    mov  ecx,10h
    pxor xmm5,xmm5
    movd xmm5,ecx   ;  xmm5=10h
    pop  ecx

    movdqu xmm4,xmm5;   xmm4=10h
    psllq xmm5,32;
    paddd xmm5,xmm4
    movdqu xmm4,xmm5;
    punpcklqdq xmm5,xmm4;
    
    paddd  xmm1,xmm5
    psrad  xmm1,5
    paddd  xmm3,xmm5
    psrad  xmm3,5
    paddd  xmm0,xmm5
    psrad  xmm0,5
    paddd  xmm6,xmm5
    psrad  xmm6,5 
    
    movdqu xmm7,[ebx+16]
    movdqu xmm2,xmm7;        xmm2=DST_SSE[2]
    movdqu [ebx+16],xmm0;    DST_SSE[1]

    movdqu xmm4,[ebx+32];    xmm4=DST_SSE[4]
    movdqu [ebx+32],xmm2;    DST_SSE[2]

    movdqu xmm0 ,[ebx+48];    xmm0=DST_SSE[6]
    movdqu [ebx+48],xmm1;    DST_SSE[3]
   
    
    movdqu [ebx+64],xmm4;    DST_SSE[4]
    movdqu [ebx+80],xmm3;    DST_SSE[5]
    movdqu [ebx+96],xmm0;    DST_SSE[6]
    movdqu [ebx+112],xmm6;   DST_SSE[7]

 
;****************************************
    movdqu      xmm0,  [eax+0*16]  
    movdqu      xmm2,  [eax+1*16]
    movdqu      xmm4,  [eax+2*16]
    movdqu      xmm6,  [eax+3*16]
 
    movdqu      xmm1,  [ebx+0*16]  
    movdqu      xmm3,  [ebx+1*16]
    movdqu      xmm5,  [ebx+2*16]
    movdqu      xmm7,  [ebx+3*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+0*16], xmm0  
    movdqu      [eax+1*16], xmm2
    movdqu      [eax+2*16], xmm4
    movdqu      [eax+3*16], xmm6
    
    movdqu      xmm0,  [eax+4*16]  
    movdqu      xmm2,  [eax+5*16]
    movdqu      xmm4,  [eax+6*16]
    movdqu      xmm6,  [eax+7*16]
 
    movdqu      xmm1,  [ebx+4*16]  
    movdqu      xmm3,  [ebx+5*16]
    movdqu      xmm5,  [ebx+6*16]
    movdqu      xmm7,  [ebx+7*16] 
    
    packssdw    xmm0, xmm1
    packssdw    xmm2, xmm3
    packssdw    xmm4, xmm5
    packssdw    xmm6, xmm7

    movdqu      [eax+4*16], xmm0  
    movdqu      [eax+5*16], xmm2
    movdqu      [eax+6*16], xmm4
    movdqu      [eax+7*16], xmm6
     
    pop  edx
    pop  ebx

    
    ret


 
