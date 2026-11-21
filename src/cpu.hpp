// cpu.hpp — ДАНО в лабі (коди операцій, Flags, CPU), плюс decode(), mnemonic(),
// instr_size() і disasm().
#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>
#include "memory.hpp"

// Коди операцій — саме з ISA.uk.md, свого не вигадуємо.
const Byte OP_HALT = 0x00;
const Byte OP_NOP  = 0x01;
const Byte OP_OUT  = 0x02;   // надрукувати A як символ
const Byte OP_OUTN = 0x03;   // надрукувати A як десяткове число і пробіл
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

// Група 0x2_ — пересилання даних (ISA.uk.md §5).
const Byte OP_LOADI_A  = 0x20;   // A = imm8                  (2 байти)
const Byte OP_LOADI_B  = 0x21;   // B = imm8                  (2)
const Byte OP_LOAD_A   = 0x22;   // A = mem[addr16]           (3)
const Byte OP_LOAD_B   = 0x23;   // B = mem[addr16]           (3)
const Byte OP_STORE_A  = 0x24;   // mem[addr16] = A           (3)
const Byte OP_STORE_B  = 0x25;   // mem[addr16] = B           (3)
const Byte OP_MOV_AB   = 0x26;   // A = B                     (1)
const Byte OP_MOV_BA   = 0x27;   // B = A                     (1)
const Byte OP_LOADH    = 0x28;   // H = imm16                 (3)
const Byte OP_LOAD_AH  = 0x29;   // A = mem[H]                (1)
const Byte OP_STORE_HA = 0x2A;   // mem[H] = A                (1)
const Byte OP_INCH     = 0x2B;   // H = H + 1                 (1)
const Byte OP_DECH     = 0x2C;   // H = H - 1                 (1)

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
    std::uint16_t h = 0;     // адресний регістр: тримає АДРЕСУ гостя, а не значення (Lab 3)
    Flags f;
    bool halted = false;     // після HALT або помилки наступні step відмовляють

    // Непорожній рядок — машина зупинилась через помилку (вихід за межі пам'яті).
    // PC при цьому лишається на інструкції, яка помилилась.
    std::string fault;

    // Куди друкують OUT і OUTN. nullptr — нікуди (так тести не засмічують екран).
    std::ostream* out = nullptr;
    unsigned long printed = 0;   // скільки разів щось надруковано
};

// Опкод = група (старший півбайт) + індекс (молодший), див. ISA.uk.md §4.
struct Decoded {
    Byte group;   // (op >> 4) & 0x0F
    Byte index;   // op & 0x0F
};

Decoded decode(Byte op);
const char* mnemonic(Byte op);   // для трасування; невідомий опкод -> "???"

// Розмір інструкції в байтах — рівно на стільки рухається PC (ISA.uk.md §5).
// Невідомий опкод поки діє як NOP на один байт, тож і розмір 1.
int instr_size(Byte op);

// Рядок для трасування: «байти  мнемоніка з операндами», напр. «28 00 08  LOADH H, 0x0800».
// Читає пам'ять тільки в межах MEM_SIZE.
std::string disasm(const Memory& mem, std::uint16_t addr);

void step(CPU& cpu);

// Крокує до HALT або до помилки (cpu.fault). Повертає кількість виконаних кроків.
unsigned long run(CPU& cpu);

void dump_regs(const CPU& cpu);
