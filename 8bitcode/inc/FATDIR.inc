;
; FAT File System directory functions
;

#bank pgm

;
; searches for a free directory entry
;
; Parms:
;  fat_dir_entry_pointer - pointer to directory entry
;                          in the parent directory
;                          0=root dir
; Returns:
;  C-Flag - 1=not found, 0=found
;  fat_dir_entry_pointer - pointer to free entry
;	
FAT_SEARCH_FREE_DIR_ENTRY:	
	mov RA, fat_dir_entry_pointer
	OR  RA, fat_dir_entry_pointer+1
	JZS .root_dir
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster+1, RA
	mov RA, #0
	JMP .init_dir_skip_area
.root_dir:
	mov fat_work_cluster, RA
	mov fat_work_cluster+1, RA
.init_dir_skip_area:
	mov fat_work_file_pointer, RA
	mov fat_work_file_pointer+1, RA
	mov fat_work_file_pointer+2, RA
	mov fat_work_file_pointer+3, RA
	mov fat_dir_entry_pointer, RA
	mov fat_dir_entry_pointer+1, RA
	
.dir_read_loop:
	JSR FAT_NEXT_DIR_ENTRY
	JCS .fail
	
	; end of directory mark
	mov RA, [RCD]
	OR  RA, #0
	JZS .entry_found
	
	; empty entry mark
	XOR RA, #0xe5 
	JNZ .dir_read_loop
	
.entry_found:	
	mov fat_dir_entry_pointer, RCD
	CLC	
	RTS
.fail:
	mov RA, #EIO
	RTS

;
; formats and copies the file name to a directory entry
;
; Parms:
;  RC/RD - pointer to 0-terminated file name
;  fat_dir_entry_pointer - pointer to the directory entry
;	
FAT_FILE_NAME_TO_DIR_ENTRY:
	JSR GET_FAT_ENTRY_NAME
	
	mva RCD, fat_entry_name
	JSR OS_TO_UPPER	
	
	mov RB, #FAT_DIRENT_NAME + 10
.loop:	
	mov RA, fat_entry_name,RB
	mov [fat_dir_entry_pointer],RB, RA
	DEC RB
	JCS .loop
	RTS

;
; fills the standard entries of a new directory entry
;
; Parms:
;  fat_dir_entry_pointer - pointer to the directory entry
;
FAT_SET_CONSTANT_DIR_ENTRY:
	mov RA, fat_file_mode
	AND RA, #O_DIR
	JZS .set_attr
	mov RA, #FAT_DIR_ATTR_DIRECTORY
	
.set_attr:	
	OR  RA, #FAT_DIR_ATTR_ARCHIVE
	mov RB, #FAT_DIRENT_ATTR
	mov [fat_dir_entry_pointer],RB, RA
	
	mov RA, #0	
	
	; reserved Windows NT
	mov RB, #FAT_DIRENT_NTRES 
	mov [fat_dir_entry_pointer],RB, RA
	
	; microseconds
	mov RB, #FAT_DIRENT_CRTTIMETENTH 
	mov [fat_dir_entry_pointer],RB, RA
	
	; no Hi Cluster
	mov RB, #FAT_DIRENT_FSTCLUSHI
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FSTCLUSHI+1
	mov [fat_dir_entry_pointer],RB, RA
	
	; empty Lo Cluster
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov [fat_dir_entry_pointer],RB, RA
	
	; initial file length
	mov RB, #FAT_DIRENT_FILESIZE
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+1
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+2
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+3
	mov [fat_dir_entry_pointer],RB, RA
	
	; copy creation time from write time
	mov RB, #FAT_DIRENT_WRTTIME
	mov RA, [fat_dir_entry_pointer],RB
	mov RB, #FAT_DIRENT_CRTTIME
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_WRTTIME+1
	mov RA, [fat_dir_entry_pointer],RB
	mov RB, #FAT_DIRENT_CRTTIME+1
	mov [fat_dir_entry_pointer],RB, RA
	
	; copy creation date from write date
	mov RB, #FAT_DIRENT_WRTDATE
	mov RA, [fat_dir_entry_pointer],RB
	mov RB, #FAT_DIRENT_CRTDATE
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_WRTDATE+1
	mov RA, [fat_dir_entry_pointer],RB
	mov RB, #FAT_DIRENT_CRTDATE+1
	mov [fat_dir_entry_pointer],RB, RA

	RTS

