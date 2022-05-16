// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <erisa/erisa.h>

void erisa_vm_init(erisa_vm_t* vm, size_t memory_size) {
    memset(&(vm->registers), 0, sizeof(erisa_regs_t));

    vm->memory = malloc(memory_size);
    vm->memory_size = memory_size;
}

void erisa_vm_dump_regs(erisa_vm_t* vm) {
    erisa_regs_t* r = &(vm->registers);
    printf("regs state: (size with padding: %zu bytes) {\n", sizeof(erisa_regs_t));

    printf("\tflagr -> |%c%c|\n\n",
        FLAG_IS_SET(r->flagr, FLAG_BIT_CARRY) ? 'C' : '-',
        FLAG_IS_SET(r->flagr, FLAG_BIT_ZERO) ? 'Z' : '-'
    );
    puts("\t<reg> <hex> (u|d)");

    for(int i = 0; i < ERISA_VM_GPR_NUM; i++) {
        printf("\tgpr%2d -> |%08x| (%u|%d)\n", i, r->gpr[i], r->gpr[i], (int32_t) r->gpr[i]);
    }
    printf("\tretr  -> |%08x| (%u|%d)\n", r->retr, r->retr, (int32_t) r->retr);
    printf("\tspr   -> |%08x| (%u|%d)\n", r->spr, r->spr, (int32_t) r->spr);
    printf("\tipr   -> |%08x| (%u|%d)\n", r->ipr, r->ipr, (int32_t) r->ipr);

    puts("}");
}

ssize_t erisa_vm_load_firmware_buffer(erisa_vm_t* vm, uint8_t* bytecode, size_t bytecode_size) {
    if(vm->memory_size < bytecode_size) return -1;

    memset(vm->memory, 0, vm->memory_size); // Clear memory first
    memcpy(vm->memory, bytecode, bytecode_size);

    return bytecode_size;
}

ssize_t erisa_vm_load_firmware_file(erisa_vm_t* vm, char* filename) {
    if(access(filename, R_OK) != 0) {
        return -1;
    }

    struct stat file_stat = { 0 };
    if(stat(filename, &file_stat) != 0) {
        return -2;
    }

    if(vm->memory_size < (size_t) file_stat.st_size) return -3;

    FILE* firmware_file = fopen(filename, "rb");
    if(firmware_file == NULL) return -4;

    memset(vm->memory, 0, vm->memory_size); // Clear memory first

    size_t bytes_read = fread(vm->memory, file_stat.st_size, 1, firmware_file);
    
    fclose(firmware_file);
    
    if(bytes_read != 1) {
        return -5;
    }

    return (size_t) file_stat.st_size;
}
