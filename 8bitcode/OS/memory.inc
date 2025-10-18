;
; Memory Functions
;
; allocates memory pages of 512 bytes
;

; marker for a free memory page
MEM_FREE_PAGE = 0xFF

#bank pgm

;
; initializes all memory pages as free
;
MEM_INIT:
	mov RA, #MEM_FREE_PAGE
	mov RB, #MEM_MAX_PAGES-1
.loop:	
	mov mem_reserve,RB, RA
	DEC RB
	JCS .loop
	RTS

;
; Allocate memory pages
;
; Parm:
;  RB - memory owner
;  RC - number of requested contignous pages
; Result:
;  c-flag: 0 = O.K., 1 = error
;  RD - high address of 1st page (c=0)
;  RA - error number (c=1)
;  
MEM_ALLOC:
	PSF
	SII
	PSH RB ; save owner
	mov RD, RC ; save page count
	mov RB, #0 ; start at lowest page
.loop:	
	mov RA, mem_reserve,RB
	XOR RA, #MEM_FREE_PAGE
	JNZ .notfree

	DEC RC
	JZS MEM_FOUND
	JMP .next
.notfree:	
	mov RC, RD ; reset page count
.next:	
	INC RB
	CMP RB, #MEM_MAX_PAGES ; (c-flag = 0)
	JNC .loop

MEM_NOT_FOUND:
	PUL RB ; restore owner
	PLF
	STC
	mov RA, #ENOMEM	
	RTS
	
MEM_FOUND:
	PUL RA ; restore owner
	
.reserve_loop:
	mov mem_reserve,RB, RA
	DEC RD
	JZS .end_reserve
	DEC RB
	JMP .reserve_loop
	
.end_reserve:		
	; page index -> pages high address
	PLF
	mov RD, RB
	CLC
	LSL RD
	RTS

;
; Allocate a single memory page
;
; Parm:
;  RB - memory owner
; Result:
;  c-flag: 0 = O.K., 1 = error
;  RD - high address of 1st page (c=0)
;  RA - error number (c=1)
;  
MEM_ALLOC_SINGLE:
	PSF
	SII
	PSH RB ; save owner
	mov RB, #MEM_MAX_PAGES-1 ; start at last page
.loop:	
	mov RA, mem_reserve,RB
	XOR RA, #MEM_FREE_PAGE
	JZS .found	
	DEC RB
	JCS .loop
	JMP MEM_NOT_FOUND
.found:	
	PUL RA ; restore owner
	mov mem_reserve,RB, RA	
	PLF
	mov RD, RB
	CLC 
	LSL RD
	RTS

;
; Free memory page
;
; Parm:
;  RB - memory owner
;  RD - high address page
;  
MEM_FREE_SINGLE:
	mov RC, #1
	
	; fall through to MEM_FREE
	
;
; Free memory pages
;
; Parm:
;  RB - memory owner
;  RC - number of requested contignous pages
;  RD - high address of 1st page
;  
MEM_FREE:	
    ;calculate index from page
	CLC
	LSR RD
	JCS .illmem1
	CMP RD, #MEM_MAX_PAGES-1 ; c-flag==0
	JCS  .illmem1
	
	mov RA, RB ; owner->RA
	mov RB, RD ; first page index
	mov RD, RA ; owner->RD
	
	PSF
	SII
.loop:
	mov RA, mem_reserve,RB
	STC
	CMP RA, RD
	JNZ .illmem
	mov RA, #MEM_FREE_PAGE
	mov mem_reserve,RB, RA
	DEC RC
	JZS .end
	INC RB 
	CMP RB, #MEM_MAX_PAGES -1 ; c-flag==0
	JNC .loop
.illmem:
	PLF
.illmem1:
	STC
	mov RA, #255 ;ESEGFAULT
	;JMP ENDTASK
	RTS
	
.end:
	PLF
	CLC
	RTS

;
; Free all memory pages of a certain owner
;
; Parm:
;  RD - memory owner
;  
MEM_FORCE_FREE:
	mov RB, #MEM_MAX_PAGES-1
	PSF
	SII
	STC
.loop:
	mov RA, mem_reserve,RB
	CMP RA, RD
	JNZ .next
	
	mov RA, #MEM_FREE_PAGE
	mov mem_reserve,RB, RA
.next:
	DEC RB
	JCS .loop
	PLF
	CLC
	RTS

#bank data
mem_reserve: #res MEM_MAX_PAGES