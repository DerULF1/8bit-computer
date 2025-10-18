#bank pgm

BLOCK_MOVE_FROM = 0
BLOCK_MOVE_TO = 2
BLOCK_MOVE_SIZE = 4

;
; Moves a block of bytes.
; 
; parm:
;  RCD - pointer to 6-byte block move structure:
;        block_move_from 16bit from address
;        block_move_to 16bit to address
;        block_move_cnt 16bit number of bytes to move
;	
BLOCK_MOVE:
	CLC
	mov RA, [RCD]
	mov block_move_from, RA
	INC RCD
	mov RA, [RCD]
	mov block_move_from+1, RA
	
	INC RCD
	mov RA, [RCD]
	mov block_move_to, RA
	INC RCD
	mov RA, [RCD]
	mov block_move_to+1, RA
	
	; byte count into RCD
	INC RCD
	mov RB, [RCD]
	INC RCD
	mov RD, [RCD]
	mov RC, RB
	
	mov RB, #0
.move_loop:
	DEC RCD
	JNC .end
	mov RA, [block_move_from],RB
	mov [block_move_to],RB, RA
	INC RB
	JNC .move_loop
	INC block_move_from+1
	INC block_move_to+1
	JMP .move_loop	
.end:	
	RTS

#bank data

block_move_from: #res 2
block_move_to: #res 2
