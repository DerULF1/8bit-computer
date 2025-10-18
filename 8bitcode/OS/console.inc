;
; Console Functions
;

#bankdef console_structure
{
    #addr 0x0000
}

LCD_CGRAM   = 0b01000000

; number of lines on the screen
CON_NUM_ROWS = 4
CON_NUM_COLS = 20

; marker for undefined console
CON_UNDEFD = 0xFE

; marker for unsued console
CON_UNUSED = 0xFF

;CON_ACTIVE_SAVE_AREA_END-CON_ACTIVE_SAVE_AREA
CON_SAVE_AREA_LEN=10

; console types
CON_TYPE_LCD = 0
CON_TYPE_VGA = 1

#bank console_structure
; Number of this console structure
CON_OFF_CON_NUM: #res 1
; pointer to next console structure
CON_OFF_NEXT:    #res 2
; Type of this console
CON_OFF_TYPE: #res 1
; current cursor row
CON_OFF_SAVE_AREA: #res CON_SAVE_AREA_LEN
; screen data
CON_OFF_SCREEN:  #res CON_NUM_ROWS*CON_NUM_COLS
; character data (8 characters, 8 bytes)
CON_OFF_CHARS:   #res 8*8

#bank pgm

CON_INIT:
	mov RA, #CON_UNDEFD
	mov con_visible_num, RA
	mov con_active_num, RA
	mov RA, #0
	mov con_struct_list_ptr, RA
	mov con_struct_list_ptr+1, RA
	RTS

;
; open console
;
; parm: 
;  RA - console number
;	
CON_OPEN:
	JSR CON_SET_STRUCT_PTR
	JNC .con_init
	JSR CON_CREATE
	JCS .end
.con_init:	
	mov RA, #255
	mov con_esc_status, RA	
	INC RA
	mov con_display_raw, RA
	mov RA, con_visible_num
	STC
	CMP RA, #CON_UNDEFD
	JNZ .end_init
	mov RA, con_active_num
	mov con_visible_num, RA
	
.end_init:
	JSR CON_CLEAR_SCREEN
	JSR CON_CLEAR_CHARS
	mov RA, #0x0F
	JSR CON_SET_CURSOR_TYPE
	CLC
.end:	
	RTS

;
; write to console
;
; parm: 
;  RA - console number
;  RC - byte to be written
;	
CON_OUT:
	PSH RC
	
	JSR CON_SET_STRUCT_PTR
	JNC .con_out
	PUL RC
	mov RA, #EBADF
	RTS
.con_out:
	PUL RA
	mov RB, con_esc_status
	INC RB
	JNZ CON_HANDLE_ESC
	
	STC 
	CMP RA, #27 ; ESCape character
	JNZ .no_esc
	INC con_esc_status
	JMP .end_OK
	
.no_esc:
	PSH RA
	mov RB, #CON_OFF_TYPE
	mov RA, [con_struct_ptr],RB
	STC
	CMP RA, #CON_TYPE_VGA
	PUL RA
	JZS .visible
	
	mov RB, con_visible_num
	STC
	CMP RB, con_active_num
	JZS .visible
	
	STC
	CMP RA, #8  ; backspace character
	JZS CON_BACKSPACE
	STC
	CMP RA, #10 ; new line character
	JZS CON_NEW_LINE
	STC
	CMP RA, #13 ; ignore carriage return
	JZS .end_OK
	
	mov RB, con_active_off
	CLC
	ADD RB, #CON_OFF_SCREEN
	mov [con_struct_ptr],RB, RA
	JSR CON_CURSOR_RIGHT
.end_OK:
	CLC
	RTS
	
.visible:
	mov RB, con_display_raw
	OR  RB, #0
	JZS .display_char
	JSR DISPLAY_SEND_DATA_BYTE
	CLC
	RTS
.display_char:	
	JSR DISPLAY_CHAR
OK_end:	
	CLC
	RTS

;
; read from console
;
; parm: 
;  RA - console number
;	
CON_IN:	
	JSR CON_SET_STRUCT_PTR
	JNC .con_in
	mov RA, #EBADF
	RTS
	
.con_in:
	mov RA, con_active_num
	STC
	CMP RA, con_visible_num
	JNZ .wouldblock
	
	JSR KEYBOARD_GET_KEY
	OR  RA, #0
	JZS .wouldblock
	CLC
	RTS
	
.wouldblock:
	mov RA, #EAGAIN
	STC
	RTS

;
; close console
;
; parm: 
;  RA - console number
;	
CON_CLOSE:
	JSR CON_SET_STRUCT_PTR
	JNC .con_close
	mov RA, #EBADF
	RTS
.con_close:
	mov RA, #CON_UNUSED
	mov RB, #CON_OFF_CON_NUM
	mov [con_struct_ptr],RB, RA
	RTS
	
CON_HANDLE_ESC:
	DEC RB
	JNZ .stage2
	STC
	CMP RA, #"["
	JZS .enter_stage2
	DEC con_esc_status
	CLC
	RTS
.enter_stage2:
	mov RA, #0
	mov con_esc_parm1, RA
	mov con_esc_parm2, RA
.next_stage:	
	INC con_esc_status
	RTS
.stage2:
	STC
	CMP RA, #";"
	JZS .next_stage
	JCS .esc_cmd ; greater than ; =>letter
	; less than ; => number
	AND RA, #15
	mov RD, RA ; new value 0-9
	; current parameter value by 10
	mov RA, con_esc_parm1-1,RB
	CLC
	LSL RA ; *2
	mov RC, RA
	LSL RA ; *4
	LSL RA ; *8
	ADD RA, RC ; (*8+*2)=*10
	; add new value
	ADD RA, RD
	; store new parameter value
	mov con_esc_parm1-1,RB, RA
	RTS
	
.esc_cmd:
	mov RB, #255
	mov con_esc_status, RB
	
	STC
	CMP RA, #"J"
	JZS CON_ESC_CLEAR
	
	STC
	CMP RA, #"H"
	JZS CON_ESC_SET_CURSOR
	
	STC
	CMP RA, #"f"
	JZS CON_ESC_SET_CURSOR
	
	STC
	CMP RA, #"q"
	JZS CON_ESC_SET_CURSOR_TYPE
	
	STC
	CMP RA, #"h"
	JZS CON_ESC_SET_DISPLAY_RAW
	
	STC
	CMP RA, #"A"
	JZS CON_ESC_CURSOR_UP
	
	STC
	CMP RA, #"B"
	JZS CON_ESC_CURSOR_DOWN
	
	STC
	CMP RA, #"C"
	JZS CON_ESC_CURSOR_RIGHT
	
	STC
	CMP RA, #"D"
	JZS CON_ESC_CURSOR_LEFT
	
	STC
	CMP RA, #"s"
	JZS CON_ESC_SAVE_CURSOR
	
	STC
	CMP RA, #"u"
	JZS CON_ESC_RESTORE_CURSOR

	RTS
	
CON_ESC_SET_DISPLAY_RAW:
	mov RA, con_esc_parm1
	mov con_display_raw, RA
	CLC
	RTS
	
CON_ESC_SET_CURSOR_TYPE:
	mov RA, con_esc_parm1
	AND RA, #15
	OR  RA, #0x0c
	
CON_SET_CURSOR_TYPE:
	mov con_cursor_type, RA
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JNZ .end
	JSR DISPLAY_SEND_COMMAND
.end:	
	CLC
	RTS
	
CON_ESC_RESTORE_CURSOR:
	mov RA, con_saved_row
	mov RB, con_saved_col
	JMP CON_ESC_SET_CURSOR_INT
	
CON_ESC_SAVE_CURSOR:
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JZS .display_pos
	mov RA, con_active_row
	mov RB, con_active_col
	JMP .save_pos
.display_pos:
	JSR DISPLAY_GET_CURSOR_POS
.save_pos:
	mov con_saved_row, RA
	mov con_saved_col, RB
	RTS
	
CON_ESC_CURSOR_UP:
	mov RA, con_active_row
	DEC RA
	JNS .end
	mov RB, con_active_col
	JMP CON_ESC_SET_CURSOR_INT
.end:	
	RTS
	
CON_ESC_CURSOR_DOWN:
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JNZ CON_CURSOR_DOWN
	JMP DISPLAY_MOVE_CURSOR_DOWN
	
CON_ESC_CURSOR_RIGHT:
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JNZ CON_CURSOR_RIGHT
	JMP DISPLAY_MOVE_CURSOR_RIGHT
	
CON_ESC_CURSOR_LEFT:
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JNZ CON_CURSOR_LEFT
	JMP DISPLAY_MOVE_CURSOR_LEFT
	
CON_ESC_SET_CURSOR:
	mov RA, con_esc_parm1
	mov RB, con_esc_parm2
CON_ESC_SET_CURSOR_INT:
	mov RC, con_active_num
	STC
	CMP RC, con_visible_num
	JZS DISPLAY_GOTO_POSITION
	mov con_active_row, RA
	mov con_active_col, RB
	JMP CON_ROW_COL_TO_OFF
	
CON_ESC_CLEAR:
	mov RA, con_esc_parm1
	STC
	CMP RA, #2
	JNZ .end
	mov RA, con_active_num
	CMP RA, con_visible_num
	JZS DISPLAY_CLEAR
	JMP CON_CLEAR_SCREEN
.end:
	RTS
	
CON_ROW_COL_TO_OFF:
	mov RA, con_active_row
	mov RC, con_active_col
	mov RD, #CON_NUM_COLS
.loop:	
	CLC
	LSR RA
	PSF
	JNC .no_add
	CLC
	ADD RC, RD
.no_add:
	LSL RD
	PLF
	JNZ .loop
	
	mov con_active_off, RC
	RTS
	
CON_SCROLL_DOWN:
	mov RA, con_struct_ptr
	CLC
	ADD RA, #CON_NUM_COLS
	mov con_struct_aux_ptr, RA	
	mov RA, con_struct_ptr+1
	ADD RA, #0
	mov con_struct_aux_ptr+1, RA
	mov RB, #CON_OFF_SCREEN
.loop:
	mov RA, [con_struct_aux_ptr],RB
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #CON_NUM_COLS*(CON_NUM_ROWS-1)+CON_OFF_SCREEN
	JNC .loop
	mov RA, #" "
.cloop:	
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #CON_NUM_COLS*CON_NUM_ROWS+CON_OFF_SCREEN
	JNC .cloop
	RTS
	
CON_BACKSPACE:
	JSR CON_CURSOR_LEFT
	JNC .end
	
	mov RB, con_active_off
	CLC
	ADD RB, #CON_OFF_SCREEN
	mov RA, #" "
	mov [con_struct_ptr],RB, RA
	
.end:
	RTS
	
CON_NEW_LINE:
	JSR CON_CURSOR_DOWN
	
	; fall through to CON_CURSOR_LINE_START
	
CON_CURSOR_LINE_START:	
	mov RA, con_active_off
	STC
	SUB RA, con_active_col
	mov con_active_off, RA
	mov RA, #0
	mov con_active_col, RA
	CLC
	RTS
	
CON_CURSOR_RIGHT:
	INC con_active_off
	INC con_active_col
	mov RA, con_active_col
	STC
	CMP RA, #CON_NUM_COLS ; end of line?
	JNC CON_END ; no!
		
	; next line
	mov RA, #0
	mov con_active_col, RA
	INC con_active_row
	
	; raw mode?
	mov RA, con_display_raw
	OR  RA, #0
	JZS CON_CHECK_SCROLL
	
	; in raw mode:
	; line order on LCD is 0, 2, 1, 3
	; => swap lines 2 and 1
	mov RA, con_active_row
	STC
	CMP RA, #1
	JZS .inc_offset
	STC
	CMP RA, #2
	JNZ .not_line_two
.inc_offset:	
	INC con_active_row
	mov RA, con_active_off
	ADD RA, #CON_NUM_COLS
	mov con_active_off, RA
	RTS
.not_line_two:
	STC
	CMP RA, #3
	JNZ WRAP_AROUND
	DEC con_active_row
	DEC con_active_row
	mov RA, con_active_off
	SUB RA, #CON_NUM_COLS*2
	mov con_active_off, RA
.raw_end:
	CLC
	RTS
	
	; fall through to CON_CHECK_SCROLL
	
CON_CHECK_SCROLL:	
	mov RA, con_active_row
	STC
	CMP RA, #CON_NUM_ROWS
	JNC .end
	mov RA, con_display_raw
	OR  RA, #0
	JNZ WRAP_AROUND
	
	JSR CON_SCROLL_DOWN
	DEC con_active_row
	mov RA, con_active_off
	STC
	SUB RA, #CON_NUM_COLS
	mov con_active_off, RA
.end:	
	RTS
	
WRAP_AROUND:
	mov RA, con_active_col
	mov con_active_off, RA
	mov RA, #0
	mov con_active_row, RA
	CLC
	RTS
	
CON_CURSOR_DOWN:
	INC con_active_row
	mov RA, con_active_off
	ADD RA, #CON_NUM_COLS
	mov con_active_off, RA
	JMP CON_CHECK_SCROLL
	
;
; return:
;  c-flag: 0 = no move, 1 = cursor moved
;	
CON_CURSOR_LEFT:
	DEC con_active_off
	JCS .move_left ; beyond (0,0)?
	INC con_active_off ; yes, stay at (0,0)
	CLC
	RTS
	
.move_left:	
	DEC con_active_col
	JCS .end ; beyond begin of row?
	
	; move to last col on previous row
	mov RA, #CON_NUM_COLS-1
	mov con_active_col, RA
	
	DEC con_active_row
.end:	
CON_END:	
	RTS

CON_CLEAR_CHARS:
	mov RB, #CON_OFF_CHARS
	mov RA, #0
.loop:
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #8*8+CON_OFF_CHARS
	JNC .loop
	RTS

CON_CLEAR_SCREEN:
	mov RB, #CON_OFF_SCREEN
	mov RA, #" "
.loop:
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #CON_NUM_COLS*CON_NUM_ROWS+CON_OFF_SCREEN
	JNC .loop
	
; fall-through to CURSOR_HOME	

CON_CURSOR_HOME:
	mov RA, #0
	mov con_active_row, RA
	mov con_active_col, RA
	mov con_active_off, RA
	RTS
;
; create new console
;
; parm: 
;  RA - console number
; result:
;  c-Flag: 0 = O.K., 1 = not successful
;	
CON_CREATE:
	PSH RA
	mov RA, #CON_UNUSED
	JSR CON_SET_STRUCT_PTR
	JNC .set_number
	
	mov RB, #SYS_OWNER
	JSR MEM_ALLOC_SINGLE
	JCS .no_mem	
	
	mov RC, #0
	
	mov con_struct_ptr, RCD
	mov RA, #CON_UNUSED
	mov [RCD], RA
	INC RCD
	mov RA, #160
	mov [RCD], RA
	INC RCD
	mov [RCD], RD
	
	mov RC, RA
	mov RA, #CON_UNUSED
	mov [RCD], RA
	INC RCD
	mov RA, #64
	mov [RCD], RA
	INC RCD
	mov RA, RD
	INC RA
	mov [RCD], RA
	
	mov RC, #64
	mov RD, RA
	mov RA, #CON_UNUSED
	mov [RCD], RA
	INC RCD
	mov RA, con_struct_list_ptr
	mov [RCD], RA
	INC RCD
	mov RA, con_struct_list_ptr+1
	mov [RCD], RA
	
	mov RC, #0
	DEC RD
	mov con_struct_list_ptr, RCD
	CLC
.set_number:	
	PUL RA
	mov RB, #CON_OFF_CON_NUM
	mov [con_struct_ptr],RB, RA
	mov con_active_num, RA
	mov RA, #CON_TYPE_LCD
	mov RB, #CON_OFF_TYPE
	mov [con_struct_ptr],RB, RA
	mov con_active_num, RA
	RTS
.no_mem:
	PUL RB
	RTS

;
; set structure pointer to requested console
;
; parm: 
;  RA - console number
; result:
;  c-Flag: 0 = O.K., 1 = not found
;	
CON_SET_STRUCT_PTR:
	mov RB, con_active_num	
	STC
	CMP RA, RB
	JZS .OK_end
	CMP RB, #CON_UNDEFD
	JZS .search
	
	; save old values for last active console
	PSH RA
	
	mov RB, #CON_OFF_SAVE_AREA
.saveloop:	
	mov RA, CON_ACTIVE_SAVE_AREA-CON_OFF_SAVE_AREA,RB
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #CON_OFF_SAVE_AREA + CON_SAVE_AREA_LEN
	JNC .saveloop
	
	PUL RA
.search:		
	mov RB, con_struct_list_ptr
	mov RD, con_struct_list_ptr+1
.loop:
	mov RC, RB
	OR  RB, RD
	JZS .not_found
	
	STC
	CMP RA, [RCD]
	JZS .found
	
	INC RCD
	mov RB, [RCD]
	INC RCD
	mov RD, [RCD]
	JMP .loop
	
.found:	
	mov con_struct_ptr, RCD
	mov con_active_num, RA
	
	PSH RA
	
	mov RB, #CON_OFF_SAVE_AREA
.restoreloop:	
	mov RA, [con_struct_ptr],RB
	mov CON_ACTIVE_SAVE_AREA-CON_OFF_SAVE_AREA,RB, RA
	INC RB
	STC
	CMP RB, #CON_OFF_SAVE_AREA + CON_SAVE_AREA_LEN
	JNC .restoreloop
	PUL RA
.OK_end:
	CLC
	RTS
	
.not_found:
	STC
.end:	
	RTS
	
CON_SAVE_STATE:	
	JSR CON_SET_STRUCT_PTR
	JCS .end
	
	JSR DISPLAY_GET_CURSOR_POS
	mov con_active_row, RA
	mov con_active_col, RB
	JSR CON_ROW_COL_TO_OFF
	
	; save character data
	mov RA, #LCD_CGRAM
	JSR DISPLAY_SEND_COMMAND
	JSR DISPLAY_WAIT_NOT_BUSY
	mov RB, #CON_OFF_CHARS
.cloop:
	PSH RB
	JSR DISPLAY_READ_BYTE
	PUL RB
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #8*8+CON_OFF_CHARS
	JNC .cloop
	
	mov RA, #0
	JSR DISPLAY_GOTO_LINE
	JSR DISPLAY_WAIT_NOT_BUSY
	mov RB, #CON_OFF_SCREEN
.loop:
	PSH RB
	JSR DISPLAY_READ_BYTE
	PUL RB
	mov [con_struct_ptr],RB, RA
	INC RB
	STC
	CMP RB, #CON_NUM_COLS*1+CON_OFF_SCREEN
	JNZ .not_1st_row
	mov RB, #CON_NUM_COLS*2+CON_OFF_SCREEN
	JMP .loop
.not_1st_row:
	CMP RB, #CON_NUM_COLS*2+CON_OFF_SCREEN
	JNZ .not_2nd_row
	mov RB, #CON_NUM_COLS*3+CON_OFF_SCREEN
	JMP .loop
.not_2nd_row:
	CMP RB, #CON_NUM_COLS*3+CON_OFF_SCREEN
	JNZ .not_3rd_row
	mov RB, #CON_NUM_COLS*1+CON_OFF_SCREEN
	JMP .loop
.not_3rd_row:
	CMP RB, #CON_NUM_COLS*4+CON_OFF_SCREEN
	JNZ .loop
.end:	
	RTS
	
CON_SET_VISIBLE:
	STC
	CMP RA, con_visible_num
	JZS OK_end
	PSH RA
	mov RA, con_visible_num
	STC
	CMP RA, #CON_UNUSED
	JZS .set_visible
	JSR CON_SAVE_STATE
.set_visible:
	PUL RA
	JSR CON_SET_STRUCT_PTR
	JNC .con_OK
	JSR CON_SET_DISPLAY_CURSOR
	mov RA, #EBADF
.end:	
	RTS
.con_OK:	
	mov con_visible_num, RA
	; restore character data
	mov RA, #LCD_CGRAM
	JSR DISPLAY_SEND_COMMAND
	mov RB, #CON_OFF_CHARS
	JSR DISPLAY_WAIT_NOT_BUSY
.cloop:
	mov RA, [con_struct_ptr],RB
	JSR DISPLAY_SEND_DATA_BYTE
	INC RB
	STC
	CMP RB, #8*8+CON_OFF_CHARS
	JNC .cloop
	
	mov RA, #0
	JSR DISPLAY_GOTO_LINE
	mov RB, #CON_OFF_SCREEN
	JSR DISPLAY_WAIT_NOT_BUSY
.loop:
	mov RA, [con_struct_ptr],RB
	JSR DISPLAY_SEND_DATA_BYTE
	INC RB
	STC
	CMP RB, #CON_NUM_COLS*1+CON_OFF_SCREEN
	JNZ .not_1st_row
	mov RB, #CON_NUM_COLS*2+CON_OFF_SCREEN
	JMP .loop
.not_1st_row:
	CMP RB, #CON_NUM_COLS*2+CON_OFF_SCREEN
	JNZ .not_2nd_row
	mov RB, #CON_NUM_COLS*3+CON_OFF_SCREEN
	JMP .loop
.not_2nd_row:
	CMP RB, #CON_NUM_COLS*3+CON_OFF_SCREEN
	JNZ .not_3rd_row
	mov RB, #CON_NUM_COLS*1+CON_OFF_SCREEN
	JMP .loop
.not_3rd_row:
	CMP RB, #CON_NUM_COLS*4+CON_OFF_SCREEN
	JNZ .loop
	
	mov RA, con_cursor_type
	JSR DISPLAY_SEND_COMMAND
	
CON_SET_DISPLAY_CURSOR:	
	mov RA, con_active_row
	mov RB, con_active_col
	JMP DISPLAY_GOTO_POSITION


#bank data
; pointer to start of console list
con_struct_list_ptr: #res 2
; pointer to active console
con_struct_ptr: #res 2
; auxliliary pointer to console structure
con_struct_aux_ptr: #res 2
; number of currently visible console
con_visible_num: #res 1
; number of currently active console
con_active_num: #res 1

CON_ACTIVE_SAVE_AREA:
; row number of currently active console
con_active_row: #res 1
; column number of currently active console
con_active_col: #res 1
; screen offset of currently active console
con_active_off: #res 1
; cursor type of currently active console
con_cursor_type: #res 1
; no scrolling
con_display_raw: #res 1
; escape status of currently active console
con_esc_status: #res 1
; 1st escape parameter of currently active console
con_esc_parm1: #res 1
; 2nd escape parameter of currently active console
con_esc_parm2: #res 1
; saved cursor of currently active console
con_saved_row: #res 1
con_saved_col: #res 1
CON_ACTIVE_SAVE_AREA_END: