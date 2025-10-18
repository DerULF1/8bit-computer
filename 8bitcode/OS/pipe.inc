;
; Pipe Functions
;

#bank pgm
	
PIPE_OPEN:	
	; allocate pipe data_buffer
	mov RB, #SYS_OWNER
	JSR MEM_ALLOC_SINGLE	
	JCS .end
	
	mov RC, #0
	mov pipe_buf_ptr, RCD
	mov pipe_read_ptr, RCD
	mov pipe_write_ptr, RCD
	
	mov RA, #1
	mov pipe_empty, RA
.end:	
	RTS
	
PIPE_CLOSE:
	mov RD, pipe_buf_ptr+1
	mov RB, #SYS_OWNER
	JSR MEM_FREE_SINGLE
	RTS
	
PIPE_PUTC:
	; pipe already closed?
	mov RB, pipe_use_cnt 
	STC
	CMP RB, #2
	JCS .still_open
	
.broken_pipe:
	mov RA, #EPIPE
	STC
	RTS

.still_open:
	; get current write pointer
	mov RCD, pipe_write_ptr
	
	; pipe currently empty?
	mov RB, pipe_empty
	OR  RB, #0
	JNZ .write_pipe
	
	; pipe full?
	STC
	CMP RC, pipe_read_ptr
	JNZ .write_pipe
	STC
	CMP RD, pipe_read_ptr+1
	JNZ .write_pipe
	
	; pipe is full
	STC
	mov RA, #EAGAIN
	RTS
	
.write_pipe:
	mov [RCD], RA
	CLC
	PSF
	SII
	INC pipe_write_ptr
	JNC .end
	CMP RD, pipe_buf_ptr+1
	JZS .inc_highbyte
	DEC pipe_write_ptr+1
	JMP .end
.inc_highbyte:
	INC pipe_write_ptr+1
.end:
	mov RA, #0
	mov pipe_empty, RA
	PLF
	RTS
	
PIPE_GETC:
	mov RA, pipe_empty
	OR  RA, #0
	JZS .read_pipe

	; no data, closed by writer?
	mov RA, pipe_use_cnt	
	STC
	CMP RA, #2
	JCS .would_block
	
.end_of_pipe:
	mov RA, #EEOF
	STC
	RTS
	
.would_block:
	; no data available
	mov RA, #EAGAIN
	RTS
	
.read_pipe:
	; get current read pointer
	mov RCD, pipe_read_ptr
	
	mov RA, [RCD]
	
	CLC
	PSF
	SII
	
	INC RC
	mov pipe_read_ptr, RC
	JNC .check_empty
	CMP RD, pipe_buf_ptr+1 ; buffer is 512 bytes
	JZS .inc_highbyte
	DEC RD
	JMP .store_highbyte
.inc_highbyte:	
	INC RD	
.store_highbyte:	
	mov pipe_read_ptr+1, RD

.check_empty:
	STC
	CMP RC, pipe_write_ptr
	JNZ .not_empty
	STC
	CMP RD, pipe_write_ptr+1
	JNZ .not_empty
	
	mov RB, #1
	mov pipe_empty, RB
	
.not_empty:		
	PLF
	RTS

#bank data
pipe_area:
pipe_buf_ptr: #res 2
pipe_read_ptr: #res 2
pipe_write_ptr: #res 2
pipe_empty: #res 1
pipe_area_end:
pipe_use_cnt: #res 1
