;
; Expression evaluation for the assembler
;
EXPR_NEXT_EXPRESSION:
	
	JSR EXPR_NEXT_SUM
	
	RTS
	
EXPR_NEXT_SUM:
	JSR EXPR_NEXT_TERM
.loop:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #"+"
	JZS .goon
	STC
	CMP RA, #"-"
	JZS .goon
	RTS
	
.goon:
	mov RB, value_current+1
	PSH RB
	mov RB, value_current
	PSH RB
	PSH RA ; save current operation
	
	JSR IO_GETC ; consume operator
	JSR IO_SKIP_WHITE_SPACE
	JSR EXPR_NEXT_TERM
	
	PUL RC ; restore current operation
	STC
	CMP RC, #"+"
	JZS .add
	
	STC
	PUL RA ; restore first summand
	SUB RA, value_current
	mov value_current, RA
	PUL RA ; restore first summand
	SUB RA, value_current+1
	mov value_current+1, RA
	JCS .loop
.check_overflow:
	; ignore overflow in first pass
	mov RA, pass_asm
	STC
	CMP RA, #1
	JZS .loop
	JMP EXPR_MATH_OVERFLOW
	
.add:
	CLC
	PUL RA ; restore first summand
	ADD RA, value_current
	mov value_current, RA
	PUL RA ; restore first summand
	ADD RA, value_current+1
	mov value_current+1, RA
	JCS .check_overflow
	JMP .loop
	
EXPR_NEXT_TERM:
	JSR EXPR_NEXT_FACTOR
.loop:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #"*"
	JZS .goon
	STC
	CMP RA, #"/"
	JZS .goon
	RTS
	
.goon:
	mov RB, value_current+1
	PSH RB
	mov RB, value_current
	PSH RB
	PSH RA ; save current operation
	
	JSR IO_GETC ; consume operator
	JSR IO_SKIP_WHITE_SPACE
	JSR EXPR_NEXT_FACTOR
	
	PUL RC ; restore current operation
	STC
	CMP RC, #"*"
	JZS .multiply
	
	PUL RA
	mov divident, RA
	PUL RA
	mov divident+1, RA
	mov RA, value_current
	mov divisor, RA
	mov RB, value_current+1
	OR  RA, RB
	JZS .err_div0
	mov divisor+1, RB
	
	JSR DIVIDE
	
	mov RA, quotient
	mov value_current, RA
	mov RA, quotient+1
	mov value_current+1, RA
	
	JMP .loop
	
.err_div0:
	; ignore div0 in first pass
	mov RA, pass_asm
	STC
	CMP RA, #1
	JZS .loop
	mov RA, #ERR_DIV0
	JMP MAIN_ERROR_MSG_EXIT
	
.multiply:
	PUL RA
	mov factor1, RA
	PUL RA
	mov factor1+1, RA
	mov RA, value_current
	mov factor2, RA
	mov RA, value_current+1
	mov factor2+1, RA
	
	JSR MULTIPLY
	JCS .check_overflow
	
	mov RA, product
	mov value_current, RA
	mov RA, product+1
	mov value_current+1, RA
	
	JMP .loop
	
.check_overflow:
	; ignore overflow in first pass
	mov RA, pass_asm
	STC
	CMP RA, #1
	JZS .loop
	JMP EXPR_MATH_OVERFLOW
	
EXPR_NEXT_FACTOR:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #"("
	JNZ .no_bracket
	JSR IO_GETC  ; consume opening bracket
	
	JSR EXPR_NEXT_EXPRESSION
	
	mov RB, #")"
	JSR ASM_ASSERT_LAST_CHAR
	JSR IO_GETC ; consume closing bracket
	
	JMP .end_factor
	
.no_bracket:
	STC
	CMP RA, #"-"
	JNC .no_number
	JZS .is_number
	CMP RA, #"0"
	JNC .no_number
	CMP RA, #"9"+1
	JCS .no_number
.is_number:	
	JSR EXPR_NEXT_NUMBER
	JMP .end_factor
	
.no_number:	
	STC
	CMP RA, #34 ; double QUOTE "
	JNZ .no_char
	JSR WORD_NEXT_QUOTED
	; exactly one character?
	mov RA, word_cur_len
	DEC RA
	JZS .char_OK
	mov RA, #ERR_TOO_LONG
	JMP MAIN_ERROR_MSG_EXIT
.char_OK:
	; clear high byte
	mov value_current+1, RA
	; copy character
	mov RA, word_cur_txt
	mov value_current, RA
	JMP .end_factor
	
.no_char:	
	JSR WORD_IS_START_CHAR
	JCS .no_word
	
	JSR WORD_NEXT
	JSR WORD_UPPERCASE
	JSR EXPR_VARIABLE_VALUE
	JMP .end_factor
	
.no_word:
	JSR PUT_ERROR_CHAR
	mov RA, #"-"
	JSR PUT_ERROR_CHAR
	mov RA, #ERR_SYNTAX
	JMP MAIN_ERROR_MSG_EXIT
	
.end_factor:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #"["
	JNZ .end
	JSR EXPR_BIT_SELECTOR
.end:	
	RTS

;
; process bit selector.
;
; format is "value[to:from]"

; with "from" and "to"
; noting the bits of "value" to be included in the
; result. "from" and "to" are both meant inclusive
; and the result is always right aligned. So "from"
; denotes the number of bits to shift to the right
; and the difference "from"-"to"+1 denotes the number
; of rightmost bits to keep.
;
EXPR_BIT_SELECTOR:
	JSR IO_GETC ; consume opening bracket
	
	; save current value
	mov RA, value_current+1
	PSH RA
	mov RA, value_current
	PSH RA
	
	JSR EXPR_NEXT_EXPRESSION
	
	JSR IO_SKIP_WHITE_SPACE
	mov RB, #":"
	JSR ASM_ASSERT_CHAR
	JSR IO_GETC ; consume colon
	
	; save upper bit value
	mov RA, value_current
	PSH RA
	
	JSR EXPR_NEXT_EXPRESSION
	
	JSR IO_SKIP_WHITE_SPACE
	mov RB, #"]"
	JSR ASM_ASSERT_LAST_CHAR
	JSR IO_GETC ; consume closing bracket
	
	mov RC, value_current ; lower bit value
	PUL RD ; restore upper bit value
	STC
	SUB RD, RC ; subtract lower bit value
	JNC EXPR_MATH_OVERFLOW

	; restore current value
	PUL RA
	mov value_current, RA
	PUL RA
	mov value_current+1, RA
	JMP .check_low

	; move right while lower bit > 0
.shift_loop:	
	LSR value_current+1
	LSR value_current
.check_low:	
	DEC RC
	JNN .shift_loop
	
	; create AND mask for number of bits
	mov RA, #0
	mov RB, #0
.and_loop:	
	STC
	LSL RA
	LSL RB
	DEC RD
	JNN .and_loop
	
	AND RA, value_current
	mov value_current, RA
	AND RB, value_current+1
	mov value_current+1, RB
	
	RTS
	
EXPR_CHECK_REGISTER:
	mov RA, word_cur_len
	STC
	CMP RA, #3
	JNC .check2
	JNZ .end
	mov RA, word_cur_txt
	CMP RA, #"R"
	JNZ .no_register
	mov RA, word_cur_txt+1
	CMP RA, #"C"
	JNZ .no_register
	mov RA, word_cur_txt+2
	CMP RA, #"D"
	JNZ .no_register
	mov RA, #REG_RCD
	CLC
	RTS
	
.check2:
	CMP RA, #1 ; c-flag = 0
	JNZ .no_register
	mov RA, word_cur_txt
	CMP RA, #"R"
	JNZ .not_r
	mov RA, word_cur_txt+1
	SUB RA, #"A"
	JNC .no_register
	CMP RA, #4
	JCS .end
	ADD RA, #REG_RA
	RTS
	
.not_r:
	JNC .no_register
	CMP RA, #"S"
	JNZ .no_register
	mov RA, word_cur_txt+1
	CMP RA, #"P"
	JNZ .no_register
	mov RA, #REG_SP
	CLC
	RTS
	
