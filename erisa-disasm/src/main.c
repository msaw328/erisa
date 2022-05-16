#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <erisa/erisa.h>

ssize_t load_firmware_file(uint8_t** buffer, char* filename) {
    if(access(filename, R_OK) != 0) {
        return -1;
    }

    struct stat file_stat = { 0 };
    if(stat(filename, &file_stat) != 0) {
        return -2;
    }

    if(buffer == NULL) return -3;

    *buffer = malloc((size_t) file_stat.st_size);

    FILE* firmware_file = fopen(filename, "rb");
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

    return  (size_t) file_stat.st_size;
}

void print_line(size_t offset, uint8_t* bytecode, size_t bytecode_len, char* disassembly) {
    printf("0x%08lx\t", offset);
    for(size_t i = 0; i < 5; i++) {
        if(i < bytecode_len) {
            printf("%02x ", bytecode[i]);
        } else {
            printf("   ");
        }
    }
    printf("\t\t%s\n", disassembly);
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("%s [firmware filename]\n", argv[0]);
        return 0;
    }

    uint8_t* firmware_contents;

    ssize_t status = load_firmware_file(&firmware_contents, argv[1]);
    
    if (status < 0) {
        printf("firmware error: %zd\n", status);
        return 0;
    }

    size_t firmware_size = (size_t) status;

    printf("Succesfully read %s (%zu bytes)\n\n\n", argv[1], firmware_size);

    erisa_ins_t ins = { 0 };
    char disasm_buffer[ERISA_DISASM_BUFFER_LEN] = { 0 };

    size_t idx = 0;
    while(idx < firmware_size) {
        erisa_decode(firmware_contents + idx, &ins);

        size_t next_ins = ins.length;

        if(next_ins == 0) next_ins = 1;

        erisa_disasm(&ins, disasm_buffer, ERISA_DISASM_BUFFER_LEN);

        print_line(idx, firmware_contents + idx, next_ins, disasm_buffer);
        idx += next_ins;
    }
}
