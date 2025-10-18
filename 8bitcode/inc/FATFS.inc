;
; FAT File System OS functions
;

#bank pgm
	
;
; Establish access to a file in the FAT file system
;
; parm:
;  RA    - open mode (O_*)
;  RC/RD - pointer to 0-terminated file name
; result:
;  c-flag - 0 = O.K., 1 = error
;  RA - 0 = O.K., >0 = error
;  fat_file_dir_sector 
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; 
FAT_OPEN:
	mov fat_file_mode, RA
	
	JSR FAT_CANONICAL_FILE_NAME
	JCS .end
	
	mov file_name_ptr, RCD
	
	JSR SPI_SELECT_CHIP0
	
	JSR FAT_FIND_DIR_ENTRY
	JNC .file_found

	mov RB, fat_file_mode
	AND RB, #O_CREATE
	JNZ .create_file
	
	STC
	RTS
	
.create_file:
	mov RCD, file_name_ptr
	JSR FAT_CREATE_DIR_ENTRY
	JCS .end
	
.file_found:
	JSR FAT_CHECK_DIR_ENTRY_TYPE
	JCS .end
	
	mov RA, fat_dir_entry_pointer
	OR  RA, fat_dir_entry_pointer+1
	JZS .root_dir
	
	JSR FAT_STORE_DIR_ENTRY
		
	; start cluster
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_dir_entry_pointer],RB
	mov RC, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_dir_entry_pointer],RB
	mov RD, RA
	
	; append to end of file?
	mov RA, fat_file_mode
	AND RA, #O_APPEND
	JZS .fp_beginning
	
	; is there any data in the file?
	OR  RC, RD
	JZS .fp_beginning
	
	mov RA, fat_file_mode
	AND RA, #O_DIR
	JNZ IS_DIR_ERROR
	
	; set file pointer to last byte of file
	CLC
	mov RC, #FAT_DIRENT_FILESIZE
	ADD RC, fat_dir_entry_pointer
	mov RD, #0
	ADD RD, fat_dir_entry_pointer+1
	JMP FAT_SEEK

.root_dir:
	mov RC, RA
	mov RD, RA
	
.fp_beginning:
	mov fat_file_active_cluster, RCD
	
	; copy canonical file to file buffer
	mov RCD, fat_file_buffer_pointer
	mov RB, #0
.fn_loop:	
	mov RA, fat_canonical_fn_buf,RB
	mov [RCD], RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .fn_loop
	
	mov RCD, fat_file_buffer_pointer

	; start at the beginning of file
	mov RA, #0	
    mov fat_file_pointer, RA
    mov fat_file_pointer+1, RA
    mov fat_file_pointer+2, RA
    mov fat_file_pointer+3, RA
.end_OK:
	CLC
.end:
	RTS

IS_DIR_ERROR:
	STC
	mov RA, #EISDIR
	RTS

;
; Write bytes to file on SD card
;
; parm:
;  RC/RD - pointer to file write structure
;  fat_file_dir_sector 
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
;  fat_file_active_cluster
;  fat_file_pointer
; 
FAT_WRITE:
	; copy caller parameters
	JSR COPY_CALLER_BUFFER_DATA
	
	JSR SPI_SELECT_CHIP0
	
	mov RA, #0
	mov fat_byte_count, RA
	mov fat_byte_count+1, RA

.write_loop:
	; write_structure_buf_size == 0?
	mov RA, fat_caller_buffer_size
	OR  RA, fat_caller_buffer_size+1
	JZS .end_OK
	
	; filepointer at cluster start?
	mov RA, fat_file_pointer
	OR  RA, #0
	JNZ .cluster_OK
	mov RA, fat_file_pointer+1
	LSR RA
	JCS .cluster_OK
	AND RA, fat_sector_in_cluster_offset_mask
	JNZ .cluster_OK
	
	; currentcluster == 0?	
	mov RA, fat_file_active_cluster
	OR  RA, fat_file_active_cluster+1
	JZS .first_cluster
	
	; filepointer == 0?
	mov RA, fat_file_pointer
	OR  RA, fat_file_pointer+1
	OR  RA, fat_file_pointer+2
	OR  RA, fat_file_pointer+3
	JZS .cluster_OK
	
	; more clusters allocated to file?
	mov RA, fat_file_active_cluster
	mov RB, fat_file_active_cluster+1
	mov fat_work_cluster, RA
	mov fat_work_cluster+1, RB
	JSR FAT_GET_NEXT_CLUSTER
	JCS .end
	
	; EOC marker?
	mov RA, fat_work_cluster
	AND RA, fat_work_cluster+1
	XOR RA, #0xff
	JZS .eoc_reached
	
	; reuse allocated clusters
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA	
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	JMP .cluster_OK
	
.eoc_reached:
	JSR RESERVE_ADDITIONAL_CLUSTER
	JCS .end
	JMP .cluster_OK
	
.first_cluster:
	JSR RESERVE_FIRST_CLUSTER
	JCS .end
	
.cluster_OK:	
	mov RA, fat_caller_buffer_addr
	mov block_move_structure+BLOCK_MOVE_FROM, RA
	mov RA, fat_caller_buffer_addr+1
	mov block_move_structure+BLOCK_MOVE_FROM+1, RA
	
	JSR FAT_CALCULATE_AVAILABLE_BUFFER_BYTES
	
	; start position in write buffer
	; bufferbase + (filepointer & 511)
	CLC
	ADD RC, fat_file_buffer_pointer
	ADD RD, fat_file_buffer_pointer+1
	mov block_move_structure+BLOCK_MOVE_TO, RCD
	
	; move a complete 512 byte buffer?
	mov RA, block_move_structure+BLOCK_MOVE_SIZE+1
	STC
	CMP RA, #2
	JNZ .partial_block
	
	; write block directly to SD card
	mov RA, block_move_structure+BLOCK_MOVE_FROM
	mov RB, block_move_structure+BLOCK_MOVE_FROM+1
	JSR FAT_WRITE_WRITE_BUFFER
	JNC .count_bytes
	RTS

.partial_block:
	; file open in read/write mode?
	mov RA, fat_file_mode
	AND RA, #O_READ
	JZS .move_block
	
	; write at buffer start?
	STC
	mov RA, block_move_structure+BLOCK_MOVE_TO
	cmp RA, fat_file_buffer_pointer
	JNZ .move_block
	mov RA, block_move_structure+BLOCK_MOVE_TO+1
	cmp RA, fat_file_buffer_pointer+1
	JNZ .move_block
	
	JSR FAT_READ_FILE_BUFFER
	JCS .end

.move_block:
	mva RCD, block_move_structure
	JSR BLOCK_MOVE
	
	; check for write buffer full
	; (new filepointer & 511) == 0?
	CLC
	mov RCD, block_move_structure+BLOCK_MOVE_SIZE
	ADD RC, fat_file_pointer
	ADD RD, fat_file_pointer+1

;	mov RA, fat_file_pointer+1
;	AND RA, #1
;	OR  RA, fat_file_pointer
	AND RD, #1
	OR  RD, RC
	JNZ .count_bytes

	JSR FAT_WRITE_FILE_BUFFER
	JCS .end
	
.count_bytes:
	JSR FAT_ADVANCE_BUFFER_POINTERS
	JMP .write_loop

.end_OK:	
	CLC
	mov RA, fat_byte_count
	mov RB, fat_byte_count+1
.end:
	RTS

;
; Get current file pointer
;
; parm:
;  fat_file_dir_sector
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
;  RA/RB/RC/RD - current filepointer
;  fat_file_active_cluster
;  fat_file_pointer
; 
FAT_TELL:
	mov RA, fat_file_pointer
	mov RB, fat_file_pointer+1
	mov RCD, fat_file_pointer+2
	CLC
	RTS

