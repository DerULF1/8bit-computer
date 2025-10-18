;
; Task Functions
;
; manages different tasks on the machine
;

#bankdef task_control_block
{
    #addr 0x0000
}

; marker for system owned task resources
SYS_OWNER = 0xF0 ; Task owner for the system

; maximum number of tasks
TASK_MAX_NUM = 16

; maximum number of files per task
TASK_MAX_FILES = 16
TASK_FILE_FREE = 0xFF ; unused file slot

; Task status codes
TASK_ST_RUNNING  = 0 ; task is schedulable
TASK_ST_INACTIVE = 0x80 ; task not to be scheduled
					; MUST be bit 7 (see IRQ: JNN)
TASK_ST_FREE     = 0xFF ; unused task slot

#bank task_control_block
TCB_OFF_TID: #res 2 ; Task ID
TCB_OFF_FILES: #res TASK_MAX_FILES ; file number table
TCB_OFF_TICKS: #res 3 ; total time ticks for this task
TCB_OFF_PARENT: #res 2 ; parent Task ID
TCB_OFF_CONS: #res 1 ; ID of the connected console
TCB_OFF_ENV: #res 2 ; pointer to task environment
TCB_OFF_CMD: ; begin of the command line of this task

#bank pgm
;
; initializes all task data
;
TASK_INIT:
	mov RA, #TASK_ST_FREE
	mov RB, #TASK_MAX_NUM-1
.loop:
	mov task_status,RB, RA
	DEC RB
	JCS .loop
	mov RA, #SYS_OWNER
	mov task_active, RA
	mov RA, #0
	mov task_next_tid, RA
	mov task_next_tid+1, RA
	mov task_active_block, RA
	mov task_active_block+1, RA	
	RTS

;
; finds a task slot and task number and
; creates a new task control block
;
; Return:
;  RB - new task slot index
;	
TASK_NEW:
	; search for a free task slot
	mov RB, #TASK_MAX_NUM-1
	PSF ; save Interrupt Inhibit state
	SII ; no parallel searches here
.loop:
	mov RA, task_status,RB
	STC
	CMP RA, #TASK_ST_FREE
	JZS .slot_found
	DEC RB
	JCS .loop
	
	; too many active tasks - try later
	PLF ; restore interrupt inhibt state
	mov RA, #EAGAIN
	STC
	RTS

.slot_found:	
	PSH RB ; save slot index
	
	; allocate task control block
	mov RB, #SYS_OWNER
	JSR MEM_ALLOC_SINGLE
	PUL RB ; restore slot index
	JCS .error_end
	
	; convert to block index
	CLC
	LSL RB 
	
	; store task control block address
	mov RC, #0
	mov RA, RC
	mov task_blocks,RB, RA
	mov RA, RD
	mov task_blocks+1,RB, RA
	
	; store task stack address
	mov RA, #0xFF
	mov task_stacks,RB, RA
	; TASK_MAX_NUM=16 => 16 stacks with 4k
	mov RA, RB
	LSL RA 
	LSL RA 
	LSL RA
	OR  RA, #15
	mov task_stacks+1,RB, RA
	
	; restore slot index
	LSR RB
	
	; reserve slot
	mov RA, #TASK_ST_INACTIVE
	mov task_status,RB, RA
	
	; set task number
	mov RA, task_next_tid
	mov [RCD], RA
	INC RCD
	mov RA, task_next_tid+1
	mov [RCD], RA
	INC RCD
	
	; count up next task number
	INC task_next_tid
	ICC task_next_tid+1
	
	; restore interrupt inhibt state
	PLF 
	
	; initialize file numbers
	PSH RB
	mov RA, #TASK_FILE_FREE
	mov RB, #TASK_MAX_FILES
.floop:
	mov [RCD], RA
	INC RCD
	DEC RB
	JNZ .floop
	
	; initialize tick counter, parent TID
	; and text area
	mov RA, #0
	mov RB, #TCB_OFF_CMD-TCB_OFF_TICKS+4
.tloop:
	mov [RCD], RA
	INC RCD
	DEC RB
	JNZ .tloop
	
	; initialize environment pointer
	mov RC, #TCB_OFF_ENV
	mov RA, #TCB_OFF_CMD
	mov [RCD], RA
	INC RCD
	mov [RCD], RD
	
	; restore task slot index
	PUL RB
	RTS
	
