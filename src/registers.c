// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include "registers.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

void init_regs(regs_t* r, uint32_t spr) {
    memset(r, 0, sizeof(regs_t));
    r->spr = spr;
}

void dump_regs(regs_t* r) {
    printf("regs state: (size with padding: %zu bytes) {\n", sizeof(regs_t));

    printf("\tflagr -> |%c%c|\n\n",
        FLAG_IS_SET(r->flagr, FLAG_BIT_CARRY) ? 'C' : '-',
        FLAG_IS_SET(r->flagr, FLAG_BIT_ZERO) ? 'Z' : '-'
    );
    puts("\t<reg> <hex> (u|d)");

    for(int i = 0; i < GPR_NUM; i++) {
        printf("\tgpr%2d -> |%08x| (%u|%d)\n", i, r->gpr[i], r->gpr[i], (int32_t) r->gpr[i]);
    }
    printf("\tretr  -> |%08x| (%u|%d)\n", r->retr, r->retr, (int32_t) r->retr);
    printf("\tspr   -> |%08x| (%u|%d)\n", r->spr, r->spr, (int32_t) r->spr);
    printf("\tipr   -> |%08x| (%u|%d)\n", r->ipr, r->ipr, (int32_t) r->ipr);

    puts("}");
}