.no_register:	
	STC
.end:	
	RTS
	
EXPR_VARIABLE_VALUE:
	JSR EXPR_CHECK_REGISTER
	JCS .no_register
	mov last_register, RA
	RTS
	
.no_register:
	JSR ASM_PREPARE_LOCAL_LABEL
	JSR VSTORE_GET_VALUE
	JCS .not_found
	mov RA, [RCD]
	mov value_current, RA
	INC RCD
	mov RA, [RCD]
	mov value_current+1, RA
	RTS
	
.not_found:
	mov RA, pass_asm
	STC
	CMP RA, #1
	JZS .end
	mov RA, #ERR_UNDEFINED_VARIABLE
	JMP MAIN_ERROR_MSG_EXIT
.end:	
	RTS

EXPR_NEXT_NUMBER:
	mov RC, #0 ; build number in RCD
	mov RD, RC

	JSR IO_GOTC
	STC
	CMP RA, #"0"
	JNZ .read_decimal
	JSR IO_GETC
	STC
	CMP RA, #"x"
	JZS .read_hex
	STC
	CMP RA, #"b"
	JZS .read_binary
	
.read_decimal:
	PSH RA ; save leading sign
		
	; check for leading sign
	STC
	CMP RA, #"-"
	JNZ .decimal_loop	
	JSR IO_GETC ; skip sign
	
.decimal_loop:
	STC
	CMP RA, #"0"
	JNC .end_decimal
	CMP RA, #"9"+1
	JCS .end_decimal
	AND RA, #15 ; isolate digit value
	
	; multiply current value by 10
	PSH RD ; save current value
	PSH RC
	CLC
	LSL RC ; by 2
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	LSL RC ; by 4
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	PUL RB ; add current value -> by 5
	ADD RC, RB 
	PUL RB
	ADD RD, RB
	JCS EXPR_MATH_NUMBER_TOO_BIG
	LSL RC ; by 10
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	ADD RC, RA ; add new digit
	ICC RD
	JCS EXPR_MATH_NUMBER_TOO_BIG

	JSR IO_GETC
	JMP .decimal_loop
	
.end_decimal:
	PUL RA ; restore sign
	STC
	CMP RA, #"-" ; negative value?
	JNZ .end_store
	
	; build two's complement	
	XOR RC, #0xff ; flip all bits
	XOR RD, #0xff
	INC RC ; add 1
	ICC RD
	JMP .end_store
	
.read_hex:
	JSR IO_GETC
	STC
	CMP RA, #"0"
	JNC .end_store
	CMP RA, #"9"+1
	JNC .hex_ok
	AND RA, #0b11011111 ; to upper case
	STC
	SUB RA, #"A"-10 ; char A becomes value 10
	JNC .end_store
	CMP RA, #16
	JCS .end_store
	
.hex_ok:
	AND RA, #15
	
	; current value by 16
	CLC
	LSL RC
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	LSL RC
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	LSL RC
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	LSL RC
	LSL RD
	JCS EXPR_MATH_NUMBER_TOO_BIG
	ADD RC, RA
	ICC RD
	JNC .read_hex
	JMP EXPR_MATH_NUMBER_TOO_BIG

.read_binary:
	JSR IO_GETC
	STC
	CMP RA, #"0"
	JNC .end_store
	CMP RA, #"1"+1
	JCS .end_store
.bin_OK:
	LSR RA ; shift new bit into c-flag
	LSL RC
	LSL RD
	JNC .read_binary
	JMP EXPR_MATH_NUMBER_TOO_BIG
	
.end_store:
	mov value_current, RC ; store end result
	mov value_current+1, RD
	
.end:	
	RTS

EXPR_MATH_NUMBER_TOO_BIG:
	mov RA, #ERR_NUMBER_TOO_BIG
	JMP MAIN_ERROR_MSG_EXIT

EXPR_MATH_OVERFLOW:
	mov RA, #ERR_OVERFLOW
	JMP MAIN_ERROR_MSG_EXIT

#bank data
value_current: #res 2
