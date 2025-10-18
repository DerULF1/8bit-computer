;
; signed 16-bit fixed point string operations
;
; Scale 8 (sign 7-bit, 8-bit)
;

;
; FXP16_A = string pointer in RCD
; 
; STC = number too large
;
FXP16_FROM_STR:
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JZS .save_sign
	STC
	CMP RA, #"+"
	JZS .save_sign
	DEC RCD
	mov RA, #"+"
.save_sign:
	PSH RA
	
	mov RB, #0
	mov FXP16_A, RB
	mov FXP16_A+1, RB

.lead_loop:
	INC RCD
	mov RA, [RCD]
	
	STC
	CMP RA, #"0"
	JNC .end_lead
	CMP RA, #"9"+1
	JCS .end_lead
	
	; RB = RB*10
	PSH RB
	CLC
	LSL RB
	JCS .too_large_2
	LSL RB
	JCS .too_large_2
	PUL RA
	ADD RB, RA
	JCS .too_large
	LSL RB
	JCS .too_large
	
	mov RA, [RCD]	
	AND RA, #15
	CLC
	ADD RB, RA
	JNC .lead_loop
.too_large_2:
	PUL RA ; delete current number from stack
.too_large:
	PUL RA ; delete sign from stack
	STC
	RTS

.end_lead:
	STC
	CMP RA, #"."
	JNZ .store_leading ; no period? skip trailing digits
	
	PSH RB ; save leading number
	
	mov RB, #4 ; read at most 4 digits
	
	mov RA, #0 ; end of digits marker
	PSH RA
	
.trail_collect_loop:
	; read next digit
	INC RCD
	mov RA, [RCD]
	
	; end reading if not a digit character
	STC
	CMP RA, #"0"
	JNC .end_trail_collect
	CMP RA, #"9"+1
	JCS .end_trail_collect
	
	PSH RA ; save digit character
	DEC RB ; count digits
	JNZ .trail_collect_loop

.end_trail_collect:
	; prepare fix point B-register for division by 10
	mov RA, #0
	mov FXP16_B, RA
	mov RA, #10
	mov FXP16_B+1, RA

.trail_loop:
	PUL RA ; restore digit character
	OR  RA, #0 ; end of digit marker?
	JZS .end_trail
	
	AND RA, #15 ; convert character to number
	CLC         ; add to current value
	ADD RA, FXP16_A+1
	mov FXP16_A+1, RA
	
	JSR FXP16_DIVIDE ; divide by 10
	
	; pick up result
	mov RA, FXP16_C
	mov FXP16_A, RA
	mov RA, FXP16_C+1
	mov FXP16_A+1, RA
	JMP .trail_loop

.end_trail:
	; check for rounding up
	mov RA, FXP16_R
	STC
	CMP RA, #5
	JNC .no_rounding ; less than 5
	JNZ .round_up    ; greater than 5
	
	mov RA, FXP16_A  ; round to even
	AND RA, #1
	JZS .no_rounding
	
.round_up:
	INC FXP16_A
	ICC FXP16_A+1
	
.no_rounding:	
	PUL RB ; restore leading number
	CLC
	ADD RB, FXP16_A+1
	
.store_leading:	
	mov FXP16_A+1, RB

	PUL RA ; restore sign
	STC
	CMP RA, #"-"
	JNZ .end ; not negative
	
	; negate value
	mov RA, FXP16_A
	XOR RA, #255
	XOR RB, #255
	INC RA
	ICC RB
	mov FXP16_A, RA
	mov FXP16_A+1, RB
.end:
	CLC
	RTS

;
; string pointer in RCD = FXP16_A
;
; nul terminated
;
FXP16_TO_STR:
	; is negative?
	mov RB, FXP16_A+1
	OR  RB, #0
	JNN .is_positive
	
	; negate value
	mov RA, FXP16_A
	XOR RA, #255
	XOR RB, #255
	INC RA
	ADD RB, #0	
	mov FXP16_A, RA
	mov FXP16_A+1, RB
	
	; write leading minus to output
	mov RA, #"-"
	mov [RCD], RA
	INC RCD
	
.is_positive:	
	; end of lead char marker
	mov RA, #0
	PSH RA
	
	; RB still contains FXP16_A+1

.lead_loop:
	PSH RC	; save character output position
	
	; divide RB by 10
	mov RA, #0  ; remainder gets into RA
	mov RC, #8  ; 8 bits
	CLC
	PSF
.div_loop:
	PLF
	LSL RB
	LSL RA
	STC
	CMP RA, #10
	JNC .no_sub
	SUB RA, #10
.no_sub:
	PSF
	DEC RC ; bit counter
	JNZ .div_loop
	PLF
	LSL RB
	
	PUL RC ; restore character output position
	
	OR  RA, #"0" ; make remainder to character
	PSH RA       ; save leading character
	
	OR  RB, #0 ; still numbers left?
	JNZ .lead_loop

	; write leading characters
.lead_out_loop:
	PUL RA
	mov [RCD], RA
	INC RCD
	OR  RA, #0 ; end marker?
	JNZ .lead_out_loop ; no!
	
	; check trailing numbers
	mov RA, FXP16_A
	OR  RA, #0 ; none? then write no period an leave
	JZS .end
	
	DEC RCD ; overwrite ending NUL character

	; write period to output buffer
	mov RA, #"."
	mov [RCD], RA
	INC RCD

	; remove leading digits
	mov RA, #0
	mov FXP16_A+1, RA
	
	; multiply by 10
	mov FXP16_B, RA
	mov RA, #10
	mov FXP16_B+1, RA
	
	mov RB, #3 ; max. number of trailing digits
	
.trail_loop:
	PSH RB ; save digit counter
	
	JSR FXP16_MULTIPLY
	
	mov RA, FXP16_C+1
	OR  RA, #"0"
	mov [RCD], RA
	INC RCD
	
	PUL RB ; restore digit counter
	
	mov RA, FXP16_C
	mov FXP16_A, RA
	OR  RA, #0
	JZS .end_zero
	
	DEC RB ; count digits
	JNZ .trail_loop
	
	OR  RA, #0
	JNN .end_zero

.inc_loop:
	DEC RCD
	
	mov RA, [RCD]
	INC RA
	STC
	CMP RA, #"9"+1
	JZS .inc_loop
	
	mov [RCD], RA
	INC RCD
	
.end_zero:	
	mov RA, #0
	mov [RCD], RA
.end:	
	RTS
