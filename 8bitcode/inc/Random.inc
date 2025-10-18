;
; Pseudo Random Generator
;

#bank pgm

;
; Defines a custom character
;
; Parms:
;  RA/RB/RC/RD - 32-bit Random Seed Number
;
RANDOM_SET_SEED:
	mov random_seed, RA
	mov random_seed+1, RB
	mov random_seed+2, RC
	mov random_seed+3, RD
	RTS

;
; Returns a random 32-bit number
;
; returns:
; RA/RB/RC/RD - random number
;
RANDOM_NUMBER:
	; x ^= x << 13
	mov RB, random_seed
	mov RC, random_seed+1
	mov RD, random_seed+2
	LSL RB
	LSL RC
	LSL RD
	LSL RB
	LSL RC
	LSL RD
	LSL RB
	LSL RC
	LSL RD
	LSL RB
	LSL RC
	LSL RD
	LSL RB
	LSL RC
	LSL RD
	AND RB, #0xE0
	XOR RB, random_seed+1
	XOR RC, random_seed+2
	XOR RD, random_seed+3
	mov random_seed+1, RB
	mov random_seed+2, RC
	mov random_seed+3, RD
	
	; x ^= x >> 17
	CLC
	LSR RD
	LSR RC
	XOR RC, random_seed
	XOR RD, random_seed+1
	mov random_seed, RC
	mov random_seed+1, RD
	
	; x ^= x << 5
	mov RA, RC
	mov RB, RD
	mov RC, random_seed+2
	mov RD, random_seed+3
	LSL RA
	LSL RB
	LSL RC
	LSL RD
	LSL RA
	LSL RB
	LSL RC
	LSL RD
	LSL RA
	LSL RB
	LSL RC
	LSL RD
	LSL RA
	LSL RB
	LSL RC
	LSL RD
	LSL RA
	LSL RB
	LSL RC
	LSL RD
	AND RA, #0xE0
	XOR RA, random_seed
	XOR RB, random_seed+1
	XOR RC, random_seed+2
	XOR RD, random_seed+3
	mov random_seed, RA
	mov random_seed+1, RB
	mov random_seed+2, RC
	mov random_seed+3, RD
	RTS

#bank data
random_seed: #res 4	