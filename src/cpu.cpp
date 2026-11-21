// cpu.cpp — форма step() дана в лабі; тут її case для груп 0x1_ і 0x2_, decode(),
// instr_size(), disasm() і dump_regs().
#include "cpu.hpp"
#include "alu.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>

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
    case OP_OUT:  return "OUT";
    case OP_OUTN: return "OUTN";
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
    case OP_LOADI_A:  return "LOADI A, imm8";
    case OP_LOADI_B:  return "LOADI B, imm8";
    case OP_LOAD_A:   return "LOAD A, [addr16]";
    case OP_LOAD_B:   return "LOAD B, [addr16]";
    case OP_STORE_A:  return "STORE [addr16], A";
    case OP_STORE_B:  return "STORE [addr16], B";
    case OP_MOV_AB:   return "MOV A, B";
    case OP_MOV_BA:   return "MOV B, A";
    case OP_LOADH:    return "LOADH H, imm16";
    case OP_LOAD_AH:  return "LOAD A, [H]";
    case OP_STORE_HA: return "STORE [H], A";
    case OP_INCH:     return "INCH";
    case OP_DECH:     return "DECH";
    default:      return "???";
    }
}

// Розміри — з ISA.uk.md §5, стовпець «Розмір». Усе, чого тут немає, — 1 байт.
int instr_size(Byte op) {
    switch (op) {
    case OP_LOADI_A:
    case OP_LOADI_B:
        return 2;                       // опкод + imm8
    case OP_LOAD_A:
    case OP_LOAD_B:
    case OP_STORE_A:
    case OP_STORE_B:
    case OP_LOADH:
        return 3;                       // опкод + 16-бітний операнд, молодший уперед
    default:
        return 1;
    }
}

static std::string hex_str(unsigned value, int width) {
    std::ostringstream os;
    os << "0x" << std::hex << std::setfill('0') << std::setw(width) << value;
    return os.str();
}

