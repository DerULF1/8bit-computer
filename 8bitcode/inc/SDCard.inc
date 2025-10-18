;
; SDCard - Secure Digital Memory Card Functions
;

#bank pgm

;
; Initializes the SD Card for access
; 
; Retuns
; C-Flag - 1=error 0=O.K.
;
SDCARD_INIT:
	mov RD, #2
.power_up_loop:
	PSH RD
	JSR SPI_INIT
	
	JSR SPI_SELECT_CHIP0

	mov RA,#0x95
	mov sdcard_cmd_crc, RA
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov sdcard_cmd_parm+2, RA
	mov sdcard_cmd_parm+3, RA
	JSR SDCARD_SEND_COMMAND		
	PUL RD
	JNC .init_no_resp
	CMP RA, #01
	JZS .CMD0_OK
	
.init_no_resp:	
	STC 
	SUB RD, #1
	JZS .error_end
	
	JSR SDCARD_WAIT
	JMP .power_up_loop
	
.CMD0_OK:	
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov RA,#0x01
	mov sdcard_cmd_parm+2, RA
	mov RA,#0xaa
	mov sdcard_cmd_parm+3, RA
	mov RA,#0x87
	mov sdcard_cmd_crc, RA
	mov RA, #8
	JSR SDCARD_SEND_COMMAND	
	JNC .error_end
	CMP RA, #01
	JNZ .error_end
	
	JSR SDCARD_READ_32BIT_RESPONSE
	
	mov RD, #0	
.init_loop:	
	PSH RD
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov sdcard_cmd_parm+2, RA
	mov sdcard_cmd_parm+3, RA
	mov RA,#0xff
	mov sdcard_cmd_crc, RA
	mov RA, #55
	JSR SDCARD_SEND_COMMAND
	
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov sdcard_cmd_parm+2, RA
	mov sdcard_cmd_parm+3, RA
	mov RA,#0xff
	mov sdcard_cmd_crc, RA
	mov RA, #41
	JSR SDCARD_SEND_COMMAND
	PUL RD
	OR  RA, #0
	JZS .init_done
	CLC
	ADD RD, #1
	JZS .error_end
	JMP .init_loop

.init_done:					
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov sdcard_cmd_parm+2, RA
	mov sdcard_cmd_parm+3, RA
	mov RA,#0xff
	mov sdcard_cmd_crc, RA
	mov RA, #58
	JSR SDCARD_SEND_COMMAND
	
	JNC .error_end
	OR  RA, #0
	JNZ .error_end
	
	JSR SDCARD_READ_32BIT_RESPONSE
	
	mov RA, sdcard_cmd_response
	AND RA, #0x40
	JNZ .ver_2_plus
	
	mov RA, #0
	mov sdcard_cmd_parm, RA
	mov sdcard_cmd_parm+1, RA
	mov sdcard_cmd_parm+3, RA
	mov RA, #0x02
	mov sdcard_cmd_parm+2, RA
	mov RA,#0xff
	mov sdcard_cmd_crc, RA
	mov RA, #16
	JSR SDCARD_SEND_COMMAND
	
	JNC .error_end
	OR  RA, #0
	JNZ .error_end
		
.ver_2_plus:
	CLC
	JMP .end
	
.error_end:
	STC
.end:
	PSH RA
	PSF
	JSR SPI_DESELECT_CHIPS
	PLF
	PUL RA
;	
	RTS
;
; waits about 500ms for the SD Card to power up
;	
SDCARD_WAIT:
	mov RA, #0x40
	mov RB, #0xFF
.wait_loop:	
	STC
	SUB RB, #1
	JCS .wait_loop
	STC
	SUB RA, #1
	JCS .wait_loop
	RTS
;
; Sends a command to the SD Card via SPI
;
; Parms
;  RA - Command Number
;  sdcard_cmd_parm - 4 command parameter bytes
;  sdcard_cmd_crc  - command CRC
; Returns
;  C-Flag - 0=no null bit found, 1=O.K.
;  RA     - Response Byte
;
SDCARD_SEND_COMMAND:
	OR  RA,#0x40
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_cmd_parm
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_cmd_parm+1
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_cmd_parm+2
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_cmd_parm+3
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_cmd_crc
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	JMP SPI_RECEIVE_RESPONSE_BYTE	
;	
; Reads the next 32 bits
;
; Returns
;  sdcard_cmd_response - the next 32 bits
;
SDCARD_READ_32BIT_RESPONSE:
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	mov sdcard_cmd_response, RA
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	mov sdcard_cmd_response+1, RA
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	mov sdcard_cmd_response+2, RA
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	mov sdcard_cmd_response+3, RA
	RTS
;
; reads a block with the block number in sdcard_block_address
;
SDCARD_LBLOCK_TO_ADDRESS:
	; multiply block number by 512 (block_size)
	; to get the byte address
	CLC
	mov RA, #0	
	mov RB, sdcard_block_address
	mov sdcard_block_address, RA
	LSL RB
	mov RA, sdcard_block_address+1
	mov sdcard_block_address+1, RB
	LSL RA
	mov RB, sdcard_block_address+2
	mov sdcard_block_address+2, RA
	LSL RB
	mov sdcard_block_address+3, RB
	RTS
;
; reads a block with the block number in sdcard_block_address
;
SDCARD_READ_LBLOCK:
	JSR SDCARD_LBLOCK_TO_ADDRESS
;
; reads a block with the byte address in sdcard_block_address
;
SDCARD_READ_BLOCK:
	PSF
	SII
	mov RA,#0x40 + 17
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+3
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+2
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+1
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,#0xff
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	JSR SPI_RECEIVE_RESPONSE_BYTE
	OR  RA,#0
	JNZ .resp_not_OK
.resp_OK:
	mov RD, #0
.resp_loop:	
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	STC
	CMP RA,#0xfe
	JZS .data_OK
	CLC
	ADD RD,#1
	JNZ .resp_loop
.resp_not_OK:
	PLF
	STC
	RTS
	
.data_OK:
	mov RB, #0
	mov RCD, sdcard_buffer_pointer
	INB [RCD],RB, #SPI_PORT
	INB [RCD],RB, #SPI_PORT
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE ; Skip CRC
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE ; Skip CRC
	PLF
	CLC
	RTS
;
; writes a block with the block number
; in sdcard_block_address
;
SDCARD_WRITE_LBLOCK:
	JSR SDCARD_LBLOCK_TO_ADDRESS
;
; writes a block with the byte address in sdcard_block_address
;
SDCARD_WRITE_BLOCK:
	PSF
	SII
	mov RA,#0x40 + 24
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+3
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+2
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address+1
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,sdcard_block_address
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	mov RA,#0xff
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	JSR SPI_RECEIVE_RESPONSE_BYTE
	OR  RA,#0
	JZS .resp_OK
	PLF
	STC
	RTS
.resp_OK:
	mov RA,#0xfe
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
	
;	mov RB, #0
;	mov RD, #2
;.oloop:	
;	mov RA, [sdcard_buffer_pointer],RB
;	PSH RB
;	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE
;	PUL RB
;	CLC
;	ADD RB, #1
;	JNC .oloop
;	mov RA, sdcard_buffer_pointer+1
;	ADD RA, #0
;	mov sdcard_buffer_pointer+1, RA
;	SUB RD, #0
;	JNZ .oloop	

	mov RB, #0
	mov RCD, sdcard_buffer_pointer
	OUB #SPI_PORT, [RCD],RB
	OUB #SPI_PORT, [RCD],RB
.data_end:	
	mov RA, #0x00
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE ; Dummy CRC
	OUB #SPI_PORT, RA ;JSR SPI_SEND_BYTE ; Dummy CRC
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	AND RA, #0x0F
	STC
	CMP RA, #5
	JZS .wait_busy
	OR  RA, #0x80
	PLF
	STC
	RTS
.wait_busy:
	INB RA, #SPI_PORT ;JSR SPI_RECEIVE_BYTE
	OR  RA, #0
	JZS .wait_busy
	PLF
	CLC
	RTS
	
#bank data

sdcard_cmd_parm: #res 4
sdcard_cmd_crc: #res 1
sdcard_cmd_response: #res 4
sdcard_block_address: #res 4
sdcard_buffer_pointer: #res 4
