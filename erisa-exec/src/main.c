// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>

#include <erisa/erisa.h>

#define RAM_SIZE (1 << 12)
#define STACK_TOP (RAM_SIZE) // Start stack at the very top

void fetch(uint8_t* decode_buff, erisa_vm_t* vm) {
    memcpy(decode_buff, vm->memory + vm->registers.ipr, ERISA_BYTECODE_BUFFER_LEN);
}

#define FIRMWARE_FILE "firmware.erisa"

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s [firmware filename]\n", argv[0]);
        return 0;
    }

    erisa_vm_t vm;
    erisa_vm_init(&vm, RAM_SIZE);

    ssize_t status = erisa_vm_load_firmware_file(&vm, argv[1]);
    
    if (status < 0) {
        printf("firmware error: %zd\n", status);
        return 0;
    }

    printf("Succesfully read %s (%zu bytes)\n", argv[1], (size_t) status);

    vm.registers.spr = STACK_TOP;

    uint8_t decode_buffer[ERISA_BYTECODE_BUFFER_LEN] = { 0 };
    erisa_ins_t decoded_instruction = { 0 };
    size_t next_ins = 0;

    char disasm_buffer[ERISA_DISASM_BUFFER_LEN] = { 0 };

    while(1) {
        erisa_vm_dump_regs(&vm);
        getc(stdin);

        // Fetch instruction
        fetch(decode_buffer, &vm);

        for(size_t i = 0; i < ERISA_BYTECODE_BUFFER_LEN; i++) {
            printf("0x%02x%c %c", decode_buffer[i],
                i == ERISA_BYTECODE_BUFFER_LEN - 1 ? ' ' : ',',
                i == ERISA_BYTECODE_BUFFER_LEN - 1 ? '\n' : ' '
            );
        }

        // Try to decode
        erisa_decode(decode_buffer, &decoded_instruction);
        next_ins = decoded_instruction.length;

        // Check if valid
        if(next_ins == 0) {
            puts("ERROR INVALID INSTRUCTION");
            break;
        }

        size_t chars_written = erisa_disasm(&decoded_instruction, disasm_buffer, ERISA_DISASM_BUFFER_LEN);
        if(chars_written > ERISA_DISASM_BUFFER_LEN) {
            puts("DISASM BUFFER TOO SHORT");
        } else {
            printf("DISASM: %s\n", disasm_buffer);
        }

        // Increment instruction pointer before execution, in case its a jump
        vm.registers.ipr += next_ins;

        // Execute
        erisa_vm_execute(&decoded_instruction, &vm);
    }
}
