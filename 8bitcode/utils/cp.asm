#include "8bit.cpu"

; UNIX cp command
;
; copy a file

#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x00
}

#bankdef data
{
    #addr 0x1000
    #size 0xC000
}

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

COPY_BUFFER_SIZE = 512*4

#include "OSCalls.inc"

#bank pgm

START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD
	
	mov RA, #0
	mov src_file_num, RA
	mov dest_file_num, RA
	
	mov RA, #1
	JSR GET_PARAMETER
	JCS SHOW_USAGE
	mov src_name_ptr, RC
	mov src_name_ptr+1, RD
	
	mov RA, #2
	JSR GET_PARAMETER
	JCS SHOW_USAGE
	mov dest_name_ptr, RC
	mov dest_name_ptr+1, RD
	
.open_src:	
	mov RA, #O_READ
	mov RC, src_name_ptr
	mov RD, src_name_ptr+1
	JSR OS_OPEN
	JCS .error_end
	mov src_file_num, RA
	
.open_dest:	
	mov RA, #O_WRITE | O_CREATE
	mov RC, dest_name_ptr
	mov RD, dest_name_ptr+1
	JSR OS_OPEN
	JCS .error_end
	mov dest_file_num, RA

.start_copy:
	mva RCD, copy_buffer
	mov read_write_structure+2, RC
	mov read_write_structure+3, RD
	
.copy_loop:
	mov RC, #COPY_BUFFER_SIZE[7:0]
	mov RD, #COPY_BUFFER_SIZE[15:8]
	mov read_write_structure, RC
	mov read_write_structure+1, RD
	mov RA, src_file_num
	mva RCD, read_write_structure
	JSR OS_READ
	JCS .error_end
	
	mov read_write_structure, RA
	mov read_write_structure+1, RB
	OR  RA, RB
	JZS .close_end
	
	mov RA, dest_file_num
	mva RCD, read_write_structure
	JSR OS_WRITE
	JNC .copy_loop	
	
.error_end:
	JSR SHOW_ERROR
	
.close_end:
	mov RA, src_file_num
	OR  RA, #0
	JZS .close_dest
	JSR OS_CLOSE
	JNC .close_dest
	JSR SHOW_ERROR
	
.close_dest:	
	mov RA, dest_file_num
	OR  RA, #0
	JZS .end
	JSR OS_CLOSE
	JNC .end
	JSR SHOW_ERROR
	
.end:	
	RTS

SHOW_USAGE:
	mva RCD, usage
	JMP PUT_ERROR_STRING

SHOW_ERROR:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JMP PUT_ERROR_HEX_BYTE	


#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
;	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	

fail_msg: #d "failed \0"
usage: #d "cp src_filename dest_filename\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
src_name_ptr: #res 2
dest_name_ptr: #res 2
src_file_num: #res 1
dest_file_num: #res 1
read_write_structure: #res 4
copy_buffer: #res COPY_BUFFER_SIZE
DATA_END:	