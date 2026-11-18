// cpu.cpp — форма step() дана в лабі; тут її case для групи 0x1_, decode() і dump_regs().
#include "cpu.hpp"
#include "alu.hpp"

#include <iomanip>
#include <iostream>

// Поле, а не магічне число на кожну інструкцію: група — старший півбайт.
Decoded decode(Byte op) {
    Decoded d;
    d.group = (Byte)((op >> 4) & 0x0F);
    d.index = (Byte)(op & 0x0F);
    return d;
}

const char* mnemonic(Byte op) {
    switch (op) {
    case OP_HALT: return "HALT";
    case OP_NOP:  return "NOP";
    case OP_ADD:  return "ADD A, B";
    case OP_SUB:  return "SUB A, B";
    case OP_AND:  return "AND A, B";
    case OP_OR:   return "OR A, B";
    case OP_XOR:  return "XOR A, B";
    case OP_NOT:  return "NOT A";
    case OP_SHL:  return "SHL A";
    case OP_SHR:  return "SHR A";
    case OP_INC:  return "INC A";
    case OP_DEC:  return "DEC A";
    default:      return "???";
    }
}

// Група 0x1_ (ALU). Усі інструкції тут мають розмір 1 (ISA.uk.md §5),
// тож і PC рухається рівно на 1. CMP (0x1A) і MUL (0x1B) додадуть пізніші лаби.
static void exec_alu(CPU& cpu, Byte op) {
    AluResult r;
    switch (op) {
    case OP_ADD: r = alu_add(cpu.a, cpu.b); break;
    case OP_SUB: r = alu_sub(cpu.a, cpu.b); break;
    case OP_AND: r = alu_and(cpu.a, cpu.b); break;
    case OP_OR:  r = alu_or (cpu.a, cpu.b); break;
    case OP_XOR: r = alu_xor(cpu.a, cpu.b); break;
    case OP_NOT: r = alu_not(cpu.a);        break;
    case OP_SHL: r = alu_shl(cpu.a);        break;
    case OP_SHR: r = alu_shr(cpu.a);        break;
    case OP_INC: r = alu_inc(cpu.a, cpu.f.c); break;   // C не чіпаємо
    case OP_DEC: r = alu_dec(cpu.a, cpu.f.c); break;   // C не чіпаємо
    default:
        cpu.pc += 1;   // поки що як NOP; у Lab 4 це стане помилкою
        return;
    }
    cpu.a = r.value;
    cpu.f = r.f;
    cpu.pc += 1;
}

void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    Byte op = mem_get(*cpu.mem, cpu.pc);

    switch (decode(op).group) {
    case 0x0:   // керування
        switch (op) {
        case OP_HALT:
            cpu.halted = true;
            cpu.pc += 1;          // розмір HALT з таблиці — 1
            break;
        case OP_NOP:
            cpu.pc += 1;
            break;
        default:
            cpu.pc += 1;          // 0x02..0x04 додадуть пізніші лаби
            break;
        }
        break;
    case 0x1:   // ALU
        exec_alu(cpu, op);
        break;
    default:
        cpu.pc += 1;              // поки що як NOP; у Lab 4 це стане помилкою
        break;
    }
}

void dump_regs(const CPU& cpu) {
    std::ios_base::fmtflags saved = std::cout.flags();
    std::cout << "PC=0x" << std::hex << std::setfill('0') << std::setw(4) << cpu.pc
              << std::dec << std::setfill(' ')
              << "  A=" << (int)cpu.a << " (0x" << std::hex << std::setfill('0')
              << std::setw(2) << (int)cpu.a << ")"
              << std::dec << std::setfill(' ')
              << "  B=" << (int)cpu.b << " (0x" << std::hex << std::setfill('0')
              << std::setw(2) << (int)cpu.b << ")"
              << std::dec << std::setfill(' ')
              << "  Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c << '\n';
    std::cout.flags(saved);
}
