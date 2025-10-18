;
; Word handling routines for the assembler
;

;
; Find the current word in a space separated list of words
;
; in: RCD - pointer to begin of word list
; out: RA - number of word in list (1st word = 1)
;      c-flag: C=0 => word found, c=1 => word not found
;
WORD_FIND_IN_LIST:
	mov RA, #1
	mov word_pos, RA
.list_loop:	
	mov RB, #0
.comp_loop:	
	mov RA, word_cur_txt,RB
	OR  RA, #0x20 ; to lower case, \0 -> " "
	STC
	CMP RA, [RCD]
	JNZ .next_word
	INC RB
	INC RCD
	XOR RA, #0x20
	JNZ .comp_loop
	mov RA, word_pos
	CLC
	RTS
	
.next_word:	
	INC word_pos ; increment word in list count
.skip_loop:	 ; find ending space
	mov RA, [RCD]
	INC RCD
	XOR RA, #0x20
	JNZ .skip_loop

	mov RA, [RCD] ; list ends at \0
	OR  RA, #0
	JNZ .list_loop
	STC
	RTS

;
; Tests if a character is a start character for a Word
;
; A word may start with a letter, a point (.),
; a number sign (#) or an underscore (_).
;
; in: RA - character to be tested
; out: c-flag: C=0 => character is start character
;
WORD_IS_START_CHAR:
	STC
	CMP RA, #"#"
	JNC .no_end
	JZS .yes_end
	CMP RA, #"."
	JNC .no_end
	JZS .yes_end
	CMP RA, #"A"
	JNC .no_end
	CMP RA, #"Z"+1
	JNC .yes_end
	CMP RA, #"_"
	JNC .no_end
	JZS .yes_end
	CMP RA, #"a"
	JNC .no_end
	CMP RA, #"z"+1
	JNC .yes_end
	
.no_end:
	STC
	RTS
	
.yes_end:
	CLC
	RTS

;
; binary search in list of three-character-words
;
; in: RCD - list start address
;     RB  - (number of entries) - 1
; out: offset of word in list (1st word = 0)
;      c-flag: C=0 => word found, c=1 => word not found
;
WORD_SEARCH_IN_LIST:
	mov word_s_base, RCD
	mov word_s_high, RB
	mov RB, #0
	mov word_s_low, RB

.search_loop:
	; RA = low + (high-low)/2
	mov RA, word_s_high
	mov RB, word_s_low
	STC
	SUB RA, RB
	JNC .not_found
	CLC
	LSR RA
	CLC
	ADD RA, RB

	; save center position
	PSH RA

	; RA = RA*3
	mov RB, RA
	LSL RA
	ADD RA, RB

	; RCD = base + RA
	mov RB, #0
	mov RCD, word_s_base
	ADD RC, RA
	ADD RD, RB

.comp_loop:
	mov RA, word_cur_txt,RB
	OR  RA, #0x20 ; lower case
	STC
	CMP RA, [RCD]
	JNZ .not_equal
	INC RB
	INC RCD
	STC
	CMP RB, #3
	JNC .comp_loop

	; restore center position
	PUL RA
	CLC
	RTS

.not_equal:
	; restore center position
	PUL RA

	; compare result
	JCS .higher
	DEC RA
	mov word_s_high, RA
	JMP .search_loop
.higher:
	INC RA
	mov word_s_low, RA
	JMP .search_loop

.not_found:
	STC
	RTS

;
; read a word in double quotes
;
WORD_NEXT_QUOTED:
	mov RA, #0
	mov word_cur_len, RA	
	
	mov RB, #34 ; Quote
	JSR ASM_ASSERT_LAST_CHAR
	JMP .next_char
	
.char_loop:
	JSR WORD_CHECK_ESC_CHAR
	mov RB, word_cur_len
	mov word_cur_txt,RB, RA
	INC RB
	mov RA, #0
	mov word_cur_txt,RB, RA
	mov word_cur_len, RB
	
	STC
	CMP RB, #MAX_WORD_LEN+1
	JNC .next_char
	mov RA, #ERR_TOO_LONG
	JMP MAIN_ERROR_MSG_EXIT
	
.next_char:	
	JSR IO_GETC
	
	STC
	; check for end of line
	CMP RA, #10
	JZS .missing_end
	
	STC
	CMP RA, #34
	JNZ .char_loop
	JMP IO_GETC ; consume ending quote and return
	
.missing_end:
	mov RA, #ERR_QUOTE_NO_END
	JMP MAIN_ERROR_MSG_EXIT

;
; convert all characters in current word to upper case
;

WORD_UPPERCASE:
	mov RB, word_cur_len
	STC
.loop:	
	mov RA, word_cur_txt-1,RB
	CMP RA, #"a"
	JNC .ok
	CMP RA, #"z"+1
	JCS .ok
	AND RA, #0b11011111
	mov word_cur_txt-1,RB, RA
.ok:
	DEC RB
	JCS .loop
	RTS

;
; check for backspace escape characters
;
; \0 => nul (0x00)
; \b => backspace (0x08)
; \t => tab (0x09)
; \n => newline (0x0A)
; \c => carriage return (0x0D)
;
; in: RA - current character
; out: RA - converted character
;

WORD_CHECK_ESC_CHAR:
	STC
	CMP RA, #"\\"
	JNZ .end
	JSR IO_GETC
	
	; check for special characters
	STC
	CMP RA, #"0"
	JNC .end
	JNZ .not_0
	mov RA, #0
	
.not_0:	
	CMP RA, #"b"
	JNC .end
	JNZ .not_b
	mov RA, #8
	
.not_b:	
	CMP RA, #"n"
	JNC .end
	JNZ .not_n
	mov RA, #10
	
.not_n:
	CMP RA, #"r"
	JNC .end
	JNZ .not_r
	mov RA, #13
	
.not_r:
	CMP RA, #"t"
	JNZ .end
	mov RA, #9

.end:
	RTS
;
; read a word 
;

WORD_NEXT:
	mov RB, #0
	mov word_cur_len, RB
	PSH RB
	JSR IO_GOTC

.char_loop:
	JSR WORD_CHECK_ESC_CHAR
	PUL RB
	mov word_cur_txt,RB, RA
	
	INC RB
	STC
	CMP RB, #MAX_WORD_LEN+1
	JNC .next_char
	mov RA, #ERR_TOO_LONG
	JMP MAIN_ERROR_MSG_EXIT
	
.next_char:
	PSH RB
	JSR IO_GETC
	
	STC
	; check for number range (0-9)
	CMP RA, #"0"
	JNC .end
	CMP RA, #"9"+1
	JNC .char_loop
	; check for upper character (A-Z)
	CMP RA, #"A"
	JNC .end
	CMP RA, #"Z"+1
	JNC .char_loop
	; check for underline (_)
	CMP RA, #"_"
	JNC .end
	JZS .char_loop
	; check for lower character (a-z)
	CMP RA, #"a"
	JNC .end
	CMP RA, #"z"+1
	JNC .char_loop

.end:
	PUL RB
	PSH RA
	mov RA, #0
	mov word_cur_txt,RB, RA
	mov word_cur_len, RB
	PUL RA

	RTS

;
; copy current word to destination location
;
; in: RCD - pointer to destination location
;
WORD_COPY_CURRENT_WORD:
	mov RB, #255
.loop:	
	INC RB
	mov RA, word_current,RB
	mov [RCD], RA
	INC RCD
	STC
	CMP RB, word_cur_len
	JNC .loop
	mov RA, #0
	mov [RCD], RA
	RTS

;
; append a null terminated string to current word
;
; in: RCD - pointer to string
;
WORD_APPEND_STRING:
	mov RB, word_cur_len
.loop:	
	mov RA, [RCD]
	mov word_cur_txt,RB, RA
	OR  RA, #0
	JZS .exit
	INC RCD
	INC RB
	JMP .loop
.exit:	
	mov word_cur_len, RB
	RTS

;
; insert a word to beginning of current word
;
; in: RCD - pointer to string
;
WORD_PREPEND_WORD:
	mov RB, word_cur_len
	
	mov RA, [RCD] ; prepended word length
	PSH RD
	PSH RC
	
	; RCD to new word end
	mva RCD, word_cur_txt
	CLC
	ADD RC, RB ; add current length
	ICC RD
	ADD RC, RA ; prepended word length
	ICC RD
	
	; set new word length
	ADD RA, RB
	mov word_cur_len, RA

.move_loop:	
	mov RA, word_cur_txt,RB
	mov [RCD], RA
	DEC RCD
	DEC RB
	JCS .move_loop
	
.move_end:	
	PUL RC
	PUL RD

	; on loop entry:
	; skip length byte, reset RB to 0
.loop:
	INC RCD
	INC RB
	mov RA, [RCD]
	OR  RA, #0
	JZS .exit
	mov word_cur_txt,RB, RA
	JMP .loop
.exit:	
	RTS

;
; deletes characetrs from current word
;
; in: RA - number of chars to delete
;     RB - offset of first character to be deleted
;
WORD_DELETE_CHARS:
	PSH RA
	
	; RCD to first character to keep after deletion
	mva RCD, word_cur_txt
	CLC
	ADD RC, RA
	ICC RD
	
.loop:
	mov RA, [RCD]
	mov word_cur_txt,RB, RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .loop
	
	PUL RA
	mov RB, word_cur_len
	STC
	SUB RB, RA
	mov word_cur_len, RB
	
	RTS
	
#bank data
word_current:
word_cur_len: #res 1
word_cur_txt: #res MAX_WORD_LEN+1

word_s_base: #res 2
word_s_low: #res 1
word_s_high: #res 1

word_pos: #res 1