;
; Get current file system status
;
; parm:
;  RC/RD - pointer to 4-byte dir fsstat buffer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
; 
FAT_FSSTAT:
	mov fat_caller_buffer_addr, RCD

	JSR SPI_SELECT_CHIP0
	
	JSR FAT_COUNT_FREE_CLUSTERS
	JCS .end
	
	mov RA, fat_total_clusters
	mov RB, #FAT_FSSTAT_TOTAL
	mov [fat_caller_buffer_addr],RB, RA
	mov RA, fat_total_clusters+1
	mov RB, #FAT_FSSTAT_TOTAL+1
	mov [fat_caller_buffer_addr],RB, RA
	
	mov RA, fat_free_cluster_count
	mov RB, #FAT_FSSTAT_FREE
	mov [fat_caller_buffer_addr],RB, RA
	mov RA, fat_free_cluster_count+1
	mov RB, #FAT_FSSTAT_FREE+1
	mov [fat_caller_buffer_addr],RB, RA
	
	mov RA, fat_sectors_per_cluster	
	mov RB, #FAT_FSSTAT_SPC
	mov [fat_caller_buffer_addr],RB, RA
	
.end:	
	RTS

;
; Get current file status (directory entry)
;
; parm:
;  RC/RD - pointer to 32-byte dir entry buffer
;  fat_file_dir_sector
;  fat_file_dir_index
;  fat_file_mode
; result:
;  c-flag - 0 = O.K., 1 = I/O error
; 
FAT_STAT:
	mov fat_caller_buffer_addr, RCD

	JSR SPI_SELECT_CHIP0
	
	JSR READ_FILE_DIR_ENTRY
	JCS .end
	
	mov RB, #31
	mov RA, fat_dir_entry_pointer
	OR  RA, fat_dir_entry_pointer+1
	JZS .root_dir

.loop:	
	mov RA, [fat_dir_entry_pointer],RB
	mov [fat_caller_buffer_addr],RB, RA
	DEC RB
	JCS .loop
	RTS
	
.root_dir:
	mov RA, #0
.loop_root:	
	mov [fat_caller_buffer_addr],RB, RA
	DEC RB
	JCS .loop_root
.end:
	RTS

;
; reposition file pointer
;
; parm:
;  RC/RD - pointer to new file pointer
;  fat_file_dir_sector
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
;  fat_file_active_cluster
;  fat_file_pointer
; 
FAT_SEEK:
	; copy new file pointer
	mov RB, #0
	
.loop:	
	mov RA, [RCD]
	mov fat_file_pointer,RB, RA
	INC RCD
	INC RB
	CMP RB, #3 ; c-flag=0 => CMP RB, #4
	JNC .loop
	
	JSR SPI_SELECT_CHIP0
	
	JSR READ_FILE_DIR_ENTRY
	JCS .end
	
	OR  RC, RD
	JZS .root_dir
	
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster+1, RA
	JSR FAT_FILE_POINTER_TO_CLUSTER
	JCS .end
	
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	
.root_dir:	
	JSR FAT_READ_FILE_BUFFER
.end:
	RTS

;
; Read bytes from file on SD card
;
; parm:
;  RC/RD - pointer to file read structure
;  fat_file_dir_sector 
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
;  RA/RB - number of bytes read
;  fat_file_active_cluster
;  fat_file_pointer
; 
FAT_READ:
	; copy caller parameters
	JSR COPY_CALLER_BUFFER_DATA
	
	JSR SPI_SELECT_CHIP0
	
	mov RA, #0
	mov fat_byte_count, RA
	mov fat_byte_count+1, RA
	
	JSR CALCULATE_FILE_BYTES_LEFT
	JCS .end

