#!/usr/bin/python

# 0x0000: sti %gpr2, $0xdd0000ff;
# 0x0005: sti %gpr1, $0xddbb00ff;
# 0x000a: sti %gpr0, $0x0;
# 0x000f: nop;
# 0x0010: push %gpr2;
# 0x0011: push %gpr1;
# 0x0012: pop %gpr2;
# 0x0013: pop %gpr1;
# 0x0014: mov %gpr4, %gpr2;    // dst first, src second
# 0x0016: xor %gpr4, %gpr1;
# 0x0018: add %gpr2, %gpr4;
# 0x001a: jmpabs $0x14;        // This loops infinitely!

import sys

bytecode = [0x72, 0xff, 0x00, 0x00, 0xdd, 0x71, 0xff, 0x00, 0xbb, 0xdd,0x70, 0x00, 0x00, 0x00, 0x00, 0x90, 0x52, 0x51, 0x62, 0x61, 0xc0, 0x42, 0xa0, 0x41, 0xa1, 0x24, 0x91, 0x14, 0x00, 0x00, 0x00]

sys.stdout.buffer.write(bytes(bytecode))
