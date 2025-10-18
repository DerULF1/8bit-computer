#include "8bit.cpu"

#include "OSCalls.inc"
#include "KeyNames.inc"
#include "taskinfo.inc"

;
; shell - simple UNIX like shell
;
; Written 26.09.2020 Ulf Caspers
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

; maximum number of chars in an input line
MAX_LINE_LEN = 60

; number of 256 bytes pages for input history
HISTORY_PAGES = 1

; character code for single quote (')
QUOTE_CHAR = "'"

#bank data
DATA_START:

#bank pgm

; RC/RD - parameter string

START:
	mov parm_pointer, RCD

	mov RCD, SP
	mov shell_exit_stack, RCD

	;JSR OUTPUT_CONSOLE_INFO
	
	JSR LOAD_PROFILE
	
	JSR INPUT_HISTORY_INIT

.input_loop:
	JSR SHOW_PROMPT
	JSR GET_LINE
	JCS ERROR_EXIT

	JSR EXECUTE_LINE
	JMP .input_loop
	
LOAD_PROFILE:
	mva RCD, profile_name
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .end
	
	PSH RA	
	JSR EXECUTE_STREAM
	PUL RA
	
	JSR OS_CLOSE
.end:	
	RTS
	
OUTPUT_CONSOLE_INFO:
	mva RCD, cons_info_start_msg
	JSR PUT_STRING

	; get information on current task
	STC ; c=1 => current task
	mva RCD, history_area
	JSR OS_GET_TASK_INFO
	
	; write console number
	mov RA, history_area+TIB_OFF_CONS
	CLC
	ADD RA, #"0"
	JSR PUT_CHAR

	mva RCD, cons_info_end_msg
	JMP PUT_STRING

;
; Execute all commands in a script file
;
; Parm:
;  RCD - pointer to 0-terminated file name of the script
;
EXECUTE_FILE:
	; save the pointer to the script file name
	mov search_name_ptr, RCD
	
	; pointer PATH variable name
	mva RCD, path_varname
	
	; find the PATH variable
	JSR TASKENV_GET_VAR
	
	; save the current start of the PATH list
	mov path_name_ptr, RCD
	
	; search script file in PATH
	JSR PATH_LIST_SEARCH	
	JCS SHOW_ERROR
	
	PSH RA	
	JSR EXECUTE_STREAM
	PUL RA
	
	JMP OS_CLOSE

;
; reads commands from an open stream
; 
; Parm:
;  RA - stream number
;
EXECUTE_STREAM:
	; save stream number in RC
	mov RC, RA
	
	; reset line number
	mov RA, #0
	mov line_nr, RA
	mov line_nr+1, RA
	
	; begin a new line
.new_line_loop:	
	mov RB, #0
	
	; count line number
	INC line_nr
	ICC line_nr+1
	
	; read all characters of the line
.in_loop:
	PSH RB
	PSH RC
	mov RA, RC
	JSR OS_GETC
	PUL RC
	PUL RB
	JCS .read_error
	PSH RA
	JSR PUT_CHAR
	PUL RA
	
	; ignore carriage return
	STC
	CMP RA, #13 ; carriage return
	JZS .in_loop
	
	; end of line?
	STC
	CMP RA, #10 ; line feed
	JZS .end_of_line
	
	; add character to current line
	mov input_line,RB, RA
	INC RB
	STC
	CMP RB, #MAX_LINE_LEN
	JNC .in_loop
	
	mva RCD, too_long_msg
	JSR PUT_ERROR_STRING
	JMP SHOW_ERROR_LINE_NUMBER

.end_of_line:
	mov RA, #0
	mov input_line,RB, RA
	PSH RC
	JSR EXECUTE_LINE
	PUL RC
	JMP .new_line_loop
	
.read_error:
	CMP RA, #EEOF ; end of file
	JNZ SHOW_ERROR
	
	; EOF? -> execute last line and return
	mov RA, #0
	mov input_line,RB, RA
	JMP EXECUTE_LINE

;
; output current line number to error stream
;
SHOW_ERROR_LINE_NUMBER:
	mva RCD, in_line_nr_msg
	JSR PUT_ERROR_STRING
	mov RA, line_nr
	mov RB, line_nr+1
	JSR NUM2STR
	mva RCD, num2str_txt
	JMP PUT_ERROR_STRING
	
;
; execute statement in the current line
; 
; global:
;  input_line - current line
; 
;
EXECUTE_LINE:
	mov RA, #0xFF
	mov pipe_in_fd, RA

	; column position in line
	mov RB, #0
	PSH RB
	
.parse_loop:
	; retrieve currnt column position
	PUL RB
	JSR PARSE_LINE
	
	; empty line?
	mov RA, fork_cmd
	OR  RA, #0
	JZS .end
	
	; save input_pointer
	PSH RB
	
	; shell internal command?
	JSR GET_INT_COMMAND
	STC 
	CMP RA, #0xFF
	JZS .not_shell_int
	JSR EXEC_INT_COMMAND
	JMP .parse_loop

.not_shell_int: ; external command
	JSR IS_SCRIPT_COMMAND
	JNC .not_a_script
	
	; save command line
	mov RB, #MAX_LINE_LEN+2
	PSB input_line_area+MAX_LINE_LEN+1, RB
	
	mva RCD, fork_cmd
	JSR EXECUTE_FILE
			
	; restore command line
	mov RB, #MAX_LINE_LEN+2
	PLB input_line_area, RB

	JMP .parse_loop

.not_a_script:
	mov RA, pipe_in_fd
	STC
	CMP RA, #0xFF
	JZS .no_pipe_stdin
	mov redir_in_fd, RA
	JMP .no_stdin_error
	
.no_pipe_stdin:
	; input redirection
	JSR OPEN_REDIR_STDIN
	JNC .no_stdin_error
.error_parse_loop:	
	JSR SHOW_ERROR
	JMP .parse_loop
	
.no_stdin_error:	
	mov fork_stdin, RA
	
	; output/input piping
	JSR OPEN_PIPE
	JCS .error_parse_loop 
	STC
	CMP RA, #0xFF
	JZS .no_pipe_stdout
	
	mov pipe_in_fd, RA
	mov RA, #0xFF
	mov redir_out_fd, RA
	mov RA, pipe_in_fd
	JMP .no_stdout_error
	
.no_pipe_stdout:
	; output redirection
	JSR OPEN_REDIR_STDOUT
	JCS .error_parse_loop

.no_stdout_error:	
	mov fork_stdout, RA
	
	; fixed error stream
	mov RA, #2
	mov fork_stderr, RA
	
	mva RCD, fork_struct
	JSR OS_FORK
	
	PSH RD
	PSH RC
	JSR CLOSE_REDIR_STDOUT
	JNC .no_close_out_error
	JSR SHOW_ERROR
.no_close_out_error:	
	JSR CLOSE_REDIR_STDIN
	JNC .no_close_in_error
	JSR SHOW_ERROR
.no_close_in_error:	
	PUL RC
	PUL RD

	mov RA, to_background
	OR  RA, #0
	JZS .exec
	
	mov RA, #"F"
	JSR OS_DISPLAY_CHAR
	mov RA, RD
	JSR OS_DISPLAY_HEX_BYTE
	mov RA, RC
	JSR OS_DISPLAY_HEX_BYTE
	JMP .parse_loop
	
.exec: 
	mov RA, pipe_cmds
	OR  RA, #0
	JNZ .parse_loop
	JSR OS_WAIT_TASK
	JMP .parse_loop
	
.end:
	RTS
	
OK_EXIT:
	mov RA, #0
	JMP EXIT_SHELL

CLOSE_REDIR_STDOUT:
	mov RA, redir_out_fd
	JMP CLOSE_REDIR
	
CLOSE_REDIR_STDIN:
	mov RA, redir_in_fd
	
CLOSE_REDIR:
	STC
	CMP RA, #0xFF
	JNZ .close
	CLC
	RTS
.close:	
	JMP OS_CLOSE
	
OPEN_PIPE:
	mov RA, pipe_cmds
	OR  RA, #0
	JNZ .do_pipe
	mov RA, #0xFF
	mov pipe_in_fd, RA
	RTS
.do_pipe:
	mov RA, #O_READ | O_WRITE
	mva RCD, pipe_name
	JSR OS_OPEN
	JCS .end
	mov pipe_in_fd, RA
.end:	
	RTS

OPEN_REDIR_STDOUT:
	mov RA, #0xFF
	mov redir_out_fd, RA
	
	mva RCD, redir_out_name
	mov RA, [RCD]
	OR  RA, #0
	JNZ .redir
	mov RA, #1 ; copy stdout
	CLC
	RTS

.redir:
	mov RB, #O_WRITE + O_CREATE
	STC
	CMP RA, #">"
	JNZ .no_append
	OR  RB, #O_APPEND
	INC RCD
	
.no_append:	
	mov RA, RB
	JSR OS_OPEN
	JCS .end

	mov redir_out_fd, RA
.end:	
	RTS	

OPEN_REDIR_STDIN:
	mov RA, #0xFF
	mov redir_in_fd, RA
	
	mva RCD, redir_in_name
	mov RA, [RCD]
	OR  RA, #0
	JNZ .redir
	mov RA, #0 ; copy stdin
	CLC
	RTS

.redir:
	mov RA, #O_READ
	JSR OS_OPEN
	JCS .end

	mov redir_in_fd, RA
.end:	
	RTS
	
;
; outputs error message thru console
;	
SHOW_ERROR:	
	PSH RA
	mva RCD, fail_msg
	JSR PUT_ERROR_STRING
	PUL RA
	JMP PUT_ERROR_HEX_BYTE

;
; outputs error message thru console and exits shell
;	
ERROR_EXIT:
	JSR SHOW_ERROR
	mov RA, #1
	
;
; exits shell
;	
EXIT_SHELL:	
	mov RCD, shell_exit_stack
	mov SP, RCD
	RTS
	
;
; outputs an input prompt onto stdout
;	
SHOW_PROMPT:
	mov RA, #"\n"
	JSR PUT_CHAR
	
	; show current directory
	mva RCD, pwd_varname
	JSR TASKENV_GET_VAR
	JCS .no_pwd
	JSR PUT_STRING
.no_pwd:

	; show prompt end
	mva RCD, prompt
	JSR PUT_STRING
	JCS ERROR_EXIT
	RTS	

SET_CURSOR_TYPE:
	mov RA, line_overwrite
	OR  RA, #0
	JZS .ins_cursor
	mov RA, #CURSOR_TYPE_LINE
	JMP .set_type
.ins_cursor:	
	mov RA, #CURSOR_TYPE_BLINK
.set_type:	
	JMP CURSOR_MODE

;
; reads a line thru console
;		
GET_LINE:
	mov RB, #0
	mov line_len, RB
	mov line_overwrite, RB
	JSR SET_CURSOR_TYPE
.loop:	
	JSR GET_CHAR
	JCS .end
	
	STC
	CMP RA, #KEY_DELETE
	JZS .del
	
	STC
	CMP RA, #KEY_BACKSPACE
	JNZ .nobs
	
.backspace:
	OR  RB, #0
	JZS .loop
	
	JSR CURSOR_LEFT
	DEC RB

.del:
	JSR INPUT_DELETE_CHAR
	JMP .loop
	
.nobs:
	STC
	CMP RA, #KEY_CURSOR_LEFT
	JNZ .nocl
	
	OR  RB, #0
	JZS .loop
	
	DEC RB
	JSR CURSOR_LEFT
	JMP .loop
	
.nocl:
	STC
	CMP RA, #KEY_CURSOR_RIGHT
	JNZ .nocr
	
	STC
	CMP RB, line_len
	JCS .loop
	
	INC RB
	JSR CURSOR_RIGHT
	JMP .loop
	
.nocr:
	STC
	CMP RA, #KEY_HOME
	JNZ .nohome
	
	JSR INPUT_CURSOR_HOME
	JMP .loop
	
.nohome:
	STC
	CMP RA, #KEY_END
	JNZ .noend
	
	JSR INPUT_CURSOR_END
	JMP .loop
	
.noend:
	STC
	CMP RA, #KEY_INSERT
	JNZ .noins	

	; toggle overwrite flag
	mov RA, line_overwrite
	XOR RA, #1
	mov line_overwrite, RA
	JSR SET_CURSOR_TYPE
	
	JMP .loop
	
.noins:
	STC
	CMP RA, #KEY_CURSOR_UP
	JNZ .noup	

	mov RC, line_len
	PSH RB
	PSH RC
	JSR INPUT_HISTORY_BACK
	PUL RC
	PUL RB
	JCS .loop

.show_input_line:
	PSH RC ; save old line length
	JSR INPUT_CURSOR_HOME
	mva RCD, input_line
	JSR PUT_STRING
	
	mov RB, line_len	
	PUL RC ; restore old line length
	
	; delete characters of old line
	mov RA, #" "
.cloop:
	STC
	CMP RB, RC	
	JCS .cloop_end
	INC RB
	JSR PUT_CHAR
	JMP .cloop
.cloop_end:
	JSR INPUT_CURSOR_HOME
	JSR INPUT_CURSOR_END
	JMP .loop
	
.noup:
	STC
	CMP RA, #KEY_CURSOR_DOWN
	JNZ .nodown

	mov RC, line_len
	PSH RB
	PSH RC
	JSR INPUT_HISTORY_FORWARD
	PUL RC
	PUL RB
	JNC .show_input_line
	JCS .loop
	
.nodown:
	STC
	CMP RA, #KEY_ESC
	JNZ .noesc

	mov RC, line_len
	mov RA, #0
	mov line_len, RA
	mov input_line, RA
	JMP .show_input_line
	
.noesc:
	STC
	CMP RA, #10
	JNZ .no_enter
	
	JSR PUT_CHAR
	JCS .end

	JMP .end_input
	
.no_enter:
	; filter out other non-printable characters
	STC
	CMP RA, #" "
	JNC .loop
	CMP RA, #128
	JCS .loop
	
	JSR PUT_CHAR
	JCS .end
	
	STC
	CMP RB, line_len
	JZS .append_char
	
	mov RC, line_overwrite
	LSR RC
	JCS .store_char
		
	PSH RA
	JSR INPUT_INSERT_CHAR
	PUL RA
	JMP .store_char
		
.append_char:
	INC line_len
	
.store_char:
	mov input_line,RB, RA
	INC RB
	mov RA, line_len
	STC
	CMP RA, #MAX_LINE_LEN-1
	JCS .backspace
	JMP .loop
	
.end_input:
	CLC
	; terminate input line with \0
	mov RA, #0
	mov RB, line_len
	mov input_line,RB, RA
	
	JSR INPUT_HISTORY_ADD
.end:	
	RTS

INPUT_INSERT_CHAR:
	mov RC, RB
	
	; shift input buffer to the right
	mov RB, line_len
	STC
.iloop:
	CMP RC, RB
	JCS .iend	
	mov RA, input_line-1,RB
	mov input_line,RB, RA
	DEC RB
	JCS .iloop
	
.iend:
	INC line_len
	
	; output shifted buffer
.oloop:
	INC RB
	STC 
	CMP RB, line_len
	JCS .oend
	mov RA, input_line,RB
	JSR PUT_CHAR
	JMP .oloop
	
	; reset cursor to input position
.oend:
	DEC RB
	CMP RC, RB
	JCS .end
	
	JSR CURSOR_LEFT
	JMP .oend
	
.end:
	RTS

INPUT_DELETE_CHAR:
	DEC line_len
	JCS .do_del
	INC line_len
	JMP .end
	
.do_del:
	mov RC, RB
.dloop:
	STC
	CMP RB, line_len
	JCS .dend
	mov RA, input_line+1,RB
	mov input_line,RB, RA
	JSR PUT_CHAR
	INC RB
	JNC .dloop
	
.dend:
	mov RA, #" "
	JSR PUT_CHAR
	INC RB

.cloop:
	JSR CURSOR_LEFT
	DEC RB
	STC
	CMP RC, RB
	JNC .cloop
	
.end:
	RTS

INPUT_CURSOR_HOME:
	OR  RB, #0
	JZS .end
	JSR CURSOR_LEFT
	DEC RB
	JNZ INPUT_CURSOR_HOME
.end:	
	RTS

INPUT_CURSOR_END:
	STC
	CMP RB, line_len
	JCS .end
	JSR CURSOR_RIGHT
	INC RB
	JMP INPUT_CURSOR_END
.end:	
	RTS

;
; Execute shell internal command
;
; Parm:
;  RA - command number
;
EXEC_INT_COMMAND:
	mva RCD, fork_cmd
	mov parm_ptr, RCD

	mva RCD, int_procs
	
	; command number * 3 + 1
	mov RB, RA
	STC     ; (+1)
	LSL RA
	ADD RD, #0
	ADD RA, RB
	ADD RD, #0
	ADD RC, RA
	ADD RD, #0
	
	mov RA, [RCD]
	INC RCD
	mov RD, [RCD]
	mov RC, RA
	JMP [RCD]

;
; Check input line for shell script command
;
; Return:
;  c-flag: 0 = no shell script
;          1 = shell script
;
IS_SCRIPT_COMMAND:
	mva RCD, fork_cmd
	
.sloop:	
	mov RB, #0 ; reset suffix counter
	
.cloop:	
	mov RA, script_suffix, RB ; suffix character
	XOR RA, [RCD] ; compare to current command character
	AND RA, #0b11011111 ; ignore upper/lower case
	JNZ .no_match ; no match
	INC RCD
	INC RB ; character matches -> advance suffix counter
	CMP RB, #script_suffix_end-script_suffix-1 ; c=0
	JNC .cloop
	RTS ; suffix found -> is a script
	
.no_match:
	mov RA, [RCD] ; end of command?
	INC RCD
	OR  RA, #0
	JNZ .sloop ; no -> restart suffix search loop
	CLC ; end of command -> is not a script
	RTS
	
;
; Check input line for shell internal command
;
; Return:
;  RA - 0xFF - not found
;       else: command number
;
GET_INT_COMMAND:
	mov RA, #0 ; command counter
	mva RCD, int_commands
.wloop:	
	PSH RA
	mov RB, #0
.cloop:	
	mov RA, fork_cmd,RB
	OR  RA, #0
	JZS .found
	STC 
	CMP RA, [RCD]
	JNZ .next_command
	INC RB
	INC RCD
	JMP .cloop
	
.next_command:
	mov RA, [RCD]
	INC RCD
	OR  RA, #0
	JNZ .next_command
	PUL RA ; restore command counter
	INC RA ; command counter up
	mov RB, [RCD] ; end of command list?
	OR  RB, #0
	JNZ .wloop
	mov RA, #0xFF ; no command marker
	RTS
.found:
	PUL RA ; restore command counter
	RTS
	
;
; parses input line
;
; eliminates spaces
; puts \0 between parameters
; ends command line with double-\0
;	
PARSE_LINE:
	mva RCD, fork_cmd
	mov RA, #0
	mov to_background, RA
	mov redir_out_name, RA
	mov redir_in_name, RA
	mov pipe_cmds, RA
	
.skip_blank:	
	mov RA, input_line,RB
	INC RB
	STC	
	CMP RA, #" "
	JZS .skip_blank
	
	STC
	CMP RA, #"&"
	JNZ .no_back
	mov to_background, RA
	JMP .skip_blank
	
.no_back:
	STC
	CMP RA, #">"
	JNZ .no_redir_out_name
	PSH RD
	PSH RC
	mva RCD, redir_out_name
	JSR COPY_WORD
	PUL RC
	PUL RD
	JMP .check_zero
	
.no_redir_out_name:	
	STC
	CMP RA, #"<"
	JNZ .no_redir_in_name
	PSH RD
	PSH RC
	mva RCD, redir_in_name
	JSR COPY_WORD
	PUL RC
	PUL RD
	JMP .check_zero
	
.no_redir_in_name:	
	JMP .check_zero

.loop:	
	STC	
	CMP RA, #" "
	JNZ .no_blank
	
	mov RA, #0
	mov [RCD], RA
	INC RCD
	JMP .skip_blank
	
.no_blank:	
	STC	
	CMP RA, #"|"
	JNZ .no_pipe
	
	mov pipe_cmds, RA
	mov RA, #0
	JMP .end_cmd
	
.no_pipe:	
	STC
	CMP RA, #"$"
	JNZ .no_variable_name
	JSR COPY_VARIABLE
	JMP .check_zero
	
.no_variable_name:
	STC
	CMP RA, #QUOTE_CHAR
	JNZ .no_quote
	
; read string in quotes as one parameter
.qloop:	
	mov RA, input_line,RB
	INC RB
	
	OR  RA, #0
	JZS .check_zero
	
	STC
	CMP RA, #QUOTE_CHAR
	JZS .next_char
	
	mov [RCD], RA
	INC RCD
	JMP .qloop

.no_quote:
	
.copy_char:
	mov [RCD], RA
	INC RCD

.next_char:	
	mov RA, input_line,RB
	INC RB
	
.check_zero:	
	OR  RA, #0
	JNZ .loop
	
	DEC RB ; reposition input pointer to ending zero
	
.end_cmd:	
	mov [RCD], RA
	INC RCD
	mov [RCD], RA
	RTS
	
COPY_VARIABLE:
	PSH RD
	PSH RC
	mva RCD, variable_name
	JSR COPY_WORD
	PSH RB ; save input pointer
	PSH RA ; save last character read
	
	mva RCD, variable_name
	JSR TASKENV_GET_VAR
	mov variable_ptr, RCD
	PUL RA
	PUL RB
	PUL RC
	PUL RD
	JCS .skip_var

	PSH RB ; save input pointer
	PSH RA ; save last character read
	mov RB, #0
.var_loop:
	mov RA, [variable_ptr],RB
	OR  RA, #0
	JZS .var_end
	mov [RCD], RA
	INC RB
	INC RCD
	JMP .var_loop
.var_end:
	PUL RA
	PUL RB
	
.skip_var:	
	RTS
	
COPY_WORD:
	mov RA, input_line,RB
	INC RB	
	mov [RCD], RA
	OR  RA, #0
	JZS .ro_exit
	STC	
	CMP RA, #" "
	JZS .ro_end
	INC RCD
	JMP COPY_WORD
.ro_end:
	mov RA, #0
	mov [RCD], RA
	mov RA, #" "
.ro_exit:
	RTS

SET_CMD:
	; get parameter for variable 
	mov RA, #1
	JSR GET_PARAMETER
	JCS .list_env ; no parameter

	JMP TASKENV_SET_VAR

.list_env:
	JSR OS_GET_ENV_POINTER
.loop:	
	mov RA, [RCD]
	OR  RA, #0
	JZS .end
.vloop:
	JSR PUT_CHAR
	JCS SHOW_ERROR
	INC RCD
	mov RA, [RCD]
	OR  RA, #0
	JNZ .vloop
	mov RA, #10
	JSR PUT_CHAR
	JCS SHOW_ERROR
	INC RCD
	JNC .loop
.end:	
	RTS

CD_CMD:
	; get parameter for cd command
	mov RA, #1
	JSR GET_PARAMETER
	JCS .end; no parameter

	; check whether new directory exists
	; also returns canonical directory name in RCD
	mov RA, #O_READ + O_DIR
	JSR OS_OPEN
	JCS SHOW_ERROR	
	PSH RA
	
	; transfer canonical name to PWD variable
	
	; copy variable name
	mov RB, #0
.vloop:
	mov RA, pwd_varname,RB
	mov fork_cmd,RB, RA
	INC RB
	OR  RA, #0
	JNZ .vloop
	
	mov RA, #"="
	mov fork_cmd-1,RB, RA
	
	; copy canonical directory name
.nloop:
	mov RA, [RCD]
	mov fork_cmd,RB, RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .nloop
	
	; write name into environemnt
	mva RCD, fork_cmd
	JSR TASKENV_SET_VAR
	
	; close dir list and exit
	PUL RA
	JMP OS_CLOSE
	
.end:
	RTS


INPUT_HISTORY_INIT:
	mov RB, #HISTORY_PAGES
	mva RCD, history_area
	JSR LSTORE_INIT

	RTS
		
INPUT_HISTORY_BACK:
	JSR LSTORE_LINE_UP
	JCS .end
	mva RCD, input_line_area
	JSR LSTORE_GET_LINE
.end:	
	RTS
		
INPUT_HISTORY_FORWARD:
	JSR LSTORE_LINE_DOWN
	JCS .end
	mva RCD, input_line_area
	JSR LSTORE_GET_LINE
.end:	
	RTS
		
INPUT_HISTORY_ADD:
	; skip to end of LINE_STORE
	JSR LSTORE_LINE_DOWN
	JNC INPUT_HISTORY_ADD
	
	; try to insert input line
	mva RCD, input_line_area
	JSR LSTORE_INS_LINE
	JNC .insline_OK
	
	; remove top entry
	JSR INPUT_HISTORY_REMOVE
	
	; retry insert
	JNC INPUT_HISTORY_ADD
	
.insline_OK:
	JMP LSTORE_LINE_DOWN

INPUT_HISTORY_REMOVE:
	JSR LSTORE_LINE_TOP
	JSR LSTORE_DEL_LINE
	RTS

#include "streamio.inc"
#bank pgm
#include "cursor.inc"
#bank pgm
#include "taskenv.inc"
#bank pgm
#include "param.inc"
#bank pgm
#include "LineStor.inc"
#bank pgm
#include "DivMul16.inc"
#bank pgm
#include "num2str.inc"
#bank pgm
#include "PathSrch.inc"
#bank pgm

int_procs:
	mov RA, CD_CMD
	mov RA, OK_EXIT
	mov RA, SET_CMD

	#d RELOC_ESC
	#d 0x00
	#d 0x00	

script_suffix: #d ".sh\0"
script_suffix_end:

cons_file_name: #d "/con0\0"	
pipe_name: #d "/pipe\0"
fail_msg: #d "failed \0"
too_long_msg: #d "too many characters\0"
in_line_nr_msg: #d " in line \0"
prompt: #d ">\0"
pwd_varname: #d "PWD\0"
path_varname: #d "PATH\0"
profile_name: #d "PROFILE.SH\0"
cons_info_start_msg: #d "Shell on /con\0"
cons_info_end_msg: #d "\0"

int_commands:
	#d "cd\0"
	#d "exit\0"
	#d "set\0"
	#d "\0"

END_TPGM:

;
; program trailer
;
	ld16 DATA_START
	ld16 DATA_END - DATA_START

#bank data
parm_pointer: #res 2
shell_exit_stack: #res 2
line_overwrite: #res 1
line_nr: #res 2
input_line_area:
line_len: #res 1
input_line: #res MAX_LINE_LEN+1

history_area: #res HISTORY_PAGES*256

variable_name: #res 20
variable_ptr: #res 2

fork_struct:
fork_stdin: #res 1
fork_stdout: #res 1
fork_stderr: #res 1
fork_cmd: #res MAX_LINE_LEN+1

to_background: #res 1
pipe_cmds: #res 1
pipe_in_fd: #res 1
redir_out_name: #res 40
redir_out_fd: #res 1
redir_in_name: #res 40
redir_in_fd: #res 1

DATA_END: