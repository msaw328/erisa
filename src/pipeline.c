// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include "pipeline.h"

#include <stdint.h>
#include <string.h>

#include "registers.h"
#include "isa.h"

void fetch(uint8_t* decode_buff, uint8_t* mem, regs_t* regs) {
    memcpy(decode_buff, mem + regs->ipr, DECODE_BUFFER_LEN);
}

size_t decode(uint8_t* buff, ins_t* result) {
    uint8_t op = buff[0];

    if(INS_MATCH(op, STI)) {
        result->id = INS_ID_STI; // Its a Store Immediate
        result->src = *((uint32_t*) (buff + 1)); // src -> imm32
        result->dst = op & ~INS_OP_MASK_STI; // dst -> reg_id
        return INS_LEN_STI;

    } else if(INS_MATCH(op, NOP)) {
        result->id = INS_ID_NOP; // Its a No Operation
        return INS_LEN_NOP;

    } else if(INS_MATCH(op, JMPABS)) {
        result->id = INS_ID_JMPABS; // Its a Jump Absolute
        result->dst = *((uint32_t*) (buff + 1)); // dst -> abs
        return INS_LEN_JMPABS;

    } else if(INS_MATCH(op, PUSH)) {
        result->id = INS_ID_PUSH; // Its a Push
        result->src = op & ~INS_OP_MASK_PUSH; // src -> reg_id
        return INS_LEN_PUSH;

    } else if(INS_MATCH(op, POP)) {
        result->id = INS_ID_POP; // Its a Pop
        result->dst = op & ~INS_OP_MASK_POP; // dst -> reg_id
        return INS_LEN_POP;

    } else if(INS_MATCH(op, MOV)) {
        result->id = INS_ID_MOV; // Its a Mov
        result->src = (uint32_t) (buff[1] & 0x0f);
        result->dst = (uint32_t) ((buff[1] >> 4) & 0x0f);
        return INS_LEN_MOV;

    } else if(INS_MATCH(op, XOR)) {
        result->id = INS_ID_XOR; // Its a Xor
        result->src = (uint32_t) (buff[1] & 0x0f);
        result->dst = (uint32_t) ((buff[1] >> 4) & 0x0f);
        return INS_LEN_XOR;

    } else if(INS_MATCH(op, ADD)) {
        result->id = INS_ID_ADD; // Its a Add
        result->src = (uint32_t) (buff[1] & 0x0f);
        result->dst = (uint32_t) ((buff[1] >> 4) & 0x0f);
        return INS_LEN_ADD;

    }

    // Invalid instruction
    result->id = INS_ID_INVALID;
    return 0;
}

// Store Immediate: src - imm32, dst - reg_id
void __execute_sti(ins_t* ins, regs_t* regs, uint8_t* mem) {
    uint32_t imm_src = ins->src;
    uint32_t reg_id = ins->dst;

    regs->gpr[reg_id] = imm_src;
}

// No Operation
void __execute_nop(ins_t* ins, regs_t* regs, uint8_t* mem) {
    return;
}

// Jump Absolute - dst - addr
void __execute_jmpabs(ins_t* ins, regs_t* regs, uint8_t* mem) {
    uint32_t abs_addr = ins->dst;
    regs->ipr = abs_addr;
}

// Push - src - reg_id
void __execute_push(ins_t* ins, regs_t* regs, uint8_t* mem) {
    uint32_t reg_id = ins->src;

    regs->spr -= sizeof(uint32_t);

    uint32_t* spr32 = (uint32_t*) (mem + regs->spr);
    *spr32 = regs->gpr[reg_id];
}

// Pop - dst - reg_id
void __execute_pop(ins_t* ins, regs_t* regs, uint8_t* mem) {
    uint32_t reg_id = ins->dst;

    uint32_t* spr32 = (uint32_t*) (mem + regs->spr);
    regs->gpr[reg_id] = *spr32;

    regs->spr += sizeof(uint32_t);
}

// Mov - dst - reg_id, src - reg_id
void __execute_mov(ins_t* ins, regs_t* regs, uint8_t* mem) {
    uint32_t dst_id = ins->dst;
    uint32_t src_id = ins->src;

    regs->gpr[dst_id] = regs->gpr[src_id];
}

// Xor - dst - reg_id, src - reg_id
void __execute_xor(ins_t* ins, regs_t* regs, uint8_t* mem) {
    regs->flagr = 0;
    uint32_t dst_id = ins->dst;
    uint32_t src_id = ins->src;

    regs->gpr[dst_id] ^= regs->gpr[src_id];

    if(regs->gpr[dst_id] == 0) {
        FLAG_SET(regs->flagr, FLAG_BIT_ZERO);
    }
}

// Add - dst - reg_id, src - reg_id
void __execute_add(ins_t* ins, regs_t* regs, uint8_t* mem) {
    regs->flagr = 0;
    uint32_t dst_id = ins->dst;
    uint32_t src_id = ins->src;

    uint32_t overflow_guard = 0xffffffff;
    uint32_t dst_val = regs->gpr[dst_id];
    uint32_t src_val = regs->gpr[src_id];

    regs->gpr[dst_id] += regs->gpr[src_id];

    if(src_val > overflow_guard - dst_val) {
        FLAG_SET(regs->flagr, FLAG_BIT_CARRY);
    }

    if(regs->gpr[dst_id] == 0) {
        FLAG_SET(regs->flagr, FLAG_BIT_ZERO);
    }
}

// Function type used to handle execution of an instruction
typedef void(__ins_execute_t)(ins_t*, regs_t*, uint8_t*);
static __ins_execute_t* _ins_id_exec_map[] = {
    [INS_ID_INVALID] =  __execute_nop, // TODO: Properly handle invalid instruction
    [INS_ID_STI] = __execute_sti,
    [INS_ID_NOP] = __execute_nop,
    [INS_ID_JMPABS] = __execute_jmpabs,
    [INS_ID_PUSH] = __execute_push,
    [INS_ID_POP] = __execute_pop,
    [INS_ID_MOV] = __execute_mov,
    [INS_ID_XOR] = __execute_xor,
    [INS_ID_ADD] = __execute_add,
};

void execute(ins_t* ins, regs_t* regs, uint8_t* mem) {
    _ins_id_exec_map[ins->id](ins, regs, mem);
}
