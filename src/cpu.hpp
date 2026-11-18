// cpu.hpp — ДАНО в лабі (коди операцій, Flags, CPU), плюс decode() і mnemonic().
#pragma once
#include <cstdint>
#include "memory.hpp"

// Коди операцій — саме з ISA.uk.md, свого не вигадуємо.
const Byte OP_HALT = 0x00;
const Byte OP_NOP  = 0x01;
const Byte OP_ADD  = 0x10;
const Byte OP_SUB  = 0x11;
const Byte OP_AND  = 0x12;
const Byte OP_OR   = 0x13;
const Byte OP_XOR  = 0x14;
const Byte OP_NOT  = 0x15;
const Byte OP_SHL  = 0x16;
const Byte OP_SHR  = 0x17;
const Byte OP_INC  = 0x18;
const Byte OP_DEC  = 0x19;

struct Flags {
    bool z = false;   // результат нульовий
    bool n = false;   // старший біт результату — одиниця
    bool c = false;   // біт, який виїхав за межу восьми
};

struct CPU {
    Memory* mem = nullptr;   // де лежить коробка
    std::uint16_t pc = 0;    // адреса НАСТУПНОЇ інструкції
    Byte a = 0;
    Byte b = 0;
    Flags f;
    bool halted = false;     // після HALT наступні step відмовляють
};

// Опкод = група (старший півбайт) + індекс (молодший), див. ISA.uk.md §4.
struct Decoded {
    Byte group;   // (op >> 4) & 0x0F
    Byte index;   // op & 0x0F
};

Decoded decode(Byte op);
const char* mnemonic(Byte op);   // для трасування; невідомий опкод -> "???"

void step(CPU& cpu);
void dump_regs(const CPU& cpu);
