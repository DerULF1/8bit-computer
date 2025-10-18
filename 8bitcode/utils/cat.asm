#include "8bit.cpu"

;
; cat - UNIX cat command
;
; Written 30.07.2020 Ulf Caspers
;

READ_BUFFER_SIZE = 1024

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

#bank pgm
#include "OSCalls.inc"

; RC/RD - parameter string

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE
	
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .error
	mov file_number, RA

.read_loop:	
	mov RA, file_number
	mov RC, #READ_BUFFER_SIZE[7:0]
	mov RD, #READ_BUFFER_SIZE[15:8]
	mov file_read, RC
	mov file_read+1, RD
	mva RCD, read_buffer
	mov file_read+2, RC
	mov file_read+3, RD
	mva RCD, file_read
	JSR OS_READ
	JCS .read_error
	mov chars_read, RA
	mov chars_read+1, RB
	OR  RA, RB
	JZS .read_end

	mva RCD, read_buffer
	
.out_loop:	
	mov RA, [RCD]
	JSR PUT_CHAR
	JCS .read_error
	
	INC RCD
	DEC chars_read
	DCC chars_read+1
	mov RA, chars_read
	OR  RA, chars_read+1
	JNZ .out_loop
	
	JMP .read_loop
	
.read_error:
	JSR .error
	
.read_end:	
	mov RA, file_number
	JSR OS_CLOSE
	JCS .error

	mov RA, #0
	mov file_number, RA
	
	JMP .end
	
.error:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JSR PUT_ERROR_HEX_BYTE
	
.end:	
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
	
fail_msg: #d " fail \0"
usage_msg: #d "Usage: cat file\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
file_number: #res 1
chars_read: #res 2
file_read:
	#res 2 ; BUFFER_SIZE
	#res 2 ; BUFFER_POINTER
	
read_buffer: #res READ_BUFFER_SIZE
DATA_END:	