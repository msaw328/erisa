// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include <erisa/erisa.h>

#include "bytecode.h"

#define REG_MAX_STR_LEN 6 // %gpr12 -> len 6
size_t __reg_id_to_string(uint32_t reg_id, char* str_buff) {
    return (size_t) sprintf(str_buff, "%%gpr%u", reg_id);
}

#define IMM_MAX_STR_LEN 11 // $0xf0000000 -> len 11
size_t __imm_to_string(uint32_t reg_id, char* str_buff) {
    return (size_t) sprintf(str_buff, "$0x%x", reg_id);
}

// <invalid>
#define INS_INVALID_MAX_STR_LEN (strlen(INS_STR_INVALID))

// nop;
#define INS_NOP_MAX_STR_LEN (strlen(INS_STR_NOP) + 1)

// jmpabs + ' ' + imm + ';'
#define INS_JMPABS_MAX_STR_LEN (strlen(INS_STR_JMPABS) + 1 + IMM_MAX_STR_LEN + 1)

// sti + ' ' + reg + ', ' + imm + ';'
#define INS_STI_MAX_STR_LEN (strlen(INS_STR_STI) + 1 + REG_MAX_STR_LEN + 2 + IMM_MAX_STR_LEN + 1)

// push + ' ' + reg + ';'
#define INS_PUSH_MAX_STR_LEN (strlen(INS_STR_PUSH) + 1 + REG_MAX_STR_LEN + 1)

// pop + ' ' + reg + ';'
#define INS_POP_MAX_STR_LEN (strlen(INS_STR_POP) + 1 + REG_MAX_STR_LEN + 1)

// mov + ' ' + reg + ', ' + reg + ';'
#define INS_MOV_MAX_STR_LEN (strlen(INS_STR_MOV) + 1 + REG_MAX_STR_LEN + 2 + REG_MAX_STR_LEN + 1)

// add + ' ' + reg + ', ' + reg + ';'
#define INS_ADD_MAX_STR_LEN (strlen(INS_STR_ADD) + 1 + REG_MAX_STR_LEN + 2 + REG_MAX_STR_LEN + 1)

// xor + ' ' + reg + ', ' + reg + ';'
#define INS_XOR_MAX_STR_LEN (strlen(INS_STR_XOR) + 1 + REG_MAX_STR_LEN + 2 + REG_MAX_STR_LEN + 1)

size_t __disasm_invalid(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_INVALID_MAX_STR_LEN + 1 > buff_size) return INS_INVALID_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_INVALID);
    size_t len = strlen(INS_STR_INVALID);

    str_buff[len] = '\0';
    len += 1;

    return len;
}

size_t __disasm_sti(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_STI_MAX_STR_LEN + 1 > buff_size) return INS_STI_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_STI); // Append mnemonic
    size_t len = strlen(INS_STR_STI);

    str_buff[len] = ' ';
    len += 1;

    size_t part_len = __reg_id_to_string(ins->operands[INS_OPERAND_STI_DST], str_buff + len); // Append register
    len += part_len;

    str_buff[len + 0] = ',';
    str_buff[len + 1] = ' ';

    len += 2;

    part_len = __imm_to_string(ins->operands[INS_OPERAND_STI_IMM], str_buff + len); // Append immediate
    len += part_len;

    str_buff[len] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_nop(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_NOP_MAX_STR_LEN + 1 > buff_size) return INS_NOP_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_NOP);
    size_t len = strlen(INS_STR_NOP);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';
    len += 2;

    return len;
}

size_t __disasm_jmpabs(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_JMPABS_MAX_STR_LEN + 1 > buff_size) return INS_JMPABS_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_JMPABS);
    size_t len = strlen(INS_STR_JMPABS);

    str_buff[len] = ' ';
    len += 1;

    len += __imm_to_string(ins->operands[INS_OPERAND_STI_IMM], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_pop(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_POP_MAX_STR_LEN + 1 > buff_size) return INS_POP_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_POP);
    size_t len = strlen(INS_STR_POP);

    str_buff[len] = ' ';
    len += 1;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_POP_DST], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_push(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_PUSH_MAX_STR_LEN + 1 > buff_size) return INS_PUSH_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_PUSH);
    size_t len = strlen(INS_STR_PUSH);

    str_buff[len] = ' ';
    len += 1;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_PUSH_SRC], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_mov(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_MOV_MAX_STR_LEN + 1 > buff_size) return INS_MOV_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_MOV);
    size_t len = strlen(INS_STR_MOV);

    str_buff[len] = ' ';
    len += 1;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_MOV_DST], str_buff + len);

    str_buff[len + 0] = ',';
    str_buff[len + 1] = ' ';

    len += 2;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_MOV_SRC], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_xor(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_XOR_MAX_STR_LEN + 1 > buff_size) return INS_XOR_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_XOR);
    size_t len = strlen(INS_STR_XOR);

    str_buff[len] = ' ';
    len += 1;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_XOR_DST], str_buff + len);

    str_buff[len + 0] = ',';
    str_buff[len + 1] = ' ';

    len += 2;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_XOR_SRC], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

size_t __disasm_add(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    if(INS_ADD_MAX_STR_LEN + 1 > buff_size) return INS_ADD_MAX_STR_LEN + 1;

    strcpy(str_buff, INS_STR_ADD);
    size_t len = strlen(INS_STR_ADD);

    str_buff[len] = ' ';
    len += 1;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_ADD_DST], str_buff + len);

    str_buff[len + 0] = ',';
    str_buff[len + 1] = ' ';

    len += 2;

    len += __reg_id_to_string(ins->operands[INS_OPERAND_ADD_SRC], str_buff + len);

    str_buff[len + 0] = ';';
    str_buff[len + 1] = '\0';

    len += 2;

    return len;
}

// Function type used to handle disassembly of an instruction
typedef size_t(__ins_disasm_t)(erisa_ins_t*, char*, size_t);
static __ins_disasm_t* _ins_id_disasm_map[] = {
    [INS_ID_INVALID] =  __disasm_invalid,
    [INS_ID_STI] = __disasm_sti,
    [INS_ID_NOP] = __disasm_nop,
    [INS_ID_JMPABS] = __disasm_jmpabs,
    [INS_ID_PUSH] = __disasm_push,
    [INS_ID_POP] = __disasm_pop,
    [INS_ID_MOV] = __disasm_mov,
    [INS_ID_XOR] = __disasm_xor,
    [INS_ID_ADD] = __disasm_add,
};

size_t erisa_disasm(erisa_ins_t* ins, char* str_buff, size_t buff_size) {
    return _ins_id_disasm_map[ins->id](ins, str_buff, buff_size);
}

