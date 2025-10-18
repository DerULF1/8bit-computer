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
; UNIX-Command DF
;
; Written 29.10.2020 Ulf Caspers
;
START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
			
	mva RCD, total_msg
	JSR PUT_STRING
	
	mva RCD, fsstat_area
	JSR OS_FSSTAT
	JCS .fail
	
	mov RA, fsstat_area+FAT_FSSTAT_TOTAL
	mov RB, fsstat_area+FAT_FSSTAT_TOTAL+1
	mov RC, #0
	JSR SHOW_CLUSTERS_AS_SECTORS
	
	mov RA, #10
	JSR PUT_CHAR
		
	mva RCD, free_msg
	JSR PUT_STRING
	
	mov RA, fsstat_area+FAT_FSSTAT_FREE
	mov RB, fsstat_area+FAT_FSSTAT_FREE+1
	mov RC, #0
	JSR SHOW_CLUSTERS_AS_SECTORS
	
	mov RA, #10
	JSR PUT_CHAR
	
	RTS

.fail:	
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	RTS

SHOW_CLUSTERS_AS_SECTORS: 
	mov RD, fsstat_area+FAT_FSSTAT_SPC
	
.mul_total:	
	LSR RD
	JCS .mul_total_done
	LSL RA
	LSL RB
	LSL RC
	JNC .mul_total
	
.mul_total_done:	
	mov quotient, RA
	mov quotient+1, RB
	mov quotient+2, RC
	JSR NUM2STR
	mva RCD, number_string
	JMP PUT_STRING
;
NUM2STR:
	mov RA, #0
	PSH RA
	
	mov RA, #10
	mov divisor, RA
	mov RA, #0
	mov divisor+1, RA
	mov divisor+2, RA
	
.divloop:
	mov RA, quotient
	mov RB, quotient+1
	mov RC, quotient+2
	mov RD, RA
	OR  RD, RB
	OR  RD, RC
	JZS .enddivloop
	mov divident, RA
	mov divident+1, RB
	mov divident+2, RC
	JSR DIVIDE
	mov RA, remainder
	CLC
	ADD RA, #"0"
	PSH RA
	JMP .divloop
	
.enddivloop:	
	mva RCD, number_string
	mov RB, RC
	CLC
.loop:
	PUL RA
	mov [RCD], RA
	OR  RA, #0
	JZS .endloop
	INC RCD
	JNC .loop
.endloop:
	STC	
	CMP RC, RB
	JNZ .exit
	CLC
	mov RA, #"0"
	mov [RCD], RA
	INC RCD
	mov RA, #0
	mov [RCD], RA
.exit:	
	RTS
;
DIVIDE:
	mov RA, divident
	mov quotient, RA
	mov RA, divident+1
	mov quotient+1, RA
	mov RA, divident+2
	mov quotient+2, RA
	mov RA, #0
	mov remainder, RA
	mov remainder+1, RA
	mov remainder+2, RA
	
	mov RA, #24
	mov arith_cycle_cnt, RA
	
	CLC 
	
.loop:	
	LSL quotient
	LSL quotient+1
	LSL quotient+2
	LSL remainder
	LSL remainder+1
	LSL remainder+2
	
	STC
	mov RA, remainder
	sub RA, divisor
	mov RB, remainder+1
	sub RB, divisor+1
	mov RC, remainder+2
	sub RC, divisor+2
	JNC .ignore
	
	mov remainder, RA
	mov remainder+1, RB
	mov remainder+2, RC
	
.ignore:
	PSF
	DEC arith_cycle_cnt
	JZS .endloop
	PLF
	JMP .loop
	
.endloop:
	PLF
	LSL quotient
	LSL quotient+1
	LSL quotient+2
	RTS
;	

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
total_msg: #d "total: \0"
free_msg:  #d "free:  \0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data

number_string: #res 9
fsstat_area: #res 5

divident: #res 3
divisor: #res 3
quotient: #res 3
remainder: #res 3
arith_cycle_cnt: #res 1	

DATA_END: