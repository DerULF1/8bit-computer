;
; Keyboard Functions
;

#bank pgm

KEYBOARD_PORT = 0x02	
;
SCAN_LSHIFT = 0x12
SCAN_RSHIFT = 0x59
SCAN_LSTRG  = 0x14
SCAN_RSTRG  = 0x14
SCAN_LALT   = 0x11
SCAN_RALT   = 0x11
SCAN_CAPS   = 0x58
SCAN_DEL    = 0x71
SCAN_NUM    = 0x77
SCAN_SCRL   = 0x7E
SCAN_LGUI   = 0x1F
SCAN_RGUI   = 0x27
SCAN_APPS   = 0x2F
SCAN_RELEASE = 0xF0
SCAN_EXTEND = 0xE0
;
LGUI        = 0b10000000
RGUI        = 0b01000000
LSHIFT      = 0b00100000
RSHIFT      = 0b00010000
LSTRG       = 0b00001000
RSTRG       = 0b00000100
LALT        = 0b00000010
RALT        = 0b00000001
;
EXTEND_KEY  = 0b10000000
RELEASE_KEY = 0b01000000
CAPS_LOCK   = 0b00000100
NUM_LOCK    = 0b00000010
SCRL_LOCK   = 0b00000001
;
DATA_LINE   = 0b10000000
DATA_LOW    = 0b10000000
CLK_LINE    = 0b01000000
CLK_LOW     = 0b01000000
CNT_INHIBIT = 0b00100000
RCVD        = 0b00100000
CMD_MODE    = 0b00010000
;		
KEYBOARD_INIT:
	; ensure empty keyboard register
	JSR KEYBOARD_RESET_COUNTER
	JSR KEYBOARD_WAIT_1200
	JSR KEYBOARD_RESET_COUNTER
	
	; send keyboard reset
	mov RA, #0xFF
	JSR KEYBOARD_SEND_BYTE
	JSR KEYBOARD_READ_BYTE	
	JSR KEYBOARD_READ_BYTE	
		
	; reset input buffer
	mov RA, #0	
	mov keyboard_bufRPtr, RA
	mov keyboard_bufWPtr, RA

	; all meta keys off
	mov keyboard_keystate, RA
	
	; Set NUMLOCK state
	mov RA, #NUM_LOCK
	mov keyboard_keystate2, RA
	JMP KEYBOARD_SET_LEDS
;	
KEYBOARD_RESET_COUNTER:
	mov RA, #CMD_MODE
	OUT #KEYBOARD_PORT, RA
	INP RA, #KEYBOARD_PORT
	AND RA, #RCVD
	JNZ .emptykeyboard_buffer
	mov RA, #CMD_MODE + CLK_LOW
	OUT #KEYBOARD_PORT, RA
	JMP KEYBOARD_RESET_COUNTER
.emptykeyboard_buffer:
	mov RA, #0
	OUT #KEYBOARD_PORT, RA
	INP RA, #KEYBOARD_PORT
	RTS
;
; Retrieves a byte from PS/2 controller if available.
;
; Parm:
;   -
;
; Result:
;  RA - received byte from PS/2 controller
;       0 = not byte received
;
KEYBOARD_READ_BYTE_COND:
	mov RA, #CMD_MODE
	OUT #KEYBOARD_PORT, RA
	INP RA, #KEYBOARD_PORT
	PSH RA
	mov RA, #0
	OUT #KEYBOARD_PORT, RA
	PUL RA
	AND RA, #RCVD
	JZS .nobyte
	INP RA, #KEYBOARD_PORT
.nobyte:	
	RTS
;	
KEYBOARD_READ_BYTE:
	PSH RC
	mov RC, #20
.tloop:	
	JSR KEYBOARD_READ_BYTE_COND
	OR  RA, #0
	JNZ .exit
	JSR KEYBOARD_WAIT_150
	STC 
	SUB RC, #1
	JNZ .tloop
.exit:	
	PUL RC
	RTS
;		
KEYBOARD_WAIT_1200:
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	JSR KEYBOARD_WAIT_150
	; fall through
;		
KEYBOARD_WAIT_150:
	PSH RA
	STC
	mov RA, #25
.loop:	
	SUB RA, #1
	JNZ .loop
	PUL RA
	RTS
;
KEYBOARD_SEND_BYTE:
    mov RD, RA ; Data byte 
    mov RA, #8 ; bit counter
	mov keyboard_bitCnt, RA
    mov RC, #1 ; parity counter
	
	; request to send
	mov RA, #CLK_LOW + CNT_INHIBIT
	OUT #KEYBOARD_PORT, RA
	JSR KEYBOARD_WAIT_150
	mov RA, #CLK_LOW + DATA_LOW + CNT_INHIBIT
	OUT #KEYBOARD_PORT, RA
	NOP
	NOP
	NOP
	mov RA, #DATA_LOW + CMD_MODE + CNT_INHIBIT
	OUT #KEYBOARD_PORT, RA
	
.dataloop:	
	JSR KEYBOARD_WAIT_CLOCK_LOW
	LSR RD
	JCS .data1
	mov RA, #DATA_LOW + CMD_MODE + CNT_INHIBIT
	JMP .setdata
.data1:	
	ADD RC, #0 ; (carry flag +1)
	mov RA, #CMD_MODE + CNT_INHIBIT	
.setdata:
	OUT #KEYBOARD_PORT, RA	
	JSR KEYBOARD_WAIT_CLOCK_HIGH
	mov RA, keyboard_bitCnt
	STC
	SUB RA, #1
	mov keyboard_bitCnt, RA
	JNZ .dataloop

	; send parity bit
	JSR KEYBOARD_WAIT_CLOCK_LOW
	LSR RC
	JCS .parity1
	mov RA, #DATA_LOW + CMD_MODE + CNT_INHIBIT
	JMP .setparity
.parity1:	
	mov RA, #CMD_MODE + CNT_INHIBIT	
.setparity:
	OUT #KEYBOARD_PORT, RA	
	JSR KEYBOARD_WAIT_CLOCK_HIGH

	; free data line
	JSR KEYBOARD_WAIT_CLOCK_LOW
	mov RA, #CMD_MODE + CNT_INHIBIT
	OUT #KEYBOARD_PORT, RA	
	JSR KEYBOARD_WAIT_CLOCK_HIGH	
	
	; skip acknowledge bit
	JSR KEYBOARD_WAIT_CLOCK_LOW
	JSR KEYBOARD_WAIT_CLOCK_HIGH	
	
	; leave command mode
	mov RA, #0
	OUT #KEYBOARD_PORT, RA
	RTS
;	
KEYBOARD_WAIT_CLOCK_LOW:
	INP RA, #KEYBOARD_PORT
	mov RB, RA
	INP RA, #KEYBOARD_PORT
	STC
	CMP RA, RB
	JNZ KEYBOARD_WAIT_CLOCK_LOW
	AND RA, #CLK_LINE
	JNZ KEYBOARD_WAIT_CLOCK_LOW
	RTS
;	
KEYBOARD_WAIT_CLOCK_HIGH:
	INP RA, #KEYBOARD_PORT
	mov RB, RA
	INP RA, #KEYBOARD_PORT
	STC
	CMP RA, RB
	JNZ KEYBOARD_WAIT_CLOCK_HIGH
	AND RA, #CLK_LINE
	JZS KEYBOARD_WAIT_CLOCK_HIGH
	RTS
;
KEYBOARD_BYTE_TO_KEY:
	mov RB, #0
	mov keyboard_special_key, RB

	STC
	CMP RA, #SCAN_EXTEND
	JNZ .noextendflag
	
	mov RA, keyboard_keystate2
	OR  RA, #EXTEND_KEY
	mov keyboard_keystate2, RA
	RTS
	
.noextendflag:
	STC
	CMP RA, #SCAN_RELEASE
	JNZ .noreleaseflag
	
	mov RA, keyboard_keystate2
	OR  RA, #RELEASE_KEY
	mov keyboard_keystate2, RA
	RTS
	
.noreleaseflag:	
	mov RB, keyboard_keystate2
	AND RB, #RELEASE_KEY
	JZS .noreleasekey

.keyreleased:	
	
	JSR KEYBOARD_GET_META_KEY_INDEX
	JNC .endrelease
	
	CMP RB, #LEDkeybits-metakeybits
	JCS .endrelease
	
	mov RA, metakeybits,RB
	XOR RA, #0xFF
	AND RA, keyboard_keystate
	mov keyboard_keystate, RA

	JMP .endrelease
	
.noreleasekey:
	JSR KEYBOARD_GET_META_KEY_INDEX
	JNC .nometakey
	
	CMP RB, #LEDkeybits-metakeybits
	JCS .ledkey

	mov RA, metakeybits,RB
	OR  RA, keyboard_keystate
	mov keyboard_keystate, RA
	JMP .endrelease
	
.ledkey:	
	mov RA, metakeybits,RB
	XOR RA, keyboard_keystate2
	mov keyboard_keystate2, RA
	JSR KEYBOARD_SET_LEDS
	JMP .endrelease
	
.nometakey:
	; STRG+ALT+DEL?
	STC
	CMP RA, #SCAN_DEL
	JNZ .nodel
	mov RB, keyboard_keystate
	AND RB, #LALT + LSTRG
	XOR RB, #LALT + LSTRG
	; RESET Vector
	JZS [0xfffc]
	
.nodel:	
	; check for function keys
	JSR KEYBOARD_FUNCTION_KEY
	
	; only scan codes from 0x10 to 0x66
	STC
	SUB RA, #0x10
	JNC .endrelease
	cmp RA, #0x57
	JCS .numpad
	
	; captial letters or lower case letters?
	mov RB, RA
	mov RA, keyboard_keystate
	AND RA, #LSHIFT+RSHIFT
	JNZ .shift
	; right ALT key?
	mov RA, keyboard_keystate
	AND RA, #RALT
	JNZ .ralt
	
	mov RA, #0
	JMP .check_caps_lock
	
.ralt:
	PSH RD
	mov RD, RB
	mov RB, #0
.ralt_loop:
	mov RA, RALT_map,RB
	INC RB
	STC 
	CMP RA, RD
	JNC .ralt_end
	JZS .ralt_found
	INC RB
	JNC .ralt_loop
.ralt_end:
	mov RB, #RALT_map_last - RALT_map
.ralt_found:
	mov RA, RALT_map,RB
	PUL RD
	JMP .storekey
	
.numpad:
	CMP RA, #0x6e
	JCS .endrelease
	STC
	SUB RA, #0x59
	mov RB, RA
	mov RA, keyboard_keystate2
	AND RA, #EXTEND_KEY
	JNZ .csrkey
	mov RA, keyboard_keystate2	
	AND RA, #NUM_LOCK
	JNZ .numbers
.csrkey:
	mov RA, numpadscan, RB
	mov RB, keyboard_keystate
	AND RB, LSTRG + RSTRG ; check for control keys
	JZS .storekey ; not pressed
	OR  RA, #0x10 ; add 16 to the keycode
	JMP .storekey
.numbers:
	mov RA, numpadlockscan, RB	
	JMP .storekey
	
.shift:
	mov RA, #CAPS_LOCK
.check_caps_lock:
	XOR RA, keyboard_keystate2
	AND RA, #CAPS_LOCK
	JNZ .capstab
	
.nocapstab:	
	mov RA, keytab,RB
	JMP .storekey
	
.capstab:
	mov RA, keytab_caps,RB
	
.storekey:	
	OR  RA, #0
	JZS .endrelease
	
	mov RB, keyboard_bufWPtr
	mov keyboard_buffer,RB, RA
	CLC
	ADD RB, #1
	AND RB, #0x0f
	mov keyboard_bufWPtr, RB
	
.endrelease:
	mov RB, keyboard_keystate2
	AND RB, #0xFF - RELEASE_KEY - EXTEND_KEY
	mov keyboard_keystate2, RB
	RTS
;
KEYBOARD_FUNCTION_KEY:
	PSH RD
	mov RB, #12
	mov RD, RA
.loop:
	DEC RB
	JNC .end
	mov RA, functionkeyscan,RB
	CMP RA, RD
	JNZ .loop
.found:
	INC RB
	mov keyboard_special_key, RB
.end:	
	mov RA, RD
	PUL RD
	RTS
;
KEYBOARD_GET_META_KEY_INDEX:
	PSH RD
	mov RD, RA
	mov RB, keyboard_keystate2
	LSL RB ; key extend to c-flag
	mov RB, #metakeybits-metakeyscan
	SUB RB, #1 ; 1 or 2 backward
.loop:
	mov RA, metakeyscan,RB
	STC
	CMP RD, RA
	JZS .found
	STC
	SUB RB, #2
	JCS .loop
.found:	
	mov RA, RD
	PUL RD
	RTS
;	
KEYBOARD_SET_LEDS:
	mov RA, #0xED
	JSR KEYBOARD_SEND_BYTE
	JSR KEYBOARD_READ_BYTE
	mov RA, keyboard_keystate2
	AND RA, #0x07
	JSR KEYBOARD_SEND_BYTE
	JSR KEYBOARD_READ_BYTE
	RTS
	
;
; Reads the next key from the keyboard buffer.
; If no key is available a null value is returned.
;
; returns:
;  RA - next key 
;       0 if no key is avaliable
;
KEYBOARD_GET_KEY:
	PSH RB
	mov RB, keyboard_bufRPtr
	STC
	cmp RB, keyboard_bufWPtr
	JNZ .readkey
	mov RA, #0
	JMP .end
.readkey:	
	mov RA, keyboard_buffer,RB
	CLC
	ADD RB, #1
	AND RB, #0x0f
	mov keyboard_bufRPtr, RB
.end:	
	PUL RB
	RTS
	
KEYBOARD_WAIT_FOR_KEY:
	PSH RB
.keyloop:	
	mov RB, keyboard_bufRPtr
	STC
	cmp RB, keyboard_bufWPtr
	JZS .keyloop
	mov RA, keyboard_buffer,RB
	CLC
	ADD RB, #1
	AND RB, #0x0f
	mov keyboard_bufRPtr, RB
	PUL RB
	RTS
	
	#d RELOC_ESC
	ld16 keytab_end-numpadscan+1

numpadscan:
; 0x69 to 0x6f
  #d8 KEY_END, 0, KEY_CURSOR_LEFT, KEY_HOME, 0, 0, 0
; 0x70 to 0x73
  #d8 KEY_INSERT, KEY_DELETE, KEY_CURSOR_DOWN, 0
; 0x74 to 0x77
  #d8 KEY_CURSOR_RIGHT, KEY_CURSOR_UP, KEY_ESC, 0
; 0x78 to 0x7d
  #d8 0, "+", KEY_PAGE_DOWN, "-", "*", KEY_PAGE_UP
  
numpadlockscan:
; 0x69 to 0x6f
  #d8 "1", 0, "4", "7", 0, 0, 0
; 0x70 to 0x73
  #d8 "0", ",", "2", "5"
; 0x74 to 0x77
  #d8 "6", "8", KEY_ESC, 0
; 0x78 to 0x7d
  #d8 0, "+", "3", "-", "*", "9"

RALT_map: ; sorted by descending scan code
	#d8 0x51, 0x7c ; < -> |
	#d8 0x3E, 0x5c ; ? -> \
	#d8 0x36, 0x5d ; 9 -> ]
	#d8 0x35, 0x7d ; 0 -> }
	#d8 0x2e, 0x5b ; 8 -> [
	#d8 0x2d, 0x7b ; 7 -> {
	#d8 0x05, 0x40 ; q -> @	
RALT_map_last:	
    #d8 0, 0 ; end of map

functionkeyscan:
  #d8 0x05, 0x06, 0x04, 0x0c, 0x03, 0x0b
  #d8 0x83, 0x0a, 0x01, 0x09, 0x78, 0x07

metakeyscan:
  #d8 0, SCAN_LGUI, 0 ,SCAN_RGUI
  #d8 SCAN_LSHIFT, 0, SCAN_RSHIFT, 0
  #d8 SCAN_LSTRG, SCAN_RSTRG
  #d8 SCAN_LALT, SCAN_RALT
  
LEDkeyscan:
  #d8 SCAN_CAPS, 0, SCAN_NUM, 0, SCAN_SCRL, 0  
  
metakeybits:  
  #d8 0, LGUI, 0, RGUI
  #d8 LSHIFT, 0, RSHIFT, 0
  #d8 LSTRG, RSTRG
  #d8 LALT, RALT
LEDkeybits:  
  #d8 CAPS_LOCK, 0, NUM_LOCK, 0, SCRL_LOCK, 0

keytab:
  #d "\0\0\0\0\0q1\0\0\0ysaw2\0" ; 1
  #d "\0cxde43\0\0 vftr5\0" ; 2
  #d "\0nbhgz6\0\0\0mju78\0" ; 3
  #d "\0,kio09\0\0.-l\0p\0\0" ; 4
  #d "\0\0\0\0['\0\0\0\0\n+\0#\0\0" ; 5
  #d "\0<\0\0\0\0" ; 6
  #d8           KEY_BACKSPACE ; Backspace

keytab_caps:
  #d "\0\0\0\0\0Q!\0\0\0YSAW" ; 1
  #d8                   34 ; "
  #d                  "\0" ; 1
  #d "\0CXDE$@\0\0 VFTR%\0" ; 2
  #d "\0NBHGZ&\0\0\0MJU/(\0" ; 3
  #d "\0;KIO=)\0\0:_L\0P?\0" ; 4
  #d "\0\0\0\0]'\0\0\0\0\n*\0'\0\0" ; 5
  #d "\0>\0\0\0\0" ; 6
  #d8                   8 ; Backspace
keytab_end:
	
#bank data

keyboard_bitCnt: #res 1

keyboard_keystate: #res 1
keyboard_keystate2: #res 1

keyboard_bufWPtr: #res 1
keyboard_bufRPtr: #res 1
keyboard_buffer: #res 16

keyboard_special_key: #res 1
