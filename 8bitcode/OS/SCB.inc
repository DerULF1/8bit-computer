;
; SCB - Stream Control Block logic
;

SCB_NUM_MAX = 10
SCB_LENGTH = 16

SCB_OFF_TYPE = 0
SCB_OFF_COUNT = 1
SCB_OFF_CON_NR = 2
SCB_OFF_FAT_AREA = 2
SCB_OFF_PIPE_AREA = 2

SCB_CON_FS = 0
SCB_FAT_FS = 1
SCB_PIPE_FS = 2
SCB_FREE_ENTRY = 0xff

#bank pgm

SCB_INIT:
	mov RA, #SCB_FREE_ENTRY
	mov RB, #SCB_LENGTH * (SCB_NUM_MAX - 1)
	STC
.scb_init_loop:
	mov scb_area+SCB_OFF_TYPE,RB, RA
	SUB RB, #SCB_LENGTH
	JCS .scb_init_loop

	RTS
	
SCB_GET_FREE_SCB:
	mov RB, #0
.search_loop:
	mov RA, scb_area+SCB_OFF_TYPE,RB
	XOR RA, #SCB_FREE_ENTRY
	JZS .free
	CLC
	ADD RB, #SCB_LENGTH
	CMP RB, #SCB_LENGTH * SCB_NUM_MAX-1 ; c-flag=0
	JNC .search_loop
	mov RA, #ENFILE
	RTS
.free:
	; start with use-count = 1
	mov RA, #1
	mov scb_area+SCB_OFF_COUNT,RB, RA
	
	; TODO: hard coded SCB_LENGTH=16, better RA=RB/SCB_LENGTH
	CLC
	mov RA, RB
	LSR RA
	LSR RA
	LSR RA
	LSR RA
	RTS
	
SCB_GET_INDEX_FOR_FD:
	JSR TASK_FD_TO_SD
	JCS AN_RTS
	; fall through to SCB_GET_INDEX

SCB_GET_INDEX:
	STC
	CMP RA, #SCB_NUM_MAX
	JNC .scb_OK
	mov RA, #EBADF
	RTS
.scb_OK:
	mov RB, RA
	; TODO: hard coded SCB_LENGTH=16, better RB=RA*SCB_LENGTH
	LSL RB
	LSL RB
	LSL RB
	LSL RB
	RTS

SCB_SET_POINTER:
	CLC 
	ADD RB, #(scb_area+SCB_OFF_FAT_AREA)[7:0]
	mov scb_pointer, RB
	mov RA, #(scb_area+SCB_OFF_FAT_AREA)[15:8]
	ADD RA, #0
	mov scb_pointer+1, RA
	mov RB, #FAT_FILE_AREA_LENGTH-1
	RTS

SCB_COPY_FROM_FAT_ENTRY:
	JSR SCB_SET_POINTER
.loop:
	mov RA, fat_file_area,RB
	mov [scb_pointer],RB, RA
	DEC RB
	JCS .loop
.end:	
	RTS

SCB_COPY_TO_FAT_ENTRY:
	JSR SCB_SET_POINTER
.loop:
	mov RA, [scb_pointer],RB
	mov fat_file_area, RB, RA
	DEC RB
	JCS .loop
.end:	
	RTS

SCB_COPY_FROM_PIPE_ENTRY:
	JSR SCB_SET_POINTER
	mov RB, #PIPE_AREA_LENGTH - 1
.loop:
	mov RA, pipe_area,RB
	mov [scb_pointer],RB, RA
	DEC RB
	JCS .loop
.end:	
	RTS

SCB_COPY_TO_PIPE_ENTRY:
	mov RA, scb_area+SCB_OFF_COUNT,RB
	mov pipe_use_cnt, RA
	
	JSR SCB_SET_POINTER
	mov RB, #PIPE_AREA_LENGTH - 1
.loop:
	mov RA, [scb_pointer],RB
	mov pipe_area, RB, RA
	DEC RB
	JCS .loop
.end:	
	RTS
	
#bank data
scb_area: #res SCB_LENGTH * SCB_NUM_MAX
