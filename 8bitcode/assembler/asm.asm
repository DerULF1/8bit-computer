#include "8bit.cpu"

;
; asm - Assembler
;
; Written 11.10.2021 Ulf Caspers
;

#bankdef pgm
{
    #addr 0x0000
    #size 0x2000
	#outp 0x0000
}

#bankdef data
{
    #addr 0x2000
    #size 0x2000
}

RELOC_ESC = 0xFB

WRITE_BUF_SIZE=128
MAX_WORD_LEN=64

BANK_ADDR_SUFFIX="a"
BANK_OUTP_SUFFIX="o"

#bank data
DATA_START:

#bank pgm
#include "OSCalls.inc"
#include "AdrTyps.inc"

;
; RC/RD - parameter string
;
START:
	mov parm_ptr, RC
	mov parm_ptr+1, RD

	mov RCD, SP
	mov shell_exit_stack, RC
	mov shell_exit_stack+1, RD

	JSR IO_INIT

	JSR READ_PARMS
	JNC .parm_ok
	JSR SHOW_USAGE
	JMP .end

.parm_ok:
	JSR VSTORE_INIT
	mov RA, #0
	mov pass_asm, RA

.pass_loop:	
	INC pass_asm

	mov RA, #0
	mov bank_store_addr_current, RA
	mov bank_store_addr_current+1, RA
	mov current_file_len, RA
	mov current_file_txt, RA

	JSR IO_OPEN_IN

	JSR ASM_FILE
	
	JSR IO_CLOSE_IN
	
	mov RA, pass_asm
	STC
	CMP RA, #1
	JNZ .last_pass
	
	JSR IO_OPEN_OUT
	JMP .pass_loop
	
.last_pass:	
	JSR IO_WRITE_BUFFER
	
	JSR SHOW_VARIABLE_LIST

.end:
	JSR IO_END
	; fall through to MAIN_OK_EXIT

;
; Exit assembler with success return code 0
;
MAIN_OK_EXIT:	
	mov RA, #0
	; fall through to MAIN_EXIT

;
; Exit assembler with given return code
;
; Parm:
;  RA - return code
;
MAIN_EXIT:	
	mov RC, shell_exit_stack
	mov RD, shell_exit_stack+1	
	mov SP, RCD
	RTS

;
; Exit assembler with IO error message
;
; Parm:
;  RA - IO error code
;
MAIN_IO_ERROR_EXIT:
	mov msg_value, RA
	mov RA, #0
	mov msg_value+1, RA
	INC RA ; RA = 1 = ERR_IO
	; fall through to MAIN_ERROR_MSG_EXIT
	
;
; Exit assembler with error message
;
; Parm:
;  RA - error message code
;
MAIN_ERROR_MSG_EXIT:
	mva RCD, error_msgs
	mov RB, RA
.search_loop:
	DEC RB
	JZS .msg_found
.skip_loop:	
	mov RA, [RCD]
	INC RCD
	OR  RA, #0
	JNZ .skip_loop
	JMP .search_loop

.msg_found:
	mov RA, [RCD]
	INC RCD
	OR  RA, #0
	JZS .show_line_nr
	STC
	CMP RA, #"$"
	JNZ .char_out
	mov RA, [RCD]
	INC RCD
	
	STC ; $x: output msg_value as hex value
	CMP RA, #"x"
	JNZ .no_hex
	mov RA, msg_value+1
	JSR PUT_ERROR_HEX_BYTE
	mov RA, msg_value
	JSR PUT_ERROR_HEX_BYTE
	JMP .msg_found
	
.no_hex:	
	STC ; $w: output current word
	CMP RA, #"w"
	JNZ .no_word
	mva RCD, word_cur_txt
	JSR PUT_ERROR_STRING
	JMP .msg_found
	
.no_word:	
	STC ; $s: output msg_value as pointer to a string
	CMP RA, #"s"
	JNZ .no_string
	mov RC, msg_value
	mov RD, msg_value+1
	JSR PUT_ERROR_STRING
	JMP .msg_found
	
.no_string:	
	STC ; $c: output msg_value as single char
	CMP RA, #"c"
	JNZ .char_out
	mov RA, msg_value
	; fall through .char_out
	
.char_out:
	JSR PUT_ERROR_CHAR
	JMP .msg_found
	
.show_line_nr:
	mva RCD, in_line_msg
	JSR PUT_ERROR_STRING
	mov RA, line_number
	mov RB, line_number+1
	JSR NUM2STR
	mva RCD, num2str_txt
	JSR PUT_ERROR_STRING
	mov RA, #":"
	JSR PUT_ERROR_CHAR
	mva RCD, current_file_txt
	JSR PUT_ERROR_STRING
	; fall through MAIN_ERROR_EXIT
	
;
; Exit assembler with error return code 1
;
MAIN_ERROR_EXIT:
	mov RA, #1
	JMP MAIN_EXIT	

;
; Display a simple usage message on stderr
;
SHOW_USAGE:
	mva RCD, usage_msg
	JMP PUT_ERROR_STRING

;
; write a list of all variables onto stdout
;
SHOW_VARIABLE_NODE:
	; skip empty nodes
	INC RCD
	mov RA, [RCD]
	DEC RCD
	OR  RA, [RCD]
	JZS .end
	
	; save original node pointer and length
	PSH RD
	PSH RC
	PSH RB

	; load new node pointer
	mov RA, [RCD]
	INC RCD
	mov RD, [RCD]
	mov RC, RA

	; get node length
	mov RB, [RCD]
	
	; count total variable storage size
	mov RA, value_current
	CLC
	ADD RA, RB
	mov value_current, RA
	ICC value_current+1

	; write out left node
	INC RCD
	JSR SHOW_VARIABLE_NODE
	
	; save pointer to right node
	INC RCD
	INC RCD
	PSH RD
	PSH RC
	
	; move foreward to variable name
	INC RCD
	INC RCD
	
	; adjust node length to variable name length
	STC
	SUB RB, #7

	; skip internal variables
	JSR IS_SPECIAL_VARIABLE
	JCS .skip_var
	
	; output variable name
.out_loop:
	mov RA, [RCD]
	INC RCD
	PSH RB
	mov RB, outfile
	JSR _PUT_CHAR
	PUL RB
	DEC RB
	JNZ .out_loop
	
	; output equal sign (=)
	PSH RD
	PSH RC
	mva RCD, var_value_msg
	mov RB, outfile
	JSR _PUT_STRING
	PUL RC
	PUL RD

	; output variable value as hex value
	mov RA, [RCD]
	PSH RA
	INC RCD
	mov RA, [RCD]
	mov RB, outfile
	JSR _PUT_HEX_BYTE
	PUL RA
	mov RB, outfile
	JSR _PUT_HEX_BYTE

	; output closing new line
	mov RA, #"\n"
	mov RB, outfile
	JSR _PUT_CHAR

.skip_var:
	; output right node
	PUL RC
	PUL RD
	JSR SHOW_VARIABLE_NODE

.exit:
	PUL RB
	PUL RC
	PUL RD
	
.end:
	RTS

;
; write a list of all variables onto stdout
;
SHOW_VARIABLE_LIST:
	mov RCD, varlist_name_ptr
	mov RA, RC
	OR  RA, RD
	JZS .end
	STC
	CMP RA, #1
	JNZ .open_file
	mov outfile, RA
	JMP .begin

.open_file:
	JSR IO_OPEN_OUT_NAME

.begin:
	mov RC, #0
	mov RD, RC
	mov value_current, RCD
	mva RCD, vstor_start
	JSR SHOW_VARIABLE_NODE
	
.end_list:
	; comment line marker
	mov RA, #";"
	mov RB, outfile
	JSR _PUT_CHAR
	
	; show number of bytes
	mov RA, value_current
	mov RB, value_current+1
	JSR NUM2STR
	mva RCD, num2str_txt
	mov RB, outfile
	JSR _PUT_STRING
	mva RCD, var_bytes_msg
	mov RB, outfile
	JSR _PUT_STRING
	
	JSR IO_END

.end:
	RTS
	
;
; checks for a special variable name
; these names contain a dot (.)
;
; Parms:
;  RB  - variable name length
;  RCD - pointer to variable name
;
IS_SPECIAL_VARIABLE:
	PSH RD
	PSH RC
	PSH RB

.sloop:
	DEC RB
	JNC .exit
	
	mov RA, [RCD]
	INC RCD
	STC
	CMP RA, #"."
	JNZ .sloop
	
.exit:	
	PUL RB
	PUL RC
	PUL RD
	RTS

;
; process all parameters on the command line
;
READ_PARMS:
	; default: no inlcude path
	mva RCD, in_line_msg-1 ; pointer to \0
	mov include_path_list, RC
	mov include_path_list+1, RD

	mov RA, #0
	mov varlist_name_ptr, RA
	mov varlist_name_ptr+1, RA
	mov line_number, RA
	mov line_number+1, RA
	PSH RA
	
.parm_loop:
	PUL RA
	INC RA
	PSH RA
	JSR GET_PARAMETER
	JNC .check_parm
	CLC
.error_exit:	
	PUL RA ; retrieve parameter number
	RTS ; no more parameters found

.check_parm:	
	; first character of next parameter
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JZS .keyword_parm

	; input filename
	mov infile_name_ptr, RC
	mov infile_name_ptr+1, RD
	
	JMP .parm_loop
	
.keyword_parm:
	INC RCD
	mov RA, [RCD]
	
	STC
	CMP RA, #"o"
	JNZ .not_o
	
	; output file parameter found
	PUL RA
	INC RA ; increment to next parameter
	PSH RA
	JSR GET_PARAMETER
	JCS .error_exit
		
	mov outfile_name_ptr, RC	
	mov outfile_name_ptr+1, RD
	
	JMP .parm_loop

.not_o:
	STC
	CMP RA, #"I"
	JNZ .not_I
	
	; Include path list parameter found
	PUL RA
	INC RA ; increment to next parameter
	PSH RA
	JSR GET_PARAMETER
	JCS .error_exit
	
	mov include_path_list, RC
	mov include_path_list+1, RD
	
	JMP .parm_loop

.not_I:	
	STC
	CMP RA, #"V"
	JNZ .not_V
	
	; Variable name list output name?
	PUL RA
	INC RA ; increment to next parameter
	PSH RA
	JSR GET_PARAMETER
	JCS .varlist_stdout
	mov RA, [RCD]
	STC
	CMP RA, #"-"
	JNZ .varlist_set_name
	
.varlist_stdout:
	PUL RA
	DEC RA ; decrement to next parameter
	PSH RA
	mov RC, #1
	mov RD, #0

.varlist_set_name:
	mov varlist_name_ptr, RC	
	mov varlist_name_ptr+1, RD
	
	JMP .parm_loop
	
.not_V:	
	; unknown keyword
	PUL RA ; retrieve parameter number
	STC
	RTS

;
; assemble all lines in the input file
;
ASM_FILE:	
.loop:
	JSR ASM_LINE
	mov RA, eof_in
	OR  RA, #0
	JZS .loop
	RTS
	
;
; assemble the next line in the input file
;
ASM_LINE:
	JSR IO_SKIP_WHITE_SPACE_AND_LINE

.line_loop:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #";"
	JZS IO_END_LINE
	
	STC
	CMP RA, #10
	JZS IO_END_LINE
	
	JSR WORD_IS_START_CHAR
	JNC .read_word
	
	JSR PUT_ERROR_CHAR
	mov RA, #"*"
	JSR PUT_ERROR_CHAR
	JMP ASM_SYNTAX_ERROR
	
.read_word:
	JSR WORD_NEXT
	
	JSR IO_GOTC
	STC
	CMP RA, #":"
	JNZ .no_label
	JSR ASM_LABEL
	JMP .line_loop

.no_label:	
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #"="
	JNZ .no_variable
	JSR ASM_VARIABLE
	JMP .end_line

.no_variable:
	mov RA, word_cur_txt
	STC
	CMP RA, #"#"
	JNZ .no_directive
	JSR ASM_DIRECTIVE
	JMP .end_line
	
.no_directive:
	mva RCD, opcode_tokens
	mov RB, #((opcode_tokens_end - opcode_tokens) / 3) - 1
	JSR WORD_SEARCH_IN_LIST
	JCS ASM_SYNTAX_ERROR
	INC RA
	JSR ASM_OPCODE
	JMP .end_line
	
.end_line:
	JSR IO_SKIP_WHITE_SPACE
	; comment? skip to end of line
	STC
	CMP RA, #";"
	JZS IO_END_LINE
	
	; end of line? -> return
	STC
	CMP RA, #10
	JNZ .syntax_error
	RTS
	
.syntax_error:
	JSR PUT_ERROR_CHAR
	mov RA, #"+"
	JSR PUT_ERROR_CHAR
	; fallt hrough to ASM_SYNTAX_ERROR
	
;
; Display a syntax error and exit the assembler
;
ASM_SYNTAX_ERROR:	
    mov RA, #ERR_SYNTAX
	JMP MAIN_ERROR_MSG_EXIT

;
; assemble a line with an OpCode
;
; Parm:
;  RA - current OpCode token
;
ASM_OPCODE:
	mov current_opcode, RA
	
	; process addressing type
	JSR ASM_NEXT_FULL_ADDRTYP
	
	; get machine code for opcode
	JSR ASM_FIND_OPCODE
	JCS .no_opcode_found
	OR  RD, #0 ; machine code > 255?
	JZS .no_ext
	
	; write extension opcode byte
	mov RA, #0xff
	JSR IO_WRITE_BYTE
	
.no_ext:		
	mov RA, RC ; machine code byte
	JSR IO_WRITE_BYTE
	
	; write parameter bytes
	mov RB, #0
.addr_loop:	
	mov RA, addrtyplist,RB
	STC
	CMP RA, #ADR_IMM
	JNZ .no_immediate
	PSH RB
	CLC
	LSL RB
	mov RA, addrvallist,RB
	JSR IO_WRITE_BYTE
	PUL RB
	JMP .next_addr
.no_immediate:
	STC
	CMP RA, #ADR_ABS
	JZS .two_bytes
	STC
	CMP RA, #ADR_ABS_IDX
	JZS .two_bytes
	STC
	CMP RA, #ADR_ABS_INDIR
	JZS .two_bytes
	STC
	CMP RA, #ADR_ABS_INDIR_IDX
	JNZ .next_addr
.two_bytes:
	PSH RB
	CLC
	LSL RB
	mov RA, addrvallist,RB
	PSH RB
	JSR IO_WRITE_BYTE
	PUL RB
	mov RA, addrvallist+1,RB 
	JSR IO_WRITE_BYTE
	PUL RB
.next_addr:
	INC RB
	STC
	CMP RB, #2
	JNC .addr_loop
	RTS
	
.no_opcode_found:	
	mov RA, #ERR_NO_OPCODE
	JMP MAIN_ERROR_MSG_EXIT

ASM_FIND_OPCODE:
	mov RA, #0
	mov current_ext, RA

	mva RCD, opcode_adrtyps_lists
	mov RB, current_opcode
	CLC
	LSL RB
	mov RA, opcode_offs-1,RB
	OR  RA, #0
	JZS .next_section
	PSH RA
	mov RA, opcode_offs-2,RB
.oloop:
	PUL RB
	CLC
	ADD RC, RA
	ADD RD, #0
	mov RA, current_addrtyp

	STC
.loop:
	CMP RA, [RCD]
	JNC .next_section
	JZS .found
	INC RCD
	DEC RB
	JNZ .loop

.next_section:
	DEC current_ext
	JCS .end

	mva RCD, eopcode_adrtyps_lists
	mov RB, current_opcode
	CLC
	LSL RB
	mov RA, eopcode_offs-1,RB
	OR  RA, #0
	JZS .next_section
	PSH RA
	mov RA, eopcode_offs-2,RB
	JMP .oloop

.found:
	mov RA, current_ext
	OR  RA, #0
	JNZ .ext
	CLC
	ADD RC, #opcode_opcodes_lists-opcode_adrtyps_lists
	JMP .goon
.ext:
	CLC
	ADD RC, #eopcode_opcodes_lists-eopcode_adrtyps_lists
.goon:
	ADD RD, #0
	mov RC, [RCD]
	mov RD, RA

.end:
	RTS
;
;  Process a label definition in a line.
;
ASM_LABEL:
	JSR IO_GETC ; consume ":" char

	mov RA, addr_current
	mov value_current, RA
	mov RA, addr_current+1
	mov value_current+1, RA
	
	JSR WORD_UPPERCASE
	JSR ASM_PREPARE_LOCAL_LABEL
	
	JSR VSTORE_SET_VALUE
	
	; check for local label
	mov RA, word_cur_txt
	STC
	CMP RA, #"."
	JZS .end
	
	; not local -> copy current name
	mva RCD, label_current
	JSR WORD_COPY_CURRENT_WORD

.end:	
	RTS

;
; Append preceeding gloabl label to local label.
;
; global:
;  word_current  - label last read
;  label_current - last global label
;
ASM_PREPARE_LOCAL_LABEL:
	; local label starts with a dot (.)
	mov RA, word_cur_txt
	STC
	CMP RA, #"."
	JNZ .end
	mov RB, word_cur_len
	mov word_cur_txt,RB, RA
	INC word_cur_len
	mva RCD, label_cur_txt
	JMP WORD_APPEND_STRING
.end:
	RTS

;
;  Process a variable definition in a line.
;
ASM_VARIABLE:
	JSR IO_GETC ; consume "=" char
	JSR IO_SKIP_WHITE_SPACE

	; save variable name
	mov RB, #MAX_WORD_LEN+2
	PSB word_current+MAX_WORD_LEN+1, RB

	JSR EXPR_NEXT_EXPRESSION

	; restore variable name
	mov RB, #MAX_WORD_LEN+2
	PLB word_current, RB

	JSR WORD_UPPERCASE
	JSR VSTORE_SET_VALUE
	
	RTS
	mva RCD, word_cur_txt
	JSR PUT_STRING
	mov RA, #"="
	JSR PUT_CHAR
	mov RA, value_current+1
	JSR PUT_HEX_BYTE
	mov RA, value_current
	JSR PUT_HEX_BYTE
	mov RA, #10
	JSR PUT_CHAR
	RTS

;
; Process an assembler directive in a line.
;
ASM_DIRECTIVE:
	mva RCD, directives
	JSR WORD_FIND_IN_LIST
	STC
	CMP RA, #TOK_INCLUDE
	JZS ASM_INCLUDE
	STC
	CMP RA, #TOK_BANKDEF
	JZS ASM_BANKDEF
	STC
	CMP RA, #TOK_BANK
	JZS ASM_BANK
	STC
	CMP RA, #TOK_RES
	JZS ASM_RES
	STC
	CMP RA, #TOK_ONCE
	JZS ASM_ONCE
	STC
	CMP RA, #TOK_D
	JZS ASM_D
	STC
	CMP RA, #TOK_D16
	JNZ ASM_SYNTAX_ERROR
	; fall through to ASM_D16

;
; Process a #D16 directive.
;
; output a 16-bit value
;
ASM_D16:
	JSR IO_SKIP_WHITE_SPACE
	JSR EXPR_NEXT_EXPRESSION
	
	mov RA, value_current
	JSR IO_WRITE_BYTE
	mov RA, value_current+1
	JSR IO_WRITE_BYTE
	
.next_value:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #","
	JNZ .end
	JSR IO_GETC ; consume comma
	JMP ASM_D16
	
.end:	
	RTS

;
; Process a #D directive.
;
; output an 8-bit value
;
ASM_D:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #34
	JNZ .single_byte
	
	JSR WORD_NEXT_QUOTED	
	mov RB, #0
.string_loop:
	; check length, string may contain \0
	STC
	CMP RB, word_cur_len
	JCS .next_value
	mov RA, word_cur_txt,RB
	PSH RB
	JSR IO_WRITE_BYTE
	PUL RB
	INC RB
	JNC .string_loop ; will always jump (word_len < 255)
	
.single_byte:
	JSR EXPR_NEXT_EXPRESSION
	
	mov RA, value_current
	JSR IO_WRITE_BYTE
	
.next_value:
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #","
	JNZ .end
	JSR IO_GETC ; consume comma
	JMP ASM_D
	
.end:	
	RTS

;
; Process a #RES directive.
;
; advance bank address / no output
;
ASM_RES:
	JSR IO_SKIP_WHITE_SPACE
	JSR EXPR_NEXT_EXPRESSION
	
	CLC
	mov RA, addr_current
	ADD RA, value_current
	mov addr_current, RA
	mov RA, addr_current+1
	ADD RA, value_current+1
	mov addr_current+1, RA
	RTS

;
; Process #once directive.
;
; Register current file as single include.
;
ASM_ONCE:
	; set word to include prefix ".i."
	mov RB, #0
.prefix_loop:	
	mov RA, include_once_prefix,RB
	mov word_current,RB, RA
	INC RB
	OR  RA, #0
	JNZ .prefix_loop

	; append current include file name
	mva RCD, current_file_txt
	JSR WORD_APPEND_STRING
	
	; set assemble pass value
	mov RA, #0
	mov value_current+1, RA
	mov RA, pass_asm
	mov value_current, RA
	
	; store pass value for include
	JMP VSTORE_SET_VALUE

;
; Process a #BANK directive.
;
; switch the active memory bank
;
ASM_BANK:
	mov RC, bank_store_addr_current
	mov RD, bank_store_addr_current+1
	mov RA, RC
	OR  RA, RD
	JZS .no_bank
	
	mov RA, addr_current
	mov [RCD], RA
	INC RCD
	mov RA, addr_current+1
	mov [RCD], RA

.no_bank:
	JSR IO_SKIP_WHITE_SPACE
	JSR WORD_NEXT
	JSR WORD_UPPERCASE
	
	mov RB, word_cur_len
	mov RA, #"."
	mov word_cur_txt,RB, RA
	INC RB
	mov RA, #BANK_ADDR_SUFFIX
	mov word_cur_txt,RB, RA
	INC RB	
	mov RA, #0
	mov word_cur_txt,RB, RA
	mov word_cur_len, RB
	
	JSR VSTORE_GET_VALUE
	JNC .bank_found
	
	mov RA, #ERR_UNDEFINED_BANK
	JMP MAIN_ERROR_MSG_EXIT
	
.bank_found:
	mov bank_store_addr_current, RC
	mov bank_store_addr_current+1, RD
	
	mov RA, [RCD]
	mov addr_current, RA
	INC RCD
	mov RA, [RCD]
	mov addr_current+1, RA

	JSR IO_SKIP_WHITE_SPACE
	
	RTS
	
;
; Process a #BANKDEF directive.
;
; define a memory bank
;
ASM_BANKDEF:
	JSR IO_SKIP_WHITE_SPACE
	JSR WORD_NEXT
	JSR WORD_UPPERCASE
	
	JSR IO_SKIP_WHITE_SPACE_AND_LINE
	mov RB, #"{"
	JSR ASM_ASSERT_CHAR
	JSR IO_GETC ; consume opening brace
	
	mov RB, word_cur_len
	mov RA, #"."
	mov word_cur_txt,RB, RA
	INC RB
	INC RB
	mov RA, #0
	mov word_cur_txt,RB, RA
	mov word_cur_len, RB

.loop:
	JSR IO_SKIP_WHITE_SPACE_AND_LINE
	STC
	CMP RA, #"#"
	JNZ .end_bankdef
	
	; save bank name
	mov RB, #MAX_WORD_LEN+2
	PSB word_current+MAX_WORD_LEN+1, RB
	
	; read keyword parameter name
	JSR WORD_NEXT
	
	; tokenize parameter
	mva RCD, directives
	JSR WORD_FIND_IN_LIST
	OR  RA, #0
	JZS ASM_SYNTAX_ERROR
	
	PSH RA ; save token

	; read keyword parameter value
	JSR IO_SKIP_WHITE_SPACE
	JSR EXPR_NEXT_EXPRESSION
	
	PUL RA ; restore token
	
	; restore bank name
	mov RB, #MAX_WORD_LEN+2
	PLB word_current, RB
	
	; BANK ADDR?
	STC
	CMP RA, #TOK_ADDR
	JNZ .no_addr
	
	mov RA, #BANK_ADDR_SUFFIX
	mov RB, word_cur_len
	mov word_current,RB, RA
	JSR VSTORE_SET_VALUE
	
	JMP .loop
	
.no_addr:	
	; BANK OUTP?
	STC
	CMP RA, #TOK_OUTP
	JNZ .no_outp
	
	mov RA, #BANK_OUTP_SUFFIX
	mov RB, word_cur_len
	mov word_current,RB, RA
	JSR VSTORE_SET_VALUE
	JMP .loop	
	
.no_outp:	
	; Ignore BANK SIZE
	STC
	CMP RA, #TOK_SIZE
	JZS .loop
	
	; Illegal Parameter
	JMP ASM_SYNTAX_ERROR
	
.end_bankdef:
	mov RB, #"}"
	JSR ASM_ASSERT_CHAR
	JSR IO_GETC ; consume closing brace

	RTS
	
;
; Process a #INCLUDE directive.
;
; include lines from another file
;
ASM_INCLUDE:
	JSR WORD_NEXT_QUOTED
	
	; add include prefix
	mva RCD, include_once_prefix
	JSR WORD_PREPEND_WORD
	
	JSR VSTORE_GET_VALUE
	JCS .read_file
	
	mov RA, [RCD] ; get pass value
	
	; check whether the file has already been included
	STC
	CMP RA, pass_asm
	JZS .skip_file
	
.read_file:
	; remove include prefix
	mov RA, include_once_prefix_len
	mov RB, #0
	JSR WORD_DELETE_CHARS
	
	mva RCD, word_cur_txt
	mov search_name_ptr, RC
	mov search_name_ptr+1, RD
	
	; pointer Include Path list
	mov RC, include_path_list
	mov RD, include_path_list+1
	
	; save the current start of the PATH list
	mov path_name_ptr, RC
	mov path_name_ptr+1, RD
	
	; search module in PATH
	JSR PATH_LIST_SEARCH	
	JCS MAIN_IO_ERROR_EXIT
	
	mov RB, line_number+1
	PSH RB
	mov RB, line_number
	PSH RB
	
	JSR IO_ASM_SUBFILE
	
	PUL RB
	mov line_number, RB
	PUL RB
	mov line_number+1, RB
	
.skip_file:	
	JMP IO_END_LINE

;
; require last character
;  RB - expected character
; 
ASM_ASSERT_LAST_CHAR:
	JSR IO_GOTC
	; fall through to ASM_ASSERT_CHAR
	
;
; require a character
;  RA - current character
;  RB - expected character
; 
ASM_ASSERT_CHAR:
	STC
	CMP RA, RB
	JZS .end
	mov msg_value, RB
	mov RA, #ERR_MISSING_CHAR
	JMP MAIN_ERROR_MSG_EXIT
.end:
	RTS
	
;
; Process all parameters of an OpCode.
;
; determines the addressing type
;
ASM_NEXT_FULL_ADDRTYP:
	mov RA, #0
	mov addrtyplist, RA
	mov addrtyplist+1, RA
	mov addrtyplist+2, RA
	
	mov RB, #0 ; addrtyp counter
	
.addr_loop:	
	PSH RB
	JSR ASM_NEXT_ADDRTYP
	PUL RB
	
	mov RA, current_addrtyp
	mov addrtyplist,RB, RA
	CLC
	LSL RB
	mov RA, value_current
	mov addrvallist,RB, RA
	mov RA, value_current+1
	mov addrvallist+1,RB, RA
	LSR RB

	PSH RB
	JSR IO_SKIP_WHITE_SPACE
	STC
	CMP RA, #","
	JNZ .last_addr
	JSR IO_GETC ; consume comma
	PUL RB
	INC RB
	STC
	CMP RB, #3
	JNC .addr_loop
	mov RA, RB
	JSR PUT_ERROR_HEX_BYTE
	JMP ASM_SYNTAX_ERROR
	
.last_addr:
	PUL RB
	STC
	CMP RB, #2 ; 3 Parameters?
	JNZ .less_than_three
	
	; third parameter is RB?
	mov RA, addrtyplist+2
	STC
	CMP RA, #REG_RB
	JNZ .test_second
	
	INC addrtyplist+1
	mov RA, addrtyplist+1
	JMP .check_idx
	
.test_second:
	; second parameter is RB?
	mov RA, addrtyplist+1
	STC
	CMP RA, #REG_RB
	JNZ ASM_ILLEGAL_ADDRESSING
	
	; shift third parameter down to second
	mov RA, addrtyplist+2
	mov addrtyplist+1, RA
	mov RA, addrvallist+4
	mov addrvallist+2, RA
	mov RA, addrvallist+5
	mov addrvallist+3, RA
	
	INC addrtyplist
	mov RA, addrtyplist
	
.check_idx:
	STC
	CMP RA, #ADR_ABS_IDX
	JNC ASM_ILLEGAL_ADDRESSING
	
.less_than_three:
	mov RA, addrtyplist
	mov RB, addrtyplist+1
	OR  RB, #0
	JZS .set_addrtyp
	
	; two parameters
	; move first parameter into high nybble
	CLC
	LSL RA
	LSL RA
	LSL RA
	LSL RA
	OR  RA, RB
		
.set_addrtyp:
	mov current_addrtyp, RA
	RTS

ASM_ILLEGAL_ADDRESSING:	
	mov RA, #ERR_ILLEGAL_ADDRESSING
	JMP MAIN_ERROR_MSG_EXIT
	
;
; Process a single OpCode parameter.
;
; determines a single adress type
;
ASM_NEXT_ADDRTYP:
	mov RA, #0
	mov last_register, RA

	JSR IO_SKIP_WHITE_SPACE
	
	; no parameter?
	STC
	CMP RA, #";"
	JZS .none
	STC
	CMP RA, #10
	JNZ .no_line_end
.none:
	mov RA, #0
	mov current_addrtyp, RA
	RTS
	
.no_line_end:	
	STC
	CMP RA, #"#"
	JNZ .no_immediate
	JSR IO_GETC ; consume #
	mov RA, #ADR_IMM
	mov current_addrtyp, RA
	JMP EXPR_NEXT_EXPRESSION ; value
	
.no_immediate:
	STC
	CMP RA, #"["
	JNZ .no_indirect
	JSR IO_GETC ; consume [
    JSR EXPR_NEXT_EXPRESSION
	mov RB, #"]"
	JSR ASM_ASSERT_LAST_CHAR
	JSR IO_GETC ; consume ]
	
	mov RA, last_register
	OR  RA, #0
	JZS .abs_indir
	
	STC
	CMP RA, #REG_RCD
	JNZ ASM_ILLEGAL_ADDRESSING
	mov RA, #ADR_RCD_INDIR
	mov current_addrtyp, RA
	RTS
	
.abs_indir:
	mov RA, #ADR_ABS_INDIR
	mov current_addrtyp, RA
	RTS
	
.no_indirect:
	JSR EXPR_NEXT_EXPRESSION
	mov RA, last_register
	OR  RA, #0
	JZS .abs
	mov current_addrtyp, RA
	RTS
.abs:
	mov RA, #ADR_ABS
	mov current_addrtyp, RA	
	RTS

#include "asmIO.inc"
#bank pgm
#include "asmWord.inc"
#bank pgm
#include "asmExpr.inc"
#bank pgm
#include "DivMul16.inc"
#bank pgm
#include "num2str.inc"
#bank pgm
#include "VarStor2.inc"
#bank pgm
#include "streamio.inc"
#bank pgm
#include "param.inc"
#bank pgm
#include "PathSrch.inc"
#bank pgm

	#d RELOC_ESC
	#d 0x00
	#d 0x00

#include "RegsTab.inc"
#include "OpTokTab.inc"
#include "OpCodTab.inc"
#include "DirecTab.inc"

#include "asmErr.inc"

usage_msg: #d "Usage: asm {file} -o outfile"
			#d " -I includePathList"
			#d " -V varListFileName\0"
default_outfile_name: #d "a.out\0"
in_line_msg: #d " in line \0"
var_value_msg: #d "=0x\0"
var_bytes_msg: #d " variable mem\n\0"
include_once_prefix:
include_once_prefix_len: #d 0x03
include_once_prefix_txt: #d ".i.\0"

;
; program trailer
; 
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
shell_exit_stack: #res 2
msg_value: #res 2

include_path_list: #res 2

varlist_name_ptr: #res 2

label_current:
label_cur_len: #res 1
label_cur_txt: #res MAX_WORD_LEN+1

pass_asm: #res 1
current_opcode: #res 1
current_addrtyp: #res 1
current_ext: #res 1

addrtyplist: #res 3
addrvallist: #res 6

last_register: #res 1

addr_current: #res 2
outp_current: #res 1

bank_store_addr_current: #res 2

current_file_name: 
current_file_len: #res 1
current_file_txt: #res MAX_WORD_LEN+1

DATA_END:
