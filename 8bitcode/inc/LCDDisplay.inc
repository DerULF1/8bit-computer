;
; LCD Display functions
;

#bank pgm

LCD_ENABLE  = 0b00010000
LCD_READ    = 0b00100000
LCD_DATA    = 0b01000000

LCD_CLEAR   = 0b00000001
LCD_CONTROL = 0b00001000
LCD_SHIFT   = 0b00010000
LCD_INIT    = 0b00100000
LCD_INIT_4  = 0b00000010

LCD_8BIT_4  = 0b00000001
LCD_2LINE   = 0b00001000
LCD_DISPLAY = 0b00001000
LCD_RIGHT   = 0b00000100
LCD_SET_DD  = 0b10000000

LCD_BUSY    = 0b10000000

LCD_LIGHT_ON = 0b10000000
LCD_LIGHT_OFF = 0b00000000

LCD_PORT = 0

LCD_NUM_LINES = 4
LCD_NUM_COLS = 20

; outputs a single byte as a 2-digit hex number to the display
;
; Parms
;  RA - the byte to display
DISPLAY_HEX_BYTE:
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
	CMP RA, #"9"
	JNC DISPLAY_CHAR
	ADD RA, #"A"-"9"-2 ; c-flag=1
	JMP DISPLAY_CHAR

; outputs a null terminated string to the display
;
; Parms
;  RC/RD - start address of the string in memory
DISPLAY_STRING:		
	JSR DISPLAY_WAIT_NOT_BUSY
.loop:
	MOV RA, [RCD]
	AND RA, #0xff
	JZS .endstr
	JSR DISPLAY_CHAR
	INC RCD
	JNC .loop
.endstr:	
	RTS
	
; moves cursor to the beginning of the specified line
;
; Parms
;  RA - line number (0 to 3)
;
DISPLAY_GOTO_LINE:
	MOV RB, #0
; fall through

; moves cursor to the specified position on the display
;
; Parms
;  RA - line number (0 to 3)
;  RB - column number (0 to 19)
;
DISPLAY_GOTO_POSITION:
	mov display_cursor_col, RB
	mov RB, RA
	MOV RA, DISPLAY_LINES, RB
	CLC
	ADD RA, display_cursor_col
; fall through

; moves cursor to the specified offset on the display
;
; Parms
;  RA - offset (0 to 0x67)
;
DISPLAY_GOTO_OFFSET:
	OR  RA, #LCD_SET_DD
	JMP DISPLAY_SEND_COMMAND

; Byte offsets of the display lines in display RAM
	
	#d RELOC_ESC
	ld16 DISPLAY_LINES_END-DISPLAY_LINES+1

DISPLAY_LINES:
	#d8 0
	#d8 0x40
	#d8 20
	#d8 0x40 + 20
DISPLAY_LINES_END:

; outputs a single character to the display
;
; Parms
;  RA - character code
;
DISPLAY_CHAR:
	STC
	CMP RA, #8  ; backspace character?
	JZS DISPLAY_BACKSPACE
	STC
	CMP RA, #10 ; new line character?
	JZS .new_line
	STC
	CMP RA, #13 ; carriage return character?
	JZS .end
	
	JSR DISPLAY_SEND_DATA_BYTE
	
	mov RA, display_cursor_col
	INC RA
	CMP RA, #LCD_NUM_COLS-1 ; c-flag==0
	JZS .col_end
	mov display_cursor_col, RA
.end:	
	RTS
.col_end:
	JSR DISPLAY_MOVE_CURSOR_LEFT_INTERN
.new_line:	
	PSH RB
	PSH RC
	PSH RD
	JSR DISPLAY_NEW_LINE
	JSR DISPLAY_WAIT_NOT_BUSY
	PUL RD
	PUL RC
	PUL RB
	RTS	
;
; Moves the cursor one position to the left
; and overwrites the character at that position
; with space
;
DISPLAY_BACKSPACE:
	JSR DISPLAY_MOVE_CURSOR_LEFT
	JCS .end
	mov RA, #" "
	JSR DISPLAY_CHAR
	JSR DISPLAY_MOVE_CURSOR_LEFT
.end:	
	RTS	
;	
; Moves the cursor to the start of the next line.
; Screen scrolling is performed if the cursor is
; currently on the last line.
;
DISPLAY_NEW_LINE:
	JSR DISPLAY_WAIT_NOT_BUSY
	JSR DISPLAY_GET_CURSOR_POS
	INC RA
	CMP RA, #LCD_NUM_LINES-1 ; c-flag=0
	JNZ DISPLAY_GOTO_LINE
	mov RA, #0
	mov display_cursor_col, RA
	
DISPLAY_SCROLL_DOWN:
	mov RA, DISPLAY_LINES+1
	JSR DISPLAY_GOTO_OFFSET
	JSR DISPLAY_WAIT_NOT_BUSY
	
	mov RB, #0	
.loop1:	
	PSH RB
	JSR DISPLAY_READ_BYTE
	PUL RB
	mov display_scroll_buffer,RB, RA
	INC RB
	CMP RB, #2*LCD_NUM_COLS-1 ; c-flag=0
	JNZ .check_line1
	
	mov RA, DISPLAY_LINES+2
	JSR DISPLAY_GOTO_OFFSET
	JSR DISPLAY_WAIT_NOT_BUSY
	mov RB, #2*LCD_NUM_COLS
	JMP .loop1
	
.check_line1:	
	CMP RB, #3*LCD_NUM_COLS
	JNZ .loop1
	
	mov RA, DISPLAY_LINES
	JSR DISPLAY_GOTO_OFFSET
	JSR DISPLAY_WAIT_NOT_BUSY
	mov RB, #0
.loop3:	
	PSH RB
	mov RA, display_scroll_buffer,RB
	JSR DISPLAY_SEND_DATA_BYTE
	PUL RB
	INC RB
	CMP RB, #2*LCD_NUM_COLS-1 ; c-flag=0
	JNZ .check_line2
	
	mov RA, DISPLAY_LINES+1
	JSR DISPLAY_GOTO_OFFSET
	JSR DISPLAY_WAIT_NOT_BUSY
	mov RB, #2*LCD_NUM_COLS
	JMP .loop3
	
.check_line2:
	CMP RB, #3*LCD_NUM_COLS
	JNZ .loop3
	
.loop5:	
	PSH RB
	mov RA, #" "
	JSR DISPLAY_SEND_DATA_BYTE
	PUL RB
	INC RB
	CMP RB, #4*LCD_NUM_COLS-1 ; c-flag=0
	JNZ .loop5
	
	mov RA, #LCD_NUM_LINES-1
	mov RB, display_cursor_col
	JMP DISPLAY_GOTO_POSITION
	
; Moves Cursor one line down (scrolls on last line)
DISPLAY_MOVE_CURSOR_DOWN:
	JSR DISPLAY_WAIT_NOT_BUSY
	JSR DISPLAY_GET_CURSOR_POS
	INC RA
	CMP RA, #LCD_NUM_LINES-1 ; c-flag=0
	JZS DISPLAY_SCROLL_DOWN
	JMP DISPLAY_GOTO_POSITION

; Moves Cursor one Position to the right
DISPLAY_MOVE_CURSOR_RIGHT:
	INC display_cursor_col
	mov RA, display_cursor_col
	STC
	CMP RA, #LCD_NUM_COLS
	JNC DISPLAY_MOVE_CURSOR_RIGHT_INTERN
	PSH RB
	PSH RC
	PSH RD
	JSR DISPLAY_GET_CURSOR_POS
	INC RB ; next column
	STC
	CMP RB, #LCD_NUM_COLS
	JNC .same_row
	INC RA ; next line
	; no cursor right at last screen position
	STC
	CMP RA, #LCD_NUM_LINES
	JCS .end
.start_row:	
	mov RB, #0 ; column 0
.same_row:
	JSR DISPLAY_GOTO_POSITION
	CLC
.end:
	PUL RD
	PUL RC
	PUL RB
	RTS
	
DISPLAY_MOVE_CURSOR_RIGHT_INTERN:
	MOV RA, #LCD_SHIFT | LCD_RIGHT
	JMP DISPLAY_SEND_COMMAND

; Moves Cursor one Position to the left
DISPLAY_MOVE_CURSOR_LEFT:
	DEC display_cursor_col
	; same line? use the short cut
	JCS DISPLAY_MOVE_CURSOR_LEFT_INTERN
	PSH RB
	PSH RC
	PSH RD
	JSR DISPLAY_GET_CURSOR_POS
	DEC RB
	JCS .same_row
	DEC RA
	JCS .start_row
	; no cursor left at pos (0,0)
	INC display_cursor_col
	JMP .end
.start_row:	
	mov RB, #LCD_NUM_COLS-1
.same_row:
	JSR DISPLAY_GOTO_POSITION
	CLC
.end:
	PUL RD
	PUL RC
	PUL RB
	RTS
	
DISPLAY_MOVE_CURSOR_LEFT_INTERN:		
	MOV RA, #LCD_SHIFT
	JMP DISPLAY_SEND_COMMAND
	
; Clears Screen (and Cursor Home)
DISPLAY_CLEAR:	
	mov RA, #0
	mov display_cursor_col, RA
	MOV RA, #LCD_CLEAR
	JMP DISPLAY_SEND_COMMAND

; Read current cursor position from display
; Returns
;  RA - current cursor line (0 to 3)
;  RB - current cursor column (0 to 19)
DISPLAY_GET_CURSOR_POS:
	JSR DISPLAY_GET_CURSOR_OFFSET
	mov RD, RA
	mov RB, #0
.line_loop:	
	mov RC, RD
	mov RA, DISPLAY_LINES,RB
	STC
	SUB RC, RA
	JNC .next_line
	CMP RC, #LCD_NUM_COLS
	JNC .end
	CLC
.next_line:
	ADD RB, #1
	JMP .line_loop
.end:	
	mov RA, RB
	mov RB, RC
	RTS
	
; Read current cursor offset from display
; Returns
;  RA - current cursor offset (0 to 0x67)
DISPLAY_GET_CURSOR_OFFSET:
	JSR DISPLAY_READ_STATE
	OR  RA, #0  
	JNS DISPLAY_GET_CURSOR_OFFSET ; retry when busy
	RTS
	
; Read instruction state from display
; Returns
;  RA - StateByte
DISPLAY_READ_STATE:
	PSH RB
	PSF
	SII
	MOV RA, #LCD_READ
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	
	INP RA, #LCD_PORT
	LSL RA
	LSL RA
	LSL RA
	LSL RA
	AND RA, #0xf0
	MOV RB, RA	
	
	MOV RA, #LCD_READ
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	
	INP RA, #LCD_PORT
	AND RA, #0x0f
	OR  RB, RA
	
	MOV RA, #LCD_READ
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	
	MOV RA, RB
	PLF
	PUL RB
	RTS
	
; Read data byte from display
; Returns
;  RA - dataByte
DISPLAY_READ_BYTE:
	MOV RA, display_light_state
	OR  RA, #LCD_DATA
	MOV display_light_state, RA
	JSR DISPLAY_READ_STATE
	MOV RB, display_light_state
	AND RB, #0xFF - LCD_DATA
	MOV display_light_state, RB
	RTS
	
; Waits until display is able to excute instruction
DISPLAY_WAIT_NOT_BUSY:
.loop:
	JSR DISPLAY_READ_STATE
	AND RA, #LCD_BUSY
	JNZ .loop
	;fall through
; Waits a few microseconds
; (JSR/RTS are sufficient at 1.8MHz)
DISPLAY_MICRO_WAIT:
	RTS	
	
; Initializes the communication to the display
;
; turns on display, blinking cursor and backlight
; clears the screen, sets cursor to home position
DISPLAY_INIT:
	MOV RA, #LCD_LIGHT_OFF
	MOV display_light_state, RA
	;JSR DISPLAY_WAIT_NOT_BUSY
	
	; force 8-bit Interface
	MOV RA, #LCD_8BIT_4 | LCD_INIT_4
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE ; ENABLE = ON
	OUT #LCD_PORT, RA
	XOR RA, #LCD_ENABLE ; ENABLE = OFF
	OUT #LCD_PORT, RA
 	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	
	XOR RA, #LCD_ENABLE ; ENABLE = ON
	OUT #LCD_PORT, RA
	XOR RA, #LCD_ENABLE ; ENABLE = OFF
	OUT #LCD_PORT, RA
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	
	; switch to 4-bit Interface (as an 8-bit Command)
	MOV RA, #LCD_INIT_4
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	XOR RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	JSR DISPLAY_MICRO_WAIT
	
	; Initialize Display (2-Line-Mode, 4-bit Command)
	MOV RA, #LCD_INIT | LCD_2LINE 
	JSR DISPLAY_SEND_COMMAND	
	
	; Clear Screen
	JSR DISPLAY_CLEAR
	
	; Turn on Display (and blinking cursor)	
	MOV RA, #LCD_LIGHT_ON
	MOV display_light_state, RA
	MOV RA, #LCD_CONTROL | 7
	; fall through
	
; sends an instruction to the display
;
; Parms
;  RA - Instruction Byte 
DISPLAY_SEND_COMMAND:
	PSH RA
	JSR DISPLAY_WAIT_NOT_BUSY
	PUL RA

;		
; Sends a Byte to the LCD Display Controler
;
; Parms
;  RA - Byte to Send
DISPLAY_SEND_BYTE:
	PSF ; save interrupt inhibit state
	SII
	
	; Send upper 4 bits
	PSH RA
	LSR RA
	LSR RA
	LSR RA
	LSR RA
	AND RA, #0x0f
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	XOR RA, #LCD_ENABLE
	OUT #LCD_PORT, RA

	; Send lower 4 bits
	PUL RA
	AND RA, #0x0f
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	OR  RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	XOR RA, #LCD_ENABLE
	OUT #LCD_PORT, RA
	
	PLF ; restore interrupt inhibit state
	RTS
	
DISPLAY_SEND_DATA_BYTE:
	PSH RA
	mov RA, display_light_state
	OR  RA, #LCD_DATA
	mov display_light_state, RA
	PUL RA
	
	JSR DISPLAY_SEND_BYTE
	
	mov RA, display_light_state
	AND RA, #0xff - LCD_DATA
	mov display_light_state, RA
	
	RTS

; switches LCD backlight on/off
;
; Parms
;  RA - LCD_LIGHT_ON / LCD_LIGHT_OFF
DISPLAY_SET_BACKLIGHT:
	MOV display_light_state, RA
	INP RA, #LCD_PORT
	AND RA, #0x7f
	OR  RA, display_light_state
	OUT #LCD_PORT, RA
	RTS	

#bank data

display_light_state: #res 1
display_cursor_col: #res 1
display_scroll_buffer: #res 3*LCD_NUM_COLS
