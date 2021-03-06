#!/usr/bin/evn python3

# ERISA - Embeddable Reduced Instruction Set Architecture
# Copyright (C) 2022  Maciej Sawka maciejsawka@gmail.com, msaw328@kretes.xyz

import yaml

from hashlib import sha256

from struct import unpack

def generate_isa_header():
    ISA_YAML_FILE = './data/isa.yaml'
    ISA_TEMPLATE_FILE = './src/isa.h.in'
    ISA_HEADER_FILE = './src/isa.h'
    
    ENTRY_FORMAT =  ('// %MNEMONIC%\n'
                    '#define INS_ID_%MNEMONIC% %ID%\n'
                    '#define INS_OP_%MNEMONIC% %OP%\n'
                    '#define INS_OP_MASK_%MNEMONIC% %MASK%\n'
                    '#define INS_LEN_%MNEMONIC% %LENGTH%\n'
                    '#define INS_STR_%MNEMONIC% %STR%\n')

    OPERAND_ENTRY_FORMAT = ('#define INS_OPERAND_%MNEMONIC%_%OPERAND% %OPERAND_ID%\n')

    # Calculates a hash digest of the ISA, which should be deterministic and
    # should not depend on whitespace in isa.yaml, ordering of instruction etc
    def calculate_isa_hash(instructions):
        # These properties matter when calculating ISA hash, in addition to the mnemonic    
        hashable_properties = ['op', 'mask', 'length', 'operands']

        mnemonics_sorted = sorted(list(instructions.keys()))

        buffer = ''

        for idx in range(0, len(mnemonics_sorted)):
            mnemonic = mnemonics_sorted[idx]

            buffer += str(mnemonic) + ':'

            for prop_idx in range(0, len(hashable_properties)):
                prop_name = hashable_properties[prop_idx]

                prop = instructions[mnemonic][prop_name]

                if type(prop) == str:
                    buffer += prop
                elif type(prop) == list:
                    buffer += prop_name + '[' + ':'.join([ str(x) for x in prop ]) + ']'
                else:
                    buffer += str(prop)

                buffer += ':'

            buffer += ':'

        print('ISA_STRING: ' + buffer)

        result_hash = sha256(buffer.encode('ascii')).hexdigest()

        print('ISA_HASH: ' + result_hash)

        return result_hash


    def hash_to_defines(hash_hex):
        hash_bytes = bytes.fromhex(hash_hex)

        # Split hash into 8 parts of 4 bytes
        parts = [ hash_bytes[i:i + 4] for i in range(0, 8) ]

        # Unpack each part as a 32 uint
        uints = [ unpack('<I', part)[0] for part in parts ]

        # Create a define string for each
        defines = [ '#define ISA_HASH{} {}\n'.format(str(i), hex(uint)) for i, uint in enumerate(uints) ]

        return '// ISA version hash (SHA256)\n' + ''.join(defines).strip()

    def instructions_to_defines(instructions):
        buffer = ''

        for i, mnemonic in enumerate(instructions.keys()):
            props = instructions[mnemonic]
    
            buffer += ENTRY_FORMAT \
                .replace('%MNEMONIC%', mnemonic) \
                .replace('%ID%', str(i + 1)) \
                .replace('%OP%', hex(props['op'])) \
                .replace('%MASK%', hex(props['mask'])) \
                .replace('%LENGTH%', str(props['length'])) \
                .replace('%STR%', '"' + mnemonic.lower() + '"')

            buffer += '\n'

            for operand_i, operand in enumerate(props['operands']):
                buffer += OPERAND_ENTRY_FORMAT \
                    .replace('%MNEMONIC%', mnemonic) \
                    .replace('%OPERAND%', operand.upper()) \
                    .replace('%OPERAND_ID%', str(operand_i))

            buffer += '\n'

        return buffer.strip()

    
    with open(ISA_YAML_FILE, 'r') as infile:
        try:
            instructions = yaml.safe_load(infile)

            isa_hash = calculate_isa_hash(instructions)
            hash_defines = hash_to_defines(isa_hash)

            instructions_defines = instructions_to_defines(instructions)

            template = open(ISA_TEMPLATE_FILE, 'r').read()
    
            result = template \
                .replace('%ENTRIES%', instructions_defines) \
                .replace('%HASH_PARTS%', hash_defines)
    
            with open(ISA_HEADER_FILE, 'w') as outfile:
                outfile.write(result)
    
        except yaml.YAMLError as exc:
            print(exc)

targets = {
    'src/isa.h': generate_isa_header
}
