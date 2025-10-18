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
#bank pgm

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

;
; UNIX-Command kill
;
; Written 02.05.2021 Ulf Caspers
;
START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD

	mov RA, #1
	JSR GET_PARAMETER
	JCS USAGE
	
	JSR DEC_TO_BIN
	mov RC, RA
	mov RD, RB
	
	mov RA, #8 ; SIGKILL
	JSR OS_SEND_TASK_SIGNAL
	JCS .fail
	
.end:
	mov RA, #0
	RTS

.fail:	
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	mov RA, #1
	RTS
	
USAGE:
	mva RCD, usage_msg
	JSR PUT_ERROR_STRING
	mov RA, #1
	RTS

DEC_TO_BIN:
	mov RA, #0
	mov number, RA
	mov number+1, RA

.loop:	
	mov RA, [RCD] ; get next char

	; end if it's not a number
	STC
	SUB RA, #"0"
	JNC .end
	CMP RA, #"9"+1-"0"
	JCS .end

	INC RCD

	PSH RA
	mov RA, number
	mov RB, number+1
	
	; multiply by 10 (no range check)
	LSL RA
	LSL RB
	LSL RA
	LSL RB
	ADD RA, number
	ADD RB, number+1
	LSL RA
	LSL RB
	
	; add new digit (no range check)
	mov number, RA
	PUL RA
	ADD RA, number
	mov number, RA
	ADD RB, #0
	mov number+1, RB
	
	JMP .loop
	
.end:
	mov RA, number
	mov RB, number+1
	RTS
	

#include "OSCalls.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	
	
fail_msg: #d "failed\0"
usage_msg: #d "Usage: kill tid\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
number: #res 2

DATA_END: