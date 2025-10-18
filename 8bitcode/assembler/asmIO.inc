;
; Input/Output parts of the assembler
;
IO_WRITE_BYTE:
	INC addr_current
	ICC addr_current+1
	
	mov RB, pass_asm
	STC
	CMP RB, #2
	JNZ .end
	
	mov RB, write_buffer_pos
	CMP RB, #WRITE_BUF_SIZE
	JNC .enough_space
	
	JSR IO_WRITE_BUFFER
	
.enough_space:
	mov write_buffer,RB, RA
	INC write_buffer_pos
	
.end:	
	RTS

IO_WRITE_BUFFER:
	mov RB, write_buffer_pos
	OR  RB, #0
	JZS .end
	
	PSH RA
	PSH RC
	PSH RD
	
	mov write_struct_size, RB
	mva RCD, write_struct
	mov RA, outfile
	JSR OS_WRITE
	JCS MAIN_IO_ERROR_EXIT
	
	mov RB, #0
	mov write_buffer_pos, RB
	
	PUL RD
	PUL RC
	PUL RA
.end:	
	RTS
	
IO_ASM_SUBFILE:
	mov RB, infile
	PSH RB
	mov RB, eof_in
	PSH RB

	; save read buffer
	mov RB, #4
    PSB read_buffer_base+3, RB
	mov RB, #0
    PSB read_buffer+255, RB
	mov RB, #0
    PSB read_buffer+511, RB

	; save current filename
	mov RB, #MAX_WORD_LEN+1
	PSB current_file_name+MAX_WORD_LEN, RB
	
	mov infile, RA
	mov RA, #0
	mov eof_in, RA
	mov read_buffer_len, RA
	mov read_buffer_len+1, RA
	mov line_number+1, RA
	INC RA
	mov line_number, RA
	
	mva RCD, current_file_name
	JSR WORD_COPY_CURRENT_WORD
	
	JSR ASM_FILE
	
	mov RA, infile
	JSR OS_CLOSE

	; restore current filename
	mov RB, #MAX_WORD_LEN+1
	PLB current_file_name, RB

	; restore read buffer
	mov RB, #0
    PLB read_buffer+256, RB
	mov RB, #0
    PLB read_buffer, RB
	mov RB, #4
    PLB read_buffer_base, RB
    
	PUL RB
	mov eof_in, RB
	PUL RB
	mov infile, RB

	RTS

IO_SKIP_WHITE_SPACE_AND_LINE:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #10
	JNZ .end
	mov RA, eof_in
	OR  RA, #0
	JNZ .end
	JSR IO_GETC
	JMP IO_SKIP_WHITE_SPACE_AND_LINE
.end:	
	RTS
	
IO_SKIP_WHITE_SPACE:
	JSR IO_GOTC
.check:	
	STC
	CMP RA, #" "
	JZS .white_space
	STC
	CMP RA, #9 ; tabulator
	JNZ .end
.white_space:
	JSR IO_GETC
	JMP .check
.end:	
	RTS

IO_END_LINE:
	JSR IO_GOTC
.loop:	
	STC
	CMP RA, #10 ; line feed
	JZS .end_OK
	JSR IO_GETC
	JMP .loop
.end_OK:	
	CLC
	RTS

IO_GOTC:
	mov RA, last_char
	RTS

IO_GETC:
	mov RA, infile
	JSR IO_GETC_BUFFERED
	STC
	CMP RA, #13 ; skip carriage return
	JZS IO_GETC
	JCS .end_OK
	CMP RA, #10-1 ; c-flag = 0 => CMP #10 (Line feed)
	JNZ .end_OK
	INC line_number ; count lines
	ICC line_number+1
.end_OK:
	mov last_char, RA
	CLC
	RTS

IO_GETC_BUFFERED:
	DEC read_buffer_len
	DCC read_buffer_len+1
	JCS .get_byte
	
	mov RA, eof_in
	OR  RA, #0
	JNZ .eof
	
	PSH RD
	PSH RC
	mov RA, infile
	mva RCD, read_struct
	JSR OS_READ
	PUL RC
	PUL RD
    JNC .read_OK
	CMP RA, #EEOF
	JNZ MAIN_IO_ERROR_EXIT
.eof:	
	mov RA, #10 ; line feed
	mov eof_in, RA
	RTS

.read_OK:
	mov read_buffer_len, RA
	mov read_buffer_len+1, RB
	OR  RA, RB
	JZS .eof
	mov RA, read_struct_ptr
	mov read_buffer_pos, RA
	mov RA, read_struct_ptr+1
	mov read_buffer_pos+1, RA
	JMP IO_GETC_BUFFERED

.get_byte:
	mov RB, #0
	mov RA, [read_buffer_pos], RB
	INC read_buffer_pos
	ICC read_buffer_pos+1
	RTS
	
IO_INIT:
	mva RCD, default_outfile_name
	mov outfile_name_ptr, RC
	mov outfile_name_ptr+1, RD
	
	mov RA, #0
	mov infile_name_ptr, RA
	mov infile_name_ptr+1, RA

	mov infile, RA
	mov outfile, RA
	
	mov read_struct_size, RA
	mov write_struct_size+1, RA
	mov RA, #2
	mov read_struct_size+1, RA
	mov RA, #WRITE_BUF_SIZE
	mov write_struct_size, RA
	
	mva RCD, read_buffer
	mov read_struct_ptr, RC
	mov read_struct_ptr+1, RD
	
	mva RCD, write_buffer
	mov write_struct_ptr, RC
	mov write_struct_ptr+1, RD
	RTS

IO_OPEN_OUT:
	mov RC, outfile_name_ptr
	mov RD, outfile_name_ptr+1
IO_OPEN_OUT_NAME:
	mov RA, #O_WRITE + O_CREATE
	JSR OS_OPEN
	JCS MAIN_IO_ERROR_EXIT
	mov outfile, RA
	
	mov RA, #0
	mov write_buffer_pos, RA
	RTS

IO_OPEN_IN:
	mov RA, #0
	mov eof_in, RA
	mov read_buffer_len, RA
	mov read_buffer_len+1, RA
	
	mov RC, infile_name_ptr
	mov RD, infile_name_ptr+1
	mov RA, RC ; no input file given?
	OR  RA, RD
	JZS .stdin ; use stdin
	mov RA, #O_READ
	JSR OS_OPEN
	JCS MAIN_IO_ERROR_EXIT
	mov infile, RA
	mov RA, #0
	mov line_number+1, RA
	INC RA
	mov line_number, RA
	mov RA, #10
	mov last_char, RA
	RTS
	
.stdin:
	mov RA, #ERR_NO_INPUT
	JMP MAIN_ERROR_MSG_EXIT
	
IO_END:
	mov RA, outfile
	JSR IO_CHECK_CLOSE
	mov RA, #0
	mov outfile, RA
	; fall through to IO_END_IN
	
IO_CLOSE_IN:
	mov RA, infile
	JSR IO_CHECK_CLOSE
	mov RA, #0
	mov infile, RA
	RTS
	
IO_CHECK_CLOSE:
	STC
	CMP RA, #3 ; check for standard file
	JNC .end  ; no close for standard files
	JSR OS_CLOSE
	JNC .end
	JSR MAIN_IO_ERROR_EXIT
.end:	
	RTS

#bank data
eof_in: #res 1
infile_name_ptr: #res 2
outfile_name_ptr: #res 2
infile: #res 1
outfile: #res 1

line_number: #res 2
last_char: #res 1

read_struct:
read_struct_size: #res 2
read_struct_ptr: #res 2

write_struct:
write_struct_size: #res 2
write_struct_ptr: #res 2

read_buffer_base:
read_buffer_len: #res 2
read_buffer_pos: #res 2
read_buffer: #res 512
;read_buffer: #res READ_BUF_SIZE

write_buffer_base:
write_buffer_pos: #res 1
write_buffer: #res WRITE_BUF_SIZE
