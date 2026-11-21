#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>
#include "memory.hpp"

const Byte OP_HALT = 0x00;
const Byte OP_NOP = 0x01;
const Byte OP_OUT = 0x02;
const Byte OP_OUTN = 0x03;
const Byte OP_ADD = 0x10;
const Byte OP_SUB = 0x11;
const Byte OP_AND = 0x12;
const Byte OP_OR = 0x13;
const Byte OP_XOR = 0x14;
const Byte OP_NOT = 0x15;
const Byte OP_SHL = 0x16;
const Byte OP_SHR = 0x17;
const Byte OP_INC = 0x18;
const Byte OP_DEC = 0x19;

const Byte OP_LOADI_A = 0x20;
const Byte OP_LOADI_B = 0x21;
const Byte OP_LOAD_A = 0x22;
const Byte OP_LOAD_B = 0x23;
const Byte OP_STORE_A = 0x24;
const Byte OP_STORE_B = 0x25;
const Byte OP_MOV_AB = 0x26;
const Byte OP_MOV_BA = 0x27;
const Byte OP_LOADH = 0x28;
const Byte OP_LOAD_AH = 0x29;
const Byte OP_STORE_HA = 0x2A;
const Byte OP_INCH = 0x2B;
const Byte OP_DECH = 0x2C;

struct Flags {
    bool z = false;
    bool n = false;
    bool c = false;
};

struct CPU {
    Memory* mem = nullptr;
    std::uint16_t pc = 0;
    Byte a = 0;
    Byte b = 0;
    std::uint16_t h = 0;
    Flags f;
    bool halted = false;

    std::string fault;

    std::ostream* out = nullptr;
    unsigned long printed = 0;
};

struct Decoded {
    Byte group;
    Byte index;
};

Decoded decode(Byte op);
const char* mnemonic(Byte op);

int instr_size(Byte op);

std::string disasm(const Memory& mem, std::uint16_t addr);

void step(CPU& cpu);

unsigned long run(CPU& cpu);

void dump_regs(const CPU& cpu);