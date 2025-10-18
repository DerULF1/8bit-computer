#bank pgm

;
; ADD two BCD numbers.
; 
; parm:
;  RA - operand one
;  RB - operand two
;  c-flag - Carry
; result:
;  RA - sum
;  c-flag - Carry
;	
BCD_ADD:
	PSH RA ; save operands for higher digit
	PSH RB
	PSF    ; save carry bit
	AND RA, #0x0f ; isolate lower digits
	AND RB, #0x0f
	PLF    ; restore carry-bit
	ADD RA, RB
	STC    ; decimal overflow?
	CMP RA, #0x0a
	JNC .high_digit
	SUB RA, #0x0a
.high_digit:
	mov RD, RA ; save low digit
	PUL RB
	PUL RA
	PSF    ; save carry-bit
	LSR RA ; shift right, so carry
	LSR RB ; fits into bit 7 
	AND RA, #0x78 ; isolate higher digit 
	AND RB, #0x78
	PLF    ; restore carry-bit
	JNC .no_carry
	ADD RA, #7 ; carry is set => +8
.no_carry:	
	ADD RA, RB
	STC    ; decimal overflow?
	CMP RA, #0x50 ; 0x50 == 0xa0/2
	JNC .no_overflow
	SUB RA, #0x50 
.no_overflow:
	PSF    ; save carry-bit
	CLC
	LSL RA ; restore high digit
	OR  RA, RD ; combine with low digit
	PLF    ; restore carry-bit
.end:	
	RTS
	
;
; Subtract two BCD numbers.
; 
; parm:
;  RA - operand one (Minuend)
;  RB - operand two (Subtrahend)
;  c-flag - Carry
; result:
;  RA - difference
;  c-flag - Carry
;	
BCD_SUB:
	PSH RA ; save operands for higher digit
	PSH RB
	PSF    ; save carry bit
	AND RA, #0x0f ; isolate lower digits
	AND RB, #0x0f
	PLF    ; restore carry-bit
	SUB RA, RB
	mov RD, RA ; save low digit
	PUL RB
	PUL RA
	JCS .high_digit ; decimal overflow?
	ADD RB, #0x10
	SUB RD, #0xf5 ; carry clear => -0xf6
.high_digit:
	AND RA, #0xf0
	AND RB, #0xf0
	STC
	SUB RA, RB
	JCS .no_overflow
	SUB RA, #0x5f ; carry clear => -0x60
	CLC
.no_overflow:
	PSF    ; save carry-bit
	OR  RA, RD ; combine with low digit
	PLF    ; restore carry-bit
.end:	
	RTS	