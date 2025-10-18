;
; Error messages of the assembler
;
error_msgs:
ERR_IO=1
	#d "IO Error: 0x$x\0" ; 0x01
ERR_TOO_LONG=2
	#d "word too long\0"  ; 0x02
ERR_SYNTAX=3	
	#d "syntax error\0"  ; 0x03
ERR_OVERFLOW=4
	#d "value over-/underflow\0"  ; 0x04
ERR_QUOTE_NO_END=5
	#d "unbalanced quotes\0"  ; 0x05
ERR_MISSING_CHAR=6
	#d "missing character $c\0" ; 0x06
ERR_UNDEFINED_VARIABLE=7	
	#d "undefined variable $w\0" ; 0x07
ERR_DIV0=8	
	#d "Division by 0\0" ; 0x08
ERR_UNDEFINED_BANK=9
	#d "undefined bank $w\0" ; 0x09
ERR_NUMBER_TOO_BIG=10
	#d "number too big\0" ; 0x0A
ERR_ILLEGAL_ADDRESSING=11
	#d "unknown addressing mode\0" ; 0x0B
ERR_NO_OPCODE=12
	#d "no opcode found\0" ; 0x0C
ERR_NO_INPUT=13
	#d "no input file\0" ; 0x0D
