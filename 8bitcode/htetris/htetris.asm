#include "8bit.cpu"

#include "OSCalls.inc"
;
; htetris - horizontal Tetris
;
; Written 27.09.2020 Ulf Caspers
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

SYS_OWNER = 0xF0
RELOC_ESC = 0xFB

#bank data
DATA_START:

BOARD_ROWS = 8
BOARD_COLS = 16

SCREEN_ROWS=4
SCREEN_COLS=20

SCREEN_SCORE_COL = 18

#bank pgm

; RC/RD - parameter string

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
		
	JSR SCREEN_INIT
	
	mov RA, #0x55
	mov RB, RA
	mov RC, RA
	mov RD, RA
	JSR RANDOM_SET_SEED

.startscreen:	
	mov RA, #1
	mov RB, #2
	JSR CURSOR_GOTO_POSITION
	mva RCD, start_msg
	JSR PUT_STRING
	
	JSR GET_CHAR
	STC
	CMP RA, #"x" ;'x'
	JZS .end
	
	STC
	CMP RA, #"r"
	JNZ .restart
	
	JSR NEW_RANDOM_SEED

.restart:
	JSR GAMEPLAY_START
	
.gameloop:	
	mov RA, piece_type
	OR  RA, #0
	JNZ .pieceOK
	JSR GAMEPLAY_GET_RANDOM_PIECE
	JSR PIECE_INIT_NEW
	JSR BOARD_CHECK_PIECE
	JNC .drawpiece
	
	JSR GAMEPLAY_GAMEOVER
	JMP .startscreen

.drawpiece:
	mov RA, piece_type
	AND RA, #1
	INC RA
	JSR BOARD_DRAW_PIECE
	JSR SCREEN_GENERATE
	JSR SCREEN_DRAW
	
.pieceOK:
	DEC gameloop_count
	JCS .checkkey

	mov RA, game_speed
	mov gameloop_count, RA
	
	JSR GAMEPLAY_PIECE_LEFT
	mov RA, piece_type
	OR  RA , #0
	JZS .gameloop
	JMP .drawpiece

.checkkey:	
	JSR OS_KEYBOARD_GET_KEY
	OR  RA, #0
	JNZ .keypressed
	JSR GAMEPLAY_DELAY
	JMP .gameloop
	
.keypressed:	
	STC
	CMP RA, #"p"
	JNZ .nopause
	JSR GAMEPLAY_PAUSE
	STC
	CMP RA, #"x"
	JZS .startscreen
	JMP .drawpiece
	
.nopause:		
	JSR GAMEPLAY_KEY_PRESSED
	JMP .drawpiece
		
.end:	
	mov RA, #3
	mov RB, #0
	JSR CURSOR_GOTO_POSITION
	mov RA, #CURSOR_TYPE_BLINK ; turn on cursor
	JSR CURSOR_MODE
	RTS

;
; Starts a new game.
;
GAMEPLAY_GAMEOVER:		
	mov RA, #1
	mov RB, #4
	JSR CURSOR_GOTO_POSITION
	mva RCD, gameover_msg
	JSR PUT_STRING
	JSR GET_CHAR
	RTS

;
; Pauses the game.
;
GAMEPLAY_PAUSE:		
	mov RA, #1
	mov RB, #4
	JSR CURSOR_GOTO_POSITION
	mva RCD, pause_msg
	JSR PUT_STRING
	JSR GET_CHAR
	RTS

;
; Starts a new game.
;
GAMEPLAY_START:	
	mov RA, #0
	mov piece_type, RA
	mov game_score, RA
	mov game_score+1, RA
	mov RA, #0x60
	mov game_speed, RA
	mov gameloop_count, RA
	JSR BOARD_CLEAR
	JSR SCREEN_SCORE_INIT
	JSR OS_GET_TICKS
	mov game_last_ticks, RA
	mov game_last_ticks+1, RB
	mov game_last_ticks+2, RC
	RTS

; 
; Tries to move the piece to the left
;
; returns:
; C-Flag: 0=piece moved, 1=piece deactivated
;
GAMEPLAY_PIECE_LEFT:
	mov RA, #0
	JSR BOARD_DRAW_PIECE
	
	mov RC, #0xFF
	mov RD, #0x00
	JSR PIECE_MOVE
	JSR BOARD_CHECK_PIECE
	JNC .end
	
	mov RC, #0x01
	mov RD, #0x00
	JSR PIECE_MOVE
	mov RA, piece_type
	AND RA, #1
	INC RA
	JSR BOARD_DRAW_PIECE
	
	JSR GAMEPLAY_DELETE_COMPLETED_COLUMNS
	
	mov RA, #0
	mov piece_type, RA
.end:	
	RTS
	
;
; Deletes the completed columns from the board.
;	
GAMEPLAY_DELETE_COMPLETED_COLUMNS:		
	mov RB, #0
.checkloop:	
	PSH RB
	JSR BOARD_CHECK_COLUMN	
	PUL RB
	JNC .nextcol
	PSH RB
	JSR BOARD_DELETE_COLUMN
	JSR GAMEPLAY_SCORE_INCREMENT
	PUL RB
	JMP .checkloop
	
.nextcol:
	CLC
	ADD RB, #1
	STC 
	CMP RB, #BOARD_COLS
	JNC .checkloop
	
	RTS
	
;
; adds one to the current score on the screen.
;
GAMEPLAY_SCORE_INCREMENT:
	JSR SCREEN_SCORE_INCREMENT
	CLC
	mov RA, game_score
	ADD RA, #1
	mov game_score, RA	
	mov RA, game_score+1
	ADD RA, #0
	mov game_score+1, RA
	
	mov RA, game_score
	AND RA, #7
	JNZ .end
	mov RA, game_speed
	STC
	CMP RA, #0x11
	JNC .end
	STC
	SUB RA, #6
	mov game_speed, RA
.end:	
	RTS
	
;
; Calculates a random number for the piece type.
;
; returns:
;  RA - piece number (1-7)
;	
GAMEPLAY_GET_RANDOM_PIECE:
	JSR RANDOM_NUMBER
	AND RA, #7
	JZS GAMEPLAY_GET_RANDOM_PIECE
	RTS
	
;
; waits a few cycles
;	
GAMEPLAY_DELAY:
	mov RA, game_last_ticks
	mov RB, game_last_ticks+1
	mov RC, game_last_ticks+2
	CLC
	ADD RA, #2
	ICC RB
	ICC RC
	JSR OS_SLEEP
	mov game_last_ticks, RA
	mov game_last_ticks+1, RB
	mov game_last_ticks+2, RC
	RTS

;
; calculates the game state changes after a key stroke.
;
; Parms:
;  RA - key (ascii code)
;
GAMEPLAY_KEY_PRESSED:
	PSH RA
	mov RA, #0
	JSR BOARD_DRAW_PIECE
	PUL RA

	; ROTATE
	STC
	CMP RA, #"d" ;'d'
	JNZ .noD
	JSR PIECE_ROTATE
	JSR BOARD_CHECK_PIECE
	JNC .end
	JSR PIECE_ROTATE_BACK
	JMP .end
	
	; MOVE LEFT
.noD:
	mov RC, #0
	mov RD, RC
	
	STC
	CMP RA, #"a" ;'a'
	JNZ .noA
	mov RC, #0xFF ; dcol=-1	
	
	; MOVE UP
.noA:
	STC
	CMP RA, #"w" ;'w'
	JNZ .noW
	mov RD, #0xFF ; drow=-1
	
	; MOVE DOWN
.noW:
	STC
	CMP RA, #"s" ;'s'
	JNZ .noS
	mov RD, #0x01 ; drow=1
	
	; Check for valid move
.noS:
	PSH RC
	PSH RD
	JSR PIECE_MOVE
	JSR BOARD_CHECK_PIECE
	PUL RD
	PUL RC
	JNC .end
	
	; RC=-RC
	mov RA, #0
	STC
	SUB RA, RC
	mov RC, RA
	; RD=-RD
	mov RA, #0
	STC
	SUB RA, RD
	mov RD, RA
	; MOVE back to old position
	JSR PIECE_MOVE
.end:	
	RTS

;
; adds one to the current score on the screen.
;
SCREEN_SCORE_INCREMENT:
	mov RB, #SCREEN_SCORE_COL+3*SCREEN_COLS+1
.loop:	
	mov RA, screen,RB
	STC
	CMP RA, #" " ; ' '
	JNZ .nospace
	mov RA, #"0" ; '0'
.nospace:	
	CLC
	ADD RA, #1
	mov screen,RB, RA
	STC
	CMP RA, #"9"+1
	JNC .end
	mov RA, #"0" ; '0'
	mov screen,RB, RA
	STC
	SUB RB, #SCREEN_COLS
	JCS .loop
.end:
	RTS
	
;
; Initializes the score display.
;	
SCREEN_SCORE_INIT:
	mov RA, #"C" ; 'C'
	mov screen+SCREEN_SCORE_COL, RA	
	mov RA, #"o" ; 'o'
	mov screen+SCREEN_SCORE_COL+SCREEN_COLS, RA	
	mov RA, #"l" ; 'l'
	mov screen+SCREEN_SCORE_COL+2*SCREEN_COLS, RA	
	mov RA, #"s" ; 's'
	mov screen+SCREEN_SCORE_COL+3*SCREEN_COLS, RA	
	mov RA, #" " ; ' '
	mov screen+SCREEN_SCORE_COL+1, RA	
	mov screen+SCREEN_SCORE_COL+SCREEN_COLS+1, RA	
	mov screen+SCREEN_SCORE_COL+2*SCREEN_COLS+1, RA	
	mov RA, #"0" ; '0'
	mov screen+SCREEN_SCORE_COL+3*SCREEN_COLS+1, RA	
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

	mov RB, #SCREEN_ROWS*SCREEN_COLS
	mov RA, #" " ; Space character
	STC
.clearloop:
	mov screen,RB, RA
	SUB RB, #1
	JCS .clearloop
	
	RTS

;
; Puts the current screen data on the display.
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
;
; Creates all screen data of the current board data.
;
; two lines of the board become one line on the screen
;	
SCREEN_GENERATE:
	mov RD, #0 ; board position
	mov RC, #0 ; screen position
	
.genloop:	
	mov RB, RD
	; lower line of the board
	mov RA,board+BOARD_COLS, RB
	
	; RD = 3*RA
	mov RD, RA
	CLC
	LSL RD
	ADD RD, RA
	
	; upper line of the board
	mov RA, board,RB
	ADD RA, RD	
	
	mov RD, RB ; save board position
	
	SUB RA, #0 ; nocarry: RA=RA-1
	JCS .graphchar
	mov RA, #32 ; empty cell = space
	
.graphchar:	
	mov RB, RC ; restore screen position
	mov screen,RB, RA
	CLC
	ADD RC, #1 ; advance screen position
	
	ADD RD, #1 ; advance board position
	mov RA, RD
	AND RA, #0x0F ; check for multiple of #BOARD_COLS
	JNZ .genloop
	
	CLC
	ADD RC, #SCREEN_COLS-BOARD_COLS ; skip to next screen line
	ADD RD, #BOARD_COLS ; skip lower line on board
	STC
	CMP RD, #BOARD_COLS*BOARD_ROWS
	JCS .graphend
	JNZ .genloop
.graphend:
	; insert score here
.end:	
	RTS
	
;
; Resets the board to all empty spaces.
;	
BOARD_CLEAR:
	mov RB, #BOARD_ROWS * BOARD_COLS - 1
	mov RA, #0
	STC
.loop:
	mov board,RB, RA
	SUB RB, #1
	JCS .loop
	RTS

;
; Deletes one column from the board
; by shifting the following colums to
; the left.
; 
; parms:
; RB - column number (0-15)
;
BOARD_DELETE_COLUMN:
.colshiftloop:
	STC 
	CMP RB, #BOARD_COLS-1
	JCS .empty
.rowshiftloop:	
	mov RA, board+1,RB
	mov board,RB, RA
	ADD RB, #BOARD_COLS
	STC
	CMP RB, #BOARD_ROWS*BOARD_COLS-1
	JNC .rowshiftloop
	SUB RB, #BOARD_ROWS*BOARD_COLS-1
	JCS .colshiftloop

.empty:
    mov RB, #BOARD_COLS-1	
	mov RA, #0
.emptyloop:	
	mov board,RB, RA
	CLC
	ADD RB, #BOARD_COLS
	STC
	CMP RB, #BOARD_ROWS*BOARD_COLS-1
	JNC .emptyloop
	
    RTS

;
; Checks one column for having all rows
; occupied.
; 
; parms:
; RB - column number (0-15)
; returns:
; C-Flag 0=empty blocks, 1=no empty blocks
;
BOARD_CHECK_COLUMN:
.rowloop:	
	mov RA, board,RB
	OR  RA, #0
	CLC
	JZS .end

	ADD RB, #BOARD_COLS
	STC
	CMP RB, #BOARD_ROWS*BOARD_COLS
	JNC .rowloop
.end:	
    RTS
	
;
; Check if piece can be placed on the
; board at the current position.
;
; returns:
;  C-Flag - 0=O.K., 1=doesn't fit
;	
BOARD_CHECK_PIECE:
	mov RB, #6
	
.pointloop:
	mov RA, piece_points,RB
	CLC
	ADD RA, piece_pos
	mov RC, RA
	
	mov RA, piece_points+1,RB
	CLC
	ADD RA, piece_pos+1
	mov RD, RA
	
	STC
	CMP RC, #BOARD_COLS
	JCS .end ; resulting column is invalid
	
	STC
	CMP RD, #BOARD_ROWS
	JCS .end; resulting row is invalid
	
	PSH RB
	JSR BOARD_GET_BLOCK
	PUL RB
	
	OR  RA, #0
	JZS .free ; block is not free
	STC
	RTS
	
.free:
	STC
	SUB RB, #2
	JCS .pointloop
.end:	
	RTS
		
;
; Draws the active piece on the board.
;
; parms:
;  RA - color code (0= remove piece)
;
;	
BOARD_DRAW_PIECE:
	mov RB, #6
.pointloop:
	PSH RA
	mov RA, piece_points,RB
	CLC
	ADD RA, piece_pos
	mov RC, RA
	mov RA, piece_points+1,RB
	CLC
	ADD RA, piece_pos+1
	mov RD, RA
	PUL RA
	PSH RB
	JSR BOARD_SET_BLOCK
	PUL RB
	STC
	SUB RB, #2
	JCS .pointloop
	RTS
	
;
; Sets a single block on the board.
;
; parms:
;  RA - color code (0= empty)
;  RC - x-axis (column 0 - 15)
;  RD - y-axis (row 0 - 7)
;	
BOARD_SET_BLOCK:	
	; boardpos = Y*BOARD_COLS+X
	; #fixed calculation: BOARD_COLS=16
	LSL RD
	LSL RD
	LSL RD
	LSL RD
	AND RD, #0x70
	CLC
	ADD RD, RC
	mov RB, RD
	mov board,RB, RA
	RTS
	
;
; Sets a single block on the board.
;
; parms:
;  RC - x-axis (column 0 - 15)
;  RD - y-axis (row 0 - 7)
; return:
;  RA - color code (0= empty)
;	
BOARD_GET_BLOCK:	
	; boardpos = Y*BOARD_COLS+X
	; #fixed calculation: BOARD_COLS=16
	LSL RD
	LSL RD
	LSL RD
	LSL RD
	AND RD, #0x70
	CLC
	ADD RD, RC
	mov RB, RD
	mov RA, board,RB
	RTS

;
; Rotates the piece anti-clockwise.
;
PIECE_ROTATE_BACK:
	mov RB, piece_type
	mov RA, PIECE_MAX_ROTATE-1,RB
	OR  RA, #0
	JZS .end
	
	PSH RA
	JSR PIECE_ROTATE
	PUL RA
	AND RA, #2
	JZS .end
	
	JSR PIECE_ROTATE
	JSR PIECE_ROTATE
	
.end:	
	RTS

;
; Rotates the piece clockwise.
;
PIECE_ROTATE:
	mov RB, piece_type
	mov RA, PIECE_MAX_ROTATE-1,RB
	
	mov RC, piece_rotate
	CLC
	ADD RC, #1
	AND RC, RA
	mov piece_rotate, RC
	JZS PIECE_INIT_POINTS

	mov RB, #6
.loop:
	; x=-y; y=x
	mov RA, piece_points,RB
	mov RC, RA
	mov RA, piece_points+1,RB
	mov RD, RA
	mov RA, RC
	mov piece_points+1,RB, RA
	mov RA, #0
	STC
	SUB RA, RD
	mov piece_points,RB, RA
	
	STC
	SUB RB, #2
	JCS .loop
	
	RTS

;
; Changes the position of the piece relatively.
;
; parms:
; RC - change in x-axis
; RD - change in y-axis
;
PIECE_MOVE:
	CLC
	ADD RC, piece_pos  
	mov piece_pos, RC
	CLC
	ADD RD, piece_pos+1
	mov piece_pos+1, RD
	RTS
	
;
; Initializes the active piece with
; the data according to its type.
;
; Parms:
;  RA - piece type (1-7)
;	
PIECE_INIT_NEW:
	mov piece_type, RA
	JSR PIECE_INIT_POINTS
	mov RA, #0
	mov piece_rotate, RA
	mov RA, #BOARD_COLS-2
	mov piece_pos, RA
	mov RA, #BOARD_ROWS / 2
	mov piece_pos+1, RA
	RTS
	
;
; Copy the initial points to the piece.
;	
PIECE_INIT_POINTS:
	mov RA, piece_type
	CLC
	LSL RA
	LSL RA
	LSL RA
	mva RCD, PIECE_POINTS-8
	ADD RC, RA
	ADD RD, #0
	
	mov RB, #0
.pointloop:
	mov RA, [RCD]
	mov piece_points,RB, RA
	CLC
	ADD RC, #1
	ADD RD, #0
	ADD RB, #1
	STC
	CMP RB, #8
	JNC .pointloop
	RTS
	
;
; Sets new Random seed
;	
NEW_RANDOM_SEED:
	mva RCD, timer_data
	JSR OS_GET_TIME
	mov RA, timer_data
	mov RB, timer_data+1
	mov RC, timer_data+2
	mov RD, timer_data+3
	JSR RANDOM_SET_SEED
	RTS

#include "LCDGraph.inc"
#include "Random.inc"
#bank pgm
#include "cursor.inc"
#bank pgm
#include "streamio.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00	

start_msg: #d "hit key to start\0"
pause_msg: #d " Paused \0"
gameover_msg: #d " Game Over \0"

PIECE_POINTS: 
; I-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 -2, 0
	#d8 1, 0
; L-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 1, 0
	#d8 -1, 1
; J-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 1, 0
	#d8 1, 1
; Z-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 0, 1
	#d8 1, 1
; S-Type
	#d8 0, 0
	#d8 -1, 1
	#d8 1, 0
	#d8 0, 1
; O-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 0, 1
	#d8 -1, 1
; T-Type
	#d8 0, 0
	#d8 -1, 0
	#d8 1, 0
	#d8 0, 1

PIECE_MAX_ROTATE:
    #d8 1,3,3,1,1,0,3

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

parm_ptr: #res 2
game_speed: #res 1
game_last_ticks: #res 3
game_score: #res 2
gameloop_count: #res 1
timer_data: #res 7
piece:
piece_type: #res 1
piece_points: #res 8
piece_rotate: #res 1
piece_pos: #res 2

board: #res BOARD_ROWS * BOARD_COLS
screen: #res SCREEN_ROWS*SCREEN_COLS
DATA_END: