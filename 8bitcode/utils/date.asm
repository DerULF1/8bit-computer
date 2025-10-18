#include "8bit.cpu"

;
; date - UNIX date command
;
; Written 24.08.2020 Ulf Caspers
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

#bank data
DATA_START:

#bank pgm
#include "OSCalls.inc"

; RC/RD - parameter string

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS .read_clock
	
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JNZ .set_clock
	INC RCD
	mov RA, [RCD]
	
	STC
	CMP RA, #"w"
	JNZ .no_wall_clock
	JMP WALL_CLOCK
	
.no_wall_clock:	
	
.set_clock:
	JSR PARSE_TIME_PARM
	JCS .end
	
	mva RCD, byte_area
	JSR OS_SET_TIME	
	
	mva RCD, set_msg
	JSR PUT_STRING	
	
.read_clock:		
	JSR SHOW_CLOCK
	
.end:	
	mov RA, #10
	JMP PUT_CHAR
	
SHOW_CLOCK:
	mva RCD, byte_area
	JSR OS_GET_TIME
	
	JSR FORMAT_TIME
	
	mva RCD, string_area
	JMP PUT_STRING	

WALL_CLOCK:
	mva RCD, CLS_msg
	JSR PUT_STRING
	
	;mov RA, #0x0c ; turn off cursor
	;JSR OS_DISPLAY_SEND_COMMAND

.loop:
	mva RCD, HOME_msg
	JSR PUT_STRING

	JSR SHOW_CLOCK
	
	JSR OS_KEYBOARD_GET_KEY
	OR  RA, #0
	JZS .loop
	
	;mov RA, #0x0F ; turn on cursor
	;JSR OS_DISPLAY_SEND_COMMAND
	
	RTS
	
PARSE_TIME_PARM:	
	mov RA, #1
	JSR GET_PARAMETER
	JCS .format_error
	mov comp_ptr, RC
	mov comp_ptr+1, RD

	JSR PARSE_WEEKDAY
	JCS .format_error
	mov byte_wday, RA
	
	mov RA, #2
	JSR GET_PARAMETER
	JCS .format_error
	mov comp_ptr, RC
	mov comp_ptr+1, RD
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_day, RA
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_month, RA
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_year, RA
	
	mov RA, #3
	JSR GET_PARAMETER
	JCS .format_error
	mov comp_ptr, RC
	mov comp_ptr+1, RD
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_hour, RA
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_min, RA
	
	JSR PARSE_BCD_BYTE
	JCS .format_error
	mov byte_sec, RA
	
	CLC
	RTS

.format_error:
	mva RCD, err_msg
	JSR PUT_ERROR_STRING
	STC
	RTS
	
PARSE_BCD_BYTE:
	mov RB, #0
	mov RC, RB
	
.leading_loop:	; skip leading characters
	mov RA, [comp_ptr], RB
	OR  RA, #0
	JZS .err_end
	STC
	CMP RA, #"0" 
	JNC .skip_char
	CMP RA, #"9"+1
	JNC .read_loop
.skip_char:
	CLC
	ADD RB, #1
	JNC .leading_loop
	
.read_loop:	
	mov RA, [comp_ptr],RB
	STC
	SUB RA, #"0"
	JNC .number_end
	CMP RA, #0x0A 
	JCS .number_end
	LSL RC
	LSL RC
	LSL RC
	LSL RC
	AND RC, #0xF0
	OR  RC, RA
	CLC 
	ADD RB, #1
	JNC .read_loop
	
.number_end:
	mov RA, RC
	mov RC, comp_ptr
	ADD RC, RB
	mov comp_ptr, RC
	mov RD, comp_ptr+1
	ADD RD, #0
	mov comp_ptr+1, RD
	RTS
	
.err_end:
	STC
	RTS
	
PARSE_WEEKDAY:		
	mva RCD, daynames
	mov wday_ptr, RC
	mov wday_ptr+1, RD
	
.wday_outer_loop:	
	mov RB, #0
	
.wday_inner_loop:	
	mov RA, [comp_ptr],RB
	mov RC, RA
	mov RA, [wday_ptr],RB
	STC
	CMP RC, #" "
	JZS .wday_word_end
	OR  RC, #0
	JZS .wday_word_end
	INC RB
	STC
	CMP RA, RC
	JZS .wday_inner_loop
	
.wday_next:	
	CLC
	mov RC, wday_ptr
	ADD RC, #4
	mov wday_ptr, RC
	mov RD, wday_ptr+1
	ADD RD, #0
	mov wday_ptr+1, RD
	mov RA, [RCD]
	OR  RA, #0
	JNZ .wday_outer_loop
	STC
	RTS
	
.wday_word_end:	
	OR  RA, #0
	JNZ .wday_next
	
.wday_found:		
	mva RCD, daynames
	STC
	mov RA, wday_ptr
	SUB RA, RC
	LSR RA
	LSR RA
	AND RA, #0x07
	INC RA
.end:	
	RTS
	
FORMAT_TIME:
	mov RB, #0
	
	JSR FILL_DAYNAME
	
	mov RA, byte_day
	JSR FILL_BCD_BYTE
	
	mov RA, #"." ; point character
	JSR ADD_CHAR
	
	mov RA, byte_month
	AND RA, #0x7f
	JSR FILL_BCD_BYTE
	
	mov RA, #"." ; point character
	JSR ADD_CHAR
	
	mov RA, byte_month
	LSL RA ; isolate century bit to carry flag
	mov RA, #0x20 ; base century
	ADD RA, #0 ; add century bit from clock
	JSR FILL_BCD_BYTE
	mov RA, byte_year
	JSR FILL_BCD_BYTE
	
	mov RA, #0x0a ; line feed
	JSR ADD_CHAR
	
	mov RA, byte_hour
	AND RA, #0x3f
	JSR FILL_BCD_BYTE
	
	mov RA, #":" ; colon character
	JSR ADD_CHAR
	
	mov RA, byte_min
	JSR FILL_BCD_BYTE
	
	mov RA, #":" ; colon character
	JSR ADD_CHAR
	
	mov RA, byte_sec
	AND RA, #0x7F
	JSR FILL_BCD_BYTE
	
	; marker for unreliable clock
	mov RA, byte_sec
	AND RA, #0x80
	JZS .end_string
	
	mov RA, #"*" ; asterisk character
	JSR ADD_CHAR
	
.end_string:
	mov RA, #0
	mov string_area,RB, RA
	RTS
	
FILL_DAYNAME:
	PSH RB
	mov RB, byte_wday ; range (1-7)
	
	; index to the daynames RB = (RB-1)*4
	DEC RB
	CLC
	LSL RB
	LSL RB	
.loop:
	mov RA, daynames,RB
	mov RC, RB	
	PUL RB
	OR  RA, #0
	JZS .end_loop
	JSR ADD_CHAR
	PSH RB
	mov RB, RC
	INC RB
	JNC .loop
	
.end_loop:	
	mov RA, #" " 
	JMP ADD_CHAR
	
FILL_BCD_BYTE:
	PSH RA
	LSR RA
	LSR RA
	LSR RA
	LSR RA	
	JSR .show_half_byte
	PUL RA
.show_half_byte:
	AND RA, #0x0f
	CLC
	ADD RA, #"0" 
	; fall through
ADD_CHAR:	
	mov string_area,RB, RA
	INC RB
	RTS
;	

#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	
	
set_msg: #d "Clock is now:\n\0"
err_msg: #d "Format must be:\nMon 24.12.20 21:45:04\0"
CLS_msg: #d 0x1b, "[2J\0"
HOME_msg: #d 0x1b, "[H\0"
daynames: #d "Mon\0Die\0Mit\0Don\0Fre\0Sam\0Son\0\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data 
wday_ptr: #res 2
comp_ptr: #res 2
byte_area:
byte_sec: #res 1
byte_min: #res 1
byte_hour: #res 1
byte_wday: #res 1
byte_day: #res 1
byte_month: #res 1
byte_year: #res 1
string_area: #res 40
DATA_END: