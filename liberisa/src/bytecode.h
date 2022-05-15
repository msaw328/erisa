// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#ifndef _ERISA_INSTRUCTIONS_H_
#define _ERISA_INSTRUCTIONS_H_

// Include generated isa.h header
#include "isa.h"

// ID equal to 0 means an invalid instruction
#define INS_ID_INVALID 0x0

// Defines for each instruction
// INS_ID_<ins>         - ID of the instruction (it is not encoded in machine code, just internal identification of instructions)
// INS_OP_<ins>         - Opcode
// INS_OP_MASK_<ins>    - Mask for opcode matching (it might be not very flexible, mask based matching might change in the future)
// INS_LEN_<ins>        - Length of the instruction in bytes

// Macro which easily matches opcode byte based on mask
#define INS_MATCH(input, instr) ((input & INS_OP_MASK_##instr) == INS_OP_##instr)

#endif
