;
; Mandelbrot set calculation
;
; based upon 6502 implementation by SlithyMatt
;
#bank pgm

;
; Input:	
;  RC, RD - (X,Y) bitmap coordinates
; Output:
;   RB - # iterations executed (0 to MAND_MAX_IT-1)
;
MAND_GET:
   PSH RC
   PSH RD
   
   mov RA, RC
   JSR FXP16_LDA_BYTE  ; A = X coordinate   
   
   mov RA, #MAND_WIDTH
   JSR FXP16_LDB_BYTE  ; B = width
   
   JSR FXP16_DIVIDE     ; C = A/B
   
   mov RCD, FXP16_C     ; A = C (scaled X with zero min)
   mov FXP16_A, RCD
   
   mov RA, #MAND_XMAX[7:0] ; B = max scaled X
   mov FXP16_B, RA
   mov RA, #MAND_XMAX[15:8]
   mov FXP16_B+1, RA
   
   JSR FXP16_MULTIPLY  ; C = A*B
   
   mov RCD, FXP16_C     ; A = C (X*Xmax)
   mov FXP16_A, RCD
   
   mov RA, #MAND_XMIN[7:0] ; B = min scaled X
   mov FXP16_B, RA
   mov RA, #MAND_XMIN[15:8]
   mov FXP16_B+1, RA
   
   JSR FXP16_ADD        ; C = A+B (scaled X)
   
   mov RCD, FXP16_C
   mov mand_x0, RCD    ; x0 = C
   
   PUL RA               ; retrieve Y from stack
   PSH RA               ; put Y back on stack
   JSR FXP16_LDA_BYTE   ; A = Y coordinate
               
   mov RA, #MAND_HEIGHT ; B = height
   JSR FXP16_LDB_BYTE
   
   JSR FXP16_DIVIDE     ; C = A/B
   
   mov RCD, FXP16_C     ; A = C (scaled Y with zero min)
   mov FXP16_A, RCD
   
   mov RA, #MAND_YMAX[7:0]  ; B = max scaled Y
   mov FXP16_B, RA
   mov RA, #MAND_YMAX[15:8]
   mov FXP16_B+1, RA
   
   JSR FXP16_MULTIPLY   ; C = A*B
   
   mov RCD, FXP16_C     ; A = C (Y*Ymax)
   mov FXP16_A, RCD
               
   mov RA, #MAND_YMIN[7:0]  ; B = min scaled Y
   mov FXP16_B, RA
   mov RA, #MAND_YMIN[15:8]
   mov FXP16_B+1, RA
   
   JSR FXP16_ADD        ; C = A+B (scaled Y)
   
   mov RCD, FXP16_C
   mov mand_y0, RCD    ; y0 = C

   mov RA, #0
   mov mand_x, RA
   mov mand_x+1, RA
   mov mand_y, RA
   mov mand_y+1, RA
   
   mov RB, #0            ; RB = I (init to 0)
.loop:
   mov RCD, mand_x       ; A = X
   mov FXP16_A, RCD
   mov FXP16_B, RCD      ; B = X   
   JSR FXP16_MULTIPLY    ; C = X^2
   
   mov RCD, FXP16_C      ; keep x^2
   mov mand_x2, RCD
   
   mov RCD, mand_y       ; A = Y
   mov FXP16_A, RCD
   mov FXP16_B, RCD   ; B = Y
   JSR FXP16_MULTIPLY    ; C = Y^2
   
   mov RCD, FXP16_C      
   ;mov mand_y2, RCD     ; keep Y^2
   mov FXP16_B, RCD      ; B = Y^2
   
   mov RCD, mand_x2      ; A = X^2
   mov FXP16_A, RCD

   JSR FXP16_ADD        ; C = X^2+Y^2
   
   mov RA, FXP16_C+1
   STC
   SUB RA, #4
   JZS .check_fraction
   JNS .do_it
   JMP .dec_i
   
.check_fraction:
   mov RA, FXP16_C
   OR  RA, #0
   JNZ .dec_i
   
.do_it:
   JSR FXP16_SUBTRACT   ; C = X^2 - Y^2
   mov RCD, FXP16_C     ; A = C (X^2 - Y^2)
   mov FXP16_A, RCD
   
   mov RCD, mand_x0    ; B = X0
   mov FXP16_B, RCD
   
   JSR FXP16_ADD        ; C = X^2 - Y^2 + X0
   
   mov RCD, FXP16_C
   mov mand_xtemp, RCD  ; Xtemp = C
   
   mov RCD, mand_x     ; A = 2*X
   CLC
   LSL RC
   LSL RD
   mov FXP16_A, RCD
   
   mov RCD, mand_y     ; B = Y
   mov FXP16_B, RCD
   JSR FXP16_MULTIPLY  ; C = 2*X*Y
   
   mov RCD, FXP16_C     ; A = C (2*X*Y)
   mov FXP16_A, RCD
   
   mov RCD, mand_y0    ; B = Y0
   mov FXP16_B, RCD
   JSR FXP16_ADD        ; C = 2*X*Y + Y0
   
   mov RCD, FXP16_C
   mov mand_y, RCD     ; Y = C (2*X*Y + Y0)
   
   mov RCD, mand_xtemp
   mov mand_x, RCD      ; X = Xtemp
   
   INC RB
   STC
   CMP RB, #MAND_MAX_IT
   JNZ .loop
   
.dec_i:
   DEC RB
   
   PUL RD
   PUL RC
   RTS
   
#bank data
mand_x0:     #res 2
mand_y0:     #res 2
mand_x:      #res 2
mand_y:      #res 2
mand_x2:     #res 2
mand_y2:     #res 2
mand_xtemp:  #res 2
