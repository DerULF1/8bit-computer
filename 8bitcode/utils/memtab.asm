#include "8bit.cpu"

; internal memtab - command

#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x00
}

#bankdef data
{
    #addr 0x1000
    #size 0x2000
}

#include "OSCalls.inc"
#bank pgm

mem_reserve=0xdac5
MEM_MAX_PAGES=0x60
MEM_FREE_PAGE=0xff

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
		
	mov RB, #MEM_MAX_PAGES-1
.loop:
	mov RA, mem_reserve,RB
	STC
	CMP RA, #MEM_FREE_PAGE
	JZS .next
	PSH RB
	JSR PUT_HEX_BYTE
	PUL RB
.next:
	DEC RB
	JCS .loop
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

#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00	

#bank pgm
fail_msg: #d "failed \0"
usage_msg: #d "Usage: memtab\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
DATA_END: