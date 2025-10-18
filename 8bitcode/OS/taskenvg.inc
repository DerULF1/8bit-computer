;
; Task Environment Get Variables Function
;

;
; task environment:
; (varname=varvalue\0)*\0
;

#bank pgm

;
; Get environment variable value
;
; parm:
;  RCD    - pointer to 0-terminated variable name
; result:
;  c-flag - 1=not found, 0=O.K.
;  RB  - variable name length
;  RCD - pointer to 0-terminated variable value
;
TASKENV_GET_VAR:
	; store search start
	mov taskenv_var_ptr, RCD
	
	; environment pointer to RCD
	JSR OS_GET_ENV_POINTER
	
	; B-Reg is index to variable name
	mov RB, #0
.loop:
	mov RA, [RCD]
	OR  RA, #0
	JZS .not_found ; end of environment
	STC
	CMP RA, #"=" ; end of variable name
	JZS .varend
	
	mov RA, [taskenv_var_ptr],RB
	OR  RA, #0
	JZS .skip_var ; variable name is shorter
	STC
	CMP RA, [RCD] 
	JNZ .skip_var ; different character
	
	INC RCD ; next char in environment
	INC RB  ; next char of variable name
	JNC .loop
	
.not_found:
	STC
	RTS
	
.varend:
	mov RA, [taskenv_var_ptr],RB
	OR  RA, #0
	JZS .found
	STC
	CMP RA, #"="
	JZS .found
	
	; skip to next environment variable
.skip_var:
	INC RCD	
	mov RA, [RCD]
	OR  RA, #0
	JNZ .skip_var
	
	; restart at next variable
	mov RB, #0
	INC RCD
	JMP .loop

.found:
	INC RCD
	RTS

#bank data
taskenv_var_ptr: #res 2
