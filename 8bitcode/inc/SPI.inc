;
; SPI - Serial Port Interface
;

#bank pgm

SPI_MISO  = 0b10000000
SPI_CS1   = 0b00001000
SPI_CS0   = 0b00000100
SPI_CLK   = 0b00000010
SPI_MOSI  = 0b00000001
;
SPI_PORT  = 1
;
; waits for the first null bit
; and reads the next byte from the bus
;
; Returns
;  C-Flag - 0=no null bit found, 1=O.K.
;  RA     - Response Byte
;
SPI_RECEIVE_RESPONSE_BYTE:
	mov RB, #0xff
	mov RC, #8*8
	INP RA, #SPI_PORT
.response_loop:
	OR  RA, #SPI_MOSI | SPI_CLK
	OUT #SPI_PORT, RA
	NOP
	INP RA, #SPI_PORT
	LSL RA
	LSL RB
	LSR RA
	XOR RA, #SPI_CLK
	OUT #SPI_PORT,RA
	OR  RB, #0
	JNN .received
	STC	
	SUB RC, #1
	JCS .response_loop
	RTS
.received:
	mov RA, RB
	STC
	RTS
;
; reads the next eight bits from the bus
;
; Returns
;  RA     - Next Byte
;
SPI_RECEIVE_BYTE:
	INB RA, #SPI_PORT
	RTS
;	INP RA, #SPI_PORT
;	OR  RA, #SPI_MOSI
;	mov RC, RA;
;	mov RB, #1
;.receive_loop:
;	OR  RA, #SPI_CLK
;	OUT #SPI_PORT, RA
;	INP RA, #SPI_PORT
;	LSL RA
;	LSL RB
;	mov RA, RC
;	OUT #SPI_PORT, RA
;	JNC .receive_loop
;.end:	
;	mov RA, RB
;	RTS
;
; sends eight bits on the bus	
;
; Parms:
;  RA - the byte to send
;
SPI_SEND_BYTE:
	OUB #SPI_PORT, RA	
	RTS		
;	mov RC, #1	
;	mov RB, RA
;	INP RA, #SPI_PORT
;.send_loop:
;	AND RA, #0xff - SPI_MOSI
;	LSL RB
;	ADD RA, #0
;	OUT #SPI_PORT,RA
;	OR  RA, #SPI_CLK
;	OUT #SPI_PORT,RA
;	AND RA, #0xff - SPI_CLK
;	LSL RC
;	OUT #SPI_PORT,RA
;	JNC .send_loop

;
; Get the bit map for the currently enabled chip
; return:
; RA - bit map for enabled chip
;
SPI_GET_SELECTED_CHIP:
	INP RA, #SPI_PORT
	AND RA, #SPI_CS0 + SPI_CS1
	RTS

;
; Disables all chip select lines
;
SPI_DESELECT_CHIPS:
	INP RA, #SPI_PORT
	OR  RA, #SPI_CS0 + SPI_CS1
	OUT #SPI_PORT,RA
	mov RA, #0xff
	JMP SPI_SEND_BYTE
;
; Enables the chip select line for chip 0
;
SPI_SELECT_CHIP0:
	mov RA, #SPI_CS1 ; low active
	JMP SPI_SELECT_CHIP
	
;
; Enables the chip select line for chip 1
;
SPI_SELECT_CHIP1:
	mov RA, #SPI_CS0 ; low active
	
;
; Enables a chip select line
; parm:
;  RA - bit map for enabled chip
;
SPI_SELECT_CHIP:
	PSH RB
	mov RB, RA
	INP RA, #SPI_PORT
	AND RA, #0xff - SPI_CS0 - SPI_CS1 - SPI_CLK
	OR  RA, RB
	OUT #SPI_PORT, RA
	PUL RB
	RTS
;
; Switches on SPI interface, puts clients into SPI mode
;
SPI_INIT:
	mov RA, #SPI_CS0 | SPI_CS1 | SPI_MOSI
	OUT #SPI_PORT, RA
	
	mov RD, #20	
.start_spi_loop:
	mov RA, #0xff
	JSR SPI_SEND_BYTE
	STC
	SUB RD, #1
	JCS .start_spi_loop
	
	RTS
