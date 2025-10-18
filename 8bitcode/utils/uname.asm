#include "8bit.cpu"

#include "OSCalls.inc"
;
; uname - UNIX uname command
;
; Written 26.09.2020 Ulf Caspers
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

SYS_OWNER = 0xF0
RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

; RC/RD - parameter string

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	JSR OS_GET_VERSION
	JSR PUT_STRING
	RTS
	
#include "streamio.inc"
#bank pgm
		
	#d RELOC_ESC
	#d 0x00
	#d 0x00	

END_PGM:

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data

parm_ptr: #res 2
DATA_END: