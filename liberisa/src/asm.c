// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz
#define _POSIX_C_SOURCE 200809L
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/types.h>

#include "bytecode.h"

#include <erisa/erisa.h>

#define IS_TABSPACE(ch) (ch == '\t' || ch ==  ' ')
#define IS_WHITESPACE(ch) (IS_TABSPACE(ch) || ch == '\n')
#define IS_NONTOKEN(ch) (IS_WHITESPACE(ch) || ch == ';')

// Max length of a token string (including null byte)
#define TOKEN_MAX_LEN 32
#define TOKEN_TYPE_NONE 0
#define TOKEN_TYPE_MNEMONIC 1
#define TOKEN_TYPE_REG 2
#define TOKEN_TYPE_IMM 3

struct token_t {
    int type;
    char str[TOKEN_MAX_LEN];
};
typedef struct token_t token_t;

// Currently the longest instructions use 3 tokens - 1 mnemonic and 2 operands
// In case an instruction with 3 or more operands is introduced, this should be increased
#define MAX_TOKENS_PER_STATEMENT 3

// Statement is an array of up to MAX_TOKENS_PER_STATEMENT tokens
// This function retrieves up to 3 tokens from a char buffer and modifies
// in_buffer ptr and in_buffer_length according to number of characters parsed
// Return number of chars read from in_buffer OR
// -1 in case of expected end of input OR
// Other negative value if parsing errors occured
ssize_t __get_next_nonempty_statement(char* in_buffer, size_t in_buffer_length, token_t* tokens) { 
    size_t current_token_idx = 0;
    size_t chars_read = 0;

    memset(tokens, 0, MAX_TOKENS_PER_STATEMENT * sizeof(token_t)); // Clear tokens

    while(1) {
        // Skip until token starts
        while(IS_NONTOKEN(in_buffer[chars_read])) {
            // ; ends the statement, but skip empty statements
            if(in_buffer[chars_read] == ';' && current_token_idx > 0) {
                return (ssize_t) chars_read + 1;
            }
            
            if(chars_read + 1 >= in_buffer_length) {
                if(current_token_idx > 0)
                    return -2; // Unexpected end of input
                else
                    return -1; // Expected end of input
            }


            chars_read++;
        }

        if(current_token_idx >= MAX_TOKENS_PER_STATEMENT) {
            return -4; // too many tokens in statement
        }

        // TOKEN STARTS HERE

        // Determine token type based on first character
        switch(in_buffer[chars_read]) {
            case '%': {
                tokens[current_token_idx].type = TOKEN_TYPE_REG;
                chars_read++;
                break;
            }
            case '$': {
                tokens[current_token_idx].type = TOKEN_TYPE_IMM;
                chars_read++;
                break;
            }
            default:
                tokens[current_token_idx].type = TOKEN_TYPE_MNEMONIC;
        }

        // Start copying token to the token_t struct
        char* dst_buff = tokens[current_token_idx].str;

        size_t token_len = 0;
        while(!IS_NONTOKEN(in_buffer[chars_read])) {

            dst_buff[token_len] = in_buffer[chars_read];

            if(chars_read + 1 >= in_buffer_length) {
                return -2; // Unexpected end of input
            }

            if(token_len + 1 >= TOKEN_MAX_LEN - 1) { // 1 byte for null
                return -3; // Token too long
            }

            chars_read++;
            token_len++;
        }

        dst_buff[token_len] = '\0';
        current_token_idx++;
    }
}


#define TOKEN_IMM_MAX_STR_LEN 10 // 0xf0000000 -> len 10
int __token_to_imm(token_t* token, uint32_t* result) {
    if(strnlen(token->str, TOKEN_IMM_MAX_STR_LEN + 1) > TOKEN_IMM_MAX_STR_LEN) {
        return -1; // Immediate too long
    }

    char* endptr = NULL;
    unsigned long conv = strtoul(token->str, &endptr, 0);

    // From the man:
    // In particular, if *nptr is not '\0' but **endptr is '\0' on return, the entire string is valid.
    if(token->str[0] != '\0' && endptr[0] == '\0') {
        *result = (uint32_t) conv;
        return 0;
    } else {
        return -2; // Invalid immediate
    }
}

#define TOKEN_REG_MAX_STR_LEN 5 // gpr15 -> len 5
int __token_to_reg_id(token_t* token, uint32_t* result) {
    if(strnlen(token->str, TOKEN_REG_MAX_STR_LEN + 1) > TOKEN_REG_MAX_STR_LEN) {
        return -1; // reg too long
    }

    if(token->str[0] != 'g' || token->str[1] != 'p' || token->str[2] != 'r') return -2; // Invalid gpr

    char* endptr = NULL;
    unsigned long conv = strtoul(token->str + 3, &endptr, 10);

    // From the man:
    // In particular, if *nptr is not '\0' but **endptr is '\0' on return, the entire string is valid.
    if(token->str[0] != '\0' && endptr[0] == '\0') {
        *result = (uint32_t) conv;
        return 0;
    } else {
        return -2; // Invalid reg
    }
}

struct __tokens_to_ins_mapping {
    char mnemonic[TOKEN_MAX_LEN];
    uint32_t ins_id;
    uint32_t ins_len;
    int operand_types[MAX_TOKENS_PER_STATEMENT - 1];
    int operand_idx[MAX_TOKENS_PER_STATEMENT - 1]; // indices where the operands go in the ins_t struct
};
static struct __tokens_to_ins_mapping __ins_map[] = {
    {
        .mnemonic = INS_STR_NOP,
        .ins_id = INS_ID_NOP,
        .ins_len = INS_LEN_NOP
    },
    {
        .mnemonic = INS_STR_JMPABS,
        .ins_id = INS_ID_JMPABS,
        .ins_len = INS_LEN_JMPABS,
        .operand_types = { TOKEN_TYPE_IMM },
        .operand_idx = { INS_OPERAND_JMPABS_ADDR }
    },
    {
        .mnemonic = INS_STR_PUSH,
        .ins_id = INS_ID_PUSH,
        .ins_len = INS_LEN_PUSH,
        .operand_types = { TOKEN_TYPE_REG },
        .operand_idx = { INS_OPERAND_PUSH_SRC }
    },
    {
        .mnemonic = INS_STR_POP,
        .ins_id = INS_ID_POP,
        .ins_len = INS_LEN_POP,
        .operand_types = { TOKEN_TYPE_REG },
        .operand_idx = { INS_OPERAND_POP_DST }
    },
    {
        .mnemonic = INS_STR_STI,
        .ins_id = INS_ID_STI,
        .ins_len = INS_LEN_STI,
        .operand_types = { TOKEN_TYPE_REG, TOKEN_TYPE_IMM },
        .operand_idx = { INS_OPERAND_STI_DST, INS_OPERAND_STI_IMM }
    },
    {
        .mnemonic = INS_STR_MOV,
        .ins_id = INS_ID_MOV,
        .ins_len = INS_LEN_MOV,
        .operand_types = { TOKEN_TYPE_REG, TOKEN_TYPE_REG },
        .operand_idx = { INS_OPERAND_MOV_DST, INS_OPERAND_MOV_SRC }
    },
    {
        .mnemonic = INS_STR_XOR,
        .ins_id = INS_ID_XOR,
        .ins_len = INS_ID_XOR,
        .operand_types = { TOKEN_TYPE_REG, TOKEN_TYPE_REG },
        .operand_idx = { INS_OPERAND_XOR_DST, INS_OPERAND_XOR_SRC }
    },
    {
        .mnemonic = INS_STR_ADD,
        .ins_id = INS_ID_ADD,
        .ins_len = INS_ID_ADD,
        .operand_types = { TOKEN_TYPE_REG, TOKEN_TYPE_REG },
        .operand_idx = { INS_OPERAND_ADD_DST, INS_OPERAND_ADD_SRC }
    }
};

#define NUM_OF_INSTRUCTIONS (sizeof(__ins_map)/sizeof(__ins_map[0]))

int __match_tokens_to_ins(token_t* tokens, erisa_ins_t* ins) {
    if(tokens[0].type != TOKEN_TYPE_MNEMONIC) {
        return -2; // Expected mnemonic, got reg or imm or something
    }

    char* mnem = tokens[0].str;
    struct __tokens_to_ins_mapping* entry_ptr = NULL;
    for(size_t i = 0; i < NUM_OF_INSTRUCTIONS; i++) {
        entry_ptr = __ins_map + i;

        if(strcmp(entry_ptr->mnemonic, mnem) != 0) // If mnemonic does not match, continue
            continue;

        token_t* operands = tokens + 1;

        for(int j = 0; j < MAX_TOKENS_PER_STATEMENT - 1; j++) { // Check operand types, and if they parse correctly
            if(operands[j].type != entry_ptr->operand_types[j])
                return -3; // Wrong operand type

            // Try parse operand
            int res = 0;
            int ins_op_idx = entry_ptr->operand_idx[j];
            switch(operands[j].type) {
                case TOKEN_TYPE_IMM: {
                    res = __token_to_imm(operands + j, ins->operands + ins_op_idx);
                    break;
                }

                case TOKEN_TYPE_REG: {
                    res = __token_to_reg_id(operands + j, ins->operands + ins_op_idx);
                    break;
                }

                default:
                case TOKEN_TYPE_NONE:
                    break;
            }

            if(res < 0) return -4; // Invalid operand
        }

        // if we got here, its the good instruction
        ins->id = entry_ptr->ins_id;
        ins->length = entry_ptr->ins_len;

        return 0;
    }

    return -1; // Unknown mnemonic
}

ssize_t erisa_asm(char* input_ptr, size_t remaining_length, erisa_ins_t* ins) {
    token_t tokens[MAX_TOKENS_PER_STATEMENT] = { 0 };

    ssize_t stmt_len = __get_next_nonempty_statement(input_ptr, remaining_length, tokens);

    if(stmt_len < 0) return stmt_len;

    int res = __match_tokens_to_ins(tokens, ins);

    if(res < 0) { 
        return res - 10;
    }

    return stmt_len;
}