;
; writes the start cluster to a directory entry
;
; Parms:
;  fat_dir_entry_pointer - pointer to the directory entry
;  fat_work_cluster - start cluster number
;
FAT_SET_FIRST_CLUSTER_DIR_ENTRY:
	mov RA, fat_work_cluster
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov [fat_dir_entry_pointer],RB, RA
	
	mov RA, fat_work_cluster+1
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov [fat_dir_entry_pointer],RB, RA

	RTS

;
; writes the current date/time to a directory entry
;
; Parms:
;  fat_dir_entry_pointer - pointer to the directory entry
;
FAT_SET_DATE_TIME_TO_ENTRY:	
	mva RCD, fat_os_time_buffer
	JSR OS_GET_TIME

	mva RCD, fat_os_time_buffer
	JSR FAT_OS_TIME_2_DIR_DATE
	
	mov RC, RA
	mov RD, RB
	; set Last Access Date
	mov RB, #FAT_DIRENT_LSTACCDATE
	mov [fat_dir_entry_pointer],RB, RA
	mov RA, RD
	mov RB, #FAT_DIRENT_LSTACCDATE+1
	mov [fat_dir_entry_pointer],RB, RA
	
	; write access?
	mov RA, fat_file_mode
	AND RA, #O_WRITE
	JZS .end
	
	; set write date/time
	mov RA, RC
	mov RB, #FAT_DIRENT_WRTDATE
	mov [fat_dir_entry_pointer],RB, RA
	mov RA, RD
	mov RB, #FAT_DIRENT_WRTDATE+1
	mov [fat_dir_entry_pointer],RB, RA
	
	mva RCD, fat_os_time_buffer
	JSR FAT_OS_TIME_2_DIR_TIME
	PSH RB
	mov RB, #FAT_DIRENT_WRTTIME
	mov [fat_dir_entry_pointer],RB, RA
	PUL RA
	mov RB, #FAT_DIRENT_WRTTIME+1
	mov [fat_dir_entry_pointer],RB, RA
	
.end:
	RTS
	
;
; compares the fat_file_mode to the directory entry
;
; Parms:
;  fat_file_mode - file mode for the current stream
;  fat_dir_entry_pointer - pointer to the directory entry
;
FAT_CHECK_DIR_ENTRY_TYPE:
	mov RC, fat_file_mode
	
	; root directory
	mov RA, fat_dir_entry_pointer
	OR  RA, fat_dir_entry_pointer+1
	JZS .is_dir
	
	mov RB, #FAT_DIRENT_ATTR
	mov RA, [fat_dir_entry_pointer],RB
	mov RB, RA
	AND RA, #FAT_DIR_ATTR_DIRECTORY
	JZS .is_file
	
.is_dir:	
	AND RC, #O_DIR
	JNZ .end_OK
	STC
	mov RA, #EISDIR
	RTS
	
.is_file:	
	AND RC, #O_DIR
	JZS .check_read_only
	STC
	mov RA, #ENOTDIR
	RTS
	
.check_read_only:	
	AND RB, #FAT_DIR_ATTR_READ_ONLY
	JZS .end_OK
	
	mov RC, fat_file_mode
	AND RC, #O_WRITE
	JZS .end_OK
	STC
	mov RA, #EROFS
	RTS

.end_OK:
	CLC
.end:
	RTS

;
; Creates a new directory entry
;
; parm:
;  RC/RD - 0-terminated file_path
; result:
;  parent_dir_name - leading path segments
;                    without the last segment
;  dir_entry_name - last path segment
;  fat_parent_dir_entry_pointer -
;                   pointer to parent directory entry
;                   0 = root directory
;	
FAT_CREATE_DIR_ENTRY:
	JSR GET_PARENT_DIR_NAME
	
	mva RCD, parent_dir_name
	JSR FAT_FIND_DIR_ENTRY
	JNC .path_found
	
	mov RA, #ENOENT
	RTS
	
.path_found:
	mov RA, fat_dir_entry_pointer
	mov fat_parent_dir_entry_pointer, RA
	mov RA, fat_dir_entry_pointer+1
	mov fat_parent_dir_entry_pointer+1, RA

	JSR FAT_SEARCH_FREE_DIR_ENTRY
	JCS .end
	
.dir_entry_found:
	mva RCD, dir_entry_name
	JSR FAT_FILE_NAME_TO_DIR_ENTRY
	JSR FAT_SET_DATE_TIME_TO_ENTRY
	JSR FAT_SET_CONSTANT_DIR_ENTRY
	
	JSR SPI_SELECT_CHIP0
	JSR FAT_REWRITE_DIR_SECTOR
.end:
	RTS

;
; Splits a path into parent dir and file name
;
; parm:
;  RC/RD - 0-terminated file_path
; result:
;  parent_dir_name - leading path segments
;                    without the last segment
;  dir_entry_name - last path segment
;	
GET_PARENT_DIR_NAME:
	mov file_name_ptr, RC
	mov file_name_ptr+1, RD
	mov RB, #0
.end_loop:	
	mov RA, [file_name_ptr],RB
	OR  RA, #0
	JZS .end_found
	INC RB
	JNC .end_loop
.end_found:
	DEC RB
.slash_loop:
	DEC RB
	JZS .end_slash
	mov RA, [file_name_ptr],RB
	CMP RA, #"/"
	JNZ .slash_loop
.end_slash:	
	mov RA,	#0
	mov parent_dir_name,RB, RA
	PSH RB
	mva RCD, dir_entry_name
.name_loop:	
	mov RA, [file_name_ptr],RB
	STC
	CMP RA, #"/"
	JZS .skip_name_char
	mov [RCD], RA	
	OR  RA, #0
	JZS .end_name_loop
	INC RCD
.skip_name_char:
	INC RB
	JNC .name_loop
.end_name_loop:
	PUL RB
.copy_loop:	
	DEC RB
	JNC .end
	mov RA,	[file_name_ptr],RB
	mov parent_dir_name,RB, RA
	JMP .copy_loop
.end:	
	RTS

FAT_CREATE_DIR_DOT_ENTRIES:
	mov RA, #" "
	mov RB, #FAT_DIRENT_ATTR-1
	
.dot_name_loop:	
	mov fat_dir_buffer,RB, RA
	mov fat_dir_buffer+32,RB, RA
	DEC RB
	JNZ .dot_name_loop
		
	mov RA, #"."
	mov fat_dir_buffer, RA
	mov fat_dir_buffer+32, RA
	mov fat_dir_buffer+33, RA

	mov RB, #FAT_DIRENT_ATTR
.copy_loop:	
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_dir_buffer,RB, RA
	mov fat_dir_buffer+32,RB, RA
	INC RB
	STC
	CMP RB, #32
	JNC .copy_loop
	
	mov RA, fat_work_cluster
	mov fat_dir_buffer+FAT_DIRENT_FSTCLUSLO, RA
	mov RA, fat_work_cluster+1
	mov fat_dir_buffer+FAT_DIRENT_FSTCLUSLO+1, RA
	
	mov RA, fat_parent_dir_entry_pointer
	OR  RA, fat_parent_dir_entry_pointer+1
	JZS .root_parent
	
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_parent_dir_entry_pointer],RB
	mov fat_dir_buffer+32+FAT_DIRENT_FSTCLUSLO, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_parent_dir_entry_pointer],RB
	mov fat_dir_buffer+32+FAT_DIRENT_FSTCLUSLO+1, RA
	RTS
	
.root_parent:
	mov fat_dir_buffer+32+FAT_DIRENT_FSTCLUSLO, RA
	mov fat_dir_buffer+32+FAT_DIRENT_FSTCLUSLO+1, RA
.end:
	RTS

FAT_CLEAR_DIR_BUFFER:
	mov RB, #0
	mov RA, #0
.clear_loop:
	mov fat_dir_buffer,RB, RA
	mov fat_dir_buffer+256,RB, RA
	INC RB
	JNC .clear_loop
	RTS
	
#bank data

file_name_ptr: #res 2
parent_dir_name: #res 40
dir_entry_name: #res 40

fat_parent_dir_entry_pointer: #res 2

fat_os_time_buffer: #res 7
