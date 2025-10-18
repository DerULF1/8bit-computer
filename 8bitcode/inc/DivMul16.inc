;
; Divides two 16-bit values.
;
; parm:
; result:
; global:
;  divident  - 16-bit input
;  divisor   - 16-bit input
;  quotient  - 16-bit output
;  remainder - 16-bit output
; 
DIVIDE:
	mov RA, divident
	mov quotient, RA
	mov RA, divident+1
	mov quotient+1, RA
	mov RA, #0
	mov remainder, RA
	mov remainder+1, RA
	
	mov RA, #16
	mov arith_cycle_cnt, RA
	
	CLC 
	
.loop:	
	LSL quotient
	LSL quotient+1
	LSL remainder
	LSL remainder+1
	
	STC
	mov RA, remainder
	sub RA, divisor
	mov RB, remainder+1
	sub RB, divisor+1
	JNC .ignore
	
	mov remainder, RA
	mov remainder+1, RB
	
.ignore:
	PSF
	DEC arith_cycle_cnt
	JZS .endloop
	PLF
	JMP .loop
	
.endloop:
	PLF
	LSL quotient
	LSL quotient+1
	RTS
	
;
; Multiply two 16 bit values.
;
; parm:
; result:
;  c-flag: 0=O.K., 1=overflow
; global:
;  factor1 - 16-bit input
;  factor2 - 16-bit input
;  product - 16-bit output
;
MULTIPLY:
	mov RA, #0
	mov product, RA
	mov product+1, RA
	
	mov RA, factor2
	OR  RA, factor2+1
	CLC
	JZS .exit
	
.loop:	
	LSR factor2+1
	LSR factor2
	JNC .ignore
	
	CLC
	mov RA, product
	add RA, factor1
	mov product, RA
	mov RA, product+1
	add RA, factor1+1
	mov product+1, RA
	JCS .exit
	
	mov RA, factor2
	OR  RA, factor2+1
	CLC
	JZS .exit
	
.ignore:
	LSL factor1
	LSL factor1+1
	JNC .loop
	
.exit:
	RTS

#bank data

divident: #res 2
divisor: #res 2
quotient: #res 2
remainder: #res 2
arith_cycle_cnt: #res 1	

factor1: #res 2
factor2: #res 2
product: #res 2
