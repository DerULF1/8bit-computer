;
; List of all opcodes
;
opcode_offs:
    #d (opcode_adrtyps_ADD - opcode_adrtyps_lists)[7:0], 0x18
    #d (opcode_adrtyps_AND - opcode_adrtyps_lists)[7:0], 0x18
    #d (opcode_adrtyps_CII - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_CLC - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_CMP - opcode_adrtyps_lists)[7:0], 0x18
    #d (opcode_adrtyps_DCC - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_DEC - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_HLT - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_ICC - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_INB - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_INC - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_INP - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_INW - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_JCS - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JMP - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JNC - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JNN - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JNS - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JNV - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JNZ - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JSR - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JVS - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_JZS - opcode_adrtyps_lists)[7:0], 0x02
    #d (opcode_adrtyps_LSL - opcode_adrtyps_lists)[7:0], 0x06
    #d (opcode_adrtyps_LSR - opcode_adrtyps_lists)[7:0], 0x06
    #d (opcode_adrtyps_MOV - opcode_adrtyps_lists)[7:0], 0x26
    #d (opcode_adrtyps_MVA - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_NOP - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_OR - opcode_adrtyps_lists)[7:0], 0x18
    #d (opcode_adrtyps_OUB - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_OUT - opcode_adrtyps_lists)[7:0], 0x08
    #d (opcode_adrtyps_OUW - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_PLB - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_PLF - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_PSB - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_PSF - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_PSH - opcode_adrtyps_lists)[7:0], 0x04
    #d (opcode_adrtyps_PUL - opcode_adrtyps_lists)[7:0], 0x04
    #d (opcode_adrtyps_RTI - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_RTS - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_SII - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_STC - opcode_adrtyps_lists)[7:0], 0x01
    #d (opcode_adrtyps_STR - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_STW - opcode_adrtyps_lists)[7:0], 0x00
    #d (opcode_adrtyps_SUB - opcode_adrtyps_lists)[7:0], 0x18
    #d (opcode_adrtyps_XOR - opcode_adrtyps_lists)[7:0], 0x18

opcode_adrtyps_lists:
opcode_adrtyps_ADD:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_adrtyps_AND:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_adrtyps_CII:
    #d (0x00)[7:0]

opcode_adrtyps_CLC:
    #d (0x00)[7:0]

opcode_adrtyps_CMP:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_adrtyps_DCC:

opcode_adrtyps_DEC:

opcode_adrtyps_HLT:
    #d (0x00)[7:0]

opcode_adrtyps_ICC:

opcode_adrtyps_INB:

opcode_adrtyps_INC:

opcode_adrtyps_INP:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]

opcode_adrtyps_INW:

opcode_adrtyps_JCS:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JMP:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JNC:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JNN:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JNS:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JNV:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JNZ:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JSR:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JVS:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_JZS:
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_LSL:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_LSR:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]

opcode_adrtyps_MOV:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS_IDX)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS_INDIR_IDX)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RCD*0x10 + ADR_SP)[7:0]
    #d (ADR_SP*0x10 + ADR_RCD)[7:0]
    #d (ADR_ABS*0x10 + ADR_RA)[7:0]
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]
    #d (ADR_ABS*0x10 + ADR_RC)[7:0]
    #d (ADR_ABS*0x10 + ADR_RD)[7:0]
    #d (ADR_ABS_IDX*0x10 + ADR_RA)[7:0]
    #d (ADR_RCD_INDIR*0x10 + ADR_RA)[7:0]
    #d (ADR_RCD_INDIR*0x10 + ADR_RB)[7:0]
    #d (ADR_RCD_INDIR*0x10 + ADR_RC)[7:0]
    #d (ADR_RCD_INDIR*0x10 + ADR_RD)[7:0]
    #d (ADR_ABS_INDIR_IDX*0x10 + ADR_RA)[7:0]

opcode_adrtyps_MVA:

opcode_adrtyps_NOP:
    #d (0x00)[7:0]

opcode_adrtyps_OR:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_adrtyps_OUB:

opcode_adrtyps_OUT:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_IMM*0x10 + ADR_RA)[7:0]

opcode_adrtyps_OUW:

opcode_adrtyps_PLB:

opcode_adrtyps_PLF:
    #d (0x00)[7:0]

opcode_adrtyps_PSB:

opcode_adrtyps_PSF:
    #d (0x00)[7:0]

opcode_adrtyps_PSH:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]

opcode_adrtyps_PUL:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]

opcode_adrtyps_RTI:
    #d (0x00)[7:0]

opcode_adrtyps_RTS:
    #d (0x00)[7:0]

opcode_adrtyps_SII:
    #d (0x00)[7:0]

opcode_adrtyps_STC:
    #d (0x00)[7:0]

opcode_adrtyps_STR:

opcode_adrtyps_STW:

opcode_adrtyps_SUB:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_adrtyps_XOR:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_RC)[7:0]
    #d (ADR_RA*0x10 + ADR_RD)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_ABS)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RC*0x10 + ADR_RA)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_RD)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_ABS)[7:0]
    #d (ADR_RC*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RD*0x10 + ADR_RA)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_RC)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RD*0x10 + ADR_RCD_INDIR)[7:0]

opcode_opcodes_lists:
opcode_opcodes_ADD:
    #d 0x09
    #d 0x10
    #d 0x17
    #d 0x5d
    #d 0x7a
    #d 0x9a
    #d 0x1e
    #d 0x25
    #d 0x2c
    #d 0x64
    #d 0x82
    #d 0xa2
    #d 0x33
    #d 0x3a
    #d 0x41
    #d 0x6b
    #d 0x8a
    #d 0xaa
    #d 0x48
    #d 0x4f
    #d 0x56
    #d 0x72
    #d 0x92
    #d 0xb2

opcode_opcodes_AND:
    #d 0x0b
    #d 0x12
    #d 0x19
    #d 0x5f
    #d 0x7c
    #d 0x9c
    #d 0x20
    #d 0x27
    #d 0x2e
    #d 0x66
    #d 0x84
    #d 0xa4
    #d 0x35
    #d 0x3c
    #d 0x43
    #d 0x6d
    #d 0x8c
    #d 0xac
    #d 0x4a
    #d 0x51
    #d 0x58
    #d 0x74
    #d 0x94
    #d 0xb4

opcode_opcodes_CII:
    #d 0x05

opcode_opcodes_CLC:
    #d 0x02

opcode_opcodes_CMP:
    #d 0x0e
    #d 0x15
    #d 0x1c
    #d 0x62
    #d 0x7f
    #d 0x9f
    #d 0x23
    #d 0x2a
    #d 0x31
    #d 0x69
    #d 0x87
    #d 0xa7
    #d 0x38
    #d 0x3f
    #d 0x46
    #d 0x70
    #d 0x8f
    #d 0xaf
    #d 0x4d
    #d 0x54
    #d 0x5b
    #d 0x77
    #d 0x97
    #d 0xb7

opcode_opcodes_DCC:

opcode_opcodes_DEC:

opcode_opcodes_HLT:
    #d 0x01

opcode_opcodes_ICC:

opcode_opcodes_INB:

opcode_opcodes_INC:

opcode_opcodes_INP:
    #d 0xed
    #d 0xeb

opcode_opcodes_INW:

opcode_opcodes_JCS:
    #d 0xd8
    #d 0xe2

opcode_opcodes_JMP:
    #d 0xd6
    #d 0xe0

opcode_opcodes_JNC:
    #d 0xdc
    #d 0xe6

opcode_opcodes_JNN:
    #d 0xde
    #d 0xe8

opcode_opcodes_JNS:
    #d 0xda
    #d 0xe4

opcode_opcodes_JNV:
    #d 0xdf
    #d 0xe9

opcode_opcodes_JNZ:
    #d 0xdd
    #d 0xe7

opcode_opcodes_JSR:
    #d 0xd7
    #d 0xe1

opcode_opcodes_JVS:
    #d 0xdb
    #d 0xe5

opcode_opcodes_JZS:
    #d 0xd9
    #d 0xe3

opcode_opcodes_LSL:
    #d 0xbe
    #d 0xbf
    #d 0xc0
    #d 0xc1
    #d 0xc2
    #d 0xc3

opcode_opcodes_LSR:
    #d 0xc4
    #d 0xc5
    #d 0xc6
    #d 0xc7
    #d 0xc8
    #d 0xc9

opcode_opcodes_MOV:
    #d 0x08
    #d 0x0f
    #d 0x16
    #d 0x5c
    #d 0x78
    #d 0xee
    #d 0x98
    #d 0xef
    #d 0x1d
    #d 0x24
    #d 0x2b
    #d 0x63
    #d 0x80
    #d 0xa0
    #d 0x32
    #d 0x39
    #d 0x40
    #d 0x6a
    #d 0x88
    #d 0xa8
    #d 0x47
    #d 0x4e
    #d 0x55
    #d 0x71
    #d 0x90
    #d 0xb0
    #d 0xd5
    #d 0xd4
    #d 0x79
    #d 0x81
    #d 0x89
    #d 0x91
    #d 0xf0
    #d 0x99
    #d 0xa1
    #d 0xa9
    #d 0xb1
    #d 0xf1

opcode_opcodes_MVA:

opcode_opcodes_NOP:
    #d 0x00

opcode_opcodes_OR:
    #d 0x0c
    #d 0x13
    #d 0x1a
    #d 0x60
    #d 0x7d
    #d 0x9d
    #d 0x21
    #d 0x28
    #d 0x2f
    #d 0x67
    #d 0x85
    #d 0xa5
    #d 0x36
    #d 0x3d
    #d 0x44
    #d 0x6e
    #d 0x8d
    #d 0xad
    #d 0x4b
    #d 0x52
    #d 0x59
    #d 0x75
    #d 0x95
    #d 0xb5

opcode_opcodes_OUB:

opcode_opcodes_OUT:
    #d 0xb8
    #d 0xb9
    #d 0xba
    #d 0xbb
    #d 0xbc
    #d 0xbd
    #d 0xec
    #d 0xea

opcode_opcodes_OUW:

opcode_opcodes_PLB:

opcode_opcodes_PLF:
    #d 0xd3

opcode_opcodes_PSB:

opcode_opcodes_PSF:
    #d 0xd2

opcode_opcodes_PSH:
    #d 0xca
    #d 0xcc
    #d 0xce
    #d 0xd0

opcode_opcodes_PUL:
    #d 0xcb
    #d 0xcd
    #d 0xcf
    #d 0xd1

opcode_opcodes_RTI:
    #d 0x07

opcode_opcodes_RTS:
    #d 0x06

opcode_opcodes_SII:
    #d 0x04

opcode_opcodes_STC:
    #d 0x03

opcode_opcodes_STR:

opcode_opcodes_STW:

opcode_opcodes_SUB:
    #d 0x0a
    #d 0x11
    #d 0x18
    #d 0x5e
    #d 0x7b
    #d 0x9b
    #d 0x1f
    #d 0x26
    #d 0x2d
    #d 0x65
    #d 0x83
    #d 0xa3
    #d 0x34
    #d 0x3b
    #d 0x42
    #d 0x6c
    #d 0x8b
    #d 0xab
    #d 0x49
    #d 0x50
    #d 0x57
    #d 0x73
    #d 0x93
    #d 0xb3

opcode_opcodes_XOR:
    #d 0x0d
    #d 0x14
    #d 0x1b
    #d 0x61
    #d 0x7e
    #d 0x9e
    #d 0x22
    #d 0x29
    #d 0x30
    #d 0x68
    #d 0x86
    #d 0xa6
    #d 0x37
    #d 0x3e
    #d 0x45
    #d 0x6f
    #d 0x8e
    #d 0xae
    #d 0x4c
    #d 0x53
    #d 0x5a
    #d 0x76
    #d 0x96
    #d 0xb6

eopcode_offs:
    #d (eopcode_adrtyps_ADD - eopcode_adrtyps_lists)[7:0], 0x02
    #d (eopcode_adrtyps_AND - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_CII - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_CLC - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_CMP - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_DCC - eopcode_adrtyps_lists)[7:0], 0x08
    #d (eopcode_adrtyps_DEC - eopcode_adrtyps_lists)[7:0], 0x09
    #d (eopcode_adrtyps_HLT - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_ICC - eopcode_adrtyps_lists)[7:0], 0x08
    #d (eopcode_adrtyps_INB - eopcode_adrtyps_lists)[7:0], 0x02
    #d (eopcode_adrtyps_INC - eopcode_adrtyps_lists)[7:0], 0x09
    #d (eopcode_adrtyps_INP - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_INW - eopcode_adrtyps_lists)[7:0], 0x04
    #d (eopcode_adrtyps_JCS - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JMP - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JNC - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JNN - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JNS - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JNV - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JNZ - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JSR - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JVS - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_JZS - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_LSL - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_LSR - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_MOV - eopcode_adrtyps_lists)[7:0], 0x07
    #d (eopcode_adrtyps_MVA - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_NOP - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_OR - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_OUB - eopcode_adrtyps_lists)[7:0], 0x02
    #d (eopcode_adrtyps_OUT - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_OUW - eopcode_adrtyps_lists)[7:0], 0x04
    #d (eopcode_adrtyps_PLB - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_PLF - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_PSB - eopcode_adrtyps_lists)[7:0], 0x01
    #d (eopcode_adrtyps_PSF - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_PSH - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_PUL - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_RTI - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_RTS - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_SII - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_STC - eopcode_adrtyps_lists)[7:0], 0x00
    #d (eopcode_adrtyps_STR - eopcode_adrtyps_lists)[7:0], 0x0a
    #d (eopcode_adrtyps_STW - eopcode_adrtyps_lists)[7:0], 0x0a
    #d (eopcode_adrtyps_SUB - eopcode_adrtyps_lists)[7:0], 0x02
    #d (eopcode_adrtyps_XOR - eopcode_adrtyps_lists)[7:0], 0x00

eopcode_adrtyps_lists:
eopcode_adrtyps_ADD:
    #d (ADR_SP*0x10 + ADR_RB)[7:0]
    #d (ADR_SP*0x10 + ADR_IMM)[7:0]

eopcode_adrtyps_AND:

eopcode_adrtyps_CII:

eopcode_adrtyps_CLC:

eopcode_adrtyps_CMP:

eopcode_adrtyps_DCC:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]
    #d (ADR_ABS_INDIR*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_DEC:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_RCD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]
    #d (ADR_ABS_INDIR*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_HLT:

eopcode_adrtyps_ICC:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]
    #d (ADR_ABS_INDIR*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_INB:
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RCD_INDIR_IDX*0x10 + ADR_IMM)[7:0]

eopcode_adrtyps_INC:
    #d (ADR_RA)[7:0]
    #d (ADR_RB)[7:0]
    #d (ADR_RC)[7:0]
    #d (ADR_RD)[7:0]
    #d (ADR_RCD)[7:0]
    #d (ADR_ABS)[7:0]
    #d (ADR_RCD_INDIR)[7:0]
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]
    #d (ADR_ABS_INDIR*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_INP:

eopcode_adrtyps_INW:
    #d (ADR_RA*0x10 + ADR_ABS)[7:0]
    #d (ADR_RA*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS_INDIR)[7:0]
    #d (ADR_RA*0x10 + ADR_ABS_INDIR_IDX)[7:0]

eopcode_adrtyps_JCS:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JMP:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JNC:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JNN:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JNS:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JNV:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JNZ:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JSR:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JVS:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_JZS:
    #d (ADR_ABS_INDIR)[7:0]

eopcode_adrtyps_LSL:

eopcode_adrtyps_LSR:

eopcode_adrtyps_MOV:
    #d (ADR_RCD*0x10 + ADR_ABS)[7:0]
    #d (ADR_RCD*0x10 + ADR_ABS_IDX)[7:0]
    #d (ADR_RCD*0x10 + ADR_RCD_INDIR)[7:0]
    #d (ADR_RCD*0x10 + ADR_ABS_INDIR_IDX)[7:0]
    #d (ADR_ABS*0x10 + ADR_RCD)[7:0]
    #d (ADR_ABS_IDX*0x10 + ADR_RCD)[7:0]
    #d (ADR_ABS_INDIR_IDX*0x10 + ADR_RCD)[7:0]

eopcode_adrtyps_MVA:
    #d (ADR_RCD*0x10 + ADR_ABS)[7:0]

eopcode_adrtyps_NOP:

eopcode_adrtyps_OR:

eopcode_adrtyps_OUB:
    #d (ADR_IMM*0x10 + ADR_RA)[7:0]
    #d (ADR_IMM*0x10 + ADR_RCD_INDIR_IDX)[7:0]

eopcode_adrtyps_OUT:

eopcode_adrtyps_OUW:
    #d (ADR_ABS*0x10 + ADR_RA)[7:0]
    #d (ADR_RCD_INDIR*0x10 + ADR_RA)[7:0]
    #d (ADR_ABS_INDIR*0x10 + ADR_RA)[7:0]
    #d (ADR_ABS_INDIR_IDX*0x10 + ADR_RA)[7:0]

eopcode_adrtyps_PLB:
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_PLF:

eopcode_adrtyps_PSB:
    #d (ADR_ABS*0x10 + ADR_RB)[7:0]

eopcode_adrtyps_PSF:

eopcode_adrtyps_PSH:

eopcode_adrtyps_PUL:

eopcode_adrtyps_RTI:

eopcode_adrtyps_RTS:

eopcode_adrtyps_SII:

eopcode_adrtyps_STC:

eopcode_adrtyps_STR:
    #d (ADR_RA*0x10 + ADR_RB)[7:0]
    #d (ADR_RA*0x10 + ADR_IMM)[7:0]
    #d (ADR_RB*0x10 + ADR_RB)[7:0]
    #d (ADR_RB*0x10 + ADR_IMM)[7:0]
    #d (ADR_RC*0x10 + ADR_RB)[7:0]
    #d (ADR_RC*0x10 + ADR_IMM)[7:0]
    #d (ADR_RD*0x10 + ADR_RB)[7:0]
    #d (ADR_RD*0x10 + ADR_IMM)[7:0]
    #d (ADR_RCD*0x10 + ADR_RB)[7:0]
    #d (ADR_RCD*0x10 + ADR_IMM)[7:0]

eopcode_adrtyps_STW:
    #d (ADR_RB*0x10 + ADR_RA)[7:0]
    #d (ADR_RB*0x10 + ADR_RB)[7:0]
    #d (ADR_RB*0x10 + ADR_RC)[7:0]
    #d (ADR_RB*0x10 + ADR_RD)[7:0]
    #d (ADR_RB*0x10 + ADR_RCD)[7:0]
    #d (ADR_IMM*0x10 + ADR_RA)[7:0]
    #d (ADR_IMM*0x10 + ADR_RB)[7:0]
    #d (ADR_IMM*0x10 + ADR_RC)[7:0]
    #d (ADR_IMM*0x10 + ADR_RD)[7:0]
    #d (ADR_IMM*0x10 + ADR_RCD)[7:0]

eopcode_adrtyps_SUB:
    #d (ADR_SP*0x10 + ADR_RB)[7:0]
    #d (ADR_SP*0x10 + ADR_IMM)[7:0]

eopcode_adrtyps_XOR:

eopcode_opcodes_lists:
eopcode_opcodes_ADD:
    #d 0x48
    #d 0x47

eopcode_opcodes_AND:

eopcode_opcodes_CII:

eopcode_opcodes_CLC:

eopcode_opcodes_CMP:

eopcode_opcodes_DCC:
    #d 0x11
    #d 0x15
    #d 0x19
    #d 0x1d
    #d 0x23
    #d 0x30
    #d 0x27
    #d 0x2b

eopcode_opcodes_DEC:
    #d 0x10
    #d 0x14
    #d 0x18
    #d 0x1c
    #d 0x1f
    #d 0x22
    #d 0x2f
    #d 0x26
    #d 0x2a

eopcode_opcodes_HLT:

eopcode_opcodes_ICC:
    #d 0x0f
    #d 0x13
    #d 0x17
    #d 0x1b
    #d 0x21
    #d 0x2e
    #d 0x25
    #d 0x29

eopcode_opcodes_INB:
    #d 0x00
    #d 0x01

eopcode_opcodes_INC:
    #d 0x0e
    #d 0x12
    #d 0x16
    #d 0x1a
    #d 0x1e
    #d 0x20
    #d 0x2d
    #d 0x24
    #d 0x28

eopcode_opcodes_INP:

eopcode_opcodes_INW:
    #d 0x4f
    #d 0x50
    #d 0x51
    #d 0x52

eopcode_opcodes_JCS:
    #d 0x06

eopcode_opcodes_JMP:
    #d 0x04

eopcode_opcodes_JNC:
    #d 0x0a

eopcode_opcodes_JNN:
    #d 0x0c

eopcode_opcodes_JNS:
    #d 0x08

eopcode_opcodes_JNV:
    #d 0x0d

eopcode_opcodes_JNZ:
    #d 0x0b

eopcode_opcodes_JSR:
    #d 0x05

eopcode_opcodes_JVS:
    #d 0x09

eopcode_opcodes_JZS:
    #d 0x07

eopcode_opcodes_LSL:

eopcode_opcodes_LSR:

eopcode_opcodes_MOV:
    #d 0x54
    #d 0x56
    #d 0x59
    #d 0x58
    #d 0x53
    #d 0x55
    #d 0x57

eopcode_opcodes_MVA:
    #d 0x2c

eopcode_opcodes_NOP:

eopcode_opcodes_OR:

eopcode_opcodes_OUB:
    #d 0x02
    #d 0x03

eopcode_opcodes_OUT:

eopcode_opcodes_OUW:
    #d 0x4b
    #d 0x4c
    #d 0x4d
    #d 0x4e

eopcode_opcodes_PLB:
    #d 0x32

eopcode_opcodes_PLF:

eopcode_opcodes_PSB:
    #d 0x31

eopcode_opcodes_PSF:

eopcode_opcodes_PSH:

eopcode_opcodes_PUL:

eopcode_opcodes_RTI:

eopcode_opcodes_RTS:

eopcode_opcodes_SII:

eopcode_opcodes_STC:

eopcode_opcodes_STR:
    #d 0x34
    #d 0x33
    #d 0x38
    #d 0x37
    #d 0x3c
    #d 0x3b
    #d 0x40
    #d 0x3f
    #d 0x44
    #d 0x43

eopcode_opcodes_STW:
    #d 0x36
    #d 0x3a
    #d 0x3e
    #d 0x42
    #d 0x46
    #d 0x35
    #d 0x39
    #d 0x3d
    #d 0x41
    #d 0x45

eopcode_opcodes_SUB:
    #d 0x4a
    #d 0x49

eopcode_opcodes_XOR:
