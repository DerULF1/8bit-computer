;
; Standard functions for screen cursor
;

#bank pgm

CURSOR_TYPE_OFF = 0 ; no visible cursor
CURSOR_TYPE_LINE = 2 ; underline cursor
CURSOR_TYPE_BLINK = 3 ; blinking block cursor

CURSOR_SCROLL = 0 ; scroll down at last screen position
CURSOR_WRAP = 1 ; wrap to home at last screen position

;
; sets the shape of the cursor
;
; parm:
;  RA - cursor type
;
CURSOR_MODE:
	PSH RA
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	PUL RA
	
	CLC
	ADD RA, #"0" 
	JSR PUT_CHAR
	
	mov RA, #"q" ; cursor mode
	JMP PUT_CHAR
	
;
; clear the screen and send cursor home
;
CURSOR_CLEAR_SCREEN:
	; clear screen
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	mov RA, #"2" 
	JSR PUT_CHAR
	mov RA, #"J" 
	JMP PUT_CHAR
	
;
; sets the cursor to a position
;
; parm:
;  RA - cursor line (0-3)
;  RB - cursor column (0-39)
;
CURSOR_GOTO_POSITION:
	PSH RA
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	PUL RA
	
	CLC
	ADD RA, #"0" ; line number
	JSR PUT_CHAR
	
	mov RA, #";" ; parameter separator
	JSR PUT_CHAR
	
	mov RA, #"0"
.tloop: ; 10th of the column
	STC
	CMP RB, #10
	JNC .end_tloop
	SUB RB, #10
	INC RA
	JMP .tloop
.end_tloop:
	JSR PUT_CHAR
	mov RA, RB
	
	CLC ; 1th of the column
	ADD RA, #"0"
	JSR PUT_CHAR
	
	mov RA, #"H" ; position cursor
	JSR PUT_CHAR
	RTS

;
; sets the scroll mode (scroll/wrap)
;
; parm:
;  RA - scroll type
;
CURSOR_SCROLL_MODE:
	PSH RA
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	PUL RA
	
	CLC
	ADD RA, #"0" ; <>0 => display raw
	JSR PUT_CHAR
	
	mov RA, #"h" ; display raw
	JSR PUT_CHAR
	RTS

;
; moves the cursor one position to the left
;
CURSOR_LEFT:
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	
	mov RA, #"D" ; cursor left
	JSR PUT_CHAR
	RTS

;
; moves the cursor one position to the right
;
CURSOR_RIGHT:
	mov RA, #27 ; ESC
	JSR PUT_CHAR
	mov RA, #"[" ; ESC 2
	JSR PUT_CHAR
	
	mov RA, #"C" ; cursor right
	JSR PUT_CHAR
	RTS
