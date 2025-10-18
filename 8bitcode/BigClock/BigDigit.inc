;
; Functions for Big LCD Screen Digits.
;

#bank pgm

;
; Define characters for Big Digits.
;
BIG_DIGIT_INIT:
	mov RA, #0
	mov RB, #8
	mva RCD, CHAR0
	JMP DISPLAY_DEFINE_CHARS

;
; Show a big colon sign.
;
; RB = screen column
;
BIG_DIGIT_COLON:
	mov RC, #COLON-DIGIT0
	mov RD, #1
	JMP BIG_DIGIT_DRAW

;
; Show a big period sign.
;
; RB = screen column
;
BIG_DIGIT_PERIOD:
	mov RC, #PERIOD-DIGIT0
	mov RD, #1
	JMP BIG_DIGIT_DRAW

;
; Show a big digit.
;
; RA = digit 0-9
; RB = screen column
;
BIG_DIGIT_SHOW:
	; RC = RA*12
	mov RC, RA
	CLC
	LSL RA
	ADD RC, RA
	LSL RC
	LSL RC
	mov RD, #3
; fall through to BIG_DIGIT_DRAW
	
;
; RB = screen column
; RC = offset to start image 
; RD = number of screen columns
;
BIG_DIGIT_DRAW:
	mov RA, #CURSOR_WRAP
	JSR CURSOR_SCROLL_MODE
	
	mov RA, #0	
.line_loop:
	PSH RA ; save line number
	PSH RB ; save column number
	PSH RD ; number of colums to draw
	PSH RC ; save current byte offset
	JSR CURSOR_GOTO_POSITION
	PUL RB ; restore current byte offset
		
	PUL RD ; number of colums to draw
	PSH RD 
.col_loop:
	mov RA, DIGIT0,RB
	JSR PUT_CHAR
	CLC
	ADD RB, #1
	STC ; count number of colums to draw
	SUB RD, #1
	JNZ .col_loop	
	mov RC, RB ; save current byte offset
	
	PUL RD ; number of colums to draw
	PUL RB ; restore column number
	PUL RA ; restore line number

	CLC    ; count line number
	ADD RA, #1
	STC
	CMP RA, #4
	JNZ .line_loop
	
	mov RA, #CURSOR_SCROLL
	JMP CURSOR_SCROLL_MODE
	
	#d RELOC_ESC
	ld16 CHAR_END-DIGIT0+1

; DIGIT bytes (3x4 characters)
DIGIT0:
	#d8  0, 6, 1
	#d8  6,32, 6
	#d8  6,32, 6
	#d8  2, 6, 3
DIGIT1:
	#d8  0, 6,32
	#d8 32, 6,32
	#d8 32, 6,32
	#d8 32, 6,32
DIGIT2:
	#d8  0, 6, 1
	#d8 32, 0, 3
	#d8  0, 3,32
	#d8  6, 6, 6
DIGIT3:
	#d8  0, 6, 1
	#d8 32, 0, 3
	#d8 32, 2, 1
	#d8  2, 6, 3
DIGIT4:
	#d8  6,32, 6
	#d8  2, 6, 6
	#d8 32,32, 6
	#d8 32,32, 6
DIGIT5:
	#d8  6, 6, 6
	#d8  6, 4,32
	#d8 32, 2, 1
	#d8  2, 6, 3
DIGIT6:
	#d8  0, 6, 1
	#d8  6,32,32
	#d8  6, 5, 1
	#d8  2, 6, 3
DIGIT7:
	#d8  6, 6, 6
	#d8 32, 0, 3
	#d8 32, 6,32
	#d8 32, 6,32
DIGIT8:
	#d8  0, 6, 1
	#d8  2, 4, 3
	#d8  0, 5, 1
	#d8  2, 6, 3
DIGIT9:
	#d8  0, 6, 1
	#d8  2, 4, 6
	#d8 32,32, 6
	#d8  2, 6, 3
; colon bytes (1x4)	
COLON:
	#d8 32
	#d8 7
	#d8 7
	#d8 32
; period bytes (1x4)	
PERIOD:
	#d8 32
	#d8 32
	#d8 32
	#d8  7
	
; special characters for Big Digits
CHAR0:
	#d8 0b00000001
	#d8 0b00000011
	#d8 0b00000011
	#d8 0b00000111
	#d8 0b00000111
	#d8 0b00001111
	#d8 0b00001111
	#d8 0b00011111
CHAR1:
	#d8 0b00010000
	#d8 0b00011000
	#d8 0b00011000
	#d8 0b00011100
	#d8 0b00011100
	#d8 0b00011110
	#d8 0b00011110
	#d8 0b00011111
CHAR2:
	#d8 0b00011111
	#d8 0b00001111
	#d8 0b00001111
	#d8 0b00000111
	#d8 0b00000111
	#d8 0b00000011
	#d8 0b00000011
	#d8 0b00000001
CHAR3:
	#d8 0b00011111
	#d8 0b00011110
	#d8 0b00011110
	#d8 0b00011100
	#d8 0b00011100
	#d8 0b00011000
	#d8 0b00011000
	#d8 0b00010000
CHAR4:
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
CHAR5:
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
CHAR6:
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
CHAR7:
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00001110
	#d8 0b00001110
	#d8 0b00001110
	#d8 0b00000000
	#d8 0b00000000
CHAR_END:	
