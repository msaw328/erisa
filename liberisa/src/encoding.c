// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdint.h>
#include <stddef.h>

#include <erisa/erisa.h>

#include "bytecode.h"

void erisa_decode(uint8_t* buff, erisa_ins_t* result) {
    uint8_t op = buff[0];

    if(INS_MATCH(op, STI)) {
        result->id = INS_ID_STI; // Its a Store Immediate
        result->operands[INS_OPERAND_STI_IMM] = *((uint32_t*) (buff + 1)); // src -> imm32
        result->operands[INS_OPERAND_STI_DST] = op & ~INS_OP_MASK_STI; // dst -> reg_id
        result->length = INS_LEN_STI;
        return;

    } else if(INS_MATCH(op, NOP)) {
        result->id = INS_ID_NOP; // Its a No Operation
        result->length = INS_LEN_NOP;
        return;

    } else if(INS_MATCH(op, JMPABS)) {
        result->id = INS_ID_JMPABS; // Its a Jump Absolute
        result->operands[INS_OPERAND_JMPABS_ADDR] = *((uint32_t*) (buff + 1)); // dst -> abs
        result->length = INS_LEN_JMPABS;
        return;

    } else if(INS_MATCH(op, PUSH)) {
        result->id = INS_ID_PUSH; // Its a Push
        result->operands[INS_OPERAND_PUSH_SRC] = op & ~INS_OP_MASK_PUSH; // src -> reg_id
        result->length = INS_LEN_PUSH;
        return;

    } else if(INS_MATCH(op, POP)) {
        result->id = INS_ID_POP; // Its a Pop
        result->operands[INS_OPERAND_POP_DST] = op & ~INS_OP_MASK_POP; // dst -> reg_id
        result->length = INS_LEN_POP;
        return;

    } else if(INS_MATCH(op, MOV)) {
        result->id = INS_ID_MOV; // Its a Mov
        result->operands[INS_OPERAND_MOV_DST] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[INS_OPERAND_MOV_SRC] = (uint32_t) (buff[1] & 0x0f);
        result->length = INS_LEN_MOV;
        return;

    } else if(INS_MATCH(op, XOR)) {
        result->id = INS_ID_XOR; // Its a Xor
        result->operands[INS_OPERAND_XOR_DST] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[INS_OPERAND_XOR_SRC] = (uint32_t) (buff[1] & 0x0f);
        result->length = INS_LEN_XOR;
        return;

    } else if(INS_MATCH(op, ADD)) {
        result->id = INS_ID_ADD; // Its a Add
        result->operands[INS_OPERAND_ADD_DST] = (uint32_t) ((buff[1] >> 4) & 0x0f);
        result->operands[INS_OPERAND_ADD_SRC] = (uint32_t) (buff[1] & 0x0f);
        result->length = INS_LEN_ADD;
        return;

    }

    // Invalid instruction
    result->id = INS_ID_INVALID;
    result->length = 0;
}