std::string disasm(const Memory& mem, std::uint16_t addr) {
    if (addr >= MEM_SIZE) return "(поза пам'яттю)";
    Byte op = mem_get(mem, addr);
    int size = instr_size(op);

    std::ostringstream os;
    os << std::hex << std::setfill('0');
    for (int i = 0; i < size; ++i) {
        if (addr + i >= MEM_SIZE) { os << (i ? " ??" : "??"); continue; }
        os << (i ? " " : "") << std::setw(2) << (int)mem_get(mem, (std::uint16_t)(addr + i));
    }
    os << "  ";

    // Операнди читаємо лише якщо вони вміщаються в пам'ять.
    bool fits = addr + size <= MEM_SIZE;
    unsigned imm8  = (fits && size == 2) ? mem_get(mem, (std::uint16_t)(addr + 1)) : 0;
    unsigned imm16 = (fits && size == 3) ? get16(mem, (std::size_t)addr + 1) : 0;

    switch (op) {
    case OP_LOADI_A:  os << "LOADI A, " << hex_str(imm8, 2);          break;
    case OP_LOADI_B:  os << "LOADI B, " << hex_str(imm8, 2);          break;
    case OP_LOAD_A:   os << "LOAD A, [" << hex_str(imm16, 4) << "]";  break;
    case OP_LOAD_B:   os << "LOAD B, [" << hex_str(imm16, 4) << "]";  break;
    case OP_STORE_A:  os << "STORE [" << hex_str(imm16, 4) << "], A"; break;
    case OP_STORE_B:  os << "STORE [" << hex_str(imm16, 4) << "], B"; break;
    case OP_LOADH:    os << "LOADH H, " << hex_str(imm16, 4);         break;
    default:          os << mnemonic(op);                          break;
    }
    return os.str();
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

// Зупинка через помилку. PC не рухаємо: він лишається на інструкції, яка помилилась,
// тож `regs` після збою показує, де саме.
static void fail(CPU& cpu, const std::string& msg) {
    cpu.fault = msg;
    cpu.halted = true;
}

// Адреса гостя — індекс у коробці. Усе від MEM_SIZE і вище не загортаємо, а відхиляємо
// (Lab 3, розділ 1: «відхиляйте її й не загортайте»). Це вказівник із перевіркою меж.
static bool addr_ok(CPU& cpu, const char* what, std::uint16_t addr) {
    if (addr < MEM_SIZE) return true;
    fail(cpu, std::string(what) + " at " + hex_str(addr, 4) +
                  ": address out of range (valid 0x000..0xFFF)");
    return false;
}

// Група 0x2_ (пересилання даних). Прапорці НЕ чіпаємо ніде: ні LOAD, ні LOADI
// не виставляють Z (ISA.uk.md §3, «пастка»). Повертає false, якщо сталася помилка;
// тоді PC не рухається.
static bool exec_move(CPU& cpu, Byte op) {
    Memory& m = *cpu.mem;
    // Операнди лежать одразу після опкода. Те, що вони вміщаються, step() уже перевірив.
    const std::size_t at = (std::size_t)cpu.pc + 1;

    switch (op) {
    case OP_LOADI_A: cpu.a = mem_get(m, (std::uint16_t)at); break;
    case OP_LOADI_B: cpu.b = mem_get(m, (std::uint16_t)at); break;

    case OP_LOAD_A:
    case OP_LOAD_B: {
        std::uint16_t addr = get16(m, at);
        if (!addr_ok(cpu, "LOAD", addr)) return false;
        (op == OP_LOAD_A ? cpu.a : cpu.b) = mem_get(m, addr);
        break;
    }
    case OP_STORE_A:
    case OP_STORE_B: {
        std::uint16_t addr = get16(m, at);
        if (!addr_ok(cpu, "STORE", addr)) return false;
        mem_set(m, addr, op == OP_STORE_A ? cpu.a : cpu.b);
        break;
    }
    case OP_MOV_AB: cpu.a = cpu.b; break;
    case OP_MOV_BA: cpu.b = cpu.a; break;

    case OP_LOADH:   cpu.h = get16(m, at); break;      // H = imm16: адреса, а не значення
    case OP_LOAD_AH:                                   // A = *H
        if (!addr_ok(cpu, "LOAD A,[H]", cpu.h)) return false;
        cpu.a = mem_get(m, cpu.h);
        break;
    case OP_STORE_HA:                                  // *H = A
        if (!addr_ok(cpu, "STORE [H],A", cpu.h)) return false;
        mem_set(m, cpu.h, cpu.a);
        break;
    case OP_INCH: cpu.h = (std::uint16_t)(cpu.h + 1); break;   // ++p: на ОДИН байт
    case OP_DECH: cpu.h = (std::uint16_t)(cpu.h - 1); break;   // --p
    default: break;   // 0x2D і далі додадуть пізніші лаби: поки що NOP (див. step)
    }
    return true;
}

void step(CPU& cpu) {
    if (cpu.halted) {
        if (cpu.fault.empty()) std::cout << "halted\n";
        else                   std::cout << "error: " << cpu.fault << '\n';
        return;
    }
    // PC — адреса гостя. Якщо він уже за краєм коробки, читати нічого.
    if (cpu.pc >= MEM_SIZE) {
        fail(cpu, "PC " + hex_str(cpu.pc, 4) + ": ran off the end of memory");
        return;
    }
    Byte op = mem_get(*cpu.mem, cpu.pc);
    int size = instr_size(op);
    // Інструкція в 3 байти на 0xFFE розтяглася б за межу: операнд прочитався б із байта 0.
    if ((std::size_t)cpu.pc + (std::size_t)size > (std::size_t)MEM_SIZE) {
        fail(cpu, "PC " + hex_str(cpu.pc, 4) + ": " + mnemonic(op) +
                      " needs " + std::to_string(size) + " bytes, past the end of memory");
        return;
    }

    switch (decode(op).group) {
    case 0x0:   // керування й вивід
        switch (op) {
        case OP_HALT:
            cpu.halted = true;
            break;
        case OP_OUT:
            if (cpu.out) *cpu.out << (char)cpu.a;
            ++cpu.printed;
            break;
        case OP_OUTN:
            if (cpu.out) *cpu.out << (int)cpu.a << ' ';
            ++cpu.printed;
            break;
        default:   // NOP і 0x04.. (їх додадуть пізніші лаби)
            break;
        }
        cpu.pc = (std::uint16_t)(cpu.pc + size);
        break;
    case 0x1:   // ALU: exec_alu сам рухає PC на 1
        exec_alu(cpu, op);
        break;
    case 0x2:   // пересилання даних
        if (!exec_move(cpu, op)) return;               // помилка: PC лишається
        cpu.pc = (std::uint16_t)(cpu.pc + size);       // розмір з таблиці, а не на око
        break;
    default:
        cpu.pc += 1;              // поки що як NOP; у Lab 4 це стане помилкою
        break;
    }
}

// Виконує до HALT або до помилки. Окремої «стелі» на кількість кроків не ставимо:
// без стрибків (Lab 4) PC лише росте, тож програма скінчиться не більше ніж за MEM_SIZE
// кроків — або HALT, або виходом за край. Коли з'являться стрибки, стеля знадобиться.
unsigned long run(CPU& cpu) {
    unsigned long steps = 0;
    while (!cpu.halted) {
        step(cpu);
        ++steps;
    }
    return steps;
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
              << "  H=0x" << std::hex << std::setfill('0') << std::setw(4) << cpu.h
              << std::dec << std::setfill(' ')
              << "  Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c << '\n';
    std::cout.flags(saved);
}
