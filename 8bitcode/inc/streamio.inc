;
; Standard functions for input and output
;

#bank pgm

;
; reads character from stdin
;
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - char / error code
;
GET_CHAR:
	PSH RD
	PSH RC
	PSH RB
	mov RA, #0 ; stdin
	JSR OS_GETC
	PUL RB
	PUL RC
	PUL RD
	RTS

;
; writes a character to a file descriptor
;
; Parm:
;  RA - char 
;  RB - file descriptor
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
_PUT_CHAR:
	PSH RD
	PSH RC
	PSH RB
	PSH RA
	mov RC, RA
	mov RA, RB
	JSR OS_PUTC
	PUL RB
	JCS .skip_RA
	mov RA, RB
.skip_RA:
	PUL RB
	PUL RC
	PUL RD
	RTS

;
; writes character to stdout
;
; Parm:
;  RA - char 
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_CHAR:
	PSH RB
	mov RB, #1 ; stdout
	JSR _PUT_CHAR
	PUL RB
	RTS

;
; writes character to stderr
;
; Parm:
;  RA - char 
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_ERROR_CHAR:
	PSH RB
	mov RB, #2 ; stderr
	JSR _PUT_CHAR
	PUL RB
	RTS

;
; writes string to file descriptor
;
; Parm:
;  RB - file descriptor
;  RCD - pointer to null terminated string
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
_PUT_STRING:
	mov RA, [RCD]
	OR  RA, #0
	JZS .end
	JSR _PUT_CHAR
	JCS .end
	INC RCD
	JNC _PUT_STRING
	mov RA, #E2BIG
.end:	
	RTS

;
; writes string to stdout
;
; Parm:
;  RCD - pointer to null terminated string
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_STRING:
	PSH RB
	mov RB, #1 ; stdout
	JSR _PUT_STRING
	PUL RB
	RTS

;
; writes string to stderr
;
; Parm:
;  RCD - pointer to null terminated string
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_ERROR_STRING:
	PSH RB
	mov RB, #2 ; stderr
	JSR _PUT_STRING
	PUL RB
	RTS

;
; writes byte as hex value to stdout
;
; Parm:
;  RA - byte
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_HEX_BYTE:
	PSH RB
	mov RB, #1 ; stdout
	JSR _PUT_HEX_BYTE
	PUL RB
	RTS

;
; writes byte as hex value to stdout
;
; Parm:
;  RA - byte
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
PUT_ERROR_HEX_BYTE:
	PSH RB
	mov RB, #2 ; stderr
	JSR _PUT_HEX_BYTE
	PUL RB
	RTS

;
; writes byte as hex value to stdout
;
; Parm:
;  RA - byte
; Result:
;  c-flag: 0=O.K., 1=error
;  RA - error code
;
_PUT_HEX_BYTE:
	PSH RA
	LSR RA
	LSR RA
	LSR RA
	LSR RA
	JSR .show_4_bit	
	PUL RA
.show_4_bit:
	AND RA, #0b00001111
	CLC
	ADD RA, #"0"
	CMP RA, #"9" ; c-flag==0
	JNC .show_char
	ADD RA, #"A"-"9"-2 ; c-flag==1
.show_char:	
	JMP _PUT_CHAR
