;
; signed 16-bit fixed point arithmetic
;
; Scale 8 (sign 7-bit, 8-bit)
;
; based upon 6502 implementation by SlithyMatt
;

; FXP16_A = RA
FXP16_LDA_BYTE:
   mov FXP16_A+1, RA
   mov RA, #0
   mov FXP16_A, RA
   RTS

; FXP16_B = RA
FXP16_LDB_BYTE: 
   mov FXP16_B+1, RA
   mov RA, #0
   mov FXP16_B, RA
   RTS

; FXP16_C = FXP16_A * FXP16_B; FXP16_R overflow
FXP16_MULTIPLY: 
   PSH RC
   PSH RD
   
   ; push original A and B to stack
   mov RA, FXP16_B+1
   PSH RA
   mov RA, FXP16_B
   PSH RA
   mov RA, FXP16_A
   PSH RA
   mov RA, FXP16_A+1
   PSH RA
   
   OR  RA, #0
   JNN .check_sign_b
   mov RCD, FXP16_A
   XOR RC, #0xFF
   XOR RD, #0xFF
   INC RCD
   mov FXP16_A, RCD ; A = |A|
   
.check_sign_b:
   mov RA, FXP16_B+1
   OR  RA, #0
   JNN .init_c
   mov RCD, FXP16_B
   XOR RC, #0xFF
   XOR RD, #0xFF
   INC RCD
   mov FXP16_B, RCD ; B = |B|
   
.init_c:
   mov RC, #0
   mov RD, RC
   
   mov RA, #16
   
.loop1:
   LSR FXP16_B+1
   LSR FXP16_B
   JNC .loop2
   CLC
   ADD RC, FXP16_A
   ADD RD, FXP16_A+1
   
.loop2:
   LSR RD
   LSR RC
   LSR FXP16_C
   
   DEC RA
   JNZ .loop1
   
   mov FXP16_R+1, RA ; = 0
   mov FXP16_C+1, RC
   mov FXP16_R, RD
      
   ; restore A and B
   PUL RA
   mov FXP16_A+1, RA
   PUL RA
   mov FXP16_A, RA
   PUL RA
   mov FXP16_B, RA
   PUL RA
   mov FXP16_B+1, RA

   XOR RA, FXP16_A+1 ; sign(A) == sign(B)?
   JNN .end
   
.negative:
   mov RCD, FXP16_C
   XOR RC, #0xFF
   XOR RD, #0xFF
   INC RCD
   mov FXP16_C, RCD
   
.end:
   PUL RD
   PUL RC
   RTS
   
; FXP16_C = FXP16_A / FXP16_B; FXP16_R = FXP16_A % FXP16_B
FXP16_DIVIDE: 
   PSH RC
   PSH RD
   
   mov RA, FXP16_B
   PSH RA
   mov RA, FXP16_B+1
   PSH RA ; preserve original B on stack
   
   mov RA, FXP16_A+1
   OR  RA, #0
   JNS .abs_a
   mov RA, FXP16_A
   mov FXP16_C, RA
   mov RA, FXP16_A+1
   mov FXP16_C+1, RA
   JMP .check_sign_b
   
.abs_a:
   mov RA, #0
   STC
   SUB RA, FXP16_A
   mov FXP16_C, RA
   mov RA, #0
   SUB RA, FXP16_A+1
   mov FXP16_C+1, RA ; C = |A|
   
.check_sign_b:
   mov RA, FXP16_B+1
   OR  RA, #0
   JNN .shift_b
   mov RA, #0
   STC
   SUB RA, FXP16_B
   mov FXP16_B, RA
   mov RA, #0
   SUB RA, FXP16_B+1
   mov FXP16_B+1, RA
   
.shift_b:
   mov RA, FXP16_B+1
   mov FXP16_B, RA
   mov RA, #0
   mov FXP16_B+1, RA
   mov FXP16_R, RA
   mov FXP16_R+1, RA
   
   mov RC, #16     ;There are 16 bits in C
   
.loop1:
   CLC 
   LSL FXP16_C    ;Shift hi bit of C into REM
   LSL FXP16_C+1  ;(vacating the lo bit, which will be used for the quotient)
   LSL FXP16_R
   LSL FXP16_R+1
   mov RA, FXP16_R
   STC         ;Trial subtraction
   SUB RA, FXP16_B
   mov RD, RA
   mov RA, FXP16_R+1
   SUB RA, FXP16_B+1
   JNC .loop2  ;Did subtraction succeed?
   mov FXP16_R+1, RA   ;If yes, save it
   mov FXP16_R, RD
   INC FXP16_C    ;and record a 1 in the quotient
   
.loop2:
   DEC RC
   JNZ .loop1
   
   PUL RA
   mov FXP16_B+1, RA
   PUL RA
   mov FXP16_B, RA   
   
   mov RA, FXP16_B+1
   XOR RA, FXP16_A+1
   JNN .end
   
.negative:
   mov RCD, FXP16_C
   XOR RC, #0xFF
   XOR RD, #0xFF
   INC RCD
   mov FXP16_C, RCD
   
.end:
   PUL RD
   PUL RC
   RTS

; FXP16_C = FXP16_A - FXP16_B
FXP16_SUBTRACT: 
   mov RA, FXP16_A
   STC
   SUB RA, FXP16_B
   mov FXP16_C, RA
   mov RA, FXP16_A+1
   SUB RA, FXP16_B+1
   mov FXP16_C+1, RA
   RTS

; FXP16_C = FXP16_A + FXP16_B
FXP16_ADD: 
   mov RA, FXP16_A
   CLC
   ADD RA, FXP16_B
   mov FXP16_C, RA
   mov RA, FXP16_A+1
   ADD RA, FXP16_B+1
   mov FXP16_C+1, RA
   RTS
   
#bank data
FXP16_A: #res 2
FXP16_B: #res 2
FXP16_C: #res 2
FXP16_R: #res 2
