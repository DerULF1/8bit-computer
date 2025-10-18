#include "8bit.cpu"

; UNIX MKDIR - command

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

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE
	
	JSR OS_MKDIR
	JCS SHOW_ERROR
	
.end:	
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

#include "OSCalls.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00	

#bank pgm
fail_msg: #d "failed \0"
usage_msg: #d "Usage: mkdir dirname\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
DATA_END: