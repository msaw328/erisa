// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdint.h>
#include <stddef.h>

#include <erisa/erisa.h>

#include "bytecode.h"

// Store Immediate: src - imm32, dst - reg_id
void __execute_sti(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    uint32_t imm_src = ins->operands[0];
    uint32_t reg_id = ins->operands[1];

    regs->gpr[reg_id] = imm_src;
}

// No Operation
void __execute_nop(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    return;
}

// Jump Absolute - dst - addr
void __execute_jmpabs(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    uint32_t abs_addr = ins->operands[0];
    regs->ipr = abs_addr;
}

// Push - src - reg_id
void __execute_push(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    uint32_t reg_id = ins->operands[0];

    regs->spr -= sizeof(uint32_t);

    uint32_t* spr32 = (uint32_t*) (mem + regs->spr);
    *spr32 = regs->gpr[reg_id];
}

// Pop - dst - reg_id
void __execute_pop(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    uint32_t reg_id = ins->operands[0];

    uint32_t* spr32 = (uint32_t*) (mem + regs->spr);
    regs->gpr[reg_id] = *spr32;

    regs->spr += sizeof(uint32_t);
}

// Mov - dst - reg_id, src - reg_id
void __execute_mov(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    uint32_t dst_id = ins->operands[0];
    uint32_t src_id = ins->operands[1];

    regs->gpr[dst_id] = regs->gpr[src_id];
}

// Xor - dst - reg_id, src - reg_id
void __execute_xor(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    regs->flagr = 0;
    uint32_t dst_id = ins->operands[0];
    uint32_t src_id = ins->operands[1];

    regs->gpr[dst_id] ^= regs->gpr[src_id];

    if(regs->gpr[dst_id] == 0) {
        FLAG_SET(regs->flagr, FLAG_BIT_ZERO);
    }
}

// Add - dst - reg_id, src - reg_id
void __execute_add(erisa_ins_t* ins, erisa_regs_t* regs, uint8_t* mem) {
    regs->flagr = 0;
    uint32_t dst_id = ins->operands[0];
    uint32_t src_id = ins->operands[1];

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
typedef void(__ins_execute_t)(erisa_ins_t*, erisa_regs_t*, uint8_t*);
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

void erisa_vm_execute(erisa_ins_t* ins, erisa_vm_t* vm) {
    erisa_regs_t* regs = &(vm->registers);
    uint8_t* mem = vm->memory;
    _ins_id_exec_map[ins->id](ins, regs, mem);
}
