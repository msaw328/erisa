// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdint.h>
#include <stddef.h>

#include <erisa/erisa.h>

#include "bytecode.h"

size_t erisa_decode(uint8_t* buff, erisa_ins_t* result) {
    uint8_t op = buff[0];

    if(INS_MATCH(op, STI)) {
        result->id = INS_ID_STI; // Its a Store Immediate
        result->operands[0] = *((uint32_t*) (buff + 1)); // src -> imm32
        result->operands[1] = op & ~INS_OP_MASK_STI; // dst -> reg_id
        return INS_LEN_STI;

    } else if(INS_MATCH(op, NOP)) {
        result->id = INS_ID_NOP; // Its a No Operation
        return INS_LEN_NOP;

    } else if(INS_MATCH(op, JMPABS)) {
        result->id = INS_ID_JMPABS; // Its a Jump Absolute
        result->operands[0] = *((uint32_t*) (buff + 1)); // dst -> abs
        return INS_LEN_JMPABS;

    } else if(INS_MATCH(op, PUSH)) {
        result->id = INS_ID_PUSH; // Its a Push
        result->operands[0] = op & ~INS_OP_MASK_PUSH; // src -> reg_id
        return INS_LEN_PUSH;

    } else if(INS_MATCH(op, POP)) {
        result->id = INS_ID_POP; // Its a Pop
        result->operands[0] = op & ~INS_OP_MASK_POP; // dst -> reg_id
        return INS_LEN_POP;

    } else if(INS_MATCH(op, MOV)) {
        result->id = INS_ID_MOV; // Its a Mov
        result->operands[0] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[1] = (uint32_t) (buff[1] & 0x0f);
        return INS_LEN_MOV;

    } else if(INS_MATCH(op, XOR)) {
        result->id = INS_ID_XOR; // Its a Xor
        result->operands[0] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[1] = (uint32_t) (buff[1] & 0x0f);
        return INS_LEN_XOR;

    } else if(INS_MATCH(op, ADD)) {
        result->id = INS_ID_ADD; // Its a Add
        result->operands[0] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[1] = (uint32_t) (buff[1] & 0x0f);
        return INS_LEN_ADD;

    }

    // Invalid instruction
    result->id = INS_ID_INVALID;
    return 0;
}
