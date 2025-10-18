#include "8bit.cpu"

#bankdef pgm
{
    #addr 0x0000
    #size 0x1000
	#outp 0x00
}

#bankdef data
{
    #addr 0x1000
    #size 0x8000
}

SYS_OWNER = 0xF0
RELOC_ESC = 0xFB

#bank data
DATA_START:

#bank pgm

MODE_TIME=0
MODE_DATE=1

NO_TIME=0xff

;
; Big Clock
;
START:	
	JSR BIG_DIGIT_INIT

	mov RA, #CURSOR_TYPE_OFF ; turn off cursor
	JSR CURSOR_MODE

	JSR CURSOR_CLEAR_SCREEN
	
	mov RA, #MODE_TIME
	mov mode, RA
	
	JSR OS_GET_TICKS
	mov last_ticks, RA
	mov last_ticks+1, RB
	mov last_ticks+2, RC
	
	mov RA, #NO_TIME
	mov start_time, RA
	mov stop_time, RA
 
.show_loop: 
	mva RCD, byte_area
	JSR OS_GET_TIME
	
	JSR COMPARE_LAST
	JNC .get_key
	
	mov RA, mode
	STC
	CMP RA, #MODE_DATE
	JZS .show_date
	
.show_time:
	JSR CHECK_STOP_WATCH
	JSR SHOW_TIME
	JMP .get_key
	
.show_date:
	JSR SHOW_DATE
	
.get_key:
	mov RA, last_ticks
	mov RB, last_ticks+1
	mov RC, last_ticks+2
	CLC
	ADD RA, #15
	ICC RB
	ICC RC
	JSR OS_SLEEP
	mov last_ticks, RA
	mov last_ticks+1, RB
	mov last_ticks+2, RC
	
	JSR OS_KEYBOARD_GET_KEY
	STC
	CMP RA, #"d"
	JNZ .no_d
	mov RA, #MODE_DATE
	mov mode, RA
	JMP .show_date
	
.no_d:
	STC
	CMP RA, #"t"
	JNZ .no_t
	mov RA, #MODE_TIME
	mov mode, RA
	JMP .show_time
	
.no_t:
	STC
	CMP RA, #"r"
	JNZ .no_r
	mov RA, #NO_TIME
	mov start_time, RA
	mov stop_time, RA
	JMP .show_loop
	
.no_r:
	STC
	CMP RA, #"s"
	JNZ .no_s
	
	mov RA, start_time
	XOR RA, #NO_TIME
	JNZ .no_start
	mov RA, byte_area
	mov start_time, RA
	mov RA, byte_area+1
	mov start_time+1, RA
	mov RA, byte_area+2
	mov start_time+2, RA
	JMP .show_time
	
.no_start:
	mov RA, stop_time
	XOR RA, #NO_TIME
	JNZ .show_time
	
.set_stop_time:
	JSR CALC_CURRENT_STOP_WATCH
	mov RA, byte_area
	mov stop_time, RA
	mov RA, byte_area+1
	mov stop_time+1, RA
	mov RA, byte_area+2
	mov stop_time+2, RA
	JMP .show_time
	
.no_s:
	STC
	CMP RA, #"x"
	JNZ .show_loop
	
	mov RA, #CURSOR_TYPE_BLINK ; turn on cursor
	JSR CURSOR_MODE
	
	JMP CURSOR_CLEAR_SCREEN
	
CHECK_STOP_WATCH:
	mov RA, start_time
	AND RA, stop_time
	XOR RA, #NO_TIME
	JZS .end
	
	mov RA, stop_time
	XOR RA, #NO_TIME
	JZS .sw_running
.sw_stopped:
	mov RA, stop_time
	mov byte_area, RA
	mov RA, stop_time+1
	mov byte_area+1, RA
	mov RA, stop_time+2
	mov byte_area+2, RA
	JMP .end

.sw_running:
	JSR CALC_CURRENT_STOP_WATCH
	
.end:	
	RTS
	
CALC_CURRENT_STOP_WATCH:	
	STC
	mov RA, byte_area
	mov RB, start_time
	JSR BCD_SUB
	JCS .sec_OK
	mov RB, #0x40
	STC
	JSR BCD_SUB
	CLC
.sec_OK:	
	mov byte_area, RA
	mov RA, byte_area+1
	mov RB, start_time+1
	JSR BCD_SUB
	JCS .min_OK
	mov RB, #0x40
	STC
	JSR BCD_SUB
	CLC
.min_OK:	
	mov byte_area+1, RA
	mov RA, byte_area+2
	mov RB, start_time+2
	JSR BCD_SUB
	JCS .hour_OK
	mov RB, #0x76
	STC
	JSR BCD_SUB
	CLC
.hour_OK:	
	mov byte_area+2, RA
	RTS
	
COMPARE_LAST:
	mov RB, #6
	; compare byte_area to last_area
.comp_loop:
	mov RA, last_area,RB
	mov RC, RA
	mov RA, byte_area,RB
	STC 
	CMP RA, RC
	JNZ .not_equal
	DEC RB
	JCS .comp_loop
	RTS
	; copy byte_area to last_area
.not_equal_loop:	
	mov RA, byte_area,RB
.not_equal:	
	mov last_area,RB, RA
	DEC RB
	JCS .not_equal_loop
	STC
	RTS
	
SHOW_DATE:
	mov RA, byte_area+4 ; day of month
	AND RA, #0b00111111
	mov RB, #0
	JSR SHOW_BCD_BYTE

	mov RB, #6
	JSR BIG_DIGIT_PERIOD
	
	mov RA, byte_area+5 ; month
	AND RA, #0b00011111
	mov RB, #7
	JSR SHOW_BCD_BYTE

	mov RB, #13
	JSR BIG_DIGIT_PERIOD
	
	mov RA, byte_area+6 ; year
	mov RB, #14
	JMP SHOW_BCD_BYTE
	
SHOW_TIME:
	mov RA, byte_area+2 ; hour
	AND RA, #0b00111111
	mov RB, #0
	JSR SHOW_BCD_BYTE

	mov RB, #6
	JSR BIG_DIGIT_COLON
	
	mov RA, byte_area+1 ; minute
	mov RB, #7
	JSR SHOW_BCD_BYTE

	mov RB, #13
	JSR BIG_DIGIT_COLON
	
	mov RA, byte_area+0 ; second
	mov RB, #14
	JMP SHOW_BCD_BYTE
	
SHOW_BCD_BYTE:
	PSH RA ; save byte
	PSH RB ; save start column
	
	LSR RA ; isolate high nybble
	LSR RA
	LSR RA
	LSR RA
	AND RA, #0x0F
	JSR BIG_DIGIT_SHOW	
	PUL RB ; restore start column
	PUL RA ; restore output byte
	
	CLC    ; move 3 columns foreward
	ADD RB, #3
	AND RA, #0x0F ; isolate low nybble
	JMP BIG_DIGIT_SHOW

#include "OSCalls.inc"
#include "LCDGraph.inc"
#include "cursor.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "BigDigit.inc"
#bank pgm
#include "param.inc"
#bank pgm
#include "BCDMath.inc"
#bank pgm

;	
	#d RELOC_ESC
	#d 0x00
	#d 0x00	

;
; program trailer
;	
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data

mode: #res 1
byte_area:
	#res 7
last_area:
	#res 7
	
; start = ff, stop = ff:
;  byte_area = time (no stop watch)
;  "S" => mov start, time
; start <> ff, stop = ff:
;  byte_area = time-start (stop watch)
;  "S" => mov stop, time-start
;      => mov start, #ff
;  "L" => mov stop, time-start+start
; start = ff, stop <> ff:
;  byte_area = stop (end of stop watch)
;  "S" => mov start, last-stop
; start <> ff, stop <> ff:
;  byte_area = stop-start (lap time)
;  "L" => mov stop, #ff
start_time:	#res 3
stop_time:	#res 3
last_ticks: #res 3	
DATA_END:	