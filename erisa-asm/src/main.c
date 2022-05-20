#include <stdio.h>
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

    ssize_t result = 0;
    while(1) {
        erisa_ins_t ins = { 0 };
        result = erisa_asm(file_contents, file_size, &ins);

        printf("res = %ld, ins { id: %u, operands: [%u, %u] }\n", result, ins.id, ins.operands[0], ins.operands[1]);

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
    };
}
