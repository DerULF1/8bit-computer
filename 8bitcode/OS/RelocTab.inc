;
; Relocator Table
;
OPCODE_TABLE:
  #d 0b00000000 ; STC; CLC; HLT; NOP
  #d 0b00000000 ; RTI; RTS; CII; SII
  #d 0b00000000 ; AND RA, RB; SUB RA, RB; ADD RA, RB; MOV RA, RB
  #d 0b00000000 ; MOV RA, RC; CMP RA, RB; XOR RA, RB; OR  RA, RB
  #d 0b00000000 ; OR  RA, RC; AND RA, RC; SUB RA, RC; ADD RA, RC
  #d 0b00000000 ; ADD RA, RD; MOV RA, RD; CMP RA, RC; XOR RA, RC
  #d 0b00000000 ; XOR RA, RD; OR  RA, RD; AND RA, RD; SUB RA, RD
  #d 0b00000000 ; SUB RB, RA; ADD RB, RA; MOV RB, RA; CMP RA, RD
  #d 0b00000000 ; CMP RB, RA; XOR RB, RA; OR  RB, RA; AND RB, RA
  #d 0b00000000 ; AND RB, RC; SUB RB, RC; ADD RB, RC; MOV RB, RC
  #d 0b00000000 ; MOV RB, RD; CMP RB, RC; XOR RB, RC; OR  RB, RC
  #d 0b00000000 ; OR  RB, RD; AND RB, RD; SUB RB, RD; ADD RB, RD
  #d 0b00000000 ; ADD RC, RA; MOV RC, RA; CMP RB, RD; XOR RB, RD
  #d 0b00000000 ; XOR RC, RA; OR  RC, RA; AND RC, RA; SUB RC, RA
  #d 0b00000000 ; SUB RC, RB; ADD RC, RB; MOV RC, RB; CMP RC, RA
  #d 0b00000000 ; CMP RC, RB; XOR RC, RB; OR  RC, RB; AND RC, RB
  #d 0b00000000 ; AND RC, RD; SUB RC, RD; ADD RC, RD; MOV RC, RD
  #d 0b00000000 ; MOV RD, RA; CMP RC, RD; XOR RC, RD; OR  RC, RD
  #d 0b00000000 ; OR  RD, RA; AND RD, RA; SUB RD, RA; ADD RD, RA
  #d 0b00000000 ; ADD RD, RB; MOV RD, RB; CMP RD, RA; XOR RD, RA
  #d 0b00000000 ; XOR RD, RB; OR  RD, RB; AND RD, RB; SUB RD, RB
  #d 0b00000000 ; SUB RD, RC; ADD RD, RC; MOV RD, RC; CMP RD, RB
  #d 0b00000000 ; CMP RD, RC; XOR RD, RC; OR  RD, RC; AND RD, RC
  #d 0b01010101 ; AND RA, #{v}; SUB RA, #{v}; ADD RA, #{v}; MOV RA, #{v}
  #d 0b01010101 ; MOV RB, #{v}; CMP RA, #{v}; XOR RA, #{v}; OR  RA, #{v}
  #d 0b01010101 ; OR  RB, #{v}; AND RB, #{v}; SUB RB, #{v}; ADD RB, #{v}
  #d 0b01010101 ; ADD RC, #{v}; MOV RC, #{v}; CMP RB, #{v}; XOR RB, #{v}
  #d 0b01010101 ; XOR RC, #{v}; OR  RC, #{v}; AND RC, #{v}; SUB RC, #{v}
  #d 0b01010101 ; SUB RD, #{v}; ADD RD, #{v}; MOV RD, #{v}; CMP RC, #{v}
  #d 0b01010101 ; CMP RD, #{v}; XOR RD, #{v}; OR  RD, #{v}; AND RD, #{v}
  #d 0b10101010 ; SUB RA, {ad}; ADD RA, {ad}; MOV {ad}, RA; MOV RA, {ad}
  #d 0b10101010 ; CMP RA, {ad}; XOR RA, {ad}; OR  RA, {ad}; AND RA, {ad}
  #d 0b10101010 ; SUB RB, {ad}; ADD RB, {ad}; MOV {ad}, RB; MOV RB, {ad}
  #d 0b10101010 ; CMP RB, {ad}; XOR RB, {ad}; OR  RB, {ad}; AND RB, {ad}
  #d 0b10101010 ; SUB RC, {ad}; ADD RC, {ad}; MOV {ad}, RC; MOV RC, {ad}
  #d 0b10101010 ; CMP RC, {ad}; XOR RC, {ad}; OR  RC, {ad}; AND RC, {ad}
  #d 0b10101010 ; SUB RD, {ad}; ADD RD, {ad}; MOV {ad}, RD; MOV RD, {ad}
  #d 0b10101010 ; CMP RD, {ad}; XOR RD, {ad}; OR  RD, {ad}; AND RD, {ad}
  #d 0b00000000 ; SUB RA, [RCD]; ADD RA, [RCD]; MOV [RCD], RA; MOV RA, [RCD]
  #d 0b00000000 ; CMP RA, [RCD]; XOR RA, [RCD]; OR  RA, [RCD]; AND RA, [RCD]
  #d 0b00000000 ; SUB RB, [RCD]; ADD RB, [RCD]; MOV [RCD], RB; MOV RB, [RCD]
  #d 0b00000000 ; CMP RB, [RCD]; XOR RB, [RCD]; OR  RB, [RCD]; AND RB, [RCD]
  #d 0b00000000 ; SUB RC, [RCD]; ADD RC, [RCD]; MOV [RCD], RC; MOV RC, [RCD]
  #d 0b00000000 ; CMP RC, [RCD]; XOR RC, [RCD]; OR  RC, [RCD]; AND RC, [RCD]
  #d 0b00000000 ; SUB RD, [RCD]; ADD RD, [RCD]; MOV [RCD], RD; MOV RD, [RCD]
  #d 0b00000000 ; CMP RD, [RCD]; XOR RD, [RCD]; OR  RD, [RCD]; AND RD, [RCD]
  #d 0b00000000 ; OUT RD; OUT RC; OUT RB; OUT RA
  #d 0b00000010 ; LSL RB; LSL RA; OUT [RCD]; OUT {ad}
  #d 0b00100000 ; LSL [RCD]; LSL {ad}; LSL RD; LSL RC
  #d 0b00000000 ; LSR RD; LSR RC; LSR RB; LSR RA
  #d 0b00000010 ; PUL RA; PSH RA; LSR [RCD]; LSR {ad}
  #d 0b00000000 ; PUL RC; PSH RC; PUL RB; PSH RB
  #d 0b00000000 ; PLF; PSF; PUL RD; PSH RD
  #d 0b10100000 ; JSR {ad}; JMP {ad}; MOV RCD, SP; MOV SP, RCD
  #d 0b10101010 ; JVS {ad}; JNS {ad}; JZS {ad}; JCS {ad}
  #d 0b10101010 ; JNV {ad}; JNN {ad}; JNZ {ad}; JNC {ad}
  #d 0b00000000 ; JZS [RCD]; JCS [RCD]; JSR [RCD]; JMP [RCD]
  #d 0b00000000 ; JNZ [RCD]; JNC [RCD]; JVS [RCD]; JNS [RCD]
  #d 0b01010000 ; INP RA, #{v}; OUT #{v}, RA; JNV [RCD]; JNN [RCD]
  #d 0b10100000 ; MOV RA, [{ad}], RB; MOV RA, {ad}, RB; INP RA, RB; OUT RB, RA
  #d 0b00001010 ; *; *; MOV [{ad}], RB, RA; MOV {ad}, RB, RA
;
EOPCODE_TABLE:
  #d 0b01010101 ; OUB #{v}, [RCD], RB; OUB #{v}, RA; INB [RCD], RB, #{v}; INB RA, #{v}
  #d 0b10101010 ; JZS [{ad}]; JCS [{ad}]; JSR [{ad}]; JMP [{ad}]
  #d 0b10101010 ; JNZ [{ad}]; JNC [{ad}]; JVS [{ad}]; JNS [{ad}]
  #d 0b00001010 ; ICC RA; INC RA; JNV [{ad}]; JNN [{ad}]
  #d 0b00000000 ; ICC RB; INC RB; DCC RA; DEC RA
  #d 0b00000000 ; ICC RC; INC RC; DCC RB; DEC RB
  #d 0b00000000 ; ICC RD; INC RD; DCC RC; DEC RC
  #d 0b00000000 ; DEC RCD; INC RCD; DCC RD; DEC RD
  #d 0b10101010 ; DCC {ad}; DEC {ad}; ICC {ad}; INC {ad}
  #d 0b10101010 ; DCC {ad}, RB; DEC {ad}, RB; ICC {ad}, RB; INC {ad}, RB
  #d 0b10101010 ; DCC [{ad}], RB; DEC [{ad}], RB; ICC [{ad}], RB; INC [{ad}], RB
  #d 0b00000010 ; DEC [RCD]; ICC [RCD]; INC [RCD]; MVA RCD, {ad}
  #d 0b01101000 ; STR RA, #{v}; PLB {ad}, RB; PSB {ad}, RB; DCC [RCD]
  #d 0b01000100 ; STR RB, #{v}; STW RB, RA; STW #{v}, RA; STR RA, RB
  #d 0b01000100 ; STR RC, #{v}; STW RB, RB; STW #{v}, RB; STR RB, RB
  #d 0b01000100 ; STR RD, #{v}; STW RB, RC; STW #{v}, RC; STR RC, RB
  #d 0b01000100 ; STR RCD, #{v}; STW RB, RD; STW #{v}, RD; STR RD, RB
  #d 0b01000100 ; ADD SP, #{v}; STW RB, RCD; STW #{v}, RCD; STR RCD, RB
  #d 0b10000100 ; OUW {ad}, RA; SUB SP, RB; SUB SP, #{v}; ADD SP, RB
  #d 0b10101000 ; INW RA, {ad}; OUW [{ad}], RB, RA; OUW [{ad}], RA; OUW [RCD], RA
  #d 0b10101000 ; MOV {ad}, RCD; INW RA, [{ad}], RB; INW RA, [{ad}]; INW RA, [RCD]
  #d 0b10101010 ; MOV [{ad}], RB, RCD; MOV RCD, {ad}, RB; MOV {ad}, RB, RCD; MOV RCD, {ad}
  #d 0b00000010 ; *; *; MOV RCD, [RCD]; MOV RCD, [{ad}], RB
