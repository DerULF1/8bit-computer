#include "8bit.cpu"

; UNIX ECHO - command

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
		
	mov RB, #1
	mov with_newline, RB

.parm_loop:	
	mov RA, RB
	PSH RB
	JSR GET_PARAMETER
	PUL RB
	JCS .last_string ; no parameter strings
	
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JNZ .loop ; not a command parameter
	
	INC RB	; next parameter string
	INC RCD ; 2nd character of command parameter
	mov RA, [RCD]
	
	STC
	CMP RA, #"n" ; "-n"=> no newline
	JNZ .not_n
	
	mov RA, #0
	mov with_newline, RA
	JMP .parm_loop
	
.not_n:
	; unknown parameter
	JMP SHOW_USAGE

.loop:
	JSR PUT_STRING
	JCS SHOW_ERROR
	
	INC RB
	mov RA, RB
	PSH RB
	JSR GET_PARAMETER
	PUL RB
	JCS .last_string
	
	mov RA, #" "
	JSR PUT_CHAR
	JCS SHOW_ERROR
	
	JMP .loop
		
.last_string:
	DEC with_newline
	JNZ .end	
	mov RA, #10 ; trailing newline
	JSR PUT_CHAR
	JCS SHOW_ERROR
	
.end:
	mov RA, #0
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
usage_msg: #d "Usage:\necho [-n] string...\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
with_newline: #res 1
DATA_END: