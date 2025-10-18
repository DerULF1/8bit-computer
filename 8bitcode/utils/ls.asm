#include "8bit.cpu"

#include "OSCalls.inc"
;
; ls - UNIX ls command
;
; Written 07.08.2020 Ulf Caspers
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

	mov RA, #0
	mov long_disp, RA
	mov paged_disp, RA
	mov file_number, RA
	
	PSH RA
.parm_loop:	
	PUL RA
	INC RA
	PSH RA
	JSR GET_PARAMETER
	
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JNZ .no_parm
	INC RCD
	mov RA, [RCD]
	
	STC
	CMP RA, #"l"
	JNZ .no_l
	mov RA, #1
	mov long_disp, RA
	JMP .parm_loop
	
.no_l:			
	STC
	CMP RA, #"p"
	JNZ .no_p	
	mov RA, #1
	mov paged_disp, RA
	JMP .parm_loop	
.no_p:	
	JMP .parm_loop
	
.no_parm:	
	PUL RA
	
	mov RA, #O_READ
	JSR OS_OPEN_DIR
	JCS .error
	mov file_number, RA
	
	mov RA, #0
    mov entry_disp_cnt, RA
.read_loop:	
	mov RA, file_number
	JSR OS_NEXT_DIR
	JCS .error
	
	mov entry_pointer, RC
	mov entry_pointer+1, RD
	
	OR  RC, RD
	JZS .read_end
	
	mov RB, #8
	mov RA, long_disp
	OR  RA, #0
	JZS .not_long
	; save attribute byte
	mov RB, #11
	mov RA, [entry_pointer],RB
	PSH RA
	
.not_long:	
	mov RA, #0
	mov [entry_pointer],RB, RA
	
	mov RC, entry_pointer
	mov RD, entry_pointer+1
	JSR PUT_STRING
	JCS .error
	
.not_long2:	
	INC entry_disp_cnt
	
	mov RA, #" "
	JSR PUT_CHAR
	JCS .error
	
	mov RA, long_disp
	OR  RA, #0
	JNZ .long_disp
		
	mov RB, #8
	JSR CHECK_PAGE
	JCS .read_end
	
	mov RA, #" "
	JSR PUT_CHAR
	JCS .error
	
	JMP .read_loop
	
.long_disp:
	PUL RB
	JSR SHOW_DIR_ATTR
	
	mov RA, #" "
	JSR PUT_CHAR
	
	mov RB, #4
	JSR CHECK_PAGE
	JCS .read_end
	
	mov RA, #" "
	JSR PUT_CHAR
	
	JMP .read_loop
	
.read_end:		
	mov RA, file_number
	mov RB, #0
	mov file_number, RB
	JSR OS_CLOSE_DIR
	JCS .error
	
	JMP .end
	
.error:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JSR PUT_ERROR_HEX_BYTE
	mov RA, #10
	JSR PUT_ERROR_CHAR
	
	mov RA, file_number
	OR  RA, #0
	JZS .end
	
	JSR OS_CLOSE_DIR
	
.end:	
	RTS

SHOW_DIR_ATTR:
	mva RCD, dir_attr_chars
.loop:	
	mov RA, [RCD]
	OR  RA, #0
	JZS .end_attr
	LSR RB
	JCS .show_attr
	mov RA, #" "
.show_attr:
	JSR PUT_CHAR
	INC RCD	
	JNC .loop
.end_attr:
	RTS
	
CHECK_PAGE:
	mov RA, paged_disp
	OR  RA, #0
	JZS .no_page
		
	mov RA, entry_disp_cnt
	STC
	CMP RA, RB
	JNZ .no_page
	mov RA, #0
	mov entry_disp_cnt, RA
	JSR GET_CHAR
	STC
	CMP RA, #"x"
	JZS .end	
.no_page:	
	CLC
.end:	
	RTS

#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	
	
fail_msg: #d " failed \0"
dir_attr_chars: #d "RHSVDA\0"

END_PGM:

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
file_name: #res 40
parm_name: #res 40
long_disp: #res 1
entry_disp_cnt: #res 1
paged_disp: #res 1
file_number: #res 1
entry_pointer: #res 2
DATA_END: