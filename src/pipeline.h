// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#ifndef _ERISA_PIPELINE_H_
#define _ERISA_PIPELINE_H_

#include <stddef.h>
#include <stdint.h>

#include "registers.h"

// A structure which describes an instruction
struct ins_t {
    uint32_t id;    // Instruction id from isa.h
    uint32_t src;   // Source and Desitnation operands - might be addresses, offsets, gpr id, depends on instruction
    uint32_t dst;
};
typedef struct ins_t ins_t;

// Length of the decode buffer
#define DECODE_BUFFER_LEN 16

// Fill the decode buffer based on the regs and mem state
void fetch(uint8_t* decode_buff, uint8_t* mem, regs_t* regs);

// Decode machine code in buffer, write data to result structure
// Returns number of bytes that were decoded
size_t decode(uint8_t* buffer, ins_t* result);

// Execute instruction described by the ins structure
// Modifies register and memory state accordingly
void execute(ins_t* ins, regs_t* regs, uint8_t* mem);
#endif
