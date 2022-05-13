#!/usr/bin/evn python3

# ERISA - Embeddable Reduced Instruction Set Architecture
# Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

import yaml

def generate_isa_header():
    ISA_YAML_FILE = './data/isa.yaml'
    ISA_TEMPLATE_FILE = './src/isa.h.in'
    ISA_HEADER_FILE = './src/isa.h'
    
    ENTRY_FORMAT =  ('// %MNEMONIC%\n'
                    '// %NAME%\n'
                    '// Opcode: %OP%\n'
                    '//\n'
                    '// for instance %EXAMPLE_BYTES% encodes\n'
                    '// %EXAMPLE_DISASM%\n'
                    '#define INS_ID_%MNEMONIC% %ID%\n'
                    '#define INS_OP_%MNEMONIC% %OP%\n'
                    '#define INS_OP_MASK_%MNEMONIC% %MASK%\n'
                    '#define INS_LEN_%MNEMONIC% %LENGTH%\n\n\n')
    
    def instruction_to_string(name, props):
        entry = ENTRY_FORMAT \
            .replace('%MNEMONIC%', name) \
            .replace('%ID%', hex(props['id'])) \
            .replace('%NAME%', props['name']) \
            .replace('%OP%', hex(props['op'])) \
            .replace('%MASK%', hex(props['mask'])) \
            .replace('%LENGTH%', str(props['length'])) \
            .replace('%EXAMPLE_BYTES%', props['example_bytes']) \
            .replace('%EXAMPLE_DISASM%', props['example_disasm'])
    
        return entry
    
    with open(ISA_YAML_FILE, 'r') as infile:
        try:
            instructions = yaml.safe_load(infile)
    
            buffer = ''
    
            for instruction in instructions:
                buffer += instruction_to_string(instruction, instructions[instruction])
    
            template = open(ISA_TEMPLATE_FILE, 'r').read()
    
            result = template.replace('%ENTRIES%', buffer)
    
            with open(ISA_HEADER_FILE, 'w') as outfile:
                outfile.write(result)
    
        except yaml.YAMLError as exc:
            print(exc)

targets = {
    'src/isa.h': generate_isa_header
}
