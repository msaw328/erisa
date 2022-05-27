#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <erisa/erisa.h>

ssize_t load_file(char** buffer, char* filename) {
    if(access(filename, R_OK) != 0) {
        return -1;
    }

    struct stat file_stat = { 0 };
    if(stat(filename, &file_stat) != 0) {
        return -2;
    }

    if(buffer == NULL) return -3;

    *buffer = malloc(((size_t) file_stat.st_size) + 1);

    FILE* firmware_file = fopen(filename, "r");
    if(firmware_file == NULL) {
        free(*buffer);
        return -4;
    }

    size_t bytes_read = fread(*buffer, file_stat.st_size, 1, firmware_file);
    
    fclose(firmware_file);
    
    if(bytes_read != 1) {
        free(*buffer);
        return -5;
    }

    (*buffer)[(size_t) file_stat.st_size] = '\0';

    return  (size_t) file_stat.st_size;
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s [source filename]\n", argv[0]);
        return 0;
    }

    char* file_contents;

    ssize_t status = load_file(&file_contents, argv[1]);
    
    if (status < 0) {
        printf("firmware error: %zd\n", status);
        return 0;
    }

    size_t file_size = (size_t) status;

    printf("Succesfully read %s (%zu bytes)\n\n\n", argv[1], file_size);

    uint32_t program_offset = 0;

    erisa_label_t all_labels[100] = { 0 };
    size_t all_labels_num = 0;

    erisa_ins_t instructions[100] = { 0 };
    size_t instructions_num = 0;

    erisa_ins_symdep_t symdeps[100] = { 0 };
    size_t symdeps_num = 0;

    erisa_ins_symdep_t* symdep_iter;

    ssize_t result = 0;
    while(1) {
        symdep_iter = symdeps + symdeps_num;
        symdep_iter->ins = instructions + instructions_num;

        result = erisa_asm(file_contents, file_size, all_labels + all_labels_num, symdep_iter);

        printf("0x%08x: ", program_offset);

        if(strlen(all_labels[all_labels_num].symbol) > 0) { // check if new label
            printf("(@%s) ", all_labels[all_labels_num].symbol);
            all_labels[all_labels_num].addr = program_offset;
            all_labels_num++;
        }

        printf("res = %ld, ins { id: %u, operands: [%u, %u] }\n", result, symdep_iter->ins->id, symdep_iter->ins->operands[0], symdep_iter->ins->operands[1]);

        program_offset += symdep_iter->ins->length;

        if(symdep_iter->needs_symbol) { // check symbol dependency
            printf("(operand %lu depends on @%s)\n", symdep_iter->op_idx, symdep_iter->symbol);
            symdeps_num++;
        }


        if(result < 0) {
            switch(result) {
                default: {
                    printf("File Err, status = %ld\n", result);
                    return 1;
                }

                case -1: {
                    puts("File Ok");
                    break;
                }
            }

            break;
        }

        file_contents += (size_t) result;
        file_size -= (size_t) result;
        instructions_num++;
    };

    puts("\nSYMS:");
    for(size_t i = 0; i < all_labels_num; i++) {
        printf("@%s: 0x%08x\n", all_labels[i].symbol, all_labels[i].addr);
    }

    puts("\nSYMDEPS:");
    for(size_t i = 0; i < symdeps_num; i++) {
        printf("ins id %u operand %lu depends on symbol @%s\n", symdeps[i].ins->id, symdeps[i].op_idx, symdeps[i].symbol);
    }

    puts("\nFIXING SYMBOL DEPENDENCIES...");
    for(size_t i = 0; i < symdeps_num; i++) {
        for(size_t j = 0; j < all_labels_num; j++) {
            if(strncmp(symdeps[i].symbol, all_labels[j].symbol, ERISA_TOKEN_BUFF_SIZE - 1) == 0) {
                symdeps[i].needs_symbol = 0;
                symdeps[i].ins->operands[symdeps[i].op_idx] = all_labels[j].addr;
                break;
            }
        }

        if(symdeps[i].needs_symbol) {
            printf("ERROR, CAN'T FIND SYMBOL @%s\n", symdeps[i].symbol);
            return 0;
        }
    }

    puts("\nFIXING OK");
    puts("FINAL CODE:");

    program_offset = 0;
    char disasm_buffer[ERISA_DISASM_BUFFER_LEN] = { 0 };
    for(size_t i = 0; i < instructions_num; i++) {
        erisa_disasm(instructions + i, disasm_buffer, ERISA_DISASM_BUFFER_LEN);
        printf("0x%08x: { id: %u, operands: [%u, %u] } \t::: %s\n", program_offset, instructions[i].id, instructions[i].operands[0], instructions[i].operands[1], disasm_buffer);
        program_offset += instructions[i].length;
    }

    // TODO: decode the resulting instructions array
}
