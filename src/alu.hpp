// alu.hpp — арифметико-логічний пристрій над uint8_t.
// Кожна функція повертає значення і прапорці Z/N/C за правилами ISA.uk.md §3 і §5.
#pragma once
#include "cpu.hpp"

struct AluResult {
    Byte value = 0;
    Flags f;
};

AluResult alu_add(Byte a, Byte b);   // Z N C: C = перенесення з сьомого біта
AluResult alu_sub(Byte a, Byte b);   // Z N C: C = позика (a < b)
AluResult alu_and(Byte a, Byte b);   // Z N, C = 0
AluResult alu_or (Byte a, Byte b);   // Z N, C = 0
AluResult alu_xor(Byte a, Byte b);   // Z N, C = 0
AluResult alu_not(Byte a);           // Z N, C = 0
AluResult alu_shl(Byte a);           // Z N C: C = біт, що виїхав із сьомої позиції
AluResult alu_shr(Byte a);           // Z N C: C = біт, що виїхав із нульової

// INC і DEC чіпають лише Z і N: C лишається таким, яким був (carry_in).
AluResult alu_inc(Byte a, bool carry_in);
AluResult alu_dec(Byte a, bool carry_in);
