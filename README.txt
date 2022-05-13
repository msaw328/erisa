ERISA - Embeddable Reduced Instruction Set Architecture

Current state of the project: very much a prototype. ISA is completely
unstable, very few instructions are actually implemented (see the YAML
for the whole list).

ERISA consists of:

-   Assembly-like language
-   Bytecode specification of said language
-   VM which properly executes the bytecode

It is a purely-for-fun project that was born out of my interest in:

-   Embeddable languages such as Lua
-   Bytecode VMs such as JVM
-   Assembler and CPUs and such
-   VM-based obfuscation techniques

Since the project is mostly done for fun, the bytecode is in no way
optimized and the instruction set might not be very well organized
either. Maybe some day i will work on that but it is pretty low on the
priorities list.

The planned structure of the project is to have a shared library which
implements instruction decoding and encoding, as well as structures and
functionality implementing the VM. That library will then be re-used by
three programs:

-   erisa-exec, the VM
-   erisa-asm, the assembler (compiler)
-   erisa-disasm, the disassembler

Additionally, since the language is meant to be embeddable, one will be
able to link with the library itself and use the VM structures and
functionality directly in their code.

Building instructions

Requirements:

-   Standard Linux-based setup involving GCC or Clang and some libc
    implementation. Other configurations might work too.
-   GNU Make
-   python with yaml module installed (used for code generation)

Building:

-   Literally just run make while in the root directory of the project,
    it calls codegen.py as needed
