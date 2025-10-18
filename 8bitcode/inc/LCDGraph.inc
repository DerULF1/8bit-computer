;
; LCD Graphic functions
;

#bank pgm

LCD_CGRAM   = 0b01000000

;
; Defines a custom character
;
; Parms:
;  RA    - character number (0 - 7)
;  RC/RD - pointer to eight character bytes 
;          (top to button, lower 5 bits)
;
DISPLAY_DEFINE_CHAR:
	mov RB, #1
	; fall through to DISPLAY_DEFINE_CHARS
	
;
; Defines a list of custom characters
;
; Parms:
;  RA    - first character number (0 - 7)
;  RB    - number of characters to define (1 - 7)
;  RC/RD - pointer to a list of character bytes 
;          eight bytes per character
;          (top to button, lower 5 bits)
;
DISPLAY_DEFINE_CHARS:
	LSL RB ; character bytes = character count by 8
	LSL RB
	LSL RB
	AND RB, #0b01111000
	
	AND RA, #7 	
	OR  RA, RB ; insert character count to bits 2 to 0
	
	PSH RA     ; save current data cursor position
	JSR OS_DISPLAY_GET_CURSOR_OFFSET
	mov RB, RA
	PUL RA
	PSH RB	
	
	PSH RA ; switch to character define mode
	LSL RA
	LSL RA
	LSL RA
	AND RA, #0b00111000
	OR  RA, #LCD_CGRAM
	JSR OS_DISPLAY_SEND_COMMAND
	PUL RB
	
	AND RB, #0b01111000
.loop:	
	mov RA, [RCD]
	JSR OS_DISPLAY_SEND_DATA_BYTE
	CLC
	ADD RC, #1
	ADD RD, #0
	STC
	SUB RB, #1
	JNZ .loop
	
	PUL RA ; restore data cursor position
	JSR OS_DISPLAY_GOTO_OFFSET
	
	RTS
	