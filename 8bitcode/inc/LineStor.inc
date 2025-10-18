;
; simple line storage
;

;
; Sets pointer to data area
;
; Parm:
;  RB  - size of data area in 256 bytes
;  RCD - pointer to data area
; 
LSTORE_INIT:
	mov lstor_start, RC
	mov lstor_start+1, RD
	CLC
	ADD RD, RB
	mov lstor_end, RC
	mov lstor_end+1, RD 

	; fall-through to LSTOR_CLEAR

;
; Deletes all lines in the data area
;
LSTOR_CLEAR:
	mov RA, #0
	mov RB, RA
	mov [lstor_start],RB, RA
	INC RB
	mov [lstor_start],RB, RA
	mov lstor_lines, RA
	mov lstor_lines+1, RA
	mov RA, lstor_start
	mov RB, lstor_start+1
	ADD RA, #2
	ICC RB
	mov lstor_last, RA
	mov lstor_last+1, RB
	
	; fall-through to LSTOR_TOP

;
; moves current line pointer to first line
;
LSTORE_LINE_TOP:
	; pointer to first line
	mov RA, lstor_start
	mov lstor_cur, RA
	mov RA, lstor_start+1
	mov lstor_cur+1, RA

	; line number is 0
	mov RA, #0
	mov lstor_lnr, RA
	mov lstor_lnr+1, RA
	CLC
	RTS

;
; moves current line pointer behind last line
;
LSTORE_LINE_BOTTOM:
	; pointer to last line
	mov RA, lstor_last
	STC
	SUB RA, #2
	mov lstor_cur, RA
	mov RA, lstor_last+1
	SUB RA, #0
	mov lstor_cur+1, RA

	; line number is total number of lines
	mov RA, lstor_lines
	mov lstor_lnr, RA
	mov RA, lstor_lines+1
	mov lstor_lnr+1, RA
	CLC
	RTS

;
; returns the total number of lines in the store
; 
; Result:
;  RA/RB - total number of lines in the store
;
LSTORE_GET_TOTAL_LINES:
	mov RA, lstor_lines
	mov RB, lstor_lines+1
	RTS

;
; moves current line pointer up one line
; 
; moving up is going towards lower line numbers
; 
; Result:
; c-flag: 0=O.K. 1=already on top
;
LSTORE_LINE_UP:
	mov RB, #0
	mov RA, [lstor_cur],RB ; pointer to predecessor +1
	OR  RA, #0
	JZS .at_top
	CLC          ; subtract one more
	mov RB, lstor_cur
	SUB RB, RA
	mov lstor_cur, RB
	DCC lstor_cur+1
	
	; dedrement line number
	DEC lstor_lnr
	DCC lstor_lnr+1
	CLC
.end:	
	RTS
	
.at_top:
	STC
	RTS

;
; moves current line pointer down one line
; 
; moving down is going towards higher line numbers
; 
; Result:
; c-flag: 0=O.K. 1=already on tail
;
LSTORE_LINE_DOWN:
	mov RB, #1
	mov RA, [lstor_cur],RB ; pointer to successor -1
	OR  RA, #0
	JZS .at_tail
	STC          ; add one more
	mov RB, lstor_cur
	ADD RB, RA
	mov lstor_cur, RB
	ICC lstor_cur+1
	
	; increment line number
	INC lstor_lnr
	ICC lstor_lnr+1
	CLC
.end:	
	RTS
	
.at_tail:
	STC
	RTS

;
; get the current line number
;
; result:
;  RA/RB  - current line number
;  c-flag - 0 = normal, 1 = pointer at tail 
;
LSTORE_GET_LINE_NR:
	mov RB, #1
	mov RA, [lstor_cur],RB
	OR  RA, #0
	JZS .at_tail
	CLC
	JMP .end
.at_tail:	
	STC
.end:	
	mov RA, lstor_lnr
	mov RB, lstor_lnr+1
	RTS
	
;
; set the line pointer to a specific line
;
; parm:
;  RA/RB  - target line number
; result:
;  c-flag - 0 = O.K., 1 = line not found
;
LSTORE_GOTO_LINE_NR:
	mov lstor_to, RA
	mov lstor_to+1, RB
.loop:
	mov RA, lstor_lnr
	mov RB, lstor_lnr+1
	STC
	CMP RA, lstor_to
	CMP RB, lstor_to+1
	JCS .greater_equal
.move_down:	
	JSR LSTORE_LINE_DOWN
	JNC .loop
	RTS
.move_up:	
	JSR LSTORE_LINE_UP
	JNC .loop
	RTS
.greater_equal:
	JNZ .move_up
	CMP RA, lstor_to
	JNZ .move_up
	CLC
.end:
	RTS

;
; get the current line data
;
; Parm:
;  RCD - pointer to the destination area (256 bytes)
;        line has leading length byte
; result:
;  c-flag - 0 = O.K., 1 = pointer at tail 
;
LSTORE_GET_LINE:
	mov RB, #1
	mov RA, [lstor_cur],RB ; line length + 1
	OR  RA, #0
	JZS .at_tail 
	
	CLC
	ADD RC, RA ; set RCD behind end of line
	ADD RD, #0
	mov RB, RA
	
	mov RA, #0 ; place end-of-string marker
	mov [RCD], RA
.loop:
	mov RA, [lstor_cur],RB
	DEC RCD
	mov [RCD], RA
	DEC RB
	JNZ .loop
	DEC [RCD] ; adjust line length
	CLC
.end:	
	RTS
	
.at_tail:
	mov [RCD], RA ; line length = 0
	INC RCD
	mov [RCD], RA ; end-of-string marker
	DEC RCD
	STC
	RTS

;
; get length of the current line data
;
; result:
;  RA - length of the current line data
;  c-flag - 0 = O.K., 1 = pointer at tail 
;
LSTORE_GET_LINE_LEN:
	mov RB, #1
	mov RA, [lstor_cur],RB ; line length + 1
	DEC RA
	JNC .at_tail
	CLC
	RTS
.at_tail:
	mov RA, #0
	STC
	RTS

;
; Insert a new line behind the current line
;
; Parm:
;  RCD - pointer to line
;        line has leading length byte
; result:
;  c-flag - 0 = O.K., 1 = out of memory 
;
LSTORE_INS_LINE:
	; move free space for new line
	CLC
	mov RA, lstor_last
	mov RB, lstor_last+1
	mov lstor_from, RA
	mov lstor_from+1, RB
	
	ADD RA, [RCD] ; length byte
	ICC RB
	ADD RA, #2 ; predecessor and successor
	ICC RB
	mov lstor_to, RA
	mov lstor_to+1, RB
	
	; check memory
	STC
	mov RA, lstor_to
	CMP RA, lstor_end
	mov RB, lstor_to+1
	CMP RB, lstor_end+1
	JCS .end ; out of memory
	
	; save new line pointer
	PSH RC	
	PSH RD
	
	; bytes to move = last - cur -1
	CLC ; subtract one more
	mov RC, lstor_last
	mov RD, lstor_last+1
	SUB RC, lstor_cur
	SUB RD, lstor_cur+1
	
	; store new end of lines
	mov lstor_last, RA
	mov lstor_last+1, RB
	
	; increment total line count
	INC lstor_lines
	ICC lstor_lines+1
	
	; subtract 1 to count till underrun
	DEC RCD
	
	mov RB, #0
.mloop:
	DEC RB
	JCS .mhigh_ok
	DEC lstor_from+1
	DEC lstor_to+1
.mhigh_ok:
	mov RA, [lstor_from],RB
	mov [lstor_to],RB, RA
	DEC RCD
	JCS .mloop	
	
	; restore new line pointer
	PUL RD
	PUL RC
	
	mov RA, [RCD] ; length byte
	INC RA
	mov RB, RA
	INC RB 
	mov [lstor_cur],RB, RA ; predecessor length
	DEC RB
	
	; set RCD behind new line
	CLC
	ADD RC, RA
	ICC RD
	
.cloop:
	DEC RCD
	mov RA, [RCD]
	mov [lstor_cur],RB, RA
	DEC RB
	JNZ .cloop	
	
	; adjust length byte
	INC RB
	INC RA
	mov [lstor_cur],RB, RA
	
.end:	
	RTS

;
; Delete the current line
;
; result:
;  c-flag - 0 = O.K., 1 = already at tail
;
LSTORE_DEL_LINE:
	mov RB, #1
	mov RA, [lstor_cur],RB  ; line length + 1
	OR  RA, #0
	JZS .at_tail
	
	; calculate pointer after line
	mov RC, lstor_cur
	mov RD, lstor_cur+1
	INC RCD
	mov lstor_to, RC
	mov lstor_to+1, RD
	
	STC ; add one more
	ADD RC, RA ; line length + 1
	ADD RD, #0
	mov lstor_from, RC
	mov lstor_from+1, RD
	
	; number of bytes to move
	STC
	mov RB, lstor_last
	SUB RB, RC
	mov RC, RB
	mov RB, lstor_last+1
	SUB RB, RD
	mov RD, RB
	
	; subtract 1 to count till underrun
	DEC RCD
	
	; new end of data
	CLC ;subtract one more
	mov RB, lstor_last
	SUB RB, RA ; line length + 1
	mov lstor_last, RB
	DCC lstor_last+1
	
	; decrease total line count
	DEC lstor_lines
	DCC lstor_lines+1
	
	mov RB, #0
.cloop:
	mov RA, [lstor_from],RB
	mov [lstor_to],RB, RA
	INC RB
	JNC .chigh_ok
	INC lstor_from+1
	INC lstor_to+1
.chigh_ok:
	DEC RCD
	JCS .cloop
	
.end:
	RTS 
	
.at_tail:
	STC
	RTS
	
#bank data
; pointer current line
lstor_cur: # res 2
; current line number
lstor_lnr: # res 2
; pointer to first line/begin of data store
lstor_start: # res 2
; pointer to last line
lstor_last: # res 2
; last line number
lstor_lines: # res 2
; end of the data store
lstor_end: # res 2
; move pointer
lstor_from: #res 2
; move pointer
lstor_to: #res 2
