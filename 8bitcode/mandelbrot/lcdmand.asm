#include "8bit.cpu"

#include "OSCalls.inc"
#include "KeyNames.inc"

;
; lcdmand - Mandelbrot on LCD Display
;
; Written 07.05.2022 Ulf Caspers
;

#bankdef pgm
{
    #addr 0x0000
    #size 0x2000
    #outp 0x0000
}
 
#bankdef data
{
    #addr 0x2000
    #size 0x2000
}

RELOC_ESC = 0xFB

MAND_XMIN = 0xFD80 ; -2.5
MAND_XMAX = 0x0380 ; 3.5
MAND_YMIN = 0xFF00 ; -1
MAND_YMAX = 0x0200 ; 2

MAND_WIDTH = 32
MAND_HEIGHT = 22
MAND_MAX_IT = 15

BOARD_ROWS = 22
BOARD_COLS = 32

SCREEN_ROWS=4
SCREEN_COLS=20

BITS_PER_CHAR_LINE=5
LINES_PER_CHAR=8

DISPLAY_TYPE_SCREEN=0
DISPLAY_TYPE_GSCREEN=1

#bank data
DATA_START:

#bank pgm

; RC/RD - parameter string

START:
    mov parm_ptr, RCD

	mva RCD, board
	mov board_base, RCD
	
	; clear screen
	JSR CURSOR_CLEAR_SCREEN
	mov RA, #0
	mov RB, RA
	JSR CURSOR_GOTO_POSITION

	; output progress message
	mva RCD, calc_msg
	JSR PUT_STRING

	; store start time
	JSR OS_GET_TICKS
	mov ticks, RA
	mov ticks+1, RB
	mov ticks+2, RC
	
	; calculate Mandelbrot set
	JSR CALC_BOARD
	
	; start display at upper left corner
	JSR SCREEN_INIT
	JSR SCREEN_GENERATE
	JSR SCREEN_DRAW
	
	; store end time
	JSR OS_GET_TICKS
	STC
	SUB RA, ticks
	SUB RB, ticks+1
	SUB RC, ticks+2
	mov ticks, RA
	mov ticks+1, RB
	mov ticks+2, RC

.display_loop:
	mov RA, display_type
	STC
	CMP RA, #DISPLAY_TYPE_SCREEN
	JZS .screen
	JSR GSCREEN_GENERATE
	JSR GSCREEN_DRAW
	JMP .key_loop
.screen:
	JSR SCREEN_GENERATE
	JSR SCREEN_DRAW

.key_loop:
	JSR GET_CHAR
	
	STC
	CMP RA, #KEY_CURSOR_UP
	JNZ .no_up
	DEC origin_Y
	JCS .display_loop
	INC origin_Y
	JMP .key_loop
	
.no_up:
	STC
	CMP RA, #KEY_CURSOR_DOWN
	JNZ .no_down
	mov RA, origin_Y
	INC RA
	STC
	CMP RA, origin_max_Y
	JCS .key_loop
	mov origin_Y, RA
	JMP .display_loop
	
.no_down:	
	STC
	CMP RA, #KEY_CURSOR_LEFT
	JNZ .no_left
	DEC origin_X
	JCS .display_loop
	INC origin_X
	JMP .key_loop
	
.no_left:
	STC
	CMP RA, #KEY_CURSOR_RIGHT
	JNZ .no_right
	mov RA, origin_X
	INC RA
	STC
	CMP RA, origin_max_X
	JCS .key_loop
	mov origin_X, RA
	JMP .display_loop
	
.no_right:
	STC
	CMP RA, #"g"
	JNZ .no_g
	mov RA, display_type
	STC
	CMP RA, #DISPLAY_TYPE_SCREEN
	JZS .to_gscreen
	JSR SCREEN_INIT
	JMP .display_loop
.to_gscreen:	
	JSR GSCREEN_INIT
	JMP .display_loop

.no_g:
	STC
	CMP RA, #"x"
	JNZ .key_loop
	
	JSR CURSOR_CLEAR_SCREEN
	mov RA, #0
	mov RB, RA
	JSR CURSOR_GOTO_POSITION

.end:
	mva RCD, end_msg
	JSR PUT_STRING
	
	mov RA, ticks+1
	JSR PUT_HEX_BYTE
	mov RA, ticks
	JSR PUT_HEX_BYTE	

	mov RA, #CURSOR_TYPE_BLINK ; turn on cursor
	JSR CURSOR_MODE
	mov RA, #0
    RTS

.fail:
    JSR PUT_HEX_BYTE
    mva RCD, fail_msg
    JMP PUT_STRING

;
; Calculate the board for the Mandelbrot set.
;
CALC_BOARD:
	mov RD, #MAND_HEIGHT-1
	
.loop_height:	
	mov RC, #MAND_WIDTH-1
	
.loop_width:	
	JSR MAND_GET
	mov RA, set_colors,RB
	PSH RC
	PSH RD
	JSR BOARD_SET_BLOCK
	PUL RD
	PUL RC
	
