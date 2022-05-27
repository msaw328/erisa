// ERISA - Embeddable Reduced Instruction Set Architecture
// Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

#ifndef _ERISA_PUBLIC_H_
#define _ERISA_PUBLIC_H_

#include <stdint.h>
#include <sys/types.h>

// Public header for the liberisa

// A structure which describes an instruction
// It is an intermediate format between encoding, decoding, execution etc.
// Each instruction might contain up to 2 operands
struct erisa_ins_t {
    uint32_t id;            // Instruction id from isa.h
    uint32_t operands[2];   // Operands may be either a register id or an immediate, address or offset up to 32 bits
    size_t length;          // Length of the instruction in bytes
};
typedef struct erisa_ins_t erisa_ins_t;

//
// Instruction coding
//

// Length of the recommended decode/encode bytecode buffer
// Buffer of this length is guaranteed to completely contain all instructions
#define ERISA_BYTECODE_BUFFER_LEN 8

// Decodes bytes present in the decode_buffer into a single instruction
void erisa_decode(uint8_t* decode_buffer, erisa_ins_t* result);

//
// Assembly/Disassembly
//

// Length of the recommended instruction string buffer when disassembling
#define ERISA_DISASM_BUFFER_LEN 64

// Convert ins instruction to string in str_buff
// Returns 0 if buffer too short, or number of bytes written to the buffer otherwise
size_t erisa_disasm(erisa_ins_t* ins, char* str_buff, size_t buff_size);

// Max length of a token string (including null byte)
#define ERISA_TOKEN_BUFF_SIZE 32

// Describes a label with a string symbol and an address it points to
struct erisa_label_t {
    char symbol[ERISA_TOKEN_BUFF_SIZE];
    uint32_t addr;
};
typedef struct erisa_label_t erisa_label_t;

// An ins structure, extended to contain information about symbol dependency
// If an operand depends on a symbol that is missing at the time of assembly,
// the "needs_symbol" field is set to 1, "symbol" is set to the symbol name
// and op_idx identifies which operand should be set to the symbol value
struct erisa_ins_symdep_t {
    erisa_ins_t* ins;
    int needs_symbol; // true/false (1/0)
    char symbol[ERISA_TOKEN_BUFF_SIZE];
    size_t op_idx;
};
typedef struct erisa_ins_symdep_t erisa_ins_symdep_t;

// A statement is a string made of tokens up until the semicolon ;
// of the form "<optional label> <mnemonic> <operand1, if necessary> <operand2, if necessary>;"
// All tokens may be separated by any amount of whitespace
// Example statements:
// 1.   @my_label mov %gpr4 %gpr7;
//
// 2.   @my_other_label
//          jmpabs @my_label;
//
// 3.   nop;
//
// The function converts the first nonempty statement found in the input buffer of up to "length" bytes
// into tokens, and attempts to parse it into an instruction
//
// If statemenmt defines a new label, its name is copied into new_label->symbol array. Otherwise, the symbol
// field remains an empty string. The caller should keep track of offset into the program based on symdep->ins->length
// and check whether strlen(new_label->symbol) > 0. If it is, the new symbol should have address equal to current
// program offset.
//
// If the instruction dpeneds on a symbol to be resolved, like "jmpabs @label;""
// then the symdep->needs_symbol is set to a nonzero (true) value and other symbol information is filled
// all fields other than the operand that depends on the symbol in the symdep->ins structure
// are usable, in particular symdep->ins->length which is necessary to keep track of the
// program offset
//
// The function returns number of bytes read from input, or a negative value that indicates
// a special code:
// Tokenizer errors:
// -1 : expected end of input - everything ok, there is just no more nonempty statements left in input, just whitespace
// -2 : unexpected end of input - there is a statement but it ends abruptly, no semicolon in sight
// -3 : token too long - token is too long and probably invalid
// -4 : extra tokens at the end of the statement
//
// Parser errors:
// -11 : unknown memonic
// -12 : expected mnemonic, got something else
// -13 : wrong operand type or number
// -14 : invalid operand of proper type
ssize_t erisa_asm(char* input, size_t length, erisa_label_t* new_label, erisa_ins_symdep_t* symdep);

//
// Instruction execution (VM)
//

// Number of General Purpose registers
#define ERISA_VM_GPR_NUM 16

// Structure which describes the state of registers of the VM
struct erisa_regs_t {
    uint32_t gpr[ERISA_VM_GPR_NUM]; // General Purpose registers
    uint32_t retr;                  // Return value register
    uint32_t spr;                   // Stack pointer register
    uint32_t ipr;                   // Instruction pointer register
    uint16_t flagr;                 // Flag register
};
typedef struct erisa_regs_t erisa_regs_t;

// Flag Register bits
#define FLAG_BIT_CARRY 0
#define FLAG_BIT_ZERO 1

// Macro to check whether a flag is set or not
#define FLAG_IS_SET(reg, flag) ((reg & (1 << flag)) != 0)

// Set/clear bit in flag register
#define FLAG_SET(reg, flag) (reg = reg | (1 << flag))
#define FLAG_CLEAR(reg, flag) (reg = reg & ~(1 << flag))

// ERISA VM structure
struct erisa_vm_t {
    erisa_regs_t registers;
    size_t memory_size;
    uint8_t* memory;
};
typedef struct erisa_vm_t erisa_vm_t;

// Initialize the virtual machine
// All registers are initialized to 0
// RAM is allocated dynamically
void erisa_vm_init(erisa_vm_t*, size_t memory_size);

// Loads bytecode from a buffer
// returns size on success, other values on failure (RAM too small to fit firmware)
ssize_t erisa_vm_load_firmware_buffer(erisa_vm_t*, uint8_t* bytecode, size_t bytecode_size);

// Loads bytecode from a file
// returns size loaded on success, negative value on failure
ssize_t erisa_vm_load_firmware_file(erisa_vm_t* vm, char* filename);

// Dumps registers to stdout
void erisa_vm_dump_regs(erisa_vm_t*);

// Executes a single instruction modifying the state of registers and RAM of the VM
void erisa_vm_execute(erisa_ins_t*, erisa_vm_t*);

#endif

