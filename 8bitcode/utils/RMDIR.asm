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

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

;
; UNIX-Command RMDIR
;
; Written 23.10.2020 Ulf Caspers
;
START:
	mov RA, #0
	mov file_number, RA
	
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE	
	
	mov RA, #O_READ
	JSR OS_OPEN_DIR
	JCS .error
	mov file_number, RA

.read_loop:	
	mov RA, file_number
	JSR OS_NEXT_DIR
	JCS .error
	
	mov RA, [RCD]
	XOR RA, #"."
	JZS .read_loop
	
	OR  RC, RD
	JZS .read_end
	
	mva RCD, not_empty_msg
	JSR PUT_ERROR_STRING
	
	JMP .close_exit

.read_end:
	mov RA, file_number	
	JSR OS_CLOSE_DIR
	
	mov RA, #1
	JSR GET_PARAMETER
	JSR OS_UNLINK	
	JNC .end
	
.error:	
	PSH RA
	mva RCD, fail_msg
	JSR OS_DISPLAY_STRING
	PUL RA
	JMP OS_DISPLAY_HEX_BYTE
	
.close_exit:
	mov RA, file_number
	OR  RA, #0
	JZS .end	
	
	JSR OS_CLOSE_DIR	
	
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
not_empty_msg: #d "dir not empty\0"
usage_msg: #d "Usage: rmdir dirname\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
file_number: #res 1
DATA_END: