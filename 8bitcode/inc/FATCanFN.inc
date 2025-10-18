;
; FAT Canonicalize File Name
;

FAT_MAX_NAME_LEN=64

#bank pgm
	
;
; Canonicalize File Name
;
; parm:
;  RCD - pointer to 0-terminated file name
; result:
;  c-flag - 0 = O.K., 1 = error
;  RA - 0 = O.K., >0 = error
;  RCD - pointer to 0-terminated canonical file name
; 
FAT_CANONICAL_FILE_NAME:
	mov RA, [RCD]
	STC
	CMP RA, #"/"
	JNZ .relative_file
	mov RB, #1
	mov fat_canonical_fn_buf, RA
	INC RCD
	JMP .next_section

.relative_file:
	PSH RD
	PSH RC
	; append current working directory
	mva RCD, PWD_NAME
	JSR TASKENV_GET_VAR
	mov RB, #0
	JCS .no_pwd

.rcopy_loop:
	mov RA, [RCD]
	OR  RA, #0
	JZS .rcopy_end
	mov fat_canonical_fn_buf,RB, RA
	INC RB
	INC RCD
	JNC .rcopy_loop
	
.no_pwd:
	mov RA, #"/"
	mov fat_canonical_fn_buf, RA
	mov RB, #1
	
.rcopy_end:	
	PUL RC
	PUL RD
	; ensure last character was a slash
	mov RA, fat_canonical_fn_buf-1,RB
	STC
	CMP RA, #"/"
	JZS .next_section
	mov RA, #"/"
	DEC RCD
	JMP .copy_loop1

.skip_slash:
	INC RCD
.next_section:
	mov RA, [RCD]
	STC
	CMP RA, #"/" ; skip doubled slashes
	JZS .skip_slash
	STC
	CMP RA, #"."
	JNZ .copy_loop1
	DEC RB ; overwrite last slash
	INC RCD ; skip dot
	mov RA, [RCD]
	STC
	CMP RA, #"."
	JNZ .check_slash ; single dort must be followed by slash
	
.double_dot:
	INC RCD ; skip second dot
.back_loop:
	DEC RB
	JNS .illegal_name ; gone above root dir
	mov RA, fat_canonical_fn_buf,RB
	STC
	CMP RA, #"/" ; begin of last section 
	JNZ .back_loop 

	; dot must be followed by a slash
.check_slash:	
	mov RA, [RCD]
	OR  RA, #0 ; end of name
	JZS .copy_loop1
	STC
	CMP RA, #"/"
	JZS .copy_loop1
	
.illegal_name:
	mov RA, #EINVAL
	STC
	RTS
		
.copy_loop:
	mov RA, [RCD]
.copy_loop1:
	mov fat_canonical_fn_buf,RB, RA
	INC RCD
	INC RB
	STC
	CMP RA, #"/"
	JZS .next_section
	OR  RA, #0
	JNZ .copy_loop
	
	
.exit:
	mva RCD, fat_canonical_fn_buf
	RTS

	
	#d RELOC_ESC
	ld16 PWD_NAME_END-PWD_NAME+1

PWD_NAME: #d "PWD\0"
PWD_NAME_END:

#bank data

fat_canonical_fn_buf: #res FAT_MAX_NAME_LEN + 1
