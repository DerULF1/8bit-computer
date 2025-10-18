;
; Relocator Function
;
; relocates program code
;

RELOC_ESC = 0xFB
CMD_EXT   = 0xFF

#bank pgm
	
;
; relocate program code
; 
; Parms:
; Returns:
;  c-flag: 0=O.K., 1=error
;
; global
;  pgm_start:          #d16
;  pgm_end:          #d16
;  data_section_start: #d16
;  data_section_end:   #d16
;  data_mem_start:     #d16
;	
RELOCATE:	
	; start analyzing program code
	mov RCD, pgm_start
	
.loop:
	; get next instruction
	mov RB, [RCD]
	
	; relocation escape code
	STC
	CMP RB, #RELOC_ESC
	JNZ .noesc
	
	INC RCD
	mov RA, [RCD]	
	INC RCD
	mov RB, [RCD]
	
	; zero offset?
	OR  RA, #0
	JNZ .jmp
	OR  RB, #0
	JZS .end ; yes!
	
.jmp:
	; skip instructions
	CLC
	ADD RC, RA
	ADD RD, RB
	JMP .loop
	
.noesc:	
	; extended instruction
	STC
	CMP RB, #CMD_EXT
	JNZ .std_tab

	; get real instruction
	INC RCD
	mov RB, [RCD]
	
	; get instruction length
	LSR RB
	CLC
	LSR RB
	mov RA, EOPCODE_TABLE,RB
	JMP .chk_reloc
	
.std_tab:
	; get instruction length
    CLC
	LSR RB
	CLC
	LSR RB
	mov RA, OPCODE_TABLE,RB
	
.chk_reloc:	
	; find length bits in opcode byte
	mov RB, [RCD]
	
	LSR RB
	JNC .even
	LSR RA
	LSR RA
.even:
	LSR RB
	JNC .lowerhalf
	LSR RA
	LSR RA
	LSR RA
	LSR RA
.lowerhalf: 
	AND RA, #3
	
	; only relocate 16 bit parameter instructions
	STC
	CMP RA, #2
	JNZ .noreloc
	
	; get lo-byte of parameter
	INC RCD
	mov RA, [RCD]
	
	; get hi-byte of parameter
	INC RCD
	mov RB, [RCD]
	
	; beyond code and data?
	STC
	CMP RA, data_section_end	
	CMP RB, data_section_end+1
	JCS .external
	
	; inside data_section?
	STC
	CMP RA, data_section_start
	CMP RB, data_section_start+1
	JCS .data_byte
	
	; adjust code address
	ADD RB, pgm_start+1
	JMP .rewrite_hi
	
.data_byte:
	DEC RCD
	
	; adjust data section address
	SUB RA, data_section_start
	SUB RB, data_section_start+1
	CLC
	ADD RA, data_mem_start
	ADD RB, data_mem_start+1
	
	mov [RCD], RA
	
	INC RCD
.rewrite_hi:
	mov [RCD], RB

.external:	
	mov RA, #0	; already skipped parameters
	
.noreloc:	
	; go to next instruction
	STC
	ADD RC, RA
	JNC .loop
	INC RD
	; check pgm end overrun
	CMP RD, pgm_end+1 ; c=0 -> subtract on more
	JNC .loop
	
	mov RA, #ERELOC
.end:	
	RTS


#bank data
pgm_start: #res 2
data_section_start: #res 2
data_section_end: #res 2
data_mem_start: #res 2
