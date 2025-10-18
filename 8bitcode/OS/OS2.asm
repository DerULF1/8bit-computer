#include "8bit.cpu"

;
; 8-bit Computer Operating System
;
; Written 27.07.2020 Ulf Caspers
;

;#bankdef "boot"
;{
;    #addr 0x0000
;    #size 0x0100
; #outp 0
;}

; Memory Start Address for Kernel Modules
MODULES_START = 0xC000

; Begin of Kernel Data Area
DATA_START    = 0xD600

; allocatable 512 Byte pages
MEM_MAX_PAGES = (MODULES_START/256) / 2

; number of task specific bytes
OS_SAVE_AREA_SIZE = 16

#bankdef pgm
{
    #addr 0xE000
    #size 0x2000
;	#outp 0x0100
	#outp 0
}

#bankdef task_data
{
    #addr DATA_START
    #size OS_SAVE_AREA_SIZE
}

#bankdef data
{
    #addr DATA_START+OS_SAVE_AREA_SIZE
    #size 0xE000-DATA_START-OS_SAVE_AREA_SIZE
}

;#bank boot
;
;	mov RA, #0x00
;	mov copy_from_pointer, RA
;	mov RA, #0x01
;	mov copy_from_pointer+1, RA
	
;	mov RC, #0
;	mov RD, #0xE0
	
;	mov RB, #0
;	CLC
;.loop:
;	mov RA, [copy_from_pointer],RB
;	mov [RCD], RA
;	ADD RB, #1
;	JNC .nocarry
;	mov RA, copy_from_pointer+1
;	ADD RA, #0
;	mov copy_from_pointer+1, RA
;.nocarry:
;	ADD RC, #1
;	ADD RD, #0
;	JNC .loop
;	JMP RESET

;copy_from_pointer: #res 2

;#bank task_data
os_save_area: ;#res OS_SAVE_AREA_SIZE

#bank data
os_save_copy: #res OS_SAVE_AREA_SIZE


#bank pgm

#include "OSCalls.inc"
#bank pgm
#include "LCDDisplay.inc"	
#bank pgm
#include "SPI.inc"	
#bank pgm
#include "SDCard.inc"	
#bank pgm
#include "Clock.inc"	
#bank pgm
#include "FAT.inc"	
#bank pgm
#include "KeyNames.inc"	
#bank pgm
#include "Keyboard.inc"	
#bank pgm
#include "Memory.inc"	
#bank pgm
#include "relocate.inc"
#bank pgm
#include "RelocTab.inc"
#bank pgm
#include "taskinfo.inc"
#bank pgm
#include "taskenvg.inc"
#bank pgm
#include "Tasks.inc"
#bank pgm
#include "Locks.inc"
#bank pgm
#include "pipe.inc"
#bank pgm
#include "PathSrch.inc"
#bank pgm
#include "SCB.inc"
#bank pgm
 
verstime:
#d incbin("asmtime.txt") @ "\n"
boot_dir_name: #d "/boot" @ incbin("osnum.txt") @ "\0"

init_keyboard_msg: #d "\nInit Keyboard...\0"
init_card_msg: #d "\nInit SD Card...\n\0"
ready_msg: #d "8-bit CPU ready\0"
fail_msg: #d " fail \0"
mod_too_large: #d "ERROR modend: \0"

sys_con_pref: #d "/con\0"
sys_pipe_pref: #d "/pipe\0"
sys_init_con_file: #d "/con0\0"
sys_init_shell: #d "/shell\0\0"

path_var_name: #d "PATH\0"

FAT_FS_LOCK = 0
EXEC_LOCK = 1
PIPE_LOCK = 2


;
; Get pointer to version string of current OS
;
SYS_GET_VERSION_STRING:
	mva RCD, verstime
	RTS

;
; Analyse file name to find stream type
;
; parm:
;  RCD - pointer to file name
; Result:
;  RB - SCB_TYPE
;
SYS_GET_FILE_TYPE:
	PSH RA
	PSH RC
	PSH RD
	
	mov RB, #0
.cloop:	
	mov RA, sys_con_pref,RB
	OR  RA, #0
	JZS .con_type
	STC
	CMP RA, [RCD]
	JNZ .no_con_type
	INC RCD
	INC RB
	JMP .cloop
.con_type:	
	mov RB, #SCB_CON_FS
	JMP .end
	
.no_con_type:	
	PUL RD
	PUL RC
	PSH RC
	PSH RD
	
	mov RB, #0
.ploop:	
	mov RA, sys_pipe_pref,RB
	OR  RA, #0
	JZS .pipe_type
	STC
	CMP RA, [RCD]
	JNZ .no_pipe_type
	INC RCD
	INC RB
	JMP .ploop
.pipe_type:	
	mov RB, #SCB_PIPE_FS
	JMP .end
	
.no_pipe_type:
	mov RB, #SCB_FAT_FS
	
.end:	
	PUL RD
	PUL RC
	PUL RA
	RTS

;
; open a file descriptor
; 
; Parms:
;  RC/RD - pointer to null terminated file name
;  RA    - flags
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - filedescriptor / error number
;
SYS_OPEN:
	; open the stream internally
	JSR SYS_OPEN_STREAM
	JCS .end

	; convert to file descriptor
	PSH RD
	PSH RC
	PSH RA
	JSR TASK_CREATE_FD
	PUL RB
	JNC .end_pull
	
	PSH RA ; save error number
	mov RA, RB ; restore stream number
	JSR SYS_CLOSE_STREAM
	PUL RA ; restore error number
	STC
.end_pull:	
	PUL RC
	PUL RD
.end:	
	RTS

;
; open stream
; 
; Parms:
;  RC/RD - pointer to null terminated file name
;  RA    - flags
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - stream number / error number
;
SYS_OPEN_STREAM:
	JSR SYS_GET_FILE_TYPE
	STC
	CMP RB, #SCB_CON_FS
	JZS SYS_CON_OPEN
	STC
	CMP RB, #SCB_PIPE_FS
	JZS SYS_PIPE_OPEN
	
	; fall through to SYS_FS_OPEN
	
SYS_FS_OPEN:
	JSR SYS_LOCK_FAT_FS

	PSH RA
	PSH RC
	PSH RD	

	; allocate file_buffer
	mov RB, #SYS_OWNER
	JSR MEM_ALLOC_SINGLE
	
	mov RC, #0
	mov fat_file_buffer_pointer, RCD
	PUL RD
	PUL RC
	PUL RB
	JCS .unlock_end 
	mov RA, RB
	
	JSR [fat_fs_open_ptr]
	JCS .open_not_ok
	
	JSR SCB_GET_FREE_SCB	
	JNC .find_OK
	
	PSH RA
	JSR [fat_fs_close_ptr]
	PUL RA
	
.open_not_ok:	
	PSH RA
	; free file buffer 
	mov RD, fat_file_buffer_pointer+1
	mov RB, #SYS_OWNER
	JSR MEM_FREE_SINGLE
	PUL RA
	
	STC
	JMP SYS_UNLOCK_FAT_FS
	
.find_OK:	
	PSH RA
	mov RA, #SCB_FAT_FS
	mov scb_area+SCB_OFF_TYPE,RB, RA
	JSR SCB_COPY_FROM_FAT_ENTRY
	PUL RA
.unlock_end:	
	JMP SYS_UNLOCK_FAT_FS

SYS_CON_OPEN:
	; get console number from file name
	; /con0
	; /con1
	; etc.
	CLC
	ADD RC, #4
	ADD RD, #0
	mov RA, [RCD]
	AND RA, #7
	PSH RA
	JSR [con_open_ptr]
	PUL RC
	JCS .end
	
	JSR SCB_GET_FREE_SCB	
	JCS .end
	PSH RA
	mov RA, #SCB_CON_FS
	mov scb_area+SCB_OFF_TYPE,RB, RA
	mov RA, RC
	mov scb_area+SCB_OFF_CON_NR,RB, RA
	PUL RA
.end:	
	RTS
	
SYS_PIPE_OPEN:
	JSR SYS_LOCK_PIPE
	JSR PIPE_OPEN
	JCS .end
	
	JSR SCB_GET_FREE_SCB	
	JCS .end
	PSH RA
	mov RA, #SCB_PIPE_FS
	mov scb_area+SCB_OFF_TYPE,RB, RA
	JSR SCB_COPY_FROM_PIPE_ENTRY
	PUL RA
	CLC
.end:	
	JMP SYS_UNLOCK_PIPE

;
; write a single byte to a stream
; 
; Parms:
;  RA    - stream number
;  RC    - byte to be written
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error mumber
;
SYS_PUTC:
	
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	mov RA, scb_area+SCB_OFF_TYPE,RB
	
	STC
	CMP RA, #SCB_CON_FS
	JZS SYS_CON_PUTC
	STC
	CMP RA, #SCB_PIPE_FS
	JZS SYS_PIPE_PUTC
	
	JSR SYS_LOCK_FAT_FS
	
	; single byte write
	mov file_byte, RC
	mov RA, #0
	mov file_read+1, RA
	INC RA
	mov file_read, RA
	mva RCD, file_byte
	mov file_read+2, RC
	mov file_read+3, RD
	mva RCD, file_read
	JMP SYS_WRITE_INT

SYS_CON_PUTC:
	mov RA, scb_area+SCB_OFF_CON_NR,RB
	
	PSF
	SII
	JSR [con_out_ptr]
	JCS .error_end
	PLF
	CLC
	RTS
.error_end:
	PLF
	STC
	RTS

SYS_PIPE_PUTC:
	JSR SYS_LOCK_PIPE
	PSH RB ; SCB index
	PSH RC ; char to be written
	JSR SCB_COPY_TO_PIPE_ENTRY
	PUL RA ; restore char
	
	PSH RA ; save char
	JSR PIPE_PUTC
	PUL RC ; restore char
	PUL RB ; restore SCB index
	JNC .leave
	
	JSR SYS_UNLOCK_PIPE
	
	CMP RA, #EAGAIN
	JNZ .error_end
	
	JSR TASK_FORCE_SWITCH
	JMP SYS_PIPE_PUTC
	
.error_end:
	STC
	RTS
	
.leave:	
	JSR SCB_COPY_FROM_PIPE_ENTRY
	CLC
.end:
	JMP SYS_UNLOCK_PIPE

;
; read a single byte from stream
; 
; Parms:
;  RA    - stream number
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - byte read or error mumber
;
SYS_GETC:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	mov RA, scb_area+SCB_OFF_TYPE,RB
	STC
	CMP RA, #SCB_CON_FS
	JZS SYS_CON_GETC
	STC
	CMP RA, #SCB_PIPE_FS
	JZS SYS_PIPE_GETC
	
	JSR SYS_LOCK_FAT_FS

	; single byte read
	mov RA, #0
	mov file_read+1, RA
	INC RA
	mov file_read, RA
	mva RCD, file_byte
	mov file_read+2, RCD
	mva RCD, file_read
	JSR SYS_READ_INT
	JCS .end
	OR  RA, RB
	JNZ .read_OK
	STC
	mov RA, #EEOF ; end of file
	JMP .end
.read_OK:
	mov RA, file_byte
	CLC
.end:	
	JMP SYS_UNLOCK_FAT_FS
	
SYS_CON_GETC:
	mov RA, scb_area+SCB_OFF_CON_NR,RB
.loop:
	PSF ; save interrupt inhibit state
	SII
	PSH RA
	JSR [con_in_ptr] 
	PUL RC
	JNC .ok_end
	PLF ; restore interrupt inhibit state
	STC
	CMP RA, #EAGAIN
	JNZ .error_end
	mov RA, RC
	JSR TASK_FORCE_SWITCH
	JMP .loop
.error_end:
	STC
	RTS
.ok_end:
	PLF
	CLC
	RTS

SYS_PIPE_GETC:
	JSR SYS_LOCK_PIPE
	PSH RB
	JSR SCB_COPY_TO_PIPE_ENTRY
	
	JSR PIPE_GETC	
	PUL RB
	JNC .leave
	
	JSR SYS_UNLOCK_PIPE
	
	CMP RA, #EAGAIN
	JNZ .error
	
	JSR TASK_FORCE_SWITCH
	JMP SYS_PIPE_GETC
	
.error:
	STC
	RTS

.leave:
	PSH RA
	JSR SCB_COPY_FROM_PIPE_ENTRY
	PUL RA
	CLC
.end:
	JMP SYS_UNLOCK_PIPE

SYS_GET_KEY:
	; get console number
	mov RB, #TCB_OFF_CONS
	mov RA, [task_active_block],RB
	
	PSF
	SII
	JSR [con_in_ptr]
	JNC .got_key
.nokey:
	mov RA, #0
.got_key:	
	PLF
	CLC
	RTS
	
;
; read from stream
; 
; Parms:
;  RA    - stream number
;  RC/RD - pointer to read_structure
;
; read_structure
;  buf_size    #d16
;  buf_pointer #d16
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA/RB - number of bytes read, 0x0000 = EOF
;
SYS_READ:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	JSR SYS_LOCK_FAT_FS
	JSR SYS_READ_INT
	JMP SYS_UNLOCK_FAT_FS

SYS_READ_INT:	
	PSH RB
	JSR SCB_COPY_TO_FAT_ENTRY
	
	JSR [fat_fs_read_ptr]
	
	PUL RC
	JMP SYS_IO_END
	
;
; write to stream
; 
; Parms:
;  RA    - stream number
;  RC/RD - pointer to write_structure
;
; write_structure
;  buf_size    #d16
;  buf_pointer #d16
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA/RB - number of bytes written
;
SYS_WRITE:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	JSR SYS_LOCK_FAT_FS
	
SYS_WRITE_INT:	
	PSH RB
	JSR SCB_COPY_TO_FAT_ENTRY
	JSR [fat_fs_write_ptr]
	PUL RC	
	JSR SYS_IO_END
	JMP SYS_UNLOCK_FAT_FS	
	
SYS_IO_END:
	PSF
	PSH RB
	PSH RA
	mov RB, RC
	JSR SCB_COPY_FROM_FAT_ENTRY
	PUL RA
	PUL RB
	PLF

AN_RTS:
	RTS	
	
;
; close stream
; 
; Parms:
;  RA - file descriptor
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error
;
SYS_CLOSE:
	PSH RA ; save file descriptor
	JSR TASK_FD_TO_SD
	JCS .error_end

	JSR SYS_CLOSE_STREAM
	JCS .error_end
	
	PUL RA
	JMP TASK_REMOVE_FD

.error_end:
	PUL RB
	RTS
	
SYS_OK_END:
	CLC
	RTS
	
;
; close stream
; 
; Parms:
;  RA - stream number
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error
;
SYS_CLOSE_STREAM:
	JSR SCB_GET_INDEX
	JCS AN_RTS
	
	; decrease use count
	DEC scb_area+SCB_OFF_COUNT,RB
	; not null then skip close
	JNZ SYS_OK_END
	
.do_close:	
	mov RA, scb_area+SCB_OFF_TYPE,RB
	STC
	CMP RA, #SCB_CON_FS
	JZS SYS_CON_CLOSE
	STC
	CMP RA, #SCB_PIPE_FS
	JZS SYS_PIPE_CLOSE

	; fall through to SYS_FS_CLOSE	
	
SYS_FS_CLOSE:
	JSR SYS_LOCK_FAT_FS

	PSH RB
	JSR SCB_COPY_TO_FAT_ENTRY
	PUL RB
	
	mov RA, #SCB_FREE_ENTRY
	mov scb_area+SCB_OFF_TYPE,RB, RA
	
	JSR [fat_fs_close_ptr]
	PSF
	PSH RA

	; free file buffer 
	mov RD, fat_file_buffer_pointer+1
	mov RB, #SYS_OWNER
	JSR MEM_FREE_SINGLE

	PUL RA
	PLF	
.end:	
	JMP SYS_UNLOCK_FAT_FS

SYS_CON_CLOSE:	
	mov RA, scb_area+SCB_OFF_CON_NR, RB
	mov RC, RA
	mov RA, #SCB_FREE_ENTRY
	mov scb_area+SCB_OFF_TYPE,RB, RA
	mov RA, RC
	JMP [con_close_ptr]

SYS_PIPE_CLOSE:
	JSR SYS_LOCK_PIPE
	mov RA, #SCB_FREE_ENTRY
	mov scb_area+SCB_OFF_TYPE,RB, RA

	JSR SCB_COPY_TO_PIPE_ENTRY

	JSR PIPE_CLOSE		
.end:	
	JMP SYS_UNLOCK_PIPE

;
; set file pointer to new position
; 
; Parms:
;  RA    - stream number
;  RC/RD - pointer to new file pointer (4 bytes)
;
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error code
;
SYS_SEEK:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS

	JSR SYS_LOCK_FAT_FS
	
	PSH RB
	JSR SCB_COPY_TO_FAT_ENTRY
	JSR [fat_fs_seek_ptr]
	PUL RC
	JSR SYS_IO_END
	JMP SYS_UNLOCK_FAT_FS

;
; get current file pointer
; 
; Parms:
;  RA    - stream number
;
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error code
;  RA/RB/RC/RD - current file pointer
;
SYS_TELL:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS

	JSR SYS_LOCK_FAT_FS
	
	JSR SCB_COPY_TO_FAT_ENTRY
	JSR [fat_fs_tell_ptr]
	JMP SYS_UNLOCK_FAT_FS


;
; get current file status
; 
; Parms:
;  RA    - stream number
;  RC/RD - pointer to dir entry buffer (32 bytes)
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error code
;
SYS_STAT:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	JSR SYS_LOCK_FAT_FS
	
	JSR SCB_COPY_TO_FAT_ENTRY
	JSR [fat_fs_stat_ptr]
	JMP SYS_UNLOCK_FAT_FS


;
; open directory list
; 
; Parms:
;  RA -  open mode (O_READ/O_WRITE/O_CREAT/O_LISTALL)
;  RC/RD - pointer to null terminated directory name
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - directory number, c=1: error number
;
SYS_OPEN_DIR:
	OR  RA, #O_DIR
	JMP SYS_OPEN
	
;
; read next directory entry
; 
; Parms:
;  RA    - directory number
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error number
;  RC/RD - pointer to next entry, 0x0000 = EOF
;
SYS_NEXT_DIR:
	JSR SCB_GET_INDEX_FOR_FD
	JCS AN_RTS
	
	JSR SYS_LOCK_FAT_FS
	
	PSH RB
	JSR SCB_COPY_TO_FAT_ENTRY
	
	mov RA, fat_file_active_cluster
	mov fat_work_cluster, RA
	mov RA, fat_file_active_cluster+1
	mov fat_work_cluster+1, RA
	mov RA, fat_file_pointer
	mov fat_work_file_pointer, RA
	mov RA, fat_file_pointer+1
	mov fat_work_file_pointer+1, RA
	
	; ensure active sector is in buffer
	JSR FAT_CLUSTER_FILE_POINTER_TO_DIR_SECTOR	
	JSR FAT_READ_DIR_SECTOR	
	PUL RB
	JCS SYS_UNLOCK_FAT_FS

	
.read_loop:	
	PSH RB
	JSR FAT_NEXT_DIR_ENTRY
	
	PUL RB
	JCS SYS_UNLOCK_FAT_FS
	
	; return all directory entries?
	mov RA, fat_file_mode
	AND RA, #O_LISTALL
	JNZ .entry_found
	
.check_entry:		
	; end of directory mark?
	mov RA, [RCD]
	OR  RA, #0
	JZS .end_of_dir
	
	; unused directory entry?
	XOR RA, #0xe5
	JZS .read_loop
	
	; special file entry?
	CLC
	ADD RC, #11
	ADD RD, #0
	mov RA, [RCD]
	AND RA, #0x0f
	XOR RA, #0x0f
	JZS .read_loop
	
	STC
	SUB RC, #11
	SUB RD, #0
	
.entry_found:
	; copy entry to file buffer
	PSH RB
	mov RB, #0
.cloop:	
	mov RA, [RCD]
	mov [fat_file_buffer_pointer],RB, RA
	INC RCD
	INC RB
	STC
	CMP RB, #32
	JNC .cloop
	mov RCD, fat_file_buffer_pointer
	PUL RB

.exit_entry:	
	; save entry pointer
	mov RA, fat_work_file_pointer
	mov fat_file_pointer, RA
	mov RA, fat_work_file_pointer+1
	mov fat_file_pointer+1, RA
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	
	JSR SCB_COPY_FROM_FAT_ENTRY
	JMP SYS_UNLOCK_FAT_FS
		
.end_of_dir:	
	mov RC, #0
	mov RD, RC
	JMP .exit_entry
	
;
; close directory
; 
; Parms:
;  RA - directory number
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error number
;
SYS_CLOSE_DIR:
	JMP SYS_CLOSE
	
;
; make new sub directory
; 
; Parms:
;  RC/RD - pointer to 0-terminated absolute path name
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error number
;
SYS_MKDIR:
	JSR SYS_LOCK_FAT_FS
	JSR [fat_fs_mkdir_ptr]
	JMP SYS_UNLOCK_FAT_FS
	
;
; delete file or sub directory
; 
; Parms:
;  RC/RD - pointer to 0-terminated absolute path name
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error number
;
SYS_UNLINK:
	JSR SYS_LOCK_FAT_FS
	JSR [fat_fs_unlink_ptr]
	JMP SYS_UNLOCK_FAT_FS
	
;
; get file system status
; 
; Parms:
;  RC/RD - pointer to file system status area (5 bytes)
; 
; Returns:
;  c-flag - 0 = O.K., 1 = error
;  RA - error number
;
SYS_FSSTAT:
	JSR SYS_LOCK_FAT_FS
	JSR [fat_fs_fsstat_ptr]
	JMP SYS_UNLOCK_FAT_FS

;
; Lock the FAT file system for the active task
;
SYS_LOCK_FAT_FS:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #FAT_FS_LOCK
	JSR LOCK_REQUEST
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS
	
;
; Release the FAT file system for the active task
;
SYS_UNLOCK_FAT_FS:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #FAT_FS_LOCK
	JSR LOCK_RELEASE
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS

;
; Lock the FAT file system for the active task
;
SYS_LOCK_PIPE:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #PIPE_LOCK
	JSR LOCK_REQUEST
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS
	
;
; Release the FAT file system for the active task
;
SYS_UNLOCK_PIPE:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #PIPE_LOCK
	JSR LOCK_RELEASE
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS
	
;
; Allocate memory pages
;
; Parm:
;  RC - number of requested contignous pages
; Result:
;  c-flag: 0 = O.K., 1 = error
;  RD - high address of 1st page (c=0)
;  RA - error number (c=1)
;
SYS_MEM_ALLOC:
	mov RB, task_active
	JMP MEM_ALLOC
	
;
; Free memory pages
;
; Parm:
;  RC - number of requested contignous pages
;  RD - high address of 1st page
;  
SYS_MEM_FREE:
	mov RB, task_active
	JMP MEM_FREE

SYS_ADD_MODULES:
	mov RA, #MODULES_START[7:0]
	mov pgm_start, RA
	mov RA, #MODULES_START[15:8]
	mov pgm_start+1, RA
	
	mva RCD, DATA_END
	mov data_mem_start, RCD
	
	mva RCD, boot_dir_name
	JSR FAT_FIND_DIR_ENTRY
	JCS .end
.search_loop:
	JSR FAT_NEXT_DIR_ENTRY
	JCS .end_OK
	
	mov RA, [RCD]
	OR  RA, #0
	JZS .end_OK
		
	; empty entry mark
	XOR RA, #0xe5 
	JZS .search_loop
	
	; file name extension "MOD"?
	CLC
	ADD RC, #8
	ADD RD, #0
	mov RA, [RCD]
	XOR RA, #"M"
	JNZ .search_loop
	
	INC RCD
	mov RA, [RCD]
	XOR RA, #"O"
	JNZ .search_loop
	
	INC RCD
	mov RA, [RCD]
	XOR RA, #"D"
	JNZ .search_loop
	
	JSR SYS_ADD_MODULE
	JCS .end

	JMP .search_loop	
.end_OK:
	CLC
.end:
	RTS
	
SYS_READ_PROGRAM_TRAILER:
	; get start of data section from program trailer
	mov RCD, pgm_end
	STC
	SUB RC, #4
	SUB RD, #0
	mov RA, [RCD]
	mov data_section_start, RA
	INC RCD
	mov RA, [RCD]
	mov data_section_start+1, RA
	
	; get size of data section from program trailer
	INC RCD
	mov RA, [RCD]
	INC RCD
	mov RB, [RCD]
	
	mov data_section_size, RA
	mov data_section_size+1, RB
	
	; calculate end of data section in program
	ADD RA, data_section_start
	mov data_section_end, RA
	ADD RB, data_section_start+1
	mov data_section_end+1, RB
	
	RTS
	
SYS_ADD_MODULE:	
	; read start cluster
	CLC
	ADD RC, #FAT_DIRENT_FSTCLUSLO-10
	ADD RD, #0
	mov RA, [RCD]
	mov fat_work_cluster, RA
	INC RCD
	mov RA, [RCD]
	mov fat_work_cluster+1, RA
	
	; set start sector
	PSH RC
	PSH RD
	JSR FAT_CLUSTER_TO_SECTOR	
	mov sdcard_block_address, RA
	mov sdcard_block_address+1, RB
	mov sdcard_block_address+2, RCD
	JSR SDCARD_LBLOCK_TO_ADDRESS
	PUL RD
	PUL RC
	
	; read file size
	INC RCD
	mov RA, [RCD] ; low byte
	INC RCD
	ADD RA, pgm_start
	mov pgm_end, RA
	mov RA, [RCD] ; high byte
	mov RB, RA
	ADD RA, pgm_start+1
	mov pgm_end+1, RA
	
	STC
	CMP RA, #(DATA_START/256)+1
	JNC .end_pgm_OK
	
	mva RCD, mod_too_large
	JSR DISPLAY_STRING
	mov RA, pgm_end+1
	JSR DISPLAY_HEX_BYTE
	mov RA, pgm_end
	JSR DISPLAY_HEX_BYTE
	HLT

.end_pgm_OK:	
	; calculate number of sectors
	LSR RB
	INC RB
	
	mov RA, pgm_start
	mov sdcard_buffer_pointer, RA
	mov RA, pgm_start+1
	mov sdcard_buffer_pointer+1, RA
	
.read_loop:
	PSH RB
	JSR SDCARD_READ_BLOCK
	PUL RB
	JCS .end
	
	mov sdcard_buffer_pointer, RC
	mov sdcard_buffer_pointer+1, RD
	
	; advance byte address by 512 bytes
	INC sdcard_block_address+1
	INC sdcard_block_address+1
	ICC sdcard_block_address+2 
	ICC sdcard_block_address+3
	
	DEC RB
	JNZ .read_loop
	
	JSR SYS_READ_PROGRAM_TRAILER
	
	JSR RELOCATE
	JCS .end
	JSR [pgm_start]
	JCS .end
	
	mov RA, pgm_end+1
	INC RA
	mov pgm_start+1, RA
	
	mov RA, data_mem_start
	ADD RA, data_section_size
	mov data_mem_start, RA
	mov RA, data_mem_start+1
	ADD RA, data_section_size+1
	mov data_mem_start+1, RA
		
.end_OK:
	CLC
.end:
	RTS

;
; IRQ routine - from pointer 0xFFFE/0xFFFF
;
IRQ:
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	
	JSR KEYBOARD_READ_BYTE_COND
	OR  RA, #0
	JZS .nokey
	JSR KEYBOARD_BYTE_TO_KEY
	mov RA, keyboard_special_key
	OR  RA, #0
	JZS IRQ_END
	OUT RA
	DEC RA
	JSR [con_visible_ptr]
	JNC IRQ_END
	CMP RA, #EBADF
	JNZ IRQ_END
	mov RA, keyboard_special_key
	DEC RA
	JSR SYS_START_CONSOLE
	JMP IRQ_END
	
.nokey:	
	INC sys_ticks
	ICC sys_ticks+1
	ICC sys_ticks+2	

IRQ_NEXT_TASK:
	mov RB, #OS_SAVE_AREA_SIZE
	PSB os_save_area+OS_SAVE_AREA_SIZE-1, RB

	mov RB, task_active
.loop:
	INC RB
	AND RB, #TASK_MAX_NUM-1
	mov RA, task_status,RB
	OR  RA, #0
	JNN .task_found ; not TASK_ST_INACTIVE
	STC
	CMP RB, task_active
	JNZ .loop
	
	; no active task?
	; that shouldn't happen
	mov RA, #30
	OUT RA
	HLT
	
.task_found:
	STC
	CMP RB, task_active
	JZS .add_tick ; only one active task
	
	; save old stack pointer
	mov RCD, SP
	PSH RB
	
	mov RB, task_active
	mov RA, task_status,RB
	OR  RA, #0
	JNS .switch ; old task inactive?
	
	JSR TASK_SET_STACK_POINTER
	
.switch:
	; load new stack pointer
	PUL RB
	mov task_active, RB
	JSR TASK_GET_STACK_POINTER
	mov SP, RCD
	
	JSR TASK_GET_BLOCK_POINTER
	mov task_active_block, RCD
	
.add_tick:
	mov RB, #TCB_OFF_TICKS
	INC [task_active_block],RB
	JNC .end
	INC RB
	INC [task_active_block],RB
	JNC .end
	INC RB
	INC [task_active_block],RB

.end:	
	mov RB, #OS_SAVE_AREA_SIZE
	PLB os_save_area, RB
	
IRQ_END:	
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	RTI

;
; RESET routine - from pointer 0xFFFC/0xFFFD
;
RESET:	
	SII
	mov RC, #0xff
	;mov RD, #0xbf
	mov RD, RC
	mov SP,RCD
	
		;mov RA, #RESET[7:0]
		;mov 0xfffc, RA
		;mov RA, #RESET[15:8]
		;mov 0xfffd, RA
	
	mov RA, #1
	OUT RA
	JSR SCB_INIT
	JSR MEM_INIT
	JSR TASK_INIT
	JSR LOCK_INIT

	mov RA, #2
	OUT RA
	JSR DISPLAY_INIT
	
	mov RA, #3
	OUT RA
	JSR SYS_GET_VERSION_STRING
	JSR DISPLAY_STRING
	
	mov RA, #4
	OUT RA
	mva RCD, init_keyboard_msg
	JSR DISPLAY_STRING
	JSR KEYBOARD_INIT
	
		;mov RA, #IRQ[7:0]
		;mov 0xfffe, RA
		;mov RA, #IRQ[15:8]
		;mov 0xffff, RA

	mov RA, #5
	OUT RA
	mva RCD, init_card_msg
	JSR DISPLAY_STRING
	
	JSR FAT_INIT
	JNC .init_OK
	
	mov RB, #1
	
.init_err:
	PSH RA
	mov RA, RB
	JSR DISPLAY_HEX_BYTE
	mov RA, #":"
	JSR DISPLAY_CHAR
	PUL RA
	JSR SYS_SHOW_ERROR
	JMP .end

.init_OK:
	mov RA, #6
	OUT RA
	
	JSR SYS_ADD_MODULES
	JNC SYS_INIT_TASK

	mov RB, #2
	JMP .init_err
	
.end:	
	HLT
	
SYS_START_CONSOLE:
	PSH RA
	mva RCD, sys_con_pref
	mov RB, #0
.loop:	
	mov RA, [RCD]
	mov sys_con_filename,RB, RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .loop
	
.setnumber:	
	mov sys_con_filename,RB, RA
	PUL RA
	PSH RA
	CLC
	ADD RA, #"0"
	mov sys_con_filename-1,RB, RA
	
	mva RCD, sys_con_filename
	mov RA, #O_READ + O_WRITE
	JSR SYS_OPEN
	JCS .endp1
	
	mov sys_fork_filedesc, RA
	mov sys_fork_filedesc+1, RA
	mov sys_fork_filedesc+2, RA
		
.init_shell:	
	mva RCD, sys_fork_filename
	mov fork_aux_pointer, RCD
	mva RCD, sys_init_shell
	JSR SYS_COPY_STRING_LIST
	
.start_con:
	mva RCD, sys_fork_area
	
	PUL RA
	JMP SYS_FORK_CONS

.endp2:
	PUL RC
.endp1:
	PUL RC
.end:	
	RTS

SYS_INIT_TASK:
	mov RA, #7
	OUT RA
	
	; start task switch timer
	JSR CLK_INIT_SQW

	; reset system tick count
	mov RA, #0
	mov sys_ticks, RA
	mov sys_ticks+1, RA
	mov sys_ticks+2, RA

	; create new task control block
	JSR TASK_NEW
	mov task_active, RB
	PSH RB
	
	JSR TASK_GET_BLOCK_POINTER
	mov task_active_block, RCD

	; switch console ID
	mov RA, #0
	;mov RB, #TCB_OFF_CONS
	mov [task_active_block],RB, RA
	JSR SYS_START_CONSOLE
	mov RA, #9
	OUT RA

	; switch console ID
	mov RA, #1
	;mov RB, #TCB_OFF_CONS
	mov [task_active_block],RB, RA
	JSR SYS_START_CONSOLE
	mov RA, #10
	OUT RA
	
	PUL RB
	JSR TASK_REMOVE
	JMP TASK_FORCE_SWITCH

.init_error:
	JSR SYS_SHOW_ERROR
	HLT

;
; Wait for a task to end
;
; Parm:
;  RC/RD - task ID
; Result:
;
;
SYS_WAIT_TASK:
	PSH RD
	PSH RC
	JSR TASK_FIND_TID
	JCS .end
	JSR TASK_FORCE_SWITCH
	PUL RC
	PUL RD
	JMP SYS_WAIT_TASK
.end:
	PUL RC
	PUL RD
	RTS

;
; Start new task
;
; Parm:
;  RCD - pointer to fork structure:
;        3 Bytes: stdin, stdout, stderr
;        command \0
;         [parameter \0]*
;        \0
;	
SYS_FORK:
	; copy parent console
	mov RB, #TCB_OFF_CONS
	mov RA, [task_active_block],RB
	
SYS_FORK_CONS:		
	PSF	; save interrupt inhibit state
	SII ; inhibit interrupt
	
	mov fork_console, RA
	
	; save pointer to fork structure
	PSH RD 
	PSH RC
	
	; create new task control block
	JSR TASK_NEW

	JSR TASK_GET_BLOCK_POINTER
	mov fork_aux_pointer, RCD

	PUL RC
	PUL RD

	PSH RB ; store new task index
	
	; copy stdin, stdout, stderr
	mov RA, #TCB_OFF_FILES
	mov fork_file_count, RA
.floop:	
	mov RA, [RCD]	
	mov RB, #TCB_OFF_FILES
	CLC
	ADD RB, RA
	mov RA, [task_active_block],RB
	
	; increase use count
	JSR SCB_GET_INDEX
	INC scb_area+SCB_OFF_COUNT,RB

	mov RB, fork_file_count
	mov [fork_aux_pointer],RB, RA
	INC RCD
	INC RB
	mov fork_file_count, RB
	STC
	CMP RB, #3+TCB_OFF_FILES
	JNC .floop
	
	mov RA, fork_aux_pointer
	PSH RA
	CLC
	ADD RA, #TCB_OFF_CMD
	mov fork_aux_pointer, RA
	mov RA, fork_aux_pointer+1
	PSH RA
	
	; copy command and parameters
	JSR SYS_COPY_STRING_LIST
	
	; behind the command string
	; comes the environment space
	mov RCD, fork_aux_pointer  
	mov RA, #0
	mov [RCD], RA
	
	; restore new TCB pointer
	PUL RA
	mov fork_aux_pointer+1, RA
	PUL RA
	mov fork_aux_pointer, RA
	
	; store environment pointer
	mov RB, #TCB_OFF_ENV
	mov RA, RC
	mov [fork_aux_pointer],RB, RA
	INC RB
	mov RA, RD
	mov [fork_aux_pointer],RB, RA
		
	; copy parent data
	mov RA, task_active_block
	OR  RA, task_active_block+1
	JZS .no_parent
	
	; parent TID
	mov RB, #TCB_OFF_TID
	mov RA, [task_active_block],RB
	mov RB, #TCB_OFF_PARENT
	mov [fork_aux_pointer],RB, RA
	mov RB, #TCB_OFF_TID+1
	mov RA, [task_active_block],RB
	mov RB, #TCB_OFF_PARENT+1
	mov [fork_aux_pointer],RB, RA
	
	; set console
	mov RA, fork_console
	mov RB, #TCB_OFF_CONS
	mov [fork_aux_pointer],RB, RA

	mov RA, fork_aux_pointer
	PSH RA
	mov RA, fork_aux_pointer+1
	PSH RA
	
	; new task's environment to fork_aux_pointer
	mov fork_aux_pointer, RCD
	
	; parent's environment pointer to RCD
	mov RB, #TCB_OFF_ENV 
	mov RA, [task_active_block],RB
	mov RC, RA
	INC RB
	mov RA, [task_active_block],RB
	mov RD, RA
	
	; copy environment
	JSR SYS_COPY_STRING_LIST
	
	PUL RA
	mov fork_aux_pointer+1, RA
	PUL RA
	mov fork_aux_pointer, RA
	
.no_parent:

	; save active stack pointer
	mov RB, task_active
	mov RCD, SP
	JSR TASK_SET_STACK_POINTER
	
	; build a stack frame for IRQ exit
	PUL RB ; restore new task index
	JSR TASK_GET_STACK_POINTER
	mov SP, RCD

	; Return for RTS at program end
	mva RCD, SYS_END_TASK ; return to end of task
	PSH RC 
	PSH RD
	
	; Return from fake-IRQ
	mva RCD, SYS_EXEC ; start at SYS_EXEC
	PSH RC ; return address
	PSH RD
	
	mov RA, #0 ; flags all zero
	PSH RA
	
	PSH RA ; Register A
	PSH RA ; Register B
	
	; RCD points to task command
	mov RCD, fork_aux_pointer
	CLC
	ADD RC, #TCB_OFF_CMD
	PSH RC ; Register C
	PSH RD ; Register D
	
	; push an empty save area onto stack
	mov RC, #OS_SAVE_AREA_SIZE
.fake_save_area_loop:
	PSH RA
	DEC RC
	JNZ .fake_save_area_loop
	
	mov RCD, SP
	JSR TASK_SET_STACK_POINTER
	
	; activate new task
	mov RA, #TASK_ST_RUNNING
	mov task_status,RB, RA

	; restore current stack pointer
	mov RB, task_active
	JSR TASK_GET_STACK_POINTER
	mov SP, RCD
	
	PUL RB ; restore new task index
	
	; get new task ID into RCD
	mov RB, #TCB_OFF_TID
	mov RA, [fork_aux_pointer],RB
	mov RC, RA
	INC RB
	mov RA, [fork_aux_pointer],RB
	mov RD, RA
	
	PLF	; restore interrupt inhibit state
	RTS
	
SYS_COPY_STRING_LIST:
	mov RB, #0
.cloop:
	mov RA, [RCD]
	mov [fork_aux_pointer],RB, RA
	INC RCD
	INC fork_aux_pointer
	ICC fork_aux_pointer+1
	OR  RA, #0
	JNZ .cloop
	
	; double zero marks the end
	mov RA, [RCD]
	mov [fork_aux_pointer],RB, RA
	INC RCD
	INC fork_aux_pointer
	ICC fork_aux_pointer+1
	OR RA, #0
	JNZ .cloop
	RTS
	
SYS_END_TASK:
	; close all files
	mov RB, #TCB_OFF_FILES+TASK_MAX_FILES-1
.floop:
	mov RA, [task_active_block],RB
	STC
	CMP RA, #TASK_FILE_FREE
	JZS .skip_fd
	PSH RB
	JSR SYS_CLOSE_STREAM
	PUL RB
.skip_fd:
	DEC RB
	CMP RB, #TCB_OFF_FILES
	JCS .floop
	
	; free all memory
	mov RD, task_active
	JSR MEM_FORCE_FREE
	
	; release all locks
	mov RD, task_active
	JSR LOCK_FORCE_RELEASE

	; remove task structure
	mov RB, task_active
	JSR TASK_REMOVE
	
	JMP TASK_FORCE_SWITCH

;
; send a signal to a task
;
; parm:
;  RA - signal number
;  RC/RD - receiver task ID
; return:
;  c-flag: 0 = O.K., 1 = task not found
;	
SYS_TASK_SIGNAL:
	PSF
	SII
	
	JSR TASK_FIND_TID
	JCS .error_end
	PSH RB

	; save active stack pointer
	mov RB, task_active
	mov RCD, SP
	
	JSR TASK_SET_STACK_POINTER
	
	; build a stack frame for TASK_END
	PUL RB ; restore new task index
	JSR TASK_GET_STACK_POINTER
	mov SP, RCD
	
	mov RA, RB
	
	; copy save_area
	mov RB, #OS_SAVE_AREA_SIZE
	PLB os_save_copy, RB
	
	; return address is END_TASK
	mva RCD, SYS_END_TASK
	PSH RC
	PSH RD
	
	mov RB, #0 ; flags all zero
	PSH RB
	
	PSH RB ; Register A
	PSH RB ; Register B
	PSH RB ; Register C
	PSH RB ; Register D
	
	; restore save_area
	mov RB, #OS_SAVE_AREA_SIZE
	PSB os_save_copy+OS_SAVE_AREA_SIZE-1, RB

	; save manipulated stack pointer
	mov RB, RA
	
	mov RCD, SP
	JSR TASK_SET_STACK_POINTER
	
	; restore current stack pointer
	mov RB, task_active
	JSR TASK_GET_STACK_POINTER
	mov SP, RCD
	
	PUL RB ; pull new task index
	
.end:
	PLF
	CLC
	RTS
	
.error_end:
	PLF
	STC
	RTS

SYS_EXEC:
	JSR SYS_LOCK_EXEC
	
	; initialize memory pointers
	mov RB, #0
	mov data_pages, RB
	mov data_page, RB
	mov pgm_pages, RB
	mov pgm_page, RB
	
	; save the pointer to the original name
	mov search_name_ptr, RCD
	
	; pointer PATH variable name
	mva RCD, path_var_name
	
	; find the PATH variable
	JSR TASKENV_GET_VAR
	
	; save the current start of the PATH list
	mov path_name_ptr, RCD
	
	; search module in PATH
	JSR PATH_LIST_SEARCH	
	JCS .error_exit

	mov file_number, RA
	
	; find program file size
	mva RCD, dir_entry_area
	JSR SYS_STAT
	JCS .error_exit
	
	; calculate number of required 512 byte pages
	mov RC, dir_entry_area+FAT_DIRENT_FILESIZE+1
	ADD RC, #2
	AND RC, #0xFE
	mov file_read+1, RC
	CLC
	LSR RC
	mov pgm_pages, RC

	; allocate memory 
	mov RB, task_active
	JSR SYS_MEM_ALLOC
	JCS .error_exit
	
	mov pgm_page, RD
	mov pgm_start+1, RD
	mov file_read+3, RD
	
	mov RC, #0
	mov pgm_start, RC
	mov file_read, RC
	mov file_read+2, RC
	
	; read program file into memory
	mva RCD, file_read
	mov RA, file_number
	JSR SYS_READ
	JCS .error_exit
	
	ADD RA, pgm_start
	mov pgm_end, RA
	ADD RB, pgm_start+1
	mov pgm_end+1, RB
	
	JSR SYS_READ_PROGRAM_TRAILER
	JSR SYS_ALLOC_PROGRAM_DATA
	JCS .error_exit
	
	mov RA, file_number
	JSR SYS_CLOSE
	JCS .error_exit
	
	mov RA, #0
	mov file_number, RA
	
	JSR RELOCATE
	JCS .error_exit
	
	mov RA, pgm_page
	PSH RA
	mov RA, pgm_pages
	PSH RA
	mov RA, data_page
	PSH RA
	mov RA, data_pages
	PSH RA
	
	mov RA, pgm_start
	mov RB, pgm_start+1
	mov RCD, search_name_ptr
	JSR SYS_UNLOCK_EXEC
	
	JSR SYS_CALL_PROGRAM
	; normal program end, exit code in RA
	
	JSR SYS_LOCK_EXEC
	
	PUL RB
	mov data_pages, RB
	PUL RB
	mov data_page, RB
	PUL RB
	mov pgm_pages, RB
	PUL RB
	mov pgm_page, RB

.OK_exit:	
	PSH RA ; save exit code
	JSR SYS_FREE_START_MEM
	
	JSR SYS_UNLOCK_EXEC
	
	PUL RA ; restore exit code
	RTS
	
.error_exit:
	JSR .OK_exit
	JMP SYS_SHOW_ERROR
	
SYS_FREE_START_MEM:
	mov RB, task_active
	mov RC, data_pages
	OR  RC, #0
	JZS .no_free_data
	mov RD, data_page
	JSR MEM_FREE
	
.no_free_data:	
	mov RB, task_active
	mov RC, pgm_pages
	OR  RC, #0
	JZS .no_free_pgm
	mov RD, pgm_page
	JSR MEM_FREE
	
.no_free_pgm:	
	RTS
	
SYS_CALL_PROGRAM:
	PSH RA ; push program start address
	PSH RB
	RTS    ; jumps to program start
	
SYS_ALLOC_PROGRAM_DATA:	
	mov RA, pgm_end
	mov RB, pgm_end+1
	
	; calculate end of last page of program
	mov RD, RB
	AND RD, #0xFE
	CLC
	ADD RD, #2
	
	; add size of data section
	ADD RA, data_section_size
	ADD RB, data_section_size+1		
	
	; check, if data section fits in last page of progrgam
	STC
	CMP RA, #0
	CMP RB, RD
	JCS .allocate_data
	
	mov RCD, pgm_end
	JMP .store_data_start

.allocate_data:
	mov RC, data_section_size+1
	; allocate memory for data section
	CLC
	LSR RC
	INC RC
	
	mov data_pages, RC
	mov RB, task_active
	JSR SYS_MEM_ALLOC
	JCS .end
	mov data_page, RD
	
	mov RC, #0 ; low-byte always zero
	
.store_data_start:	
	mov data_mem_start, RCD
	
.end:	
	RTS

;
; Lock the Task Exec section for the active task
;
SYS_LOCK_EXEC:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #EXEC_LOCK
	JSR LOCK_REQUEST
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS
	
;
; Release the Task Exec section for the active task
;
SYS_UNLOCK_EXEC:
	PSF
	PSH RA
	PSH RB
	PSH RC
	PSH RD
	mov RB, #EXEC_LOCK
	JSR LOCK_RELEASE
	PUL RD
	PUL RC
	PUL RB
	PUL RA
	PLF
	RTS
	
SYS_SHOW_ERROR:	
	PSH RA
	mva RCD, fail_msg
	JSR DISPLAY_STRING
	PUL RA
	JMP DISPLAY_HEX_BYTE	
	
SYS_GET_TICKS:
	PSF
	SII
	mov RA, sys_ticks
	mov RB, sys_ticks+1
	mov RC, sys_ticks+2
	PLF
	CLC
	RTS
	
SYS_SLEEP:
	PSH RC
	PSH RB
	PSH RA
.loop:	
	JSR SYS_GET_TICKS
	STC
	PUL RD
	CMP RA, RD
	mov RA, RD
	PUL RD
	CMP RB, RD
	mov RB, RD
	PUL RD
	CMP RC, RD
	mov RC, RD
	JCS SYS_GET_TICKS
	PSH RC
	PSH RB
	PSH RA
	JSR TASK_FORCE_SWITCH
	JMP .loop

END_PGM:

	#res OS_SEND_TASK_SIGNAL - END_PGM

	JMP SYS_TASK_SIGNAL
	JMP TASK_GET_INFO
	JMP TASK_GET_LIST
	JMP SYS_SLEEP
	JMP SYS_GET_TICKS
	JMP TASK_GET_ENV_POINTER
	JMP SYS_WAIT_TASK
	JMP SYS_FORK
	JMP SYS_EXEC
	JMP SYS_GETC
	JMP SYS_PUTC
	JMP SYS_FSSTAT
	JMP SYS_MEM_FREE
	JMP SYS_MEM_ALLOC
	JMP SYS_STAT
	JMP SYS_UNLINK
	JMP SYS_MKDIR
	JMP SYS_TELL
	JMP SYS_SEEK
	JMP SYS_WRITE
	JMP DISPLAY_SEND_COMMAND
	JMP SYS_GET_KEY
	JMP SYS_GET_VERSION_STRING
	JMP CLK_WRITE_TIME
	JMP CLK_READ_TIME
	JMP SYS_OPEN_DIR
	JMP SYS_NEXT_DIR
	JMP SYS_CLOSE_DIR
	JMP SYS_OPEN
	JMP SYS_READ
	JMP SYS_CLOSE
    JMP KEYBOARD_WAIT_FOR_KEY
	JMP DISPLAY_SEND_DATA_BYTE
	JMP DISPLAY_SET_BACKLIGHT
	JMP DISPLAY_READ_BYTE
	JMP DISPLAY_READ_STATE
	JMP DISPLAY_GET_CURSOR_OFFSET
	JMP DISPLAY_GET_CURSOR_POS
	JMP DISPLAY_CLEAR
	JMP DISPLAY_MOVE_CURSOR_LEFT
	JMP DISPLAY_MOVE_CURSOR_RIGHT
	JMP DISPLAY_GOTO_OFFSET
	JMP DISPLAY_GOTO_POSITION
	JMP DISPLAY_GOTO_LINE
	JMP DISPLAY_HEX_BYTE
	JMP DISPLAY_STRING
	JMP DISPLAY_CHAR

	#res 4
	
	ld16 RESET
	ld16 IRQ

#bank data

file_number: #res 1
file_byte: #res 1
dir_entry_area: #res 32
	
sys_init_con: #res 1
sys_con_filename: #res 6
sys_fork_area:
sys_fork_filedesc: #res 3
sys_fork_filename: #res 40

sys_ticks: #res 3

#bank task_data
file_read: #res 4
scb_pointer: #res 2

#bank data

pgm_end: #res 2
data_section_size: #res 2

data_pages: #res 1
data_page: #res 1
pgm_pages: #res 1
pgm_page: #res 1

fork_aux_pointer: #res 2
fork_file_count: #res 1
fork_console: #res 1

fat_file_area:
fat_file_dir_sector: #res 4 ; dir sector with the file entry
fat_file_dir_index: #res 1 ; offset/2 in dir sector to file entry
fat_file_mode: #res 1
fat_file_buffer_pointer: #res 2
fat_file_active_cluster: #res 2
fat_file_pointer: #res 4
fat_file_area_end:
FAT_FILE_AREA_LENGTH = fat_file_area_end - fat_file_area

; FAT driver functions
fat_fs_open_ptr: #res 2
fat_fs_close_ptr: #res 2
fat_fs_read_ptr: #res 2
fat_fs_write_ptr: #res 2
fat_fs_seek_ptr: #res 2
fat_fs_tell_ptr: #res 2
fat_fs_mkdir_ptr: #res 2
fat_fs_unlink_ptr: #res 2
fat_fs_stat_ptr: #res 2
fat_fs_fsstat_ptr: #res 2
fat_fs_flush_ptr: #res 2

; console driver functions
con_open_ptr: #res 2
con_close_ptr: #res 2
con_out_ptr: #res 2
con_in_ptr: #res 2
con_visible_ptr: #res 2

PIPE_AREA_LENGTH = pipe_area_end - pipe_area

DATA_END: