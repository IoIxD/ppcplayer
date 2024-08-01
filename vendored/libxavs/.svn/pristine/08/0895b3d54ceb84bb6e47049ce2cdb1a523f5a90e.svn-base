%include "i386inc.asm"

%macro STORE8x8 2
    movq        [edx +   ecx], %1       ; 0
    movq        [edx + 2*ecx], %1       ; 1
    movq        [edx + 4*ecx], %1       ; 3
    movq        [edx + 8*ecx], %2       ; 7
    add         edx, eax
    movq        [edx        ], %1       ; 2
    movq        [edx + 2*ecx], %2       ; 4
    movq        [edx +   eax], %2       ; 5
    movq        [edx + 4*ecx], %2       ; 6
%endmacro

SECTION_RODATA
 
ALIGN 8
pw_2: times 4 dw 2
pw_8: times 4 dw 8
pb_1: times 8 db 1
pw_3210:
    dw 0
    dw 1
    dw 2
    dw 3
ALIGN 16

pb_11: times 16 db 1

;=============================================================================
; Code
;=============================================================================

SECTION .text

cglobal predict_8x8c_dc_top_mmxext
cglobal predict_8x8c_v_mmxext
cglobal predict_8x8c_dc_128_mmxext
cglobal predict_8x8c_dc_left_mmxext
cglobal predict_8x8c_dc_mmxext 
cglobal predict_8x8c_p_core_mmxext 
cglobal predict_8x8c_h_mmxext 
cglobal predict_8x8_dc_128_mmxext
cglobal predict_8x8_dc_left_mmxext
cglobal predict_8x8_dc_mmxext
cglobal predict_8x8_dc_top_mmxext
cglobal predict_8x8_v_mmxext
cglobal predict_8x8_h_mmxext
cglobal predict_8x8_ddr_sse2
cglobal predict_8x8_ddl_sse2

%macro PRED8x8_LOWPASS 2
    movq        mm3, mm1
    pavgb       mm1, mm2
    pxor        mm2, mm3
    movq        %1 , %2
    pand        mm2, [pb_1 GOT_ebx]
    psubusb     mm1, mm2
    pavgb       %1 , mm1     ; %1 = (t[n-1] + t[n]*2 + t[n+1] + 2) >> 2
%endmacro
%macro PRED8x8_LOWPASS_XMM 5
    movdqa      %5, %2
    pavgb       %2, %3
    pxor        %3, %5
    movdqa      %1, %4
    pand        %3, [pb_11 GOT_ebx]
    psubusb     %2, %3
    pavgb       %1, %2
%endmacro
%macro PRED8x8_LOAD_TOP 0
    mov         edx, [picesp + 12]
    mov         ecx, FDEC_STRIDE
    mov         eax, [picesp + 16]
    sub         edx, ecx

    and         eax, 12
    movq        mm1, [edx-1]
    movq        mm2, [edx+1]

    cmp         eax, byte 8
    jge         .have_topleft
    mov         al,  [edx]
    mov         ah,  al
    pinsrw      mm1, eax, 0
    mov         eax, [picesp + 16]
.have_topleft:

    and         eax, byte 4
    jne         .have_topright
    mov         al,  [edx+7]
    mov         ah,  al
    pinsrw      mm2, eax, 3
.have_topright:

    PRED8x8_LOWPASS mm0, [edx]
%endmacro

;-----------------------------------------------------------------------------
;
; void predict_8x8c_dc_128_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8c_dc_128_mmxext:
    picpush     ebx
    picgetgot   ebx
    
    mov         edx, [picesp+4]
    mov         ecx, FDEC_STRIDE
    mov         eax, 128
    movd        mm0, eax
    punpcklbw   mm0, mm0
    pshufw      mm0, mm0,0
    movq        [edx],mm0           
  
    movq        [edx +  ecx],  mm0     
    movq        [edx + 2*ecx], mm0      
    movq        [edx + 4*ecx], mm0       
    lea         eax,[ecx+2*ecx]
    add         edx, eax
    movq        [edx        ], mm0     
    movq        [edx + 2*ecx], mm0     
    movq        [edx +   eax], mm0      
    movq        [edx + 4*ecx], mm0     
    picpop      ebx
    ret
;-----------------------------------------------------------------------------
;
; void predict_8x8c_dc_top_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------
ALIGN 16
predict_8x8c_dc_top_mmxext:
   ; picpush     ebx
   ; picgetgot   ebx
    push     ebx
    push     edi
    
    PRED8x8_LOAD_TOP    
    lea         eax, [ecx + 2*ecx]
    STORE8x8    mm0, mm0
   
    pop    edi
    pop    ebx
   ; picpop      ebx
    ret

   
;-----------------------------------------------------------------------------
;
; void predict_8x8c_v_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8c_v_mmxext:
  
    mov         edx, [esp + 4]
    mov         ecx, FDEC_STRIDE
    sub         edx, ecx

    movq        mm0, [edx]
    lea         eax,[ecx+2*ecx]
    STORE8x8    mm0, mm0 
    
    ret

;-----------------------------------------------------------------------------
;
; void predict_8x8c_dc_left_mmxext( uint8_t *src, int i_neighbor );
;
;-----------------------------------------------------------------------------

ALIGN 8
predict_8x8c_dc_left_mmxext:
    push     ebx
   ; picgetgot   ebx
    push     edi
    

    mov         edx, [picesp+12]
    lea         edi, [picesp-40]
   
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
       
    mov         ecx, [picesp+16]
    and         ecx,12
    cmp         ecx,byte 8
    jge         topleft0
    movzx      eax, byte [edx-0*FDEC_STRIDE-1] 
    movd       mm6,eax
    jmp        do0
topleft0:
     movzx      eax, byte [edx-1*FDEC_STRIDE-1] 
     movd       mm6,eax
do0:  
     mov         ecx, [picesp+16]
     and        ecx,16
     je         downleft0
     xor        eax,eax
     movzx      eax, byte [edx+8*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
     jmp        doneit0
downleft0:
     xor        eax,eax
     movzx      eax, byte [edx+7*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
; edi-> { l0 l1 l2 l3 l4 l5 l6 l7 }

doneit0:
    movq      mm0,[edi]
    movq      mm1, mm0
    psllq     mm1, 8
    por       mm1, mm6
    movq      mm2, mm0
    psrlq     mm2, 8
    por       mm2, mm4
    PRED8x8_LOWPASS mm5,mm0
 

    mov         eax,  8  
.loop:
    movq         mm7,mm5
    punpcklbw    mm7,mm7
    pshufw       mm7,mm7,0
    movq        [edx],mm7
    psrlq       mm5,8
    add         edx,FDEC_STRIDE
    dec         eax
    jg         .loop
    nop
   
    pop      edi
    pop      ebx
    ret
    
 ;-----------------------------------------------------------------------------
;
; void predict_8x8c_dc_mmxext( uint8_t *src,int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 8
predict_8x8c_dc_mmxext:
    push     ebx
    ;picgetgot   ebx
    push     edi
    lea         edi, [picesp-40]
    PRED8x8_LOAD_TOP
    mov         edx, [picesp+12]
  
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
       
    mov         ecx, [picesp+16]
    and         ecx,12
    cmp         ecx,byte 8
    jge         topleft110
     movzx      eax, byte [edx-0*FDEC_STRIDE-1] 
     movd       mm6,eax
     jmp        do110
topleft110:
     movzx      eax, byte [edx-1*FDEC_STRIDE-1] 
     movd       mm6,eax
do110:  
     mov        ecx, [picesp+16]
     and        ecx,16
     je         downleft110
     xor        eax,eax
     movzx      eax, byte [edx+8*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56 
     jmp        doneit110
downleft110:
     xor        eax,eax
     movzx      eax, byte [edx+7*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
doneit110:
    movq      mm7,[edi]
    movq      mm1, mm7
    psllq     mm1, 8
    por       mm1, mm6
    movq      mm2, mm7
    psrlq     mm2, 8
    por       mm2, mm4
    PRED8x8_LOWPASS mm5,mm7
     mov         edx, [picesp+12]
     pxor        mm6,mm6
     movq        mm3,mm0
     movq        mm4,mm0
     punpcklbw   mm3,mm6
     punpckhbw   mm4,mm6
     mov         eax, 8
.loop:
    movq        mm1, mm5
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0   
    movq        mm2, mm1
    punpcklbw   mm1, mm6
    punpckhbw   mm2, mm6
    paddw       mm1,mm3
    psraw       mm1,1
    paddw       mm2,mm4
    psraw       mm2,1
    packuswb    mm1,mm2   
    movq        [edx],mm1
    psrlq       mm5, 8
    add         edx,FDEC_STRIDE
    dec         eax
    jg          .loop
    nop
    pop      edi
    pop      ebx
    ret   

;-----------------------------------------------------------------------------
;
; predict_8x8c_p_core_mmxext( src, i00, b, c )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8c_p_core_mmxext:
    picpush     ebx
    picgetgot   ebx

    mov         edx, [picesp + 4]
    mov         ecx, FDEC_STRIDE
    pshufw      mm0, [picesp + 8], 0
    pshufw      mm2, [picesp +12], 0
    pshufw      mm4, [picesp +16], 0
    movq        mm1, mm2
    pmullw      mm2, [pw_3210 GOT_ebx]
    psllw       mm1, 2
    paddsw      mm0, mm2        ; mm0 = {i+0*b, i+1*b, i+2*b, i+3*b}
    paddsw      mm1, mm0        ; mm1 = {i+4*b, i+5*b, i+6*b, i+7*b}

    mov         eax, 8
ALIGN 4
.loop:
    movq        mm5, mm0
    movq        mm6, mm1
    psraw       mm5, 5
    psraw       mm6, 5
    packuswb    mm5, mm6
    movq        [edx], mm5

    paddsw      mm0, mm4
    paddsw      mm1, mm4
    add         edx, ecx
    dec         eax
    jg          .loop

    nop
    picpop      ebx
    ret

;-----------------------------------------------------------------------------
;
; void predict_8x8c_h_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8c_h_mmxext:
    picpush     ebx
    picgetgot   ebx


    mov         edx, [picesp + 4]
    mov         ecx, FDEC_STRIDE 
    
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
    
       add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
         add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
         add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
          add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
    
    picpop      ebx
    ret
    
;-----------------------------------------------------------------------------
;
; void predict_8x8_dc_128_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8_dc_128_mmxext:
    picpush     ebx
    picgetgot   ebx
    
    mov         edx, [picesp+4]
    mov         ecx, FDEC_STRIDE
    mov         eax, 128
    movd        mm0, eax
    punpcklbw   mm0, mm0
    pshufw      mm0, mm0,0
    movq        [edx],mm0           
  
    movq        [edx +  ecx],  mm0     
    movq        [edx + 2*ecx], mm0      
    movq        [edx + 4*ecx], mm0       
    lea         eax,[ecx+2*ecx]
    add         edx, eax
    movq        [edx        ], mm0     
    movq        [edx + 2*ecx], mm0     
    movq        [edx +   eax], mm0      
    movq        [edx + 4*ecx], mm0     
    picpop      ebx
    ret
;-----------------------------------------------------------------------------
;
; void predict_8x8_dc_top_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------
ALIGN 16
predict_8x8_dc_top_mmxext:
   ; picpush     ebx
   ; picgetgot   ebx
    push        ebx
    push        edi
    
    PRED8x8_LOAD_TOP    
    lea         eax, [ecx + 2*ecx]
    STORE8x8    mm0, mm0

   ; picpop      ebx
    pop          edi
    pop          ebx
    ret

   
;-----------------------------------------------------------------------------
;
; void predict_8x8_v_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8_v_mmxext:
  
    mov         edx, [esp + 4]
    mov         ecx, FDEC_STRIDE
    sub         edx, ecx

    movq        mm0, [edx]
    lea         eax,[ecx+2*ecx]
    STORE8x8    mm0, mm0 
    
    ret

;-----------------------------------------------------------------------------
;
; void predict_8x8_dc_left_mmxext( uint8_t *src, int i_neighbor );
;
;-----------------------------------------------------------------------------

ALIGN 8
predict_8x8_dc_left_mmxext:
    ;picpush     ebx
    ;picgetgot   ebx
    push        ebx
    push        edi
    
    mov         edx, [picesp+12]
    lea         edi, [picesp-40]
   
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
       
    mov         ecx, [picesp+16]
    and         ecx,12
    cmp         ecx,byte 8
    jge         topleft
    movzx      eax, byte [edx-0*FDEC_STRIDE-1] 
    movd       mm6,eax
    jmp        doo
topleft:
     movzx      eax, byte [edx-1*FDEC_STRIDE-1] 
     movd       mm6,eax
doo:  
     mov         ecx, [picesp+16]
     and        ecx,16
     je         downleft
     xor        eax,eax
     movzx      eax, byte [edx+8*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
     jmp        doneit
downleft:
     xor        eax,eax
     movzx      eax, byte [edx+7*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
; edi-> { l0 l1 l2 l3 l4 l5 l6 l7 }

doneit:
    movq      mm0,[edi]
    movq      mm1, mm0
    psllq     mm1, 8
    por       mm1, mm6
    movq      mm2, mm0
    psrlq     mm2, 8
    por       mm2, mm4
    PRED8x8_LOWPASS mm5,mm0
 

    mov         eax,  8  
.loop:
    movq         mm7,mm5
    punpcklbw    mm7,mm7
    pshufw       mm7,mm7,0
    movq        [edx],mm7
    psrlq       mm5,8
    add         edx,FDEC_STRIDE
    dec         eax
    jg         .loop
    nop
    pop         edi
    pop          ebx
   ; picpop      ebx
    ret
    
 ;-----------------------------------------------------------------------------
;
; void predict_8x8_dc_mmxext( uint8_t *src,int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 8
predict_8x8_dc_mmxext:
   ; picpush     ebx
   ; picgetgot   ebx
    push         ebx
    push        edi
    
    lea         edi, [picesp-40]
    PRED8x8_LOAD_TOP
    mov         edx, [picesp+12]
  
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
       
    mov         ecx, [picesp+16]
    and         ecx,12
    cmp         ecx,byte 8
    jge         topleft111
     movzx      eax, byte [edx-0*FDEC_STRIDE-1] 
     movd       mm6,eax
     jmp        do111
topleft111:
     movzx      eax, byte [edx-1*FDEC_STRIDE-1] 
     movd       mm6,eax
do111:  
     mov        ecx, [picesp+16]
     and        ecx,16
     je         downleft111
     xor        eax,eax
     movzx      eax, byte [edx+8*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56 
     jmp        doneit111
downleft111:
     xor        eax,eax
     movzx      eax, byte [edx+7*FDEC_STRIDE-1] 
     movd       mm4,eax
     psllq      mm4,56
doneit111:
    movq      mm7,[edi]
    movq      mm1, mm7
    psllq     mm1, 8
    por       mm1, mm6
    movq      mm2, mm7
    psrlq     mm2, 8
    por       mm2, mm4
    PRED8x8_LOWPASS mm5,mm7
     mov         edx, [picesp+12]
     pxor        mm6,mm6
     movq        mm3,mm0
     movq        mm4,mm0
     punpcklbw   mm3,mm6
     punpckhbw   mm4,mm6
     mov         eax, 8
.loop:
    movq        mm1, mm5
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0   
    movq        mm2, mm1
    punpcklbw   mm1, mm6
    punpckhbw   mm2, mm6
    paddw       mm1,mm3
    psraw       mm1,1
    paddw       mm2,mm4
    psraw       mm2,1
    packuswb    mm1,mm2   
    movq        [edx],mm1
    psrlq       mm5, 8
    add         edx,FDEC_STRIDE
    dec         eax
    jg          .loop
    nop 
    pop         edi
    pop         ebx
   ; picpop      ebx
    ret   

;-----------------------------------------------------------------------------
;
; void predict_8x8_h_mmxext( uint8_t *src, int i_neighbor )
;
;-----------------------------------------------------------------------------

ALIGN 16
predict_8x8_h_mmxext:
    picpush     ebx
    picgetgot   ebx


    mov         edx, [picesp + 4]
    mov         ecx, FDEC_STRIDE 
    
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
    
       add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
         add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
         add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
          add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
        add         edx, ecx
    mov         ax,[edx-1]
    movd        mm1, eax
    punpcklbw   mm1, mm1
    pshufw      mm1, mm1,0
    movq       [edx],mm1
    
    picpop      ebx
    ret
    

;-----------------------------------------------------------------------------
;
; void predict_8x8_ddr_sse2( uint8_t *src, int i_neighbor)
;
;-----------------------------------------------------------------------------
ALIGN 16
predict_8x8_ddr_sse2:
   ; picpush     ebx
   ; picgetgot   ebx
    push         ebx
    push         edi
    
    mov         edx, [picesp+12]
    lea         edi, [picesp-24]
    movq        mm0, [edx-FDEC_STRIDE]
    movq        [edi+8], mm0
    mov         ecx, [picesp+16]
    and         ecx, byte 4
    jne          topright
    mov         al,  [edx-FDEC_STRIDE+7]
    mov         [edi+16], al
    jmp         done2
   ; cmovnz      ax,  [edx-FDEC_STRIDE+8]
topright:
    mov         al,  [edx-FDEC_STRIDE+8]
    mov         [edi+16], al
done2:  
    mov         bh,  [edx+3*FDEC_STRIDE-1]
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         ah,  [edx-1*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    shl         ebx, 16
    shl         eax, 16
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         [edi+4], eax
    mov         [edi], ebx
    movzx       eax, byte [edx+7*FDEC_STRIDE-1]
    movd        xmm4,eax

; edi-> {l6 l5 l4 l3 l2 l1 l0 lt t0 t1 t2 t3 t4 t5 t6 t7 t8}

    movdqu      xmm0, [edi]
    movdqu      xmm2, [edi+1]
    movdqa      xmm1, xmm0
    pslldq      xmm1, 1
    por         xmm1, xmm4
    PRED8x8_LOWPASS_XMM xmm3, xmm1, xmm2, xmm0, xmm4
    movdqa      xmm1, xmm3
    psrldq      xmm1, 1
%assign Y 7
%rep 3
    movq        [edx+Y*FDEC_STRIDE], xmm3
    psrldq      xmm3, 2
    movq        [edx+(Y-1)*FDEC_STRIDE], xmm1
    psrldq      xmm1, 2
%assign Y (Y-2)
%endrep
    movq        [edx+1*FDEC_STRIDE], xmm3
    movq        [edx+0*FDEC_STRIDE], xmm1
    
    pop         edi
    pop         ebx
   ; picpop      ebx
    ret
    
;-----------------------------------------------------------------------------
;
;void predict_8x8_ddl_sse2 (uint8_t * src, int i_neighbor)
;
;-----------------------------------------------------------------------------
ALIGN 16
predict_8x8_ddl_sse2:
   ; picpush     ebx
   ; picgetgot   ebx
    push         ebx
    push         edi
    
    lea         edi, [picesp-40]
    mov         edx, [picesp+12]
    mov         ecx, [picesp+16]
    and         ecx, byte 4
    jne         topright00
    movq        mm4, [edx-FDEC_STRIDE]
    movq        [edi],mm4
    movq        mm1, [edx-FDEC_STRIDE-1]
    mov         al,  [edx-FDEC_STRIDE+7]
    mov         [edi+8], al       
    movq        mm2, [edi+1] 
    PRED8x8_LOWPASS mm0,mm4
    movq        [edi],mm0
    mov          al,  [edx-FDEC_STRIDE+7]
    mov          ah, al  
    pinsrw       mm1,eax,0
    pshufw       mm1,mm1,0
    movq         [edi+8],mm1
    movdqu       xmm0,[edi]
    jmp          done00
topright00:
    movdqu       xmm1, [edx-FDEC_STRIDE-1]
    movdqu       xmm4, [edx-FDEC_STRIDE]
    movdqu       [edi],xmm4
    mov          al,  [edx-FDEC_STRIDE+15]
    mov          [edi+16], al
    movdqu       xmm2,[edi+1]
    PRED8x8_LOWPASS_XMM xmm0,xmm1,xmm2,xmm4,xmm5
done00:    
    mov        ecx, [picesp+16]
    and        ecx,  16
    jne        downleft00
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
    movzx      eax, byte [edx-1*FDEC_STRIDE-1] 
    movd       mm6,eax
    movzx      eax, byte [edx+7*FDEC_STRIDE-1] 
    movd       mm4,eax
    psllq      mm4,56
    movq      mm7,[edi]
    movq      mm1, mm7
    psllq     mm1, 8
    por       mm1, mm6
    movq      mm2, mm7
    psrlq     mm2, 8
    por       mm2, mm4
    PRED8x8_LOWPASS mm5,mm7
    movq      [edi],mm5
    mov       al,[edx+7*FDEC_STRIDE-1] 
    mov       ah,al
    pinsrw    mm6,eax,0
    pshufw    mm6,mm6,0 
    movq      [edi+8],mm6
    movdqu    xmm3,[edi]
    jmp       doit00
downleft00:    
    mov         bl,  [edx+6*FDEC_STRIDE-1]
    mov         bh,  [edx+7*FDEC_STRIDE-1]
    mov         al,  [edx+2*FDEC_STRIDE-1]
    mov         ah,  [edx+3*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+4*FDEC_STRIDE-1]
    mov         bh,  [edx+5*FDEC_STRIDE-1]
    mov         al,  [edx+0*FDEC_STRIDE-1]
    mov         ah,  [edx+1*FDEC_STRIDE-1]
    mov         [edi], eax
    mov         [edi+4], ebx
    mov         bl,  [edx+14*FDEC_STRIDE-1]
    mov         bh,  [edx+15*FDEC_STRIDE-1]
    mov         al,  [edx+10*FDEC_STRIDE-1]
    mov         ah,  [edx+11*FDEC_STRIDE-1] 
    shl         ebx, 16
    shl         eax, 16
    mov         bl,  [edx+12*FDEC_STRIDE-1]
    mov         bh,  [edx+13*FDEC_STRIDE-1]
    mov         al,  [edx+8*FDEC_STRIDE-1]
    mov         ah,  [edx+9*FDEC_STRIDE-1]
    mov         [edi+8], eax
    mov         [edi+12], ebx
    movzx       eax, byte [edx+15*FDEC_STRIDE-1]
    movd        xmm4,eax
    pslldq      xmm4,15
    movzx       eax, byte [edx-1*FDEC_STRIDE-1]
    movd        xmm6,eax
    movdqu      xmm5, [edi]
    movdqa      xmm1, xmm5
    pslldq      xmm1, 1
    por         xmm1, xmm6
    movdqa      xmm2, xmm5
    psrldq      xmm2, 1
    por         xmm2, xmm4
    PRED8x8_LOWPASS_XMM xmm3, xmm1, xmm2, xmm5, xmm4

doit00:
    pxor        xmm6,xmm6
    movdqa      xmm4,xmm0
    movdqa      xmm5,xmm0
    punpcklbw   xmm4,xmm6
    punpckhbw   xmm5,xmm6
    movdqa      xmm1,xmm3
    movdqa      xmm2,xmm3
    punpcklbw   xmm1,xmm6
    punpckhbw   xmm2,xmm6
    paddw       xmm1,xmm4
    psraw       xmm1,1
    paddw       xmm2,xmm5
    psraw       xmm2,1
    packuswb    xmm1,xmm2
    movdqa      xmm0,xmm1
    psrldq      xmm0,1
%assign Y 0
%rep 4
    movq        [edx+Y*FDEC_STRIDE], xmm0
    psrldq      xmm0, 1
    movq        [edx+(Y+1)*FDEC_STRIDE],xmm0
    psrldq      xmm0, 1
%assign Y (Y+2)
%endrep
    
    pop         edi
    pop         ebx
  ;  picpop      ebx
    ret