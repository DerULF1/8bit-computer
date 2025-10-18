;
; Lock Functions
;
; manages locks in the system
;

; maximum number of locks
LOCK_MAX_NUM = 32

LOCK_ST_FREE = 0xFF ; free lock


#bank pgm
;
; initializes all task data
;
LOCK_INIT:
	mov RA, #LOCK_ST_FREE
	mov RB, #LOCK_MAX_NUM-1
.loop:
	mov lock_owners,RB, RA
	DEC RB
	JCS .loop
	RTS

;
; requests a lock (blocks if not available)
;
; Parm:
;  RB - lock number
; Result:
;  c-flag - 0=O.K., 1=error
;  RA - error code (EEXIST=already reserved)
;
LOCK_REQUEST:
	STC
	CMP RB, #LOCK_MAX_NUM
	JCS _LOCK_ILLEGAL
	
.retry:	
	SII
	mov RA, lock_owners,RB
	STC
	CMP RA, #LOCK_ST_FREE
	JZS .reserve
	CII
	
	STC
	CMP RA, task_active
	JZS .reserved
	
	PSH RB
	JSR TASK_FORCE_SWITCH
	PUL RB
	JMP .retry
	
.reserve:
	mov RA, task_active
	mov lock_owners,RB, RA
	CII
	CLC
	RTS
	
.reserved:	
	mov RA, #EEXIST
	STC
	RTS
	
_LOCK_ILLEGAL:
	mov RA, #ENOENT
	STC
	RTS
	
;
; releases a lock
;
; Parm:
;  RB - lock number
;
LOCK_RELEASE:
	STC
	CMP RB, #LOCK_MAX_NUM
	JCS _LOCK_ILLEGAL
	
	STC
	SII
	mov RA, lock_owners,RB
	CMP RA, task_active
	JNZ .not_owner
	mov RA, #LOCK_ST_FREE
	mov lock_owners,RB, RA
	CII
	CLC
	RTS
	
.not_owner:
	CII
	mov RA, #EACCES
	STC
	RTS
	
;
; Free all locks of a certain owner
;
; Parm:
;  RD - lock owner
;  
LOCK_FORCE_RELEASE:
	mov RB, #LOCK_MAX_NUM-1
	SII	
	STC
.loop:
	mov RA, lock_owners,RB
	CMP RA, RD
	JNZ .skip
	mov RA, #LOCK_ST_FREE
	mov lock_owners,RB, RA
.skip:	
	DEC RB
	JCS .loop
	
	CII
	RTS
	
#bank data
lock_owners: #res LOCK_MAX_NUM