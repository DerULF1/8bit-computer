#include "8bit.cpu"

#include "OSCalls.inc"

;
; CLEAR - Clear terminal screen
;
; Written 17.05.2021 Ulf Caspers
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

	JSR CURSOR_CLEAR_SCREEN
	JCS .fail
.end:	
	RTS

.fail:
	JSR PUT_HEX_BYTE
	mva RCD, fail_msg
	JMP PUT_STRING
	
#include "param.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "cursor.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00
	
fail_msg: #d " fail\n\0"

END_PGM:

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
DATA_END:
