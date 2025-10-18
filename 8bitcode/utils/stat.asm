#include "8bit.cpu"

#include "OSCalls.inc"
;
; stat - UNIX stat command
;
; Written 18.09.2020 Ulf Caspers
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

; RC/RD - parameter string

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD

	mov RA, #0	
	mov file_number, RA
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE
	
	mov RA, #O_READ
	JSR OS_OPEN
	JNC .open_OK
	
	CMP RA, #EISDIR
	STC
	JNZ .error 
	
	mov RA, #1
	JSR GET_PARAMETER
	
	mov RA, #O_READ + O_DIR
	JSR OS_OPEN
	JCS .error 
	
.open_OK:
	mov file_number, RA
	
	mva RCD, fat_dir_entry
	JSR OS_STAT
	JCS .error
	
	mov RA, file_number
	JSR OS_CLOSE
	JCS .error
	mov RA, #0	
	mov file_number, RA
	
.found:	
	mva RCD, file_msg
	JSR PUT_STRING
	
	mov RB, #11
	mov RA, fat_dir_entry,RB
	PSH RA
	mov RA, #0
	mov fat_dir_entry,RB, RA
	mva RCD, fat_dir_entry
	JSR PUT_STRING
	mov RA, #32
	JSR PUT_CHAR
	PUL RA
	mov fat_dir_entry,RB, RA
	JSR PUT_HEX_BYTE
	
	mov RA, #10
	JSR PUT_CHAR
	
	mva RCD, size_msg
	JSR PUT_STRING
	
	mov RB, #31
	mov RA, fat_dir_entry,RB
	JSR PUT_HEX_BYTE
	mov RB, #30
	mov RA, fat_dir_entry,RB
	JSR PUT_HEX_BYTE
	mov RB, #29
	mov RA, fat_dir_entry,RB
	JSR PUT_HEX_BYTE
	mov RB, #28
	mov RA, fat_dir_entry,RB
	JSR PUT_HEX_BYTE
	mov RA, #10
	JSR PUT_CHAR
	
	mva RCD, change_msg
	JSR PUT_STRING
	
	mov RB, #25
	mov RA, fat_dir_entry,RB
	mov RC, RA
	mov RB, #24
	mov RA, fat_dir_entry,RB
	mov RB, RC
	mva RCD, dir_date_time_buffer
	JSR FAT_DIR_DATE_2_STRING
	mov RA, #" "
	JSR INSERT_CHAR
	mov RB, #23
	mov RA, fat_dir_entry,RB
	PSH RA
	mov RB, #22
	mov RA, fat_dir_entry,RB
	PUL RB
	JSR FAT_DIR_TIME_2_STRING
	mva RCD, dir_date_time_buffer
	JSR PUT_STRING
	mov RA, #10
	JSR PUT_CHAR
	
	JMP .close_end
	
.error:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JSR PUT_ERROR_HEX_BYTE
	mov RA, #10
	JSR PUT_CHAR
	
.close_end:	
	mov RA, file_number
	OR  RA, #0
	JZS .end
	JSR OS_CLOSE
.end:
	RTS
	
;
; converts FAT Directory Time bytes
; to a human readable time string
; 
; parm:
;  RA/RB - FAT DIR time bytes
;  RC/RD - pointer to string buffer
;
FAT_DIR_TIME_2_STRING:
	PSH RB
    PSH RA
	
	LSR RB
	LSR RA 
	LSR RB
	LSR RA
	LSR RB
	LSR RA
	LSR RA
	LSR RA
	PSH RA
	mov RA, RB
	AND RA, #0x1f
	JSR INSERT_TWO_DIGITS
	
	PUL RA
	AND RA, #0x3f
	JSR INSERT_TWO_DIGITS
	
	PUL RA
	PSH RA
	LSL RA ; 2 seconds per bit
	AND RA, #0x3e
	JSR INSERT_TWO_DIGITS
	
	mov RA, #0
	mov [RCD], RA
	
	PUL RA
	PUL RB
	RTS
	
FAT_DIR_DATE_2_STRING:
	PSH RA
	AND RA, #0x1F ; isolate day of month
	
	JSR INSERT_TWO_DIGITS
	
	PUL RA
	PSH RA
	PSH RB
	LSR RB ; isolate month
	LSR RA
	LSR RA
	LSR RA
	LSR RA
	LSR RA
	AND RA, #0x0F
	JSR INSERT_TWO_DIGITS
	
	STC 		; year is counted from 1980
	SUB RB, #20 ; carry set, when RB >= 20 (century 20)
	mov RA, #19
    ADD RA, #0  ; Add carry flag (century flag)
	JSR INSERT_TWO_DIGITS
	OR  RB, #0
	JNN .yearout
	CLC
	ADD RB, #100
.yearout:
	mov RA, RB
	JSR INSERT_TWO_DIGITS
	
	mov RA, #0
	mov [RCD], RA
	
	PUL RB
	PUL RA
	RTS
	
INSERT_TWO_DIGITS:
	PSH RA
	PSH RB
	mov RB, #0
.loop10:
	STC	
	SUB RA, #10
	JNC .end10
	ADD RB, #0 ; plus carry
	JNC .loop10
.end10:	
	ADD RA, #10
	PSH RA
	mov RA, RB
	JSR INSERT_ONE_DIGIT
	PUL RA
	JSR INSERT_ONE_DIGIT
	mov RA, #0 ; end string
	mov [RCD], RA
	PUL RB
	PUL RA
	RTS

INSERT_ONE_DIGIT:
	STC
	CMP RA, #10
	JNC .below10
	ADD RA, #6
.below10:	
	ADD RA, #"0"
INSERT_CHAR:
	mov [RCD], RA
	ADD RC, #1
	ADD RD, #0
	RTS

SHOW_USAGE:
	mva RCD, usage_msg
	JMP PUT_ERROR_STRING

#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00	
	
file_msg: #d "File:\0"
size_msg: #d "Size:\0"
change_msg: #d "Chg:\0"
fail_msg: #d " failed \0"
usage_msg: #d "Usage: stat file\0"
;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

END_PGM:

#bank data 
fat_dir_entry: #res 32
file_number: #res 1
dir_date_time_buffer: #res 24
DATA_END: