#include "alu.hpp"

static AluResult make(Byte value, bool carry) {
    AluResult r;
    r.value = value;
    r.f.z = (value == 0);
    r.f.n = ((value >> 7) & 1) != 0;
    r.f.c = carry;
    return r;
}

AluResult alu_add(Byte a, Byte b) {
    unsigned sum = (unsigned)a + (unsigned)b;
    return make((Byte)(sum & 0xFF), ((sum >> 8) & 1) != 0);
}

AluResult alu_sub(Byte a, Byte b) {
    return make((Byte)((unsigned)a - (unsigned)b), a < b);
}

AluResult alu_and(Byte a, Byte b) { return make((Byte)(a & b), false); }
AluResult alu_or (Byte a, Byte b) { return make((Byte)(a | b), false); }
AluResult alu_xor(Byte a, Byte b) { return make((Byte)(a ^ b), false); }

AluResult alu_not(Byte a) { return make((Byte)(~a), false); }

AluResult alu_shl(Byte a) { return make((Byte)(a << 1), (a & 0x80) != 0); }
AluResult alu_shr(Byte a) { return make((Byte)(a >> 1), (a & 0x01) != 0); }

AluResult alu_inc(Byte a, bool carry_in) { return make((Byte)(a + 1), carry_in); }
AluResult alu_dec(Byte a, bool carry_in) { return make((Byte)(a - 1), carry_in); }