.next_col:
	DEC RC
	JCS .loop_width
	
	; progress bar
	mov RA, #"."
	JSR PUT_CHAR
	
	DEC RD
	JCS .loop_height

	RTS	
	
;
; Sets a single block on the board.
;
; parms:
;  RA - color code (0= empty)
;  RC - x-axis (column 0 - 31)
;  RD - y-axis (row 0 - 21)
;	
BOARD_SET_BLOCK:	
	; boardpos = Y*BOARD_COLS+X
	
	mov RB, RC ; save X-Value
	
	; #fixed calculation: BOARD_COLS=32
	mov RC, #0	
	CLC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	OR  RC, RB ; ADD X-Value
	CLC
	ADD RC, board_base
	ADD RD, board_base+1
	mov [RCD], RA
	RTS

;
; Initializes the graphic screen.
;
GSCREEN_INIT:
	JSR CURSOR_CLEAR_SCREEN
	
	mov RA, #CURSOR_TYPE_OFF ; turn off cursor
	JSR CURSOR_MODE
	
	mov RA, #CURSOR_WRAP
	JSR CURSOR_SCROLL_MODE
		
    mov RA, #0
	mov origin_X, RA
	mov origin_Y, RA
	mov RA, #BOARD_COLS-4*BITS_PER_CHAR_LINE
	mov origin_max_X, RA
	mov RA, #BOARD_ROWS-2*LINES_PER_CHAR
	mov origin_max_Y, RA
	mov RA, #DISPLAY_TYPE_GSCREEN
	mov display_type, RA
		
	mov RA, #1
	mov RB, #8
	JSR CURSOR_GOTO_POSITION
	mov RA, #0
	JSR PUT_CHAR
	mov RA, #1
	JSR PUT_CHAR
	mov RA, #2
	JSR PUT_CHAR
	mov RA, #3
	JSR PUT_CHAR
	
	mov RA, #2
	mov RB, #8
	JSR CURSOR_GOTO_POSITION
	mov RA, #4
	JSR PUT_CHAR
	mov RA, #5
	JSR PUT_CHAR
	mov RA, #6
	JSR PUT_CHAR
	mov RA, #7
	JSR PUT_CHAR
	
.end:
	mov RA, #CURSOR_SCROLL
	JSR CURSOR_SCROLL_MODE
	RTS	
	
;
; Creates all screen data of the current board data.
;
; all board data will become user defined characters
;	
GSCREEN_GENERATE:
	; board position
	; #fixed calculation: BOARD_COLS=32
	mov RC, #0	
	mov RD, origin_Y
	CLC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	OR  RC, origin_X ; ADD X-Value
	CLC
	ADD RC, board_base
	ADD RD, board_base+1
	mov board_ptr, RCD

	mov RB, #0 ; screen position
	
.genloop:
	PSH RB ; save screen position
	
	mov RB, #8 ; 128>>(BITS_PER_CHAR_LINE-1)
	
.bitloop:	
	mov RA, [RCD] ; get board value
	INC RCD ; next board column
	
	OR  RA, #0 ; 0 -> CLC, else STC
	CLC
	JZS .zero
	STC
.zero:	
	LSL RB    ; shift c-flag into RB
	JNC .bitloop ; less than 5 bits -> loop
	
	mov RA, RB ; store byte
	PUL RB
	mov screen,RB, RA
	INC RB ; next character byte
	
	mov RA, RB
	AND RA, #7
	JZS .next_char
	
	CLC ; next line in board
	ADD RC, #BOARD_COLS-BITS_PER_CHAR_LINE
	ADD RD, #0
	JMP .genloop
	
.next_char:
	STC
	CMP RB, #8*LINES_PER_CHAR
	JCS .end
	STC
	CMP RB, #4*LINES_PER_CHAR
	JZS .next_screen_line
	
	STC
	SUB RC, #((LINES_PER_CHAR-1)*BOARD_COLS)[7:0]
	SUB RD, #((LINES_PER_CHAR-1)*BOARD_COLS)[15:8]
	JMP .genloop
	
.next_screen_line:
	CLC
	ADD RC, #BOARD_COLS - 4*BITS_PER_CHAR_LINE
	ADD RD, #0
	JMP .genloop
	
.end:	
	RTS

;
; Puts the current screen data into the display characters.
;
GSCREEN_DRAW:	
	mov RA, #0
	mov RB, #8
	mva RCD, screen
	JSR DISPLAY_DEFINE_CHARS
.end:
	RTS	

;
; Initializes the screen.
; Sets up the graphic characters.
;
SCREEN_INIT:
	JSR CURSOR_CLEAR_SCREEN
	
	mov RA, #CURSOR_TYPE_OFF ; turn off cursor
	JSR CURSOR_MODE
	
	mov RA, #0
	mov RB, #8
	mva RCD, CHAR1
	JSR DISPLAY_DEFINE_CHARS
	
    mov RA, #0
	mov origin_X, RA
	mov origin_Y, RA
	mov RA, #BOARD_COLS-SCREEN_COLS
	mov origin_max_X, RA
	mov RA, #BOARD_ROWS-2*SCREEN_ROWS
	mov origin_max_Y, RA
	mov RA, #DISPLAY_TYPE_SCREEN
	mov display_type, RA
	
	RTS
	
;
; Creates all screen data of the current board data.
;
; two lines of the board become one line on the screen
;	
SCREEN_GENERATE:
	; board position
	; #fixed calculation: BOARD_COLS=32
	mov RC, #0	
	mov RD, origin_Y
	CLC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	LSR RD
	LSR RC
	OR  RC, origin_X ; ADD X-Value
	CLC
	ADD RC, board_base
	ADD RD, board_base+1
	mov board_ptr, RCD

	mov RB, #0 ; screen position
	
.genloop:
	PSH RB ; save screen position
	
	mov RCD, board_ptr
	; upper line of the board
	mov RA, [RCD]
	PSH RA
	
	; lower line of the board
	CLC
	ADD RC, #BOARD_COLS
	ADD RD, #0
	mov RA, [RCD]
	
	; RA = 3*RA
	mov RB, RA
	CLC
	LSL RB
	ADD RB, RA
	
	PUL RA
	ADD RA, RB	
		
	SUB RA, #0 ; nocarry: RA=RA-1
	JCS .graphchar
	mov RA, #32 ; empty cell = space
	
.graphchar:	
	PUL RB ; restore screen position
	mov screen,RB, RA
	CLC
	
	INC RB ; advance screen position
	
	INC board_ptr ; advance board position
	ICC board_ptr+1
	
	STC 
	CMP RB, #SCREEN_COLS
	JNC .genloop
	JZS .nextrow
	
	STC 
	CMP RB, #SCREEN_COLS*2
	JNC .genloop
	JZS .nextrow
		
	STC 
	CMP RB, #SCREEN_COLS*3
	JNC .genloop
	JZS .nextrow
		
	STC 
	CMP RB, #SCREEN_COLS*4
	JNC .genloop
	JZS .graphend
	
.nextrow:
	mov RCD, board_ptr
	CLC
	ADD RC, #2*BOARD_COLS-SCREEN_COLS
	ADD RD, #0
	mov board_ptr, RCD
	JMP .genloop
	
.graphend:
	; insert score here
.end:	
	RTS

;
; Puts the current screen data onto the display.
;
SCREEN_DRAW:
	mov RA, #0
	mov RB, #0
	JSR CURSOR_GOTO_POSITION
	mov RA, #CURSOR_WRAP
	JSR CURSOR_SCROLL_MODE
	mov RB, #0
.loop:
	mov RA, screen,RB
	JSR PUT_CHAR
	CLC
	ADD RB, #1
	STC
	CMP RB, #SCREEN_COLS
	JNZ .not20
	mov RB, #2*SCREEN_COLS
	JMP .loop
.not20:
	STC
	CMP RB, #3*SCREEN_COLS
	JNZ .not60
	mov RB, #1*SCREEN_COLS
	JMP .loop
.not60:
	STC
	CMP RB, #2*SCREEN_COLS
	JNZ .not40
	mov RB, #3*SCREEN_COLS
	JMP .loop
.not40:
	STC
	CMP RB, #SCREEN_ROWS*SCREEN_COLS
	JNZ .loop
.end:
	mov RA, #CURSOR_SCROLL
	JSR CURSOR_SCROLL_MODE
	RTS	
	

#include "param.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "cursor.inc"
#bank pgm
#include "LCDGraph.inc"
#bank pgm
#include "mandelbr.inc"
#bank pgm
#include "fixp16.inc"
#bank pgm

    #d RELOC_ESC
    #d 0x00
    #d 0x00

fail_msg: #d " fail\n\0"
calc_msg: #d "Mandelbrot Set\nCalculating...\n\0"
end_msg:  #d "Ticks: 0x\0"
set_colors: #d8 2,2,2,2,2,2,2,1,1,1,1,1,1,1,0

CHAR1:
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
CHAR2:
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
CHAR3:
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00001010
CHAR4:
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00000000
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00001010
CHAR5:
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00000000
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00001010
CHAR6:
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00000000
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
CHAR7:
	#d8 0b00010101
	#d8 0b00001010
	#d8 0b00010101
	#d8 0b00000000
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
CHAR8:
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00000000
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111
	#d8 0b00011111

END_PGM:

;
; program trailer
;
    ld16 DATA_START
    ld16 DATA_END - DATA_START

#bank data
board_base: #res 2
board_ptr: #res 2
origin_X: #res 1
origin_max_X: #res 1
origin_Y: #res 1
origin_max_Y: #res 1
ticks: #res 3
display_type: #res 1

board:  #res BOARD_ROWS * BOARD_COLS
screen: #res SCREEN_ROWS*SCREEN_COLS

DATA_END:
