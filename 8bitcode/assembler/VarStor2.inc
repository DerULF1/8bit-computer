;
; simple binary tree variable storage
;

;
; initializes data area
;

VSTORE_PAGE_SKIP=1
VSTOR_INITIAL_PAGES=4 ; 2k of variable space

VSTORE_INIT:
	; allocate heap storage
	mov RC, #VSTOR_INITIAL_PAGES
	JSR OS_MEM_ALLOC
	JCS MAIN_IO_ERROR_EXIT
	
	; store heap start
	mov RC, #0
	mov vstor_next_free, RCD
	mov RD, #VSTOR_INITIAL_PAGES
	
	; convert number of pages to number of bytes
	LSL RD 
	mov vstor_space_left, RCD
	
	; reset pointer to root
	mov RD, RC
	mov vstor_start, RCD

	RTS
	
;
; VSTORE_SET_VALUE - set/add value of a variable
;
; parm:
; returns:
;  carry flag (0=replaced value, 1=new variable)
; global:
;  word_current  - name of the variable
;  value_current - 2 byte value for the variable
;		
VSTORE_SET_VALUE:
	JSR VSTORE_GET_VALUE
	JCS .new_variable
	mov RA, value_current
	mov [RCD], RA
	INC RCD
	mov RA, value_current+1
	mov [RCD], RA
	RTS
	
.new_variable:
	; calculated entry length
	mov RB, vstor_comp_len
	
	JSR _VSTOR_CHECK_SPACE
	
	mov RA, vstor_next_free
	mov [RCD], RA
	INC RCD
	mov RA, vstor_next_free+1
	mov [RCD], RA
	mov RCD, vstor_next_free
	
	; set entry length
	mov [RCD], RB
	
	; initialize left and right pointer  
	mov RA, #0
	INC RCD
	mov [RCD], RA
	INC RCD
	mov [RCD], RA
	INC RCD
	mov [RCD], RA
	INC RCD
	mov [RCD], RA
	
	mov RCD, vstor_next_free
	CLC
	ADD RC, RB
	ADD RD, #0
	mov vstor_next_free, RCD
	
	DEC RCD
	mov RA, value_current+1
	mov [RCD], RA
	DEC RCD
	mov RA, value_current
	mov [RCD], RA

	; current word length
	mov RB, word_current
.copy_loop:	
	DEC RB
	JNC .end 
	DEC RCD
	mov RA, word_current+1,RB
	mov [RCD], RA
	JMP .copy_loop
	
.end:	
	STC
	RTS

;
; parm:
;  RB  - required space
; returns:
;  RB  - required space
;	
_VSTOR_CHECK_SPACE:
	STC
	mov RA, vstor_space_left
	SUB RA, RB
	mov vstor_space_left, RA
	DCC vstor_space_left+1
	JCS .end
	
.extend_space:
	PSH RD
	PSH RC
	PSH RB
	
	mov RC, #1 ; one new page
	JSR OS_MEM_ALLOC
	JCS MAIN_IO_ERROR_EXIT
	
	mov RC, #0
	mov vstor_next_free, RCD
	
	PUL RB

	mov RD, #1 ; (512 - RB) new bytes left
	STC
	SUB RC, RB
	mov vstor_space_left, RCD
	
	PUL RC
	PUL RD

.end:
	RTS
	

;
; VSTORE_GET_VALUE - read current value of a variable
;
; parm:
; returns:
;  carry flag (0=found, 1=not found)
;  RCD - if found: pointer to 2-byte variable value
;        if not found: pointer to end of variable list
; global:
;  word_current - name of the variable
;		
VSTORE_GET_VALUE:
	mva RCD, vstor_start
	
	; entry length is 3 bytes longer than name length
	mov RA, word_cur_len
	CLC
	ADD RA, #7 ; word length
	mov vstor_comp_len, RA

.search_loop:
	; check if nect entry is empty
	mov RB, [RCD]
	INC RCD
	mov RA, [RCD]

	OR  RA, RB
	JZS .not_found

	; move RCD to next entry
	mov RD, [RCD]
	mov RC, RB

	; get entry length
	mov RA, [RCD]
	STC
	CMP RA, vstor_comp_len
	JZS .compare
	JCS .goleft
.goright:	
	ADD RC, #2
	ADD RD, #0
.goleft:
	INC RCD
	JMP .search_loop

.compare:
	CLC
	ADD RC, #4
	ADD RD, #0
	mov RB, #0
.compare_loop:	
	INC RCD
	INC RB
	mov RA, word_current,RB
	STC
	CMP RA, [RCD]
	JNZ .not_same
	CMP RB, word_cur_len
	JNC .compare_loop
	
	INC RCD ; set RCD to value of variable
	        ; (also resets carry)
	RTS	
.not_same:
	JCS .greater
	; reset to left node entry
	ADD RB, #2
.greater:
	; reset RCD to start of entry
	; c-flag=0 => -1
	CLC
	SUB RC, RB 
	SUB RD, #0
	JMP .search_loop

.not_found:	
	DEC RCD
	STC
	RTS
	
#bank data
; pointer to root node
vstor_start: #res 2
; pointer to next free space in allocated heap
vstor_next_free: #res 2
; number of unused bytes in allocated heap
vstor_space_left: #res 2
; helper variable for length of current search entry
vstor_comp_len: #res 1
