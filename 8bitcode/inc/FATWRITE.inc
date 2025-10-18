;
; FAT File System write functions
;

#bank pgm

;
; Write FAT sector in the FAT buffer
;
; Parms:
;  fat_block_buffer - content of the FAT sector
;  fat_block_buffer_sector - FAT sector number
; Returns:
;  C-Flag - 1= write error, 0= O.K.
;
FAT_REWRITE_FAT_SECTOR:
	mov RA, fat_block_buffer_sector
	mov RB, fat_block_buffer_sector+1
	mov RCD, fat_block_buffer_sector+2
	mov sdcard_block_address, RA
	mov sdcard_block_address+1, RB
	mov sdcard_block_address+2, RCD
	mva RCD, fat_block_buffer
	JMP _FAT_WRITE_SDCARD_BUFFER

;
; Write last directory sector back onto media
;
; Parms:
;  fat_dir_buffer - content of the FAT sector
;  fat_dir_buffer_sector - FAT sector number
; Returns:
;  C-Flag - 1= write error, 0= O.K.
;
FAT_REWRITE_DIR_SECTOR:
	mov RA, fat_dir_buffer_sector
	mov RB, fat_dir_buffer_sector+1
	mov RC, fat_dir_buffer_sector+2
	mov RD, fat_dir_buffer_sector+3
	mov sdcard_block_address, RA
	mov sdcard_block_address+1, RB
	mov sdcard_block_address+2, RCD
	mva RCD, fat_dir_buffer
	
_FAT_WRITE_SDCARD_BUFFER:	
	mov sdcard_buffer_pointer, RCD
	JSR SDCARD_WRITE_LBLOCK
	JNC .end
	mov RA, #EIO
.end:
	RTS

;
; Counts all unallocated clusters
; in the File Allocation Table
;
; returns:
;  C-Flag - 0 = O.K., 1 = read error
;  RA -  error number
;  fat_free_cluster_count - number of free clusters
;	
FAT_COUNT_FREE_CLUSTERS:
	mov RA, #0
	mov fat_free_cluster_count, RA
	mov fat_free_cluster_count+1, RA
	
	JSR _FAT_START_FREE_CLUSTER
	JCS .last_cluster
	
	STC 
	mov RA, fat_work_sector
	SUB RA, fat_first_sector
	mov fat_work_cluster+1, RA
	mov fat_free_cluster_hint_offset, RA

.loop:	
	INC fat_free_cluster_count
	ICC fat_free_cluster_count+1
	
	JSR _FAT_NEXT_FREE_CLUSTER
	JNC .loop
	
.last_cluster:	
	; check for NO SPACE ERROR (no free cluster)
	CMP RA, #ENOSPC
	JNZ .error_end	
	; no free cluster is O.K. for counting
	CLC
	RTS

.error_end:	
	STC
	RTS

;
; Searches for an unallocated cluster
; in the File Allocation Table
;
; returns:
;  C-Flag - 0 = O.K., 1 = read error
;  RA -  error number
;  fat_work_sector - FAT sector number
;                    for the free cluster
;  fat_work_cluster - free cluster number
;	
FAT_FIND_FREE_CLUSTER:
	mov RA, fat_free_cluster_hint_offset
.loop:	
	JSR _FAT_START_FREE_CLUSTER
	JNC .cluster_found

	CMP RA, #EIO
	JZS .end
	mov RB, fat_free_cluster_hint_offset
	OR  RB, #0
	JNZ .try_again
	STC
	RTS
.try_again:
	mov RA, #0
	mov fat_free_cluster_hint_offset, RA
	JMP .loop
	
.cluster_found:
	mov fat_work_cluster, RB
	STC 
	mov RA, fat_work_sector
	SUB RA, fat_first_sector
	mov fat_work_cluster+1, RA
	mov fat_free_cluster_hint_offset, RA
	CLC
.end:	
	RTS
	

_FAT_START_FREE_CLUSTER:	
	CLC
	mov RB, fat_first_sector
	ADD RB, RA
	mov fat_work_sector, RB
	mov RB, fat_first_sector+1
	ADD RB, #0
	mov fat_work_sector+1, RB
	mov RB, fat_first_sector+2
	ADD RB, #0
	mov fat_work_sector+2, RB
	mov RB, fat_first_sector+3
	ADD RB, #0
	mov fat_work_sector+3, RB
	
	STC
	mov RB, fat_sectors_for_one_fat
	SUB RB, RA
	mov fat_block_counter, RB
	OUT RB
	
_FAT_FREE_CLUSTER_fat_loop:		
	mov RA, fat_work_sector
	mov RB, fat_work_sector+1
	mov RCD, fat_work_sector+2
	JSR FAT_READ_FAT_SECTOR	
	JCS _FAT_WRITE_end

	mov RB, #0
	mva RCD, fat_block_buffer
_FAT_FREE_CLUSTER_block_loop:	
	; empty cluster? (FAT entry = 0x0000)
	mov RA, [RCD]
	INC RCD
	OR  RA, [RCD]
	; yes, empty cluster found
	JZS _FAT_WRITE_end
	
_FAT_NEXT_FREE_CLUSTER:
    ; skip second byte of FAT entry
	INC RCD
	
	; check next FAT entry in the sector
	INC RB
	; end of sector?
	JNZ _FAT_FREE_CLUSTER_block_loop
	
	; check next FAT sector
	INC fat_work_sector
	ICC fat_work_sector+1
	ICC fat_work_sector+2
	ICC fat_work_sector+3
	
	DEC fat_block_counter
	mov RA, fat_block_counter
	OUT RA
	JNZ _FAT_FREE_CLUSTER_fat_loop
	
.no_empty_cluster:
	mov RA, #ENOSPC
	
_FAT_WRITE_end:
	RTS

;
; Searches for an unallocated cluster
; in the File Allocation Table an reserves
; it with an EOF mark
;
; returns:
;  C-Flag - 0 = O.K., 1 = read error or disk full
;  RA -  error number
;  fat_work_cluster - free cluster number
;                  
;
FAT_RESERVE_NEW_CLUSTER:
	JSR FAT_FIND_FREE_CLUSTER
	JCS _FAT_WRITE_end
	
.cluster_OK:	
	mva RCD, fat_block_buffer
	; add cluster offset 
	mov RA, fat_work_cluster
	CLC
	LSL RA
	ADD RD, #0
	ADD RC, RA
	ADD RD, #0
	
	; EOF mark 0xffff
	mov RA, #0xff
	mov [RCD], RA
	INC RCD
	mov [RCD], RA
		
	JMP FAT_REWRITE_FAT_SECTOR

;
; free all clusters in the FAT that
; belong to the chain starting at
; fat_work_cluster
;
; parms:
;  fat_work_cluster - first cluster in chain
; return:
;  C-flag - 0 = O.K., 1 = error
;
FAT_FREE_CLUSTER_CHAIN:
	JSR FAT_GET_FAT_SECTOR
	JSR FAT_READ_FAT_SECTOR
	JCS .end
	
	JSR FAT_GET_FAT_OFFSET
	mov RA, RC
	mov RB, RD
	mva RCD, fat_block_buffer
	ADD RC, RA
	ADD RD, RB
	
	mov RA, [RCD]
	mov fat_work_cluster, RA
	mov RA, #0
	mov [RCD], RA

	INC RCD

	mov RA, [RCD]
	mov fat_work_cluster+1, RA
	mov RA, #0
	mov [RCD], RA
	
	JSR FAT_REWRITE_FAT_SECTOR
	JCS .end
	
	; check for end of chain (>=0xfff8)
	STC
	mov RA, fat_work_cluster
	CMP RA, #0xf8
	mov RA, fat_work_cluster+1
	CMP RA, #0xff
	JNC FAT_FREE_CLUSTER_CHAIN
	
	CLC
.end:	
	RTS

;
; converts the time in OS Format
; to time in FAT DIR format
;
; parm:
;  RC/RD - pointer to OS time buffer
; returns:
;  RA/RB - FAT DIR time bytes
;	
FAT_OS_TIME_2_DIR_DATE:
	; day
	CLC
	ADD RC, #4
	ADD RD, #0
	mov RA, [RCD]
	JSR BCD_TO_BYTE
	mov fat_date_time_area, RA
	
	; month
	INC RCD
	mov RA, [RCD]
	AND RA, #0x7f
	JSR BCD_TO_BYTE
	mov RB, #0
	LSL RA
	LSL RA
	LSL RA
	LSL RA
	LSL RA
	LSL RB
	OR  RA, fat_date_time_area
	mov fat_date_time_area, RA
	mov fat_date_time_area+1, RB
	
	; year
	INC RCD
	mov RA, [RCD]
	JSR BCD_TO_BYTE
	ADD RA, #20
	LSL RA
	OR  RA, fat_date_time_area+1
	mov fat_date_time_area+1, RA
	
	mov RA, fat_date_time_area
	mov RB, fat_date_time_area+1
	RTS
	
;
; converts the time in OS Format
; to time in FAT DIR format
;
; parm:
;  RC/RD - pointer to OS time buffer
; returns:
;  RA/RB - FAT DIR time bytes
;	
FAT_OS_TIME_2_DIR_TIME:
	; seconds
	mov RA, [RCD]
	JSR BCD_TO_BYTE
	LSR RA
	mov fat_date_time_area, RA
	
	; minutes
	INC RCD
	mov RA, [RCD]
	JSR BCD_TO_BYTE
	mov RB, #0
	LSL RA
	LSL RA
	LSL RA
	LSL RB
	LSL RA
	LSL RB
	LSL RA
	LSL RB
	OR  RA, fat_date_time_area
	mov fat_date_time_area, RA
	mov fat_date_time_area+1, RB
	
	; hours
	INC RCD
	mov RA, [RCD]
	JSR BCD_TO_BYTE
	LSL RA
	LSL RA
	LSL RA
	OR  RA, fat_date_time_area+1
	mov fat_date_time_area+1, RA
	
	mov RA, fat_date_time_area
	mov RB, fat_date_time_area+1
	RTS
	
BCD_TO_BYTE:
	PSH RA
	; multiply High nybble by 10
	; =hi*8+hi*2
	AND RA, #0b11110000 ; RA=hi*16
	CLC
	LSR RA ; RA=hi*8
	mov RB, RA ; RB=hi*8
	LSR RB ; RB=hi*4
	LSR RB ; RB=hi*2
	ADD RA, RB ; RA=hi*8+hi*2
	; add low nybble
	PUL RB
	AND RB, #0x0f
	ADD RA, RB
	RTS

#bank data

fat_block_counter: #res 1
fat_free_cluster_hint_offset: #res 1
fat_date_time_area: #res 2
fat_free_cluster_count: #res 2
