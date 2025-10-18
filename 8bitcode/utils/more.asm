#include "8bit.cpu"

;
; more - UNIX more command
;
; Written 27.04.2021 Ulf Caspers
;

READ_BUFFER_SIZE = 512

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
	mov parm_number, RA
.parm_loop:	
	mov RA, parm_number
	JSR GET_PARAMETER	
	JCS SHOW_STDIN
	
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JNZ SHOW_FILES
	
	INC RCD
	mov RA, [RCD]
	OR  RA, #0
	JZS SHOW_STDIN
	
	INC parm_number
	JMP .parm_loop
	
SHOW_FILES:
	INC parm_number ; increment parameter counter
	
	JSR SHOW_FILE_BY_NAME
	JCS .end
	
	mov RA, parm_number
	JSR GET_PARAMETER	
	JNC SHOW_FILES ; more file names?
	
.end:
	mov RA, #0
	CLC
	RTS
	
SHOW_STDIN:
	mov RA, #0
	mov file_number, RA
	JSR SHOW_FILE_BY_NUMBER
	mov RA, #0
	CLC
	RTS
	
SHOW_FILE_BY_NAME:	
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .error
	mov file_number, RA
	
	JSR SHOW_FILE_BY_NUMBER	
	PSF
	
	mov RA, file_number
	JSR OS_CLOSE
	JCS .error

	PLF
	mov RA, #0
	mov file_number, RA
	RTS
	
.error:
	PLF
	JSR SHOW_ERROR
	STC
	RTS
	
SHOW_FILE_BY_NUMBER:
	mov RC, #20
	mov RD, #4
.read_loop:	
	mov RA, file_number
	PSH RD
	PSH RC
	
	JSR OS_GETC
	
	PUL RC
	PUL RD
	JNC .char_ok
	CMP RA, #EEOF
	JNZ .error
	CLC
	JMP .end
.char_ok:
	STC
	CMP RA, #10 ; line feed?
	JZS .line_end
	DEC RC
	JNZ .char_out
.line_end:
	mov RC, #20
	DEC RD
	JNZ .char_out
.page_end:
	mov RD, #4
	JSR ASK_USER
	JCS .end_more
.char_out:
	JSR PUT_CHAR
	JCS .error

	JMP .read_loop
	
.error:	
	JSR SHOW_ERROR
.end_more:	
	STC	
.end:	
	RTS
	
ASK_USER:
	PSH RA
	PSH RC
.key_loop:	
	JSR OS_KEYBOARD_GET_KEY
	JCS .leave
	STC
	CMP RA, #"q"
	JZS .leave
	STC
	CMP RA, #" "
	JZS .page
	STC
	CMP RA, #10 ; enter key_loop
	JNZ .key_loop
	mov RD, #1
	JMP .end_OK
.page:
	mov RD, #4
.end_OK:	
	CLC
.leave:	
	PUL RC
	PUL RA
	RTS

SHOW_ERROR:
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JMP PUT_ERROR_HEX_BYTE

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
	
fail_msg: #d " fail \0"
usage_msg: #d "Usage: more [file...]\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
file_number: #res 1
parm_number: #res 1
DATA_END:	