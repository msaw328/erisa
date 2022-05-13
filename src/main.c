// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdint.h>
#include <stdio.h>

#include "registers.h"
#include "pipeline.h"

#define RAM_SIZE (1 << 12)
#define STACK_TOP (RAM_SIZE) // Start stack at the very top

uint8_t ram[RAM_SIZE] = {
    0x72, 0xff, 0x00, 0x00, 0xdd,   // 0x0000: sti %gpr2, $0xdd0000ff;
    0x71, 0xff, 0x00, 0xbb, 0xdd,   // 0x0005: sti %gpr1, $0xddbb00ff;
    0x70, 0x00, 0x00, 0x00, 0x00,   // 0x000a: sti %gpr0, $0x0;
    0x90,                           // 0x000f: nop;
    0x52,                           // 0x0010: push %gpr2;
    0x51,                           // 0x0011: push %gpr1;
    0x62,                           // 0x0012: pop %gpr2;
    0x61,                           // 0x0013: pop %gpr1;
    0xc0, 0x42,                     // 0x0014: mov %gpr4, %gpr2;    // dst first, src second
    0xa0, 0x41,                     // 0x0016: xor %gpr4, %gpr1;
    0xa1, 0x24,                     // 0x0018: add %gpr2, %gpr4;
    0x91, 0x14, 0x00, 0x00, 0x00    // 0x001a: jmpabs $0x14;        // This loops infinitely!
};

int main() {
    regs_t regs;
    init_regs(&regs, STACK_TOP);

    uint8_t decode_buffer[DECODE_BUFFER_LEN] = { 0 };
    ins_t decoded_instruction = { 0 };
    size_t next_ins = 0;

    while(1) {
        dump_regs(&regs);

        // Fetch instruction
        fetch(decode_buffer, ram, &regs);

        for(size_t i = 0; i < DECODE_BUFFER_LEN; i++) {
            printf("0x%02x%c %c", decode_buffer[i],
                i == DECODE_BUFFER_LEN - 1 ? ' ' : ',',
                i == DECODE_BUFFER_LEN - 1 ? '\n' : ' '
            );
        }

        // Try to decode
        next_ins = decode(decode_buffer, &decoded_instruction);

        // Check if valid
        if(next_ins == 0) {
            puts("ERROR INVALID INSTRUCTION");
            break;
        }

        // Increment instruction pointer before execution, in case its a jump
        regs.ipr += next_ins;

        // Execute
        execute(&decoded_instruction, &regs, ram);
    }
}
