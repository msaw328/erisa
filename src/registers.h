// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#ifndef _ERISA_REGISTERS_H_
#define _ERISA_REGISTERS_H_

#include <stdint.h>

// Number of General Purpose Registers
#define GPR_NUM 16

struct regs_t {
    uint32_t gpr[GPR_NUM];  // General Purpose registers
    uint32_t retr;          // Return value register
    uint32_t spr;           // Stack pointer register
    uint32_t ipr;           // Instruction pointer register
    uint16_t flagr;         // Flag register
};
typedef struct regs_t regs_t;

// Flag Register flags
#define FLAG_BIT_CARRY 0
#define FLAG_BIT_ZERO 1

// Macro to check whether a flag is set or not
#define FLAG_IS_SET(reg, flag) ((reg & (1 << flag)) != 0)

// Set/clear flag in register
#define FLAG_SET(reg, flag) (reg = reg | (1 << flag))
#define FLAG_CLEAR(reg, flag) (reg = reg & ~(1 << flag))

// Initialize the regs structure
void init_regs(regs_t* r, uint32_t spr);

// Dumps registers to stdout
void dump_regs(regs_t* r);

#endif
