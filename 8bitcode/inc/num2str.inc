;
; 16 bit integer to String
;
; Parm:
;  RA/RB - 16-bit Integer
; Result:
;  num2str_txt - \0 terminated number string
NUM2STR:
	; store number
	mov divident, RA
	mov divident+1, RB
	
	; always divide by 10
	mov RA, #10
	mov divisor, RA
	mov RA, #0
	mov divisor+1, RA
	PSH RA ; put end marker on stack
		
.not_zero:
	JSR DIVIDE ; divide by 10
	
	; get remainder
	mov RA, remainder
	OR  RA, #"0" ; convert remainder to ascci char
	PSH RA ; push character onto stack
	
	; get division result
	mov RA, quotient 
	mov RB, quotient+1
	mov divident, RA
	mov divident+1, RB
	
	; goon until result is zero
	OR  RB, RA
	JNZ .not_zero
	
.outloop:	
	PUL RA
	mov num2str_txt,RB, RA
	INC RB
	OR  RA, #0
	JNZ .outloop
	DEC RB
	mov num2str_len, RB
	RTS

#bank data
num2str_area:
num2str_len: #res 1
num2str_txt: #res 6