.error_end:	
	PLF	; restore interrupt inhibt state
	STC
	RTS

;
; get task control block pointer
;
; Parm:
;  RB - task index
; Return:
;  RCD - pointer to task control block
;	
TASK_GET_BLOCK_POINTER:
	CLC
	LSL RB
	mov RA, task_blocks,RB
	mov RC, RA
	mov RA, task_blocks+1,RB
	mov RD, RA
	LSR RB
	RTS

;
; get task stack pointer
;
; Parm:
;  RB - task index
; Return:
;  RCD - stack pointer of that task
;	
TASK_GET_STACK_POINTER:
	CLC
	LSL RB
	mov RA, task_stacks,RB
	mov RC, RA
	mov RA, task_stacks+1,RB
	mov RD, RA
	LSR RB
	RTS

;
; get task stack pointer
;
; Parm:
;  RB - task index
;  RCD - stack pointer of that task
;	
TASK_SET_STACK_POINTER:
	CLC
	LSL RB
	mov RA, RC
	mov task_stacks,RB, RA
	mov RA, RD
	mov task_stacks+1,RB, RA
	LSR RB
	RTS

;
; frees a task slot
;
; Parm:
;  RB - task index
;	
TASK_REMOVE:
	PSH RB ; save task index
	
	; convert to block index
	CLC 
	LSL RB
	
	; free task control block
	mov RA, task_blocks+1,RB
	mov RD, RA ; control block page
	mov RB, #SYS_OWNER
	JSR MEM_FREE_SINGLE
	
	PUL RB ; restore task index
	
	; free task slot
	mov RA, #TASK_ST_FREE
	mov task_status,RB, RA
	
TASK_OK_END:	
	CLC
	RTS

;
; Convert File descritor to stream number
;
; Parm:
; RA - file descriptor number 
;
TASK_FD_TO_SD:
	STC 
	CMP RA, #TASK_MAX_FILES
	JCS _TASK_FILE_ERROR_END
	PSH RB
	mov RB, #TCB_OFF_FILES
	ADD RB, RA
	mov RA, [task_active_block],RB
	PUL RB
	STC
	CMP RA, #TASK_FILE_FREE
	JNZ TASK_OK_END
_TASK_FILE_ERROR_END:
	STC
	mov RA, #EBADF
	RTS

;
; Removes a File descritor from current task
;
; Parm:
; RA - file descriptor number 
;
TASK_REMOVE_FD:
	STC 
	CMP RA, #TASK_MAX_FILES
	JCS _TASK_FILE_ERROR_END
	PSH RB
	mov RB, #TCB_OFF_FILES
	ADD RB, RA
	mov RA, #TASK_FILE_FREE
	mov [task_active_block],RB, RA
	PUL RB
	CLC
	RTS

;
; Converts a stream number to a File Descritor
;
; Parm:
;  RA - stream number 
; Result:
;  RA - file descriptor 
;
TASK_CREATE_FD:
	PSH RA ; save stream number
	mov RB, #0 ; file descriptor count
	mov RC, task_active_block
	mov RD, task_active_block+1
	ADD RC, #TCB_OFF_FILES
	
.loop:
	; check file slot
	mov RA, [RCD] 
	STC
	CMP RA, #TASK_FILE_FREE
	JZS .slot_found
	
	INC RCD ; next slot
	INC RB  ; next file descriptor
	STC
	CMP RB, #TASK_MAX_FILES
	JNC .loop
	
.error_end:
	; file table overflow
	PUL RA
	STC
	mov RA, #ENFILE
	RTS
	
.slot_found:	
	PUL RA ; restore stream number
	mov [RCD], RA
	mov RA, RB ; return file descriptor
	CLC
	RTS
	
TASK_FORCE_SWITCH:	
	; create IRQ entry stack frame
	PSF ; flags
	
	PSH RA ; registers
	PSH RB
	PSH RC
	PSH RD
	
	SII
	JMP IRQ_NEXT_TASK
	
;
; Find task for given task id
;
; parm:
;  RC/RD - task id
; return:
;  c-flag: 0 = found, 1 = not found
;  RB - task index
;	
TASK_FIND_TID:
	PSF
	SII
	mov task_aux_tid, RC
	mov task_aux_tid+1, RD
	mov RB, #TASK_MAX_NUM-1
