;
; FAT File System functions
;

#bank pgm

FAT_BLOCK_SIZE = 512


; Boot Sector Field Offsets
BS_FAT_SECTORS_PER_CLUSTER = 13
BS_FAT_RESERVED_SECTORS = 14
BS_FAT_NUMBER_OF_FATS = 16
BS_FAT_NUMBER_OF_ROOT_ENTRIES = 17
BS_FAT_TOTAL_SECTORS_16 = 19
BS_FAT_SECTORS_PER_FAT = 22
BS_FAT_TOTAL_SECTORS_32 = 32
	
FAT_INIT:
	JSR SDCARD_INIT
	JCS .end
	
	JSR SPI_SELECT_CHIP0
	
	mov RA,#0
	mov fat_entry_name+11, RA
	
	mov fat_boot_sector_address, RA
	mov fat_boot_sector_address+1, RA
	mov fat_boot_sector_address+2, RA
	mov fat_boot_sector_address+3, RA
	
	mov RB, #1
	mov fat_block_buffer_sector, RB
	
	mov RB, RA
	mov RC, RA
	mov RD, RA
	
	JSR FAT_READ_FAT_SECTOR
	JCS .end
	
	; Check for Partition Entry	
	mov RA, fat_block_buffer+450
	OR  RA, #0
	JZS .boot_block_found
	
	mov RA, fat_block_buffer+454
	mov fat_boot_sector_address, RA
	mov RB, fat_block_buffer+455
	mov fat_boot_sector_address+1, RB
	mov RCD, fat_block_buffer+456
	mov fat_boot_sector_address+2, RCD

	JSR FAT_READ_FAT_SECTOR
	JCS .end
	
.boot_block_found:

	JSR FAT_CALCULATE_FIRST_FAT_SECTOR
	
	JSR FAT_CALCULATE_FIRST_ROOT_DIR_SECTOR
	JCS .end
	
	JSR FAT_CALCULATE_ROOT_DIR_SECTORS
	
	JSR FAT_CALCULATE_FIRST_DATA_SECTOR
			
	JSR FAT_CALCULATE_TOTAL_CLUSTERS
	JCS .end
			
.end:	
	RTS
;
FAT_CALCULATE_FIRST_FAT_SECTOR:
	CLC
	mov RA, fat_boot_sector_address
	ADD RA, fat_block_buffer+BS_FAT_RESERVED_SECTORS
	mov fat_first_sector, RA
	mov RA, fat_boot_sector_address+1
	ADD RA, fat_block_buffer+BS_FAT_RESERVED_SECTORS+1
	mov fat_first_sector+1, RA
	mov RA, fat_boot_sector_address+2
	ADD RA, #0
	mov fat_first_sector+2, RA
	mov RA, fat_boot_sector_address+3
	ADD RA, #0
	mov fat_first_sector+3, RA
	RTS
;
FAT_CALCULATE_FIRST_DATA_SECTOR:
	CLC
	mov RA, fat_first_root_dir_sector
	ADD RA, fat_root_dir_sectors
	mov fat_first_data_sector, RA
	mov RA, fat_first_root_dir_sector+1
	ADD RA, fat_root_dir_sectors+1
	mov fat_first_data_sector+1, RA
	mov RA, fat_first_root_dir_sector+2
	ADD RA, #0
	mov fat_first_data_sector+2, RA
	mov RA, fat_first_root_dir_sector+3
	ADD RA, #0
	mov fat_first_data_sector+3, RA
	RTS
;
FAT_CALCULATE_FIRST_ROOT_DIR_SECTOR:
	mov RC, fat_block_buffer+BS_FAT_NUMBER_OF_FATS
	STC 
	CMP RC, #2
	JZS .fat_numbers_OK
	STC
	RTS
	
.fat_numbers_OK:
	mov fat_number_of_fats, RC
	
	mov RA, fat_block_buffer+BS_FAT_SECTORS_PER_FAT	
	mov RB, fat_block_buffer+BS_FAT_SECTORS_PER_FAT+1
	mov fat_sectors_for_one_fat, RA
	mov fat_sectors_for_one_fat+1, RB
	CLC
	LSL RA
	LSL RB
	ADD RA, fat_first_sector
	mov fat_first_root_dir_sector, RA
	ADD RB, fat_first_sector+1
	mov fat_first_root_dir_sector+1, RB
	mov RA, fat_first_sector+2
	ADD RA, #0
	mov fat_first_root_dir_sector+2, RA
	mov RA, fat_first_sector+3
	ADD RA, #0
	mov fat_first_root_dir_sector+3, RA	
.end:	
	RTS
;
FAT_CALCULATE_ROOT_DIR_SECTORS:
	mov RA, fat_block_buffer+BS_FAT_NUMBER_OF_ROOT_ENTRIES
	mov RB, fat_block_buffer+BS_FAT_NUMBER_OF_ROOT_ENTRIES+1
	; divide by 16 (16 entries per sector)
	LSR RB
	LSR RA
	LSR RB
	LSR RA
	LSR RB
	LSR RA
	LSR RB
	LSR RA
	AND RB, #0x0f
	mov fat_root_dir_sectors, RA
	mov fat_root_dir_sectors+1, RB
	RTS
;
FAT_CALCULATE_TOTAL_CLUSTERS:
	mov RA, fat_block_buffer+BS_FAT_SECTORS_PER_CLUSTER
	mov fat_sectors_per_cluster, RA
	
	JSR FAT_CALCULATE_TOTAL_SECTORS
	
	STC
	SUB RA, fat_block_buffer+BS_FAT_RESERVED_SECTORS
	SUB RB, fat_block_buffer+BS_FAT_RESERVED_SECTORS+1
	SUB RC, #0
	SUB RD, #0
	
	; there are always two FATs
	SUB RA, fat_sectors_for_one_fat
	SUB RB, fat_sectors_for_one_fat+1
	SUB RC, #0
	SUB RD, #0
	SUB RA, fat_sectors_for_one_fat
	SUB RB, fat_sectors_for_one_fat+1
	SUB RC, #0
	SUB RD, #0
	
	SUB RA, fat_root_dir_sectors
	SUB RB, fat_root_dir_sectors+1
	SUB RC, #0
	SUB RD, #0
	
    mov fat_sector_in_cluster_offset_mask, RD
.divide_loop:
	LSR fat_sectors_per_cluster
	JCS .divide_end	
	LSR RD
	LSR RC
	LSR RB
	LSR RA
	STC
	LSL fat_sector_in_cluster_offset_mask
	JMP .divide_loop
	
.divide_end:	
	OR  RD, RC ; RA, RB RC, RD must be < 65525 (testing only 65535)
	JZS .fat16
	STC
	JMP .end
	
.fat16:	
	mov fat_total_clusters, RA
	mov fat_total_clusters+1, RB
	
	mov RA, fat_block_buffer+BS_FAT_SECTORS_PER_CLUSTER
	mov fat_sectors_per_cluster, RA
	
	CLC
.end:	
	RTS
;
FAT_CALCULATE_TOTAL_SECTORS:
	mov RA, fat_block_buffer+BS_FAT_TOTAL_SECTORS_16
	mov RB, fat_block_buffer+BS_FAT_TOTAL_SECTORS_16+1
	mov RC, RA
	OR  RC, RB
	JNZ .sect16
	mov RA, fat_block_buffer+BS_FAT_TOTAL_SECTORS_32
	mov RB, fat_block_buffer+BS_FAT_TOTAL_SECTORS_32+1
	mov RCD, fat_block_buffer+BS_FAT_TOTAL_SECTORS_32+2
	RTS
.sect16:
	mov RC, #0
	mov RD, RC
	RTS
	
;
; calculate the FAT sector for the current cluster
;
; Parms:
;  fat_work_cluster      - current cluster number
; Returns:
;  RA/RB/RC/RD       - sector number with the FAT entry for this cluster
;
FAT_GET_FAT_SECTOR:
	; sector = cluster*2/512 + fat_first_sector
	mov RA, fat_work_cluster+1
	mov RB, #0
	mov RC, #0
	mov RD, #0
	CLC
	ADD RA, fat_first_sector
	ADD RB, fat_first_sector+1
	ADD RC, fat_first_sector+2
	ADD RD, fat_first_sector+3
	RTS

;
; get the offset of the FAT entry in the FAT sector of the current cluster
;
; Parms:
;  fat_work_cluster      - current cluster number
; Returns:
;  RC/RD  - FAT entry offset
;
FAT_GET_FAT_OFFSET:
	; offset = (cluster % 256) *2
	mov RC, fat_work_cluster
	mov RD, #0
	CLC
	LSL RC
	LSL RD
	RTS

;
; Read the FAT and get the next cluster for the current cluster
;
; Parms:
;  fat_work_cluster      - current cluster number
; Returns:
;  fat_work_cluster      - next cluster number
;  C-Flag - 1= read error, 0= O.K.
;
FAT_GET_NEXT_CLUSTER:
	JSR FAT_GET_FAT_SECTOR
	JSR FAT_READ_FAT_SECTOR
	JCS .end
	
	JSR FAT_GET_FAT_OFFSET
	
	ADD RC, #fat_block_buffer[7:0]
	ADD RD, #fat_block_buffer[15:8]
	mov RA, [RCD]
	mov fat_work_cluster, RA
	INC RCD
	mov RA, [RCD]
	mov fat_work_cluster+1, RA
	
.end:	
	RTS

;
; Read a FAT sector into the FAT buffer
;
; Parms:
;  RA/RB/RC/RD - FAT sector number
; Returns:
;  fat_block_buffer - content of the FAT sector
;  C-Flag - 1= read error, 0= O.K.
;
FAT_READ_FAT_SECTOR:
	STC
	CMP RA, fat_block_buffer_sector
	JNZ .read_fat
	CMP RB, fat_block_buffer_sector+1
	JNZ .read_fat
	CMP RC, fat_block_buffer_sector+2
	JNZ .read_fat
	CMP RD, fat_block_buffer_sector+3
	JNZ .read_fat
	CLC
	RTS
	
.read_fat:	
	mov sdcard_block_address, RA
	mov sdcard_block_address+1, RB
	mov sdcard_block_address+2, RCD
	mov fat_block_buffer_sector, RA
	mov fat_block_buffer_sector+1, RB
	mov fat_block_buffer_sector+2, RCD
	
	mov RC, #fat_block_buffer[7:0]
	mov RD, #fat_block_buffer[15:8]
	mov sdcard_buffer_pointer, RCD
	JSR SDCARD_READ_LBLOCK
	JNC .end
	
.error_exit:	
	mov RA, #0
	mov fat_block_buffer_sector, RA
	mov fat_block_buffer_sector+1, RA
	mov fat_block_buffer_sector+2, RA
	mov fat_block_buffer_sector+3, RA
	mov RA, #EIO
.end:	
	RTS

;
; Read a directory sector into the DIR buffer
;
; Parms:
;  RA/RB/RC/RD - directory sector number
; Returns:
;  fat_dir_buffer - content of the directory sector
;  C-Flag - 1= read error, 0= O.K.
;
FAT_READ_DIR_SECTOR:
	STC
	CMP RA, fat_dir_buffer_sector
	JNZ .read_fat
	CMP RB, fat_dir_buffer_sector+1
	JNZ .read_fat
	CMP RC, fat_dir_buffer_sector+2
	JNZ .read_fat
	CMP RD, fat_dir_buffer_sector+3
	JNZ .read_fat
	CLC
	RTS
	
.read_fat:	
	mov sdcard_block_address, RA
	mov sdcard_block_address+1, RB
	mov sdcard_block_address+2, RCD
	mov fat_dir_buffer_sector, RA
	mov fat_dir_buffer_sector+1, RB
	mov fat_dir_buffer_sector+2, RCD
	
	mov RC, #fat_dir_buffer[7:0]
	mov RD, #fat_dir_buffer[15:8]
	mov sdcard_buffer_pointer, RCD
	JSR SDCARD_READ_LBLOCK
	JNC .end
	
.error_exit:	
	mov RA, #0
	mov fat_dir_buffer_sector, RA
	mov fat_dir_buffer_sector+1, RA
	mov fat_dir_buffer_sector+2, RA
	mov fat_dir_buffer_sector+3, RA
	mov RA, #EIO
.end:	
	RTS
	
;
; calculate sector for the root dir file pointer
;
; Parms:
;  fat_work_file_pointer - current root dir file pointer
; Returns:
;  RA/RB/RC/RD - Sector number
;  sdcard_block_address - Sector number
;  C-Flag - 1= read error, 0= O.K.
;
FAT_FILE_POINTER_TO_ROOT_DIR_SECTOR:
	CLC
	mov RC, fat_work_file_pointer+3 
	LSR RC
	mov RB, fat_work_file_pointer+2
	LSR RB
	mov RA, fat_work_file_pointer+1
	LSR RA
	CLC
	ADD RA, fat_first_root_dir_sector
	mov sdcard_block_address, RA
	ADD RB, fat_first_root_dir_sector+1
	mov sdcard_block_address+1, RB
	ADD RC, fat_first_root_dir_sector+2
	mov RD, #0
	ADD RD, fat_first_root_dir_sector+3
	mov sdcard_block_address+2, RCD
.end:
	RTS
	
;	
; calculate sector for the current cluster and file pointer
;
; Parms:
;  fat_work_cluster - current cluster number of the file
;  fat_work_file_pointer - current file pointer
; Returns:
;  RA/RB/RC/RD - Sector number
;  sdcard_block_address - Sector number
;
FAT_CLUSTER_FILE_POINTER_TO_SECTOR:
	JSR FAT_CLUSTER_TO_SECTOR
	; FAT_FILE_POINTER to SECTOR_OFFSET
	mov RA, fat_work_file_pointer+1
	LSR RA
	AND RA, fat_sector_in_cluster_offset_mask
	
	CLC
	ADD RA, fat_work_sector
	mov sdcard_block_address, RA
	mov RB, #0
	ADD RB, fat_work_sector+1
	mov sdcard_block_address+1, RB
	mov RC, #0
	ADD RC, fat_work_sector+2
	mov RD, #0
	ADD RD, fat_work_sector+3
	mov sdcard_block_address+2, RCD
.end:
	RTS
	
;	
; calculate first sector for cluster
;
; Parms:
;  fat_work_cluster - Cluster number
; Returns:
;  fat_work_sector - Sector number
;  RA/RB/RC/RD - Sector number
;
FAT_CLUSTER_TO_SECTOR:
; sector = (cluster - 2) * fat_sectors_per_cluster + fat_first_data_sector
	STC
	mov RA, fat_work_cluster
	SUB RA, #2
	mov fat_work_sector, RA
	mov RA, fat_work_cluster+1
	SUB RA, #0
	mov fat_work_sector+1, RA
	mov RA, #0
	mov fat_work_sector+2, RA
	mov fat_work_sector+3, RA
	
	mov RA, fat_sectors_per_cluster
.mul_loop:	
	LSR RA
	JCS .mul_end
	LSL fat_work_sector
	LSL fat_work_sector+1
	LSL fat_work_sector+2
	LSL fat_work_sector+3
	JMP .mul_loop
.mul_end:	
	CLC
	mov RA, fat_work_sector
	ADD RA, fat_first_data_sector
	mov fat_work_sector, RA
	mov RB, fat_work_sector+1
	ADD RB, fat_first_data_sector+1
	mov fat_work_sector+1, RB
	mov RC, fat_work_sector+2
	ADD RC, fat_first_data_sector+2
	mov RD, fat_work_sector+3
	ADD RD, fat_first_data_sector+3
	mov fat_work_sector+2, RCD
.end:	
	RTS
	
;
; searches the directory entry for a file in the FAT directory
;
; Parms:
;  RC/RD - pointer to null terminated path and filename
; Returns:
;  C-Flag - 1=not found, 0=found
;  fat_dir_entry_pointer - pointer to directory entry
;                          0=root dir
;	
FAT_FIND_DIR_ENTRY:
	mov RA, #2
	mov fat_trials_left_cnt, RA
	mov fat_name_start_pointer, RCD
.retry:
	mov RA, #0
	mov fat_dir_entry_pointer, RA
	mov fat_dir_entry_pointer+1, RA
	mov fat_work_cluster, RA
	mov fat_work_cluster+1, RA

.path_loop:	
	mov RA, #0
	mov fat_work_file_pointer, RA
	mov fat_work_file_pointer+1, RA
	mov fat_work_file_pointer+2, RA
	mov fat_work_file_pointer+3, RA
	
.name_loop:	
	mov RA, [RCD]
	OR  RA, #0
	JNZ .more_parts
	CLC
	RTS
	
.more_parts:	
	STC 
	CMP RA, #"/"
	JNZ .name_start
	INC RCD
	JMP .name_loop
	
.name_start:	
	JSR GET_FAT_ENTRY_NAME
	
	mov fat_name_pointer, RCD
	mva RCD, fat_entry_name
	JSR OS_TO_UPPER
	
.dir_read_loop:
	JSR FAT_NEXT_DIR_ENTRY
	JNC .read_OK
	DEC fat_trials_left_cnt
	JZS .end
	JSR FAT_INIT
	mov RCD, fat_name_start_pointer
	JMP .retry
	
.read_OK:
	mov fat_dir_entry_pointer, RCD
	
	; end of directory mark
	mov RB, #0
	mov RA, [fat_dir_entry_pointer],RB
	OR  RA, #0
	JZS .notfound
	
	; empty entry mark
	XOR RA, #0xe5 
	JZS .dir_read_loop
	
	; special file mark
	mov RB, #FAT_DIRENT_ATTR
	mov RA, [fat_dir_entry_pointer],RB
	AND RA, #0x0f
	XOR RA, #0x0f
	JZS .dir_read_loop
	
	mov RB, #FAT_DIRENT_ATTR-1
.testloop:
	mov RA, [fat_dir_entry_pointer],RB
	mov RD, RA
	mov RA, fat_entry_name,RB
	STC
	CMP RA, RD
	JNZ .dir_read_loop
	DEC RB
	JCS .testloop
	
.namefound:	
	mov RCD, fat_name_pointer
	mov RB, #FAT_DIRENT_FSTCLUSLO
	mov RA, [fat_dir_entry_pointer], RB
	mov fat_work_cluster, RA
	mov RB, #FAT_DIRENT_FSTCLUSLO+1
	mov RA, [fat_dir_entry_pointer], RB
	mov fat_work_cluster+1, RA
	JMP .path_loop
	
.notfound:
    mov RA, #ENOENT
	STC
.end:	
	RTS

;
; get first part of a path name in directory format
;
; parm:
;  RC/RD - pointer to 0-terminated path
; result:
;  fat_entry_name - 11 byte formated first part
;	
GET_FAT_ENTRY_NAME:	
	mov RB, #0
.loop:	
	mov RA, [RCD]
	OR  RA, #0
	JZS .fillend
	STC
	CMP RA, #0x2F ; "/"
	JZS .fillend
	STC
	CMP RA, #0x2E ; "."
	JZS .fill8
	mov fat_entry_name,RB, RA
	INC RB
	INC RCD
	
.checkend:	
	STC 
	CMP RB, #11
	JNC .loop
	
.endoutstring:
	mov RA, [RCD]
	OR  RA, #0
	JZS .end
	INC RCD
.end:
	RTS
	
.fill8:	
	INC RCD
	PSH RD
	mov RD, #8
	JMP .fillD
.fillend:	
	PSH RD
	mov RD, #11
.fillD:
	mov RA, #" "
.fillDloop:
	STC 
	CMP RD, RB
	JZS .fillDend
	mov fat_entry_name,RB, RA
	INC RB
	JMP .fillDloop
.fillDend:	
	PUL RD
	JMP .checkend
;

;
; convert string to upper case
;
; parm:
;  RC/RD - pointer to 0-terminated string
; result:
;  (converted string inplace)
;
OS_TO_UPPER:
.loop:
	mov RA, [RCD]
	OR  RA, #0
	JZS .end
	STC
	CMP RA, #"a" ; 'a'
	JNC .next
	CMP RA, #("z"+1) ; 'z'
	JCS .next
	AND RA, #0b11011111
	mov [RCD], RA
.next:	
	INC RCD
	JNC .loop
.end:	
	RTS
	
;
; calculate directory sector for the current file pointer
;
; Parms:
;  fat_work_cluster - current cluster number of the directory
;  fat_work_file_pointer - current file pointer
; Returns:
;  RA/RB/RC/RD - Sector number
;
FAT_CLUSTER_FILE_POINTER_TO_DIR_SECTOR:
	mov RA, fat_work_cluster
	OR  RA, fat_work_cluster+1
	JZS FAT_FILE_POINTER_TO_ROOT_DIR_SECTOR
	
	JMP FAT_CLUSTER_FILE_POINTER_TO_SECTOR
	
;
; returns the next directory entry in the FAT directory
;
; Parms:
;  fat_work_cluster -
;  fat_work_file_pointer - 
; Returns:
;  C-Flag - 1=error, 0=O.K.
;  RA - error number
;  RC/RD  - pointer to next 32 byte FAT DIR ENTRY
;	
FAT_NEXT_DIR_ENTRY:	
	mov RCD, fat_work_file_pointer
	
	; filepointer (fp) at sector start (fp%512 == 0)?
	AND RD, #1
	JNZ .sector_OK
	OR  RC, #0
	JZS .sector_not_OK
.sector_OK:
	CLC
	ADD RC, #fat_dir_buffer[7:0]
	ADD RD, #fat_dir_buffer[15:8]
	JMP .ok_end

.sector_not_OK:
	; first entry?
	mov RD, fat_work_file_pointer+1	
	OR  RD, #0
	JZS .read_cluster
	
	; filepointer at cluster start?
	LSR RD
	AND RD, fat_sector_in_cluster_offset_mask
	JNZ .read_cluster
	
	; end of root directory?
	mov RA, fat_work_cluster
	OR  RA, fat_work_cluster+1
	JZS .dirfull
	
.next_cluster:
	JSR FAT_GET_NEXT_CLUSTER
	JCS .end
	
.check_EOC:
	mov RA, fat_work_cluster
	AND RA, fat_work_cluster+1
	XOR RA, #0xff
	JZS .read_cluster
.dirfull:	
	mov RA, #ENOSPC
	STC
	RTS
		
.read_cluster:
	JSR FAT_CLUSTER_FILE_POINTER_TO_DIR_SECTOR
	JSR FAT_READ_DIR_SECTOR
	JCS .end
	
	mov RC, #fat_dir_buffer[7:0]
	mov RD, #fat_dir_buffer[15:8]
	
.ok_end:
	mov RA, fat_work_file_pointer
	ADD RA, #32
	mov fat_work_file_pointer, RA
	ICC fat_work_file_pointer+1
	ICC fat_work_file_pointer+2
	ICC fat_work_file_pointer+3
	
.end:
	RTS

#bank data

fat_name_pointer: #res 2
fat_name_start_pointer: #res 2
fat_entry_name: #res 12
fat_dir_entry_pointer: #res 2

fat_trials_left_cnt: #res 1

fat_work_cluster: #res 2
fat_work_sector: #res 4
fat_work_cluster_index: #res 2
fat_work_file_pointer: #res 4
fat_boot_sector_address: #res 4
fat_sectors_for_one_fat: #res 2
fat_sectors_per_cluster: #res 1
fat_sector_in_cluster_offset_mask: #res 1
fat_number_of_fats: #res 1
fat_first_sector: #res 4
fat_first_root_dir_sector: #res 4
fat_first_data_sector: #res 4
fat_root_dir_sectors: #res 2
fat_total_clusters: #res 2
fat_block_buffer_sector: #res 4
fat_dir_buffer_sector: #res 4

fat_block_buffer: #res FAT_BLOCK_SIZE
fat_dir_buffer: #res FAT_BLOCK_SIZE