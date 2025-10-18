#include "8bit.cpu"

#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x00
}

#bankdef data
{
    #addr 0x1000
    #size 0xD000
}

SYS_OWNER = 0xF0
RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

;
; UNIX-Command RM
;
; Written 25.10.2020 Ulf Caspers
;
START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE	
	
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .error

	JSR OS_CLOSE

	mov RA, #1
	JSR GET_PARAMETER
	JSR OS_UNLINK	
	JNC .end
	
.error:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JSR PUT_ERROR_HEX_BYTE
	mov RA, #10
	JSR PUT_ERROR_CHAR
	
.end:	
	RTS
	
SHOW_USAGE:
	mva RCD, usage_msg
	JMP PUT_ERROR_STRING

#include "OSCalls.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	

fail_msg: #d "failed \0"
usage_msg: #d "Usage: rm file\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
DATA_END: