#include "8bit.cpu"

#include "OSCalls.inc"
#include "KeyNames.inc"

;
; EDIT - simple text editor
;
; Written 23.05.2021 Ulf Caspers
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

LSTORE_PAGES=4
SCREEN_ROWS=4
SCREEN_COLS=20

#bank data
DATA_START:

#bank pgm
;
; RC/RD - parameter string
;
START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD	
	
.load:
	mov RA, #1
	JSR GET_PARAMETER
	JCS .end
	
	JSR LOAD
	JNC .load_OK
	PSH RA
	mov RA, #100
	JSR .fail
	PUL RA
	JMP PUT_HEX_BYTE
	
.load_OK:
	mov RA, #0xff
	mov edit_line_nr, RA
	mov edit_line_nr+1, RA

.show:
	mov RA, #0
	mov view_row, RA
	mov view_row+1, RA
	mov view_col, RA
	mov csr_row, RA
	mov csr_col, RA
	mov edit_overwrite, RA
	
	JSR EDIT_MODE
	JCS .end
	
	mov RA, #1
	JSR GET_PARAMETER
	JSR SAVE
	
.end:	
	MOV RA, #0
	RTS

.fail:
	JSR PUT_HEX_BYTE
	mva RCD, fail_msg
	JSR PUT_STRING
	
	MOV RA, #2
	RTS

;
; Program loop in edit mode
;
EDIT_MODE:	
	mov RA, #CURSOR_WRAP
	JSR CURSOR_SCROLL_MODE
	
	JSR SET_CURSOR_TYPE
	
.sloop:
	JSR SHOW_STORE

.cloop:	
	JSR PUT_CURSOR

	JSR GET_CHAR
	
	STC
	CMP RA, #KEY_CURSOR_DOWN
	JNZ .no_down
	JSR MOVE_DOWN
	JMP .cloop
	
.no_down:	
	STC
	CMP RA, #KEY_CURSOR_UP
	JNZ .no_up
	JSR MOVE_UP
	JMP .cloop
	
.no_up:
	STC
	CMP RA, #KEY_CURSOR_RIGHT
	JNZ .no_right
	JSR MOVE_RIGHT
	JMP .cloop
	
.no_right:
	STC
	CMP RA, #KEY_CURSOR_LEFT
	JNZ .no_left
	JSR MOVE_LEFT
	JMP .cloop		

.no_left:
	STC
	CMP RA, #KEY_PAGE_UP
	JNZ .no_page_up
	JSR MOVE_PAGE_UP
	JMP .cloop
	
.no_page_up:	
	STC
	CMP RA, #KEY_PAGE_DOWN
	JNZ .no_page_down
	JSR MOVE_PAGE_DOWN
	JMP .cloop
	
.no_page_down:
	STC
	CMP RA, #KEY_HOME
	JNZ .no_home
	JSR MOVE_HOME
	JMP .cloop
		
.no_home:	
	STC
	CMP RA, #KEY_HOME + 16
	JNZ .no_chome
	JSR MOVE_TOP
	JMP .cloop
		
.no_chome:	
	STC
	CMP RA, #KEY_END
	JNZ .no_end
	JSR MOVE_END
	JMP .cloop
	
.no_end:
	STC
	CMP RA, #KEY_END + 16
	JNZ .no_cend
	JSR MOVE_BOTTOM
	JMP .cloop
	
.no_cend:
	STC
	CMP RA, #KEY_INSERT
	JNZ .noins	

	; toggle overwrite flag
	mov RA, edit_overwrite
	XOR RA, #1
	mov edit_overwrite, RA
	JSR SET_CURSOR_TYPE
	
	JMP .cloop

.noins:
	STC
	CMP RA, #KEY_DELETE
	JNZ .nodel

	JSR DELETE_CHAR
	
	JMP .cloop
	
.nodel:
	STC
	CMP RA, #KEY_BACKSPACE
	JNZ .nobackspace

	JSR MOVE_LEFT
	JSR DELETE_CHAR
	
	JMP .cloop
	
.nobackspace:
	STC
	CMP RA, #KEY_ENTER
	JNZ .noenter

	JSR EDIT_SPLIT_LINE
	
	JMP .sloop
	
.noenter:
	STC
	CMP RA, #KEY_ESC
	JZS .end
	
	; filter out other non-printable characters
	STC
	CMP RA, #" "
	JNC .cloop ; too low
	CMP RA, #128
	JCS .cloop ; too high

	JSR EDIT_CHAR
	JMP .cloop
	
.end:
	JSR STORE_EDIT_LINE
	
	mov RA, #CURSOR_SCROLL
	JSR CURSOR_SCROLL_MODE	

	RTS

;
; moves cursor one position to the left
;	
MOVE_LEFT:
	; move cursor
	DEC csr_col
	JCS .csr_only ; already in 1 column?
	
	INC csr_col ; undo cursor move
	
	; move view port
	DEC view_col
	JCS SHOW_STORE ; already at beginning?
	
	; undo viewport move
	INC view_col
	
	; in top row?
	mov RA, view_row
	OR  RA, view_row+1
	OR  RA, csr_row
	JZS .csr_only
	
	JSR MOVE_UP
	JSR MOVE_END
.csr_only:
	RTS
	
;
; moves cursor one position to the right
;	
MOVE_RIGHT:
	mov RA, csr_col
	STC ; add one more column
	ADD RA, view_col
	JCS .csr_only; never get above column 255
	
	mov RA, csr_col
	INC RA
	STC	
	CMP RA, #SCREEN_COLS	
	JCS .move_right
	mov csr_col, RA
.csr_only:
	RTS
.move_right:
	INC view_col
	JMP SHOW_STORE
	
;
; moves cursor to first column in line
;
MOVE_HOME:
	mov RA, #0
	mov csr_col, RA
	mov RB, view_col
	OR  RB, #0
	JZS .csr_only
	mov view_col, RA
	JMP SHOW_STORE
.csr_only:
	RTS
	
;
; moves cursor to last column in line
;
MOVE_END:
	mov RB, csr_row
	mov RA, view_len_tab,RB
	STC
	SUB RA, view_col
	JNC .move_view
	CMP RA, #SCREEN_COLS
	JCS .move_view
	mov csr_col, RA
	RTS
.move_view:	
	mov RA, view_len_tab,RB
	STC
	CMP RA, #SCREEN_COLS
	JCS .long_line
	mov csr_col, RA
	mov RA, #0
	mov view_col, RA
	JMP SHOW_STORE
.long_line:
	SUB RA, #SCREEN_COLS/2
	mov view_col, RA
	mov RA, #SCREEN_COLS - (SCREEN_COLS/2)
	mov csr_col, RA
	JMP SHOW_STORE

;
; moves cursor down one line
;
MOVE_DOWN:
	JSR STORE_EDIT_LINE
	JCS .end
	; try to move cursor only
	mov RA, csr_row
	INC RA
	STC
	CMP RA, #SCREEN_ROWS
	JCS .move_down
	mov csr_row, RA
	RTS
.move_down:
	; move view port
	INC view_row
	ICC view_row+1
	mov RA, view_row
	mov RB, view_row+1
	ADD RA, csr_row
	ICC RB
	JSR LSTORE_GOTO_LINE_NR
	JNC SHOW_STORE
	; end of lines
	DEC view_row
	DCC view_row+1
.end:	
	RTS

;
; moves one page up
;
MOVE_PAGE_UP:
	JSR STORE_EDIT_LINE
	JCS .end
	mov RA, view_row
	mov RB, view_row+1
	STC
	SUB RA, #4
	SUB RB, #0
	JNC MOVE_TOP
	mov view_row, RA
	mov view_row+1, RB
	JMP SHOW_STORE
.end:
	RTS

;
; moves one page down
;
MOVE_PAGE_DOWN:
	JSR STORE_EDIT_LINE
	JCS .end
	mov RA, view_row
	mov RB, view_row+1
	ADD RA, #4
	ADD RB, #0
	mov view_row, RA
	mov view_row+1, RB
	JSR LSTORE_GOTO_LINE_NR
	JCS MOVE_BOTTOM
	JMP SHOW_STORE
.end:
	RTS

;
; moves to first row of file
;
MOVE_TOP:
	JSR STORE_EDIT_LINE
	JCS .end
	mov RA, #0
	mov view_row, RA
	mov view_row+1, RA
	mov csr_row, RA
	mov csr_col, RA
	JMP SHOW_STORE
.end:
	RTS

;
; moves to last row of file
;
MOVE_BOTTOM:
	JSR STORE_EDIT_LINE
	JCS .end
	JSR LSTORE_LINE_BOTTOM
	JSR LSTORE_GET_LINE_NR
	STC
	SUB RA, #SCREEN_ROWS
	SUB RB, #0
	JNC .small_file
	mov view_row, RA
	mov view_row+1, RB
	mov RA, #SCREEN_ROWS-1
	JMP .put_csr
.small_file:
	mov RA, #0
	mov view_row, RA
	mov view_row+1, RA
	JSR LSTORE_GET_LINE_NR
.put_csr:
	mov csr_row, RA
	mov RA, #0
	mov csr_col, RA
	JMP SHOW_STORE
.end:	
	RTS
	
;
; moves cursor up one line
;
MOVE_UP:
	JSR STORE_EDIT_LINE
	JCS .end
	; try to move cursor only
	DEC csr_row
	JCS .end
	
	INC csr_row ; undo cursor move
	; move view_port
	DEC view_row
	DCC view_row+1
	JCS SHOW_STORE
	; already on top - undo move
	INC view_row
	ICC view_row+1
.end:	
	RTS

;
; joins current line with following line
;
EDIT_JOIN_LINE:
	JSR STORE_EDIT_LINE
	JCS .end

	mov RA, view_row
	mov RB, view_row+1
	CLC
	ADD RA, csr_row
	ICC RB
	JSR LSTORE_GOTO_LINE_NR
	
	mva RCD, line_area
	JSR LSTORE_GET_LINE ; read first line
	JCS .bottom
	
	JSR LSTORE_DEL_LINE ; delete first line
	
	mva RCD, line_area ; find end of first line
	CLC
	ADD RC, line_len
	ICC RD
	
	mov RA, [RCD] ; save last character of first line
	PSH RA
	PSH RC
	PSH RD
	JSR LSTORE_GET_LINE ; append second line
	PUL RD
	PUL RC
	PUL RB
	
	mov RA, [RCD] ; second line length
	PSH RA
	mov RA, RB ; last char of first line 
	mov [RCD], RA
	PUL RA ; restore second line length
	JCS .store_line
	
	ADD RA, line_len ; add first line length
	JCS .store_line ; line too long
	mov line_len, RA ; store new line length
	
	JSR LSTORE_DEL_LINE ; delete second line
	
.store_line:
	mva RCD, line_area
	JSR LSTORE_INS_LINE
	
.bottom:
	CLC
.end:
	RTS

;
; splits the current line at the curso position
;
EDIT_SPLIT_LINE:
	JSR STORE_EDIT_LINE
	JCS .end
	
	mov RA, view_row
	mov RB, view_row+1
	CLC
	ADD RA, csr_row
	ICC RB
	JSR LSTORE_GOTO_LINE_NR
	mva RCD, line_area
	JSR LSTORE_GET_LINE
	JCS .bottom
	
	JSR LSTORE_DEL_LINE
	JMP .goon
.bottom:
	mov RA, #0
	mov line_len, RA
.goon:
	mov RB, line_len
	mov RA, view_col
	CLC
	ADD RA, csr_col
	STC
	CMP RA, RB
	JNC .goon2
	mov RA, RB
.goon2:
	mov line_len, RA
	STC
	SUB RB, line_len
	PSH RB
	mva RCD, line_area
	JSR LSTORE_INS_LINE
	PUL RA
	JCS .end
	
	PSH RA
	JSR LSTORE_LINE_DOWN
	PUL RA
	JNC .not_bottom

	OR  RA, #0
	JZS .end

.not_bottom:
	mov RB, line_len
	mva RCD, line_area
	CLC
	ADD RC, RB
	ICC RD
	mov [RCD], RA
	JSR LSTORE_INS_LINE
	
	JSR MOVE_DOWN
	JSR MOVE_HOME
	CLC
.end:	
	RTS

;
; deletes a character from the edit line
;
DELETE_CHAR:
	JSR ENSURE_EDIT_LINE
	mov RB, view_col
	CLC
	ADD RB, csr_col
	STC
	CMP RB, edit_line_len
	JNC .dloop
	JSR EDIT_JOIN_LINE
	JMP SHOW_STORE
.dloop:
	mov RA, edit_line_data+1,RB
	mov edit_line_data,RB, RA
	INC RB
	STC
	CMP RB, edit_line_len
	JNC .dloop
	
.dend:
	DEC edit_line_len
	JMP SHOW_EDIT_LINE

;
; starts edit line if not already established
;
; Parm:
;  RA - typed character
;
EDIT_CHAR:
	PSH RA
	JSR ENSURE_EDIT_LINE
	mov RC, view_col
	CLC
	ADD RC, csr_col
	CLC ; compare less or equal
	CMP RC, edit_line_len
	JNC .len_OK
	
	; fill gap with spaces
	mov RB, edit_line_len
	mov RA, #" "
.floop:	
	mov edit_line_data,RB, RA
	INC RB
	STC
	CMP RB, RC
	JNC .floop
	mov edit_line_len, RC
	
.len_OK:
	mov RB, edit_overwrite
	OR  RB, #0
	JZS .insert
	STC
	CMP RC, edit_line_len
	JNZ .write_char
.insert:
	mov RB, edit_line_len
	STC
	CMP RB, #0xff
	JNZ .iloop
	PUL RA
	RTS
.iloop:
	CLC ; compare less or equal
	CMP RB, RC
	JNC .iloop_end
	mov RA, edit_line_data-1,RB
	mov edit_line_data,RB, RA
	DEC RB
	JNZ .iloop
.iloop_end:
	INC edit_line_len
.write_char:	
	PUL RA
	mov RB, RC
	mov edit_line_data,RB, RA
	
	; cursor one position to the right
	mov RA, csr_col
	INC RA
	STC	
	CMP RA, #SCREEN_COLS	
	JCS .move_right
	mov csr_col, RA
	JMP SHOW_EDIT_LINE
.move_right:
	INC view_col
	JMP SHOW_STORE

;
; starts edit line if not already established
;
ENSURE_EDIT_LINE:
	JSR HAS_EDIT_LINE
	JNC .end
	JSR START_EDIT_LINE
.end:	
	RTS

;
; starts edit line
;
START_EDIT_LINE:
	mov RA, view_row
	CLC
	ADD RA, csr_row
	mov RB, view_row+1
	ICC RB
	mov edit_line_nr, RA
	mov edit_line_nr+1, RB
	
	JSR LSTORE_GOTO_LINE_NR
	JCS .empty_line
	
	mva RCD, edit_line_area
	JMP LSTORE_GET_LINE
	
.empty_line:
	mov RA, #0
	mov edit_line_len, RA
	RTS

;
; checks if edit line is already established
;
; return:
;  c-flag: 0 = edit established, 1 = no edit line
;  RA/RB - edit line number
;
HAS_EDIT_LINE:
	mov RA, edit_line_nr
	mov RB, edit_line_nr+1
	STC
	CMP RA, #0xff
	JNZ .end
	STC
	CMP RB, #0xff
.end:	
	RTS

;
; stores edit line and ends edit
;
STORE_EDIT_LINE:
	JSR HAS_EDIT_LINE
	JCS .end_OK

.goto_line:
	JSR LSTORE_GOTO_LINE_NR
	JCS .insert_line
	
	JSR LSTORE_DEL_LINE
	
.insert_line:
	mva RCD, edit_line_area
	JSR LSTORE_INS_LINE
	JCS .end
	
	mov RA, edit_line_nr
	mov RB, edit_line_nr+1
	STC
	SUB RA, view_row
	SUB RB, view_row+1
	JNZ .not_visible
	CMP RA, #SCREEN_ROWS
	JCS .not_visible
	
	mov RB, RA
	mov RA, line_len
	mov view_len_tab,RB, RA
	
.not_visible:	
	mov RA, #0xff
	mov edit_line_nr, RA
	mov edit_line_nr+1, RA
.end_OK:
	CLC
.end:	
	RTS

;
; saves line storage to file
;
; Parm:
;  RCD - pointer to file name
; Return:
;  c-flag - 0=O.K., 1=error
;
SAVE:
	mov RA, #O_WRITE + O_CREATE
	JSR OS_OPEN
	JCS .end
	
	mov file_number, RA

	JSR LSTORE_LINE_TOP
	
	mva RCD, line_area+1 ; skip length byte
	mov write_pointer, RC
	mov write_pointer+1, RD
	
	mov RA, #0 ; all lines are < 255 characters
	mov write_size+1, RA
	
.sloop:	
	mva RCD, line_area
	JSR LSTORE_GET_LINE
	JCS .save_end
	
	mov RB, line_area ; length of line
	
	; add line feed
	INC RB
	mov RA, #10 
	mov line_area,RB, RA
	
	mov write_size, RB
	
	mov RA, file_number
	mva RCD, write_area
	JSR OS_WRITE
	JCS .end
	
	JSR LSTORE_LINE_DOWN
	JNC .sloop
	
.save_end:

.close:
	mov RA, file_number
	JSR OS_CLOSE
	
.end:
	RTS

;
; loads line storage from file
;
; Parm:
;  RCD - pointer to file name
; Return:
;  c-flag - 0=O.K., 1=error
;
LOAD:
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .end
	
	mov file_number, RA
	
	mva RCD, dir_entry_area
	JSR OS_STAT
	JCS .end
	
	; filesize rounded up to next 512 byte block
	mov RC, dir_entry_area+FAT_DIRENT_FILESIZE+1
	ADD RC, #2
	CLC
	LSR RC
	INC RC ; and 512 byte extra for expansion
	
	PSH RC ; save number of 512 byte blocks

	; allocate memory 
	JSR OS_MEM_ALLOC
	JCS .end
	
	PUL RB ; restore number of 512 byte blocks
	LSL RB ; convert to number of 256 byte pages
	
	mov RC, #0 ; low byte of start address
	JSR LSTORE_INIT

.lloop:	
	mov RB, #0
	
.rloop:	
	PSH RB
	mov RA, file_number
	JSR OS_GETC
	PUL RB
	JCS .error
	
	STC ; ignore carriage return
	CMP RA, #13
	JZS .rloop
	
	STC ; line feed = end of line
	CMP RA, #10
	JZS .end_of_line 
	
	INC RB
	mov line_area,RB, RA
	JMP .rloop
	
.end_of_line:
	mov line_area, RB ; set length of line
	mva RCD, line_area
	JSR LSTORE_INS_LINE
	JCS .out_of_memory
	
	JSR LSTORE_LINE_DOWN
	
	JMP .lloop
	
.out_of_memory:	
	mov RA, #ENOMEM
	RTS
		
.error:
	CMP RA, #EEOF
	JZS .end_of_file
	STC
	RTS
	
.end_of_file:
	OR  RB, #0
	JZS .close
	mov line_area, RB
	mva RCD, line_area
	JSR LSTORE_INS_LINE
	JCS .out_of_memory	

.close:
	JSR LSTORE_LINE_TOP

	mov RA, file_number
	JSR OS_CLOSE
	
.end:
	RTS

;
; set cursor shape
;
SET_CURSOR_TYPE:
	mov RA, edit_overwrite
	OR  RA, #0
	JZS .ins_cursor
	mov RA, #CURSOR_TYPE_LINE
	JMP .set_type
.ins_cursor:	
	mov RA, #CURSOR_TYPE_BLINK
.set_type:	
	JMP CURSOR_MODE
	
;
; writes current view port on to console
;
SHOW_STORE:
	mov RA, #0 ; view row counter
.lloop:
	PSH RA
	mov RB, #0
	JSR CURSOR_GOTO_POSITION

	PUL RA
	PSH RA
	CLC
	ADD RA, view_row
	mov RB, view_row+1
	ICC RB
	
	STC
	CMP RA, edit_line_nr
	JNZ .show_store_line
	CMP RB, edit_line_nr+1
	JNZ .show_store_line
	
.show_edit_line:
	JSR SHOW_EDIT_LINE2
	JMP .next_row
	
.show_store_line:
	JSR LSTORE_GOTO_LINE_NR
	JCS .blank_line
	
	mva RCD, line_area
	JSR LSTORE_GET_LINE
	JNC .show_line

.blank_line:
	mov RA, #0
	mov line_len, RA
	
.show_line:	
	PUL RB ; restore row counter
	PSH RB
	mov RA, line_len
	mov view_len_tab,RB, RA
	
	mov RB, view_col
	mov RC, #SCREEN_COLS
.cloop:	
	mov RA, #" "
	STC
	CMP RB, line_len
	JCS .show_char
	mov RA, line_data,RB
.show_char:
	JSR PUT_CHAR
	INC RB
	DEC RC
	JNZ .cloop
.next_row:	
	PUL RA
	INC RA
	STC
	CMP RA, #SCREEN_ROWS
	JNC .lloop
	
	RTS

;
; writes the current state of
; the edit line to the screen
;
SHOW_EDIT_LINE:
	mov RA, edit_line_nr
	STC
	SUB RA, view_row
	mov RB, #0
	JSR CURSOR_GOTO_POSITION

	; fall through to SHOW_EDIT_LINE2
	
;
; writes the current state of
; the edit line to the screen
; with the cursor already positioned
;
SHOW_EDIT_LINE2:
	mov RB, view_col
	mov RC, #SCREEN_COLS
.cloop:	
	mov RA, #" "
	STC
	CMP RB, edit_line_len
	JCS .show_char
	mov RA, edit_line_data,RB
.show_char:
	JSR PUT_CHAR
	INC RB
	DEC RC
	JNZ .cloop
	RTS

;
; places cursor on console
;
PUT_CURSOR:
	mov RA, csr_row
	mov RB, csr_col
	JMP CURSOR_GOTO_POSITION

#include "LineStor.inc"
#bank pgm
#include "param.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "cursor.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00
	
fail_msg: #d " fail\n\0"

END_PGM:

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data

file_number: #res 1
dir_entry_area: #res 32

write_area:
write_size: #res 2
write_pointer: #res 2

line_area:
line_len: #res 1
line_data: #res 255

edit_line_area:
edit_line_len: #res 1
edit_line_data: #res 255

edit_line_nr: #res 2

edit_overwrite: #res 1

view_row: #res 2
view_col: #res 1

csr_row: #res 1
csr_col: #res 1

view_len_tab: #res SCREEN_ROWS

DATA_END:
