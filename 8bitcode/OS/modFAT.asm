#include "8bit.cpu"

#include "OSCalls.inc"
#include "OSint.inc"

;
; modFAT - OS module for FAT file system
;
; Written 03.03.2021 Ulf Caspers
;
 
#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x0000
}

#bankdef data
{
    #addr 0x1000
    #size 0x2000
}

RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm
MOD_FAT_INIT:
	mva RCD, init_msg
	JSR DISPLAY_STRING
	
	mov RA, #0
	mov fat_free_cluster_hint_offset, RA
	
	mva RCD, FAT_OPEN
	mov fat_fs_open_ptr, RCD
	
	mva RCD, FAT_CLOSE
	mov fat_fs_close_ptr, RCD
	
	mva RCD, FAT_READ
	mov fat_fs_read_ptr, RCD
	
	mva RCD, FAT_WRITE
	mov fat_fs_write_ptr, RCD
	
	mva RCD, FAT_SEEK
	mov fat_fs_seek_ptr, RCD
	
	mva RCD, FAT_TELL
	mov fat_fs_tell_ptr, RCD
	
	mva RCD, FAT_MKDIR
	mov fat_fs_mkdir_ptr, RCD
	
	mva RCD, FAT_UNLINK
	mov fat_fs_unlink_ptr, RCD
	
	mva RCD, FAT_STAT
	mov fat_fs_stat_ptr, RCD
	
	mva RCD, FAT_FSSTAT
	mov fat_fs_fsstat_ptr, RCD
	
	mva RCD, FAT_FLUSH
	mov fat_fs_flush_ptr, RCD

	CLC
	RTS
	
#bank pgm
#include "BlockMov.inc"
#bank pgm
#include "FATWrite.inc"
#bank pgm
#include "FATDIR.inc"
#bank pgm
#include "FATCanFN.inc"
#bank pgm
#include "FATFS.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00	

init_msg: #d "Init FAT...\n\0"

;
; program trailer
;	
	ld16 DATA_START
	ld16 MOD_DATA_END - DATA_START

#bank data
MOD_DATA_END: