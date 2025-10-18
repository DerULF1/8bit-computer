;
; Standard functions for parameter handling
;

#bank pgm

;
; Searches the RA-th parameter
;
; count includes the command name
; 0 = command name
;
; Parm:
;   RA - number of the parameter
; Result:
;  c-flag - 0=found, 1=not found
;  RCD - pointer to parameter
;
GET_PARAMETER:
	mov RCD, parm_ptr
.ploop:	
	mov RB, [RCD]
	OR  RB, #0
	JZS .not_found
.count:	
	DEC RA
	JNC .end
.cloop:	
	INC RCD
	mov RB, [RCD]
	OR  RB, #0
	JNZ .cloop
	INC RCD
	JNC .ploop
.not_found:
	STC
.end:	
	RTS

;
; Counts all parameters
;
; count includes the command name
;
; Result:
;  c-flag - 0=found, 1=illegal parameter list
;  RA - number of the parameters
;
GET_PARAMETER_COUNT:
	mov RCD, parm_ptr
	mov RA, #0
.ploop:	
	mov RB, [RCD]
	OR  RB, #0
	JZS .end
.count:	
	INC RA
	JCS .end
.cloop:	
	INC RCD
	mov RB, [RCD]
	OR  RB, #0
	JNZ .cloop
	INC RCD
	JNC .ploop
.end:	
	RTS

#bank data

parm_ptr: #res 2
