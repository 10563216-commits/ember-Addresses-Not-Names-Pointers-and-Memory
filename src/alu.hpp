#pragma once
#include "cpu.hpp"

struct AluResult {
    Byte value = 0;
    Flags f;
};

AluResult alu_add(Byte a, Byte b);
AluResult alu_sub(Byte a, Byte b);
AluResult alu_and(Byte a, Byte b);
AluResult alu_or (Byte a, Byte b);
AluResult alu_xor(Byte a, Byte b);
AluResult alu_not(Byte a);
AluResult alu_shl(Byte a);
AluResult alu_shr(Byte a);

AluResult alu_inc(Byte a, bool carry_in);
AluResult alu_dec(Byte a, bool carry_in);