.task_loop:	
	mov RA, task_status,RB
	STC
	CMP RA, #TASK_ST_FREE
	JZS .next_task
	
	JSR TASK_GET_BLOCK_POINTER	
	CLC
	ADD RC, #TCB_OFF_TID
	ADD RD, #0
	
	mov RA, [RCD]
	STC
	CMP RA, task_aux_tid
	JNZ .next_task
	
	INC RCD
	mov RA, [RCD]
	STC
	CMP RA, task_aux_tid+1
	JNZ .next_task
	
.found:	
	PLF
	CLC
	RTS
	
.next_task:
	DEC RB
	JCS .task_loop	
.not_found:
	PLF
	STC
	RTS

;
; Get environment pointer for current task
;
; parm:
;  -
; result:
;  RCD - pointer to task environment
;
TASK_GET_ENV_POINTER:	
	mov RB, #TCB_OFF_ENV
	mov RA, [task_active_block],RB
	mov RC, RA
	INC RB
	mov RA, [task_active_block],RB
	mov RD, RA
	RTS
	
;
; Get a list of all currently used task numbers.
;
; parm:
;  RCD - pointer to task list area
;     (minimum size = TASK_MAX_NUM*2)
;
; result:
;  c-flag: 0 = O.K., 1 = error
;  RA - number of used tasks
;
TASK_GET_LIST:
	PSF
	SII
	
	; save destination pointer
	mov task_aux_pointer, RC
	mov task_aux_pointer+1, RD
	
	; initialize counter
	mov RA, #0
	PSH RA
	
	; task index
	mov RB, #TASK_MAX_NUM
.loop:
	DEC RB
	JNC .end_loop
	
	; skip free task slots
	mov RA, task_status,RB
	XOR RA, #TASK_ST_FREE
	JZS .loop
	
	JSR TASK_GET_BLOCK_POINTER
	CLC
	ADD RC, #TCB_OFF_TID
	ICC RD
	
	; increment used tasks counter
	PUL RA
	INC RA
	PSH RA
	
	; copy task ID
	PSH RB
	mov RB, RA
	DEC RB
	CLC
	LSL RB ; RB = task counter * 2
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	PUL RB
	
	JMP .loop
	
.end_loop:
	; restore counter
	PUL RA
	PLF
	CLC
	RTS
	
;
; Get information about a task.
;
; parm:
;  RA/RB  - task ID, if c-flag = 0
;  c-flag - 1 = get info on current task
;  RCD    - pointer to task info area
;
; result:
;  c-flag: 0 = O.K., 1 = task not found
;
TASK_GET_INFO:
	PSF
	SII
	
	mov task_aux_pointer, RC
	mov task_aux_pointer+1, RD	
	
	JNC .find_task
	
	mov RC, task_active_block
	mov RD, task_active_block+1
	JMP .block_found
	
.find_task:	
	; get pointer to task control block
	mov RC, RA
	mov RD, RB
	JSR TASK_FIND_TID
	JCS .error_end
	JSR TASK_GET_BLOCK_POINTER
	
.block_found:		
	; transfer number of ticks
	CLC
	ADD RC, #TCB_OFF_TICKS
	ADD RD, #0
	mov RB, #TIB_OFF_TICKS	
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	
	; transfer parent ID
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	
	; transfer console ID
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	
	; transfer command name
	CLC
	ADD RC, #TCB_OFF_CMD-TCB_OFF_TICKS-5
	ADD RD, #0
	mov RB, #TIB_OFF_CMD
.loop:	
	mov RA, [RCD]
	mov [task_aux_pointer],RB, RA
	INC RCD
	INC RB
	OR  RA, #0
	JNZ .loop
.end:	
	PLF
	CLC
	RTS
	
.error_end:	
	PLF
	STC
	RTS

#bank data
; task index of active task
task_active: #res 1
; task block pointer of active task
task_active_block: #res 2
; area for task status
task_status: #res TASK_MAX_NUM
; area for task block pointers
task_blocks: #res 2*TASK_MAX_NUM
; area for task stack pointers
task_stacks: #res 2*TASK_MAX_NUM
; next available task ID
task_next_tid: #res 2
; auxiliary task id
task_aux_tid: #res 2
; auxiliary pointer
task_aux_pointer: #res 2