.read_loop:
	; read_structure_buf_size == 0?
	mov RA, fat_caller_buffer_size
	OR  RA, fat_caller_buffer_size+1
	JZS .end_OK
	
	; filepointer >= file length?
	mov RA, fat_file_bytes_left
	OR  RA, fat_file_bytes_left+1
	OR  RA, fat_file_bytes_left+2
	OR  RA, fat_file_bytes_left+3
	JZS .end_OK
	
	; filepointer at cluster start?
	mov RA, fat_file_pointer
	OR  RA, #0
	JNZ .cluster_OK
	mov RA, fat_file_pointer+1
	LSR RA
	JCS .cluster_OK
	AND RA, fat_sector_in_cluster_offset_mask
	JNZ .cluster_OK
	
	; currentcluster == 0? (empty file)	
	mov RA, fat_file_active_cluster
	OR  RA, fat_file_active_cluster+1
	JZS .end_OK
	
	; filepointer == 0?
	mov RA, fat_file_pointer
	OR  RA, fat_file_pointer+1
	OR  RA, fat_file_pointer+2
	OR  RA, fat_file_pointer+3
	JZS .cluster_OK
	
	; more clusters allocated to file?
	mov RA, fat_file_active_cluster
	mov RB, fat_file_active_cluster+1
	mov fat_work_cluster, RA
	mov fat_work_cluster+1, RB
	JSR FAT_GET_NEXT_CLUSTER
	JCS .end
	
	; EOC marker? (EOC before EOF!)
	mov RA, fat_work_cluster
	AND RA, fat_work_cluster+1
	XOR RA, #0xff
	JZS .end_OK
	
	; use next cluster
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA	
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	
.cluster_OK:	
	mov RA, fat_caller_buffer_addr
	mov block_move_structure+BLOCK_MOVE_TO, RA
	mov RA, fat_caller_buffer_addr+1
	mov block_move_structure+BLOCK_MOVE_TO+1, RA
	
	JSR FAT_CALCULATE_AVAILABLE_BUFFER_BYTES
	
	; start position in file buffer
	; bufferbase + (filepointer & 511)
	CLC
	ADD RC, fat_file_buffer_pointer
	ADD RD, fat_file_buffer_pointer+1
	mov block_move_structure+BLOCK_MOVE_FROM, RCD

	; move a complete 512 byte buffer?
	mov RA, block_move_structure+BLOCK_MOVE_SIZE+1
	STC
	CMP RA, #2
	JNZ .partial_block
	
	; read block directly from SD card
	mov RA, block_move_structure+BLOCK_MOVE_TO
	mov RB, block_move_structure+BLOCK_MOVE_TO+1
	JSR FAT_READ_READ_BUFFER
	JCS .end
	JMP .count_bytes

.partial_block:
	; read at buffer start?
	mov RA, block_move_structure+BLOCK_MOVE_FROM
	XOR RA, fat_file_buffer_pointer
	JNZ .move_block
	mov RA, block_move_structure+BLOCK_MOVE_FROM+1
	XOR RA, fat_file_buffer_pointer+1
	JNZ .move_block
	
	JSR FAT_READ_FILE_BUFFER
	JCS .end

.move_block:
	mva RCD, block_move_structure
	JSR BLOCK_MOVE
	
.count_bytes:
	JSR FAT_ADVANCE_BUFFER_POINTERS
	JMP .read_loop

.end_OK:	
	CLC
	mov RA, fat_byte_count
	mov RB, fat_byte_count+1
.end:	
	RTS

;
; write dirty file buffer to SD card
;
FAT_FLUSH:	
	; file in write mode?
	mov RA, fat_file_mode
	AND RA, #O_WRITE
	JZS .empty
	
	; check for write buffer empty
	; (filepointer & 511) == 0?
	mov RB, fat_file_pointer+1
	AND RB, #1
	JNZ .notempty
	mov RA, fat_file_pointer
	OR  RA, #0
	JZS .empty
	
.notempty:	
	JSR SPI_SELECT_CHIP0
	JSR FAT_WRITE_FILE_BUFFER
.empty:	
	RTS	
	
;
; close a file on the FAT file system
;
; parm:
;  fat_file_dir_sector 
;  fat_file_dir_index
;  fat_file_mode
;  fat_file_buffer_pointer
;  fat_file_active_cluster
;  fat_file_pointer
; result:
;  c-flag - 0 = O.K., 1 = I/O error
;
FAT_CLOSE:
	; write mode?
	mov RA, fat_file_mode
	AND RA, #O_WRITE
	JZS .end
	
	; rewrite dirty buffer
	JSR FAT_FLUSH
	JCS .end

	; read directory entry
	JSR READ_FILE_DIR_ENTRY
	JCS .end
	
	; root directry?
	OR  RC, RD
	JZS .end
	
	; set last access date
	JSR FAT_SET_DATE_TIME_TO_ENTRY
	
	; set new file length
	mov RB, #FAT_DIRENT_FILESIZE
	mov RA, fat_file_pointer
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+1
	mov RA, fat_file_pointer+1
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+2
	mov RA, fat_file_pointer+2
	mov [fat_dir_entry_pointer],RB, RA
	mov RB, #FAT_DIRENT_FILESIZE+3
	mov RA, fat_file_pointer+3
	mov [fat_dir_entry_pointer],RB, RA
	
.rewrite_dir_sector:
	JSR SPI_SELECT_CHIP0
	JSR FAT_REWRITE_DIR_SECTOR
.end:	
	RTS

;
; create subdirectory
;
; parm:
;  RC/RD - pointer to 0-terminated directory name
;  fat_file_buffer_pointer
; result:
;  c-flag - 0 = O.K., 1 = error
;  RA - 0 = O.K., >0 = error
; 
FAT_MKDIR:	
	JSR FAT_CANONICAL_FILE_NAME
	JCS .end

	mov file_name_ptr, RCD
	
	JSR SPI_SELECT_CHIP0
	
	JSR FAT_FIND_DIR_ENTRY
	JCS .dir_not_found
	STC
	mov RA, #EEXIST
	RTS
	
.dir_not_found:
	mov RA, #O_DIR | O_WRITE | O_CREATE
	mov fat_file_mode, RA
	mov RCD, file_name_ptr
	JSR FAT_CREATE_DIR_ENTRY
	JCS .end
	
	JSR FAT_STORE_DIR_ENTRY
	
	JSR RESERVE_FIRST_CLUSTER
	JCS .end
	
	JSR FAT_CLUSTER_TO_SECTOR
	mov fat_dir_buffer_sector, RA
	mov fat_dir_buffer_sector+1, RB
	mov fat_dir_buffer_sector+2, RCD
	
	JSR FAT_CLEAR_DIR_BUFFER
	JSR FAT_CREATE_DIR_DOT_ENTRIES
	
	JSR FAT_REWRITE_DIR_SECTOR
	JCS .end
	
	JSR FAT_CLEAR_DIR_BUFFER
	
	mov RB, fat_sectors_per_cluster	
	
.empty_write_loop:
	DEC RB
	JZS .end_empty_write
	
	INC fat_dir_buffer_sector
	ICC fat_dir_buffer_sector+1
	ICC fat_dir_buffer_sector+2
	ICC fat_dir_buffer_sector+3
	
	PSH RB
	JSR FAT_REWRITE_DIR_SECTOR
	PUL RB
	JNC .empty_write_loop
	RTS
	
.end_empty_write:
	CLC
	
.end:
	RTS

;
; delete a directory entry
;
; parm:
;  RC/RD - pointer to 0-terminated file or directory name
;  fat_file_buffer_pointer
; result:
;  c-flag - 0 = O.K., 1 = error
;  RA - 0 = O.K., >0 = error
; 
FAT_UNLINK:	
	JSR FAT_CANONICAL_FILE_NAME
	JCS .end
	
	mov file_name_ptr, RCD
	
	JSR SPI_SELECT_CHIP0
	
	JSR FAT_FIND_DIR_ENTRY
	JCS .end
	
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_dir_entry_pointer],RB
	mov fat_work_cluster+1, RA
	
	mov RB, #0
	mov RA, #0xe5
	mov [fat_dir_entry_pointer],RB, RA
	
	JSR FAT_REWRITE_DIR_SECTOR
	JCS .end
	
	JSR FAT_FREE_CLUSTER_CHAIN

.end:	
	RTS

FAT_STORE_DIR_ENTRY:
	
	; store directory entry sector number
	mov RA, fat_dir_buffer_sector
	mov fat_file_dir_sector, RA
	mov RA, fat_dir_buffer_sector+1
	mov fat_file_dir_sector+1, RA
	mov RA, fat_dir_buffer_sector+2
	mov fat_file_dir_sector+2, RA
	mov RA, fat_dir_buffer_sector+3
	mov fat_file_dir_sector+3, RA
	
	; calc_dir_index
	STC
	mva RCD, fat_dir_buffer
	mov RA, fat_dir_entry_pointer
	SUB RA, RC
	mov RB, fat_dir_entry_pointer+1
	SUB RB, RD
	LSR RB
	LSR RA
    mov fat_file_dir_index, RA
	
	RTS

CALCULATE_FILE_BYTES_LEFT:
	JSR READ_FILE_DIR_ENTRY
	JCS .end
	
	STC
	mov RB, #FAT_DIRENT_FILESIZE
	mov RA, [fat_dir_entry_pointer],RB
	SUB RA, fat_file_pointer
	mov fat_file_bytes_left, RA
	
	mov RB, #FAT_DIRENT_FILESIZE+1
	mov RA, [fat_dir_entry_pointer],RB
	SUB RA, fat_file_pointer+1
	mov fat_file_bytes_left+1, RA
	
	mov RB, #FAT_DIRENT_FILESIZE+2
	mov RA, [fat_dir_entry_pointer],RB
	SUB RA, fat_file_pointer+2
	mov fat_file_bytes_left+2, RA
	
	mov RB, #FAT_DIRENT_FILESIZE+3
	mov RA, [fat_dir_entry_pointer],RB
	SUB RA, fat_file_pointer+3
	mov fat_file_bytes_left+3, RA
	
	CLC
.end:
	RTS
	
RESERVE_ADDITIONAL_CLUSTER:
	; end of cluster chain => allocate new cluster
	JSR FAT_RESERVE_NEW_CLUSTER
	JCS .end
	
	; save last cluster
	mov RCD, fat_file_active_cluster
	
	; activate new cluster
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	
	; link new cluster to last cluster
	mov fat_work_cluster, RCD
	
	JSR FAT_GET_FAT_SECTOR
	JSR FAT_READ_FAT_SECTOR
	JCS .end
	
	JSR FAT_GET_FAT_OFFSET
	
	mov RA, RC
	mov RB, RD
	mva RCD, fat_block_buffer
	ADD RC, RA
	ADD RD, RB
	
	mov RA, fat_file_active_cluster
	mov [RCD], RA
	INC RCD
	mov RA, fat_file_active_cluster+1
	mov [RCD], RA
	JSR FAT_REWRITE_FAT_SECTOR
.end:
	RTS
	
RESERVE_FIRST_CLUSTER:
	JSR FAT_RESERVE_NEW_CLUSTER
	JCS .end
	
	mov RA, fat_work_cluster
	mov fat_file_active_cluster, RA
	mov RA, fat_work_cluster+1
	mov fat_file_active_cluster+1, RA
	
	JSR READ_FILE_DIR_ENTRY
	JCS .end
	JSR FAT_SET_FIRST_CLUSTER_DIR_ENTRY
	JSR FAT_REWRITE_DIR_SECTOR
	
.end:	
	RTS

COPY_CALLER_BUFFER_DATA:
	; copy caller parameter
	mov RB, #0
	
.loop:	
	mov RA, [RCD]
	mov fat_caller_buffer_data,RB, RA
	INC RCD
	INC RB
	CMP RB, #3 ; c=0 => CMP RB,#4
	JNC .loop
	
	RTS
	
FAT_CALCULATE_AVAILABLE_BUFFER_BYTES:		
	; filepointer & 511
	mov RCD, fat_file_pointer
	AND RD, #1
	
	; in write mode virtually no EOF
	mov RA, fat_file_mode
	AND RA, #O_WRITE
	JNZ .complete_buffer
	
	mov RA, fat_file_bytes_left+3
	AND RA, #0xff
	JNZ .complete_buffer
	mov RA, fat_file_bytes_left+2
	AND RA, #0xff
	JNZ .complete_buffer
	mov RA, fat_file_bytes_left+1
	AND RA, #0xfe
	JNZ .complete_buffer
	
	; file bytes left + filepointer > buffer size?
	CLC
	mov RA, fat_file_bytes_left
	ADD RA, RC
	mov RB, fat_file_bytes_left+1
	ADD RB, RD
	AND RB, #0xfe
	JNZ .complete_buffer
	
	mov RA, fat_file_bytes_left
	mov RB, fat_file_bytes_left+1
	JMP .partial_buffer
	
.complete_buffer:	
	; available bytes in file buffer
	; 512 - (filepointer & 511)	
	mov RA, #512[7:0]
	mov RB, #512[15:8]
	
	STC	
	SUB RA, RC
	SUB RB, RD
	
.partial_buffer:		
	; check if caller buffer size is less
	; than available fat buffer bytes
	STC
	CMP RA, fat_caller_buffer_size
	CMP RB, fat_caller_buffer_size+1
	JNC .size_OK
	mov RA, fat_caller_buffer_size
	mov RB, fat_caller_buffer_size+1
.size_OK:	
	mov block_move_structure+BLOCK_MOVE_SIZE, RA
	mov block_move_structure+BLOCK_MOVE_SIZE+1, RB

	RTS
	
FAT_ADVANCE_BUFFER_POINTERS:
	mov RCD, block_move_structure+BLOCK_MOVE_SIZE
	
	; advance file pointer
	CLC
	mov RA, fat_file_pointer
	ADD RA, RC
	mov fat_file_pointer, RA
	mov RA, fat_file_pointer+1
	ADD RA, RD
	mov fat_file_pointer+1, RA
	ICC fat_file_pointer+2
	ICC fat_file_pointer+3
	
	; advance caller buffer address
	mov RA, fat_caller_buffer_addr
	ADD RA, RC
	mov fat_caller_buffer_addr, RA
	mov RA, fat_caller_buffer_addr+1
	ADD RA, RD
	mov fat_caller_buffer_addr+1, RA

	; count moved bytes
	mov RA, fat_byte_count
	ADD RA, RC
	mov fat_byte_count, RA
	mov RA, fat_byte_count+1
	ADD RA, RD
	mov fat_byte_count+1, RA
	
	; reduce caller bytes left
	STC
	mov RA,  fat_caller_buffer_size
	SUB RA, RC
	mov fat_caller_buffer_size, RA
	mov RA,  fat_caller_buffer_size+1
	SUB RA, RD
	mov fat_caller_buffer_size+1, RA
	
	; reduce bytes left in file
	mov RA, fat_file_bytes_left
	SUB RA, RC
	mov fat_file_bytes_left, RA
	mov RA, fat_file_bytes_left+1
	SUB RA, RD
	mov fat_file_bytes_left+1, RA
	DCC fat_file_bytes_left+2
	DCC fat_file_bytes_left+3

	RTS

;
; Writes the file buffer to the SD card
;
; parms:
;   fat_file_active_cluster - active cluster number
;   fat_file_pointer - current file pointer
;   fat_file_buffer_pointer - pointer to file buffer
; result:
;   c-flag - 0 = O.K., 1 = error
;
FAT_WRITE_FILE_BUFFER:
	
	mov RA, fat_file_buffer_pointer
	mov RB, fat_file_buffer_pointer+1
	; fall-through to FAT_WRITE_WRITE_BUFFER
	
;
; Writes the file buffer to the SD card
;
; parms:
;   RA/RB - pointer to file buffer
;   fat_file_active_cluster - active cluster number
;   fat_file_pointer - current file pointer
; result:
;   c-flag - 0 = O.K., 1 = error
;
FAT_WRITE_WRITE_BUFFER:

	mov sdcard_buffer_pointer, RA
	mov sdcard_buffer_pointer+1, RB

;	mov RA, fat_file_pointer
;	mov RB, fat_file_pointer+1
;	PSH RB
;	; RAB = filepointer-1
;	DEC RA
;	DCC RB
;	mov fat_file_pointer+1, RB
	
	JSR FAT_FS_CLUSTER_FILE_POINTER_TO_SECTOR
	
;	PUL RB
;	mov fat_file_pointer+1, RB
	
	JSR SDCARD_WRITE_LBLOCK 
	JNC _FAT_NO_IO_ERROR

_FAT_IO_ERROR:
	mov RA, #EIO
_FAT_NO_IO_ERROR:
	RTS

;
; Reads a sector from the SD card into the file buffer
;
; parms:
;   fat_file_active_cluster - active cluster number
;   fat_file_pointer - current file pointer
;   fat_file_buffer_pointer - pointer to file buffer
; result:
;   c-flag - 0 = O.K., 1 = error
;
FAT_READ_FILE_BUFFER:	
	mov RA, fat_file_buffer_pointer
	mov RB, fat_file_buffer_pointer+1
	; fall-through to FAT_READ_READ_BUFFER

;
; Reads a sector from the SD card to the destination address
;
; parms:
;   RA/RB - pointer to file buffer
;   fat_file_active_cluster - active cluster number
;   fat_file_pointer - current file pointer
; result:
;   c-flag - 0 = O.K., 1 = error
;   fat_dir_entry_pointer - pointer to dir entry
;   RC/RD - pointer to dir entry
;
FAT_READ_READ_BUFFER:
	mov sdcard_buffer_pointer, RA
	mov sdcard_buffer_pointer+1, RB
	
	JSR FAT_FS_CLUSTER_FILE_POINTER_TO_SECTOR
	
	JSR SDCARD_READ_LBLOCK 
	JCS _FAT_IO_ERROR
.end:
	RTS
			
READ_FILE_DIR_ENTRY:
	; check for root directory
	mov RA, fat_file_mode
	AND RA, #O_DIR
	JZS .normal ; is a file
	mov RC, fat_file_active_cluster
	OR  RC, fat_file_active_cluster+1
	JNZ .normal ; is a subdirectory
	mov RD, RC ; is the root directory
	JMP .store_end
	
.normal: ; file or subdirectory
	mov RA, fat_file_dir_sector
	mov RB, fat_file_dir_sector+1
	mov RCD, fat_file_dir_sector+2

	JSR FAT_READ_DIR_SECTOR
	JCS .end
	
	mva RCD, fat_dir_buffer
	mov RA, fat_file_dir_index
	
	CLC ; add index * 2
	LSL RA
	ADD RD, #0
	ADD RC, RA
	ADD RD, #0
	
.store_end:	
	mov fat_dir_entry_pointer, RCD
	
.end:
	RTS
	
;	
; calculate the cluster index for the current file pointer
;
; Which position in the cluster chain for this file has the
; cluster that holds the byte on the current file pointer.
;
; Parms:
;  fat_file_pointer - current file pointer
; Returns:
;  RA/RB - Cluster index
;
FAT_FILE_POINTER_TO_CLUSTER_INDEX:
	mov RD, fat_sectors_per_cluster
	mov RA, fat_file_pointer+1
	mov RB, fat_file_pointer+2
	mov RC, fat_file_pointer+3
	CLC
.loop:
	LSR RC
	LSR RB
	LSR RA
	LSR RD
	JNC .loop
	RTS
	
;	
; calculate cluster for the current file pointer
;
; Parms:
;  fat_work_cluster      - first cluster number of the file
;  fat_file_pointer - current file pointer
; Returns:
;  fat_work_cluster      - Cluster number
;  C-Flag - 1= read error, 0= O.K.
;
FAT_FILE_POINTER_TO_CLUSTER:
	JSR FAT_FILE_POINTER_TO_CLUSTER_INDEX
.fat_chain_loop:
	mov fat_work_cluster_index, RA
	mov fat_work_cluster_index+1, RB
	OR  RA, RB
	OR  RA, #0
	JZS .end

	JSR FAT_GET_NEXT_CLUSTER
	JCS .end
	mov RA, fat_work_cluster_index
	mov RB, fat_work_cluster_index+1
	DEC RA
	DCC RB
	JCS .fat_chain_loop
.end:	
	RTS
	
FAT_FS_CLUSTER_FILE_POINTER_TO_SECTOR:
	mov RA, fat_file_pointer
	mov fat_work_file_pointer, RA
	mov RA, fat_file_pointer+1
	mov fat_work_file_pointer+1, RA
	mov RA, fat_file_pointer+2
	mov fat_work_file_pointer+2, RA
	mov RA, fat_file_pointer+3
	mov fat_work_file_pointer+3, RA
	
	mov RA, fat_file_active_cluster
	mov fat_work_cluster, RA
	mov RA, fat_file_active_cluster+1
	mov fat_work_cluster+1, RA
	JMP FAT_CLUSTER_FILE_POINTER_TO_SECTOR

#bank data

fat_caller_file_pointer:
fat_caller_buffer_data:
fat_caller_buffer_size: #res 2
fat_caller_buffer_addr: #res 2
fat_byte_count: #res 2
fat_file_bytes_left: #res 4

block_move_structure: #res 6
