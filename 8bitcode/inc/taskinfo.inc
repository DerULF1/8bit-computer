;
; structure of the task info block
;

#bankdef task_info_block
{
    #addr 0x0000
}

#bank task_info_block

; total time ticks for this task
TIB_OFF_TICKS: #res 3 
; parent Task ID
TIB_OFF_PARENT: #res 2 
; console ID
TIB_OFF_CONS: #res 1
; begin of the command line of this task
TIB_OFF_CMD: 
