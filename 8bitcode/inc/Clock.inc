;
; Clock - Real Time Clock Interface
;

#bank pgm

CLK_REG_SEC   = 0x00
CLK_REG_MIN   = 0x01
CLK_REG_HOUR  = 0x02
CLK_REG_WDAY  = 0x03
CLK_REG_DAY   = 0x04
CLK_REG_MONTH = 0x05
CLK_REG_YEAR  = 0x06
CLK_REG_CTRL  = 0x0E
CLK_REG_STAT  = 0x0F
;
; Reads the seven clock bytes from the Real Time Clock
;
; Parms:
;  RC/RD  - pointer to seven byte area
;
CLK_READ_TIME:
	PSF ; save interrupt inhibit state
	SII 
	
	; save current SPI bus enable state
	JSR SPI_GET_SELECTED_CHIP
	PSH RA
	
	; save memory pointer
	PSH RD
	PSH RC
	
	; enable RTC chip
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1
	
	; start read at second register
	mov RA, #CLK_REG_SEC
	JSR SPI_SEND_BYTE
	
	; restore memory pointer
	PUL RC
	PUL RD
	
	; save memory pointer
	PSH RD
	PSH RC
	
	; read RTC registers up to year register
	mov RB, #CLK_REG_YEAR+1
	INB [RCD],RB, #SPI_PORT
	
	; restart RTC read cycle
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1
	
	; read status register
	mov RA, #CLK_REG_STAT
	JSR SPI_SEND_BYTE
	JSR SPI_RECEIVE_BYTE
	
	; isolate Oscilator Stop Flag
	AND RA, #0x80
	
	; restore memory pointer
	PUL RC
	PUL RD
	
	; write OSF into second byte
	OR  RA, [RCD]
	mov [RCD], RA
	
	; restore SPI bus enable state
	JSR SPI_DESELECT_CHIPS
	PUL RA
	JSR SPI_SELECT_CHIP
	
	PLF ; restore interrupt inhibit state
	RTS

;
; Writes the seven clock bytes into the Real Time Clock
;
; Parms:
;  RC/RD  - pointer to seven byte area
;
CLK_WRITE_TIME:
	PSF ; save interrupt inhibit state
	SII 
	
	; save memory pointer
	PSH RD
	PSH RC
	
	; save current SPI bus enable state
	JSR SPI_GET_SELECTED_CHIP
	PSH RA
	
	; enable RTC chip
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1
	
	; start write at second register
	mov RA, #CLK_REG_SEC + 0x80
	JSR SPI_SEND_BYTE
	
	; restore memory pointer
	PUL RA
	PUL RC
	PUL RD
	PSH RA
	
	; write RTC registers up to year register
	mov RB, #CLK_REG_YEAR+1
	OUB #SPI_PORT,[RCD],RB

	; restart read cycle
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1
	
	; read status register
	mov RA, #CLK_REG_STAT
	JSR SPI_SEND_BYTE
	JSR SPI_RECEIVE_BYTE
	PSH RA
	
	; restart read cycle
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1

	; write status register
	mov RA, #CLK_REG_STAT + 0x80
	JSR SPI_SEND_BYTE
	PUL RA
	
	; reset OSF bit
	AND RA, #0x7F
	JSR SPI_SEND_BYTE
	
	; restore SPI bus enable state
	JSR SPI_DESELECT_CHIPS
	PUL RA
	JSR SPI_SELECT_CHIP
	
	PLF ; restore interrupt inhibit state
	RTS

;
; Initializes the RTC chip to issue a
; 1kHz square wave on pin INT
;
CLK_INIT_SQW:
	; enable RTC chip
	JSR SPI_DESELECT_CHIPS
	JSR SPI_SELECT_CHIP1
	
	; write to control register
	mov RA, #CLK_REG_CTRL + 0x80
	JSR SPI_SEND_BYTE
	
	; bit7=0 -> Oszillator on
	; bit6=0 -> battery buffered square wave off
	; bit5=0 -> no manual temperature conversion
	; bit4/3=01 -> 1kHz square wave frequency
	; bit2=0 -> square wave output on
	; bit1=0 -> alarm 2 interrupt off
	; bit0=0 -> alarm 1 interrupt off
	mov RA, #0x08
	JSR SPI_SEND_BYTE
	
	; deallocate SPI bus
	JMP SPI_DESELECT_CHIPS
