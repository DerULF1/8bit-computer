#include "8bit.cpu"

#include "OSCalls.inc"
#include "OSint.inc"

;
; modCON - OS module for a multi console
;
; Written 14.03.2021 Ulf Caspers
;

#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x0000
}

#bankdef data
{
    #addr 0x1000
    #size 0x2000
}

SYS_OWNER = 0xF0 ; Task owner for the system
RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm
MOD_CON_INIT:
	mva RCD, init_msg
	JSR DISPLAY_STRING

	JSR CON_INIT
	
	mva RCD, CON_OPEN
	mov con_open_ptr, RCD
	
	mva RCD, CON_CLOSE
	mov con_close_ptr, RCD
	
	mva RCD, CON_OUT
	mov con_out_ptr, RCD
	
	mva RCD, CON_IN
	mov con_in_ptr, RCD
	
	mva RCD, CON_SET_VISIBLE
	mov con_visible_ptr, RCD

	CLC
	RTS
	
#include "console.inc"
#bank pgm
 
	#d RELOC_ESC
	#d 0x00
	#d 0x00	

init_msg: #d "Init Console...\n\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 MOD_DATA_END - DATA_START

#bank data
MOD_DATA_END:
