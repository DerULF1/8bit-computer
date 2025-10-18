
;
; searches a file using a list of path names
;
; the path list entries are divided by colons (:)
; and ends with a \0
;
; parm:
; result:
;   c-flag: 0 = O.K., 1 = error
;   RA - open file number, error number
; global:
;   path_name_ptr - pointer to path list (input)
;   search_name_ptr - pointer to file name (input)
;
PATH_LIST_SEARCH:
	
	; qualified name? -> no PATH resolution
	mov RB, #0
	mov RA, [search_name_ptr],RB
	STC
	CMP RA, #"/"
	JZS .not_in_path
	STC
	CMP RA, #"."
	JZS .not_in_path
	
	; copy next path entry
.ploop:	
	mov RB, #0
	
.cloop:
	mov RA, [path_name_ptr],RB
	; end of PATH list?
	OR  RA, #0
	JZS .add_name
	; end of list entry
	STC
	CMP RA, #":"
	JZS .add_name1 ; c-flag is set to skip :

	mov path_file_name,RB, RA
	INC RB
	JNC .cloop
	
.add_name:
	CLC
.add_name1: ; enter with c-flag set
	; advance PATH list pointer to next entry
	mov RA, path_name_ptr
	ADD RA, RB
	mov path_name_ptr, RA
	ICC path_name_ptr+1
	
	; no characters copied? -> end of list
	OR  RB, #0
	JZS .not_in_path
	
	; check for ending /
	DEC RB
	mov RA, path_file_name,RB
	INC RB
	STC
	CMP RA, #"/"
	JZS .slash_OK
	
	mov RA, #"/"
	mov path_file_name,RB, RA
	INC RB
	
	; add original file name
.slash_OK:	
	mov RCD, search_name_ptr
.ncopy:
	mov RA, [RCD]
	mov path_file_name,RB, RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .ncopy
	
	; try to open file 
	mva RCD, path_file_name
	mov RA, #O_READ
	JSR OS_OPEN
	JNC .end ; success? -> Exit
	
	; try next PATH list entry
	JMP .ploop

.not_in_path:
	; check with the original name
	mov RCD, search_name_ptr
	mov RA, #O_READ
	JSR OS_OPEN

.end:
	RTS

#bank data
path_name_ptr: #res 2
search_name_ptr: #res 2
path_file_name: #res 64
