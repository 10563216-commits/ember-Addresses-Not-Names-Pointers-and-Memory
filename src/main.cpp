// main.cpp — мінімальна оболонка `ember`: команди Lab 1 (set/get/dump) + Lab 2 (reg/regs/step/alu).
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define EMBER_ISATTY(fd) _isatty(fd)
#define EMBER_STDIN_FD 0
#else
#include <unistd.h>
#define EMBER_ISATTY(fd) isatty(fd)
#define EMBER_STDIN_FD STDIN_FILENO
#endif

#include "alu.hpp"
#include "cpu.hpp"
#include "memory.hpp"

// Число: десяткове, 0x.. шістнадцяткове або 0b.. двійкове.
static bool parse_number(const std::string& s, long& out) {
    std::string t = s;
    int base = 10;
    if (t.size() > 2 && t[0] == '0' && (t[1] == 'x' || t[1] == 'X')) {
        base = 16;
        t = t.substr(2);
    } else if (t.size() > 2 && t[0] == '0' && (t[1] == 'b' || t[1] == 'B')) {
        base = 2;
        t = t.substr(2);
    }
    if (t.empty()) return false;
    char* end = nullptr;
    long v = std::strtol(t.c_str(), &end, base);
    if (*end != '\0') return false;
    out = v;
    return true;
}

static void print_hex(unsigned value, int width) {
    std::ios_base::fmtflags saved = std::cout.flags();
    std::cout << std::hex << std::setfill('0') << std::setw(width) << value;
    std::cout.flags(saved);
}

static void print_help() {
    std::cout <<
        "set <addr> <byte> [byte...]   записати байти з адреси\n"
        "get <addr>                    прочитати байт\n"
        "dump <addr> [len]             шістнадцятковий дамп (len за замовч. 16)\n"
        "reg <a|b> <0..255>            покласти значення в регістр\n"
        "regs                          надрукувати PC A B Z N C\n"
        "step [n]                      виконати n інструкцій (за замовч. 1)\n"
        "alu <op> <a> [b]              ALU без процесора: add sub and or xor | not shl shr inc dec\n"
        "help, quit\n";
}

// step з трасуванням: «адреса: байт мнемоніка», потім нові регістри.
static void do_step(CPU& cpu) {
    if (cpu.halted) {
        step(cpu);
        return;
    }
    std::uint16_t pc = cpu.pc;
    Byte op = mem_get(*cpu.mem, pc);
    print_hex(pc, 4);
    std::cout << ": ";
    print_hex(op, 2);
    std::cout << "  " << mnemonic(op) << '\n';
    step(cpu);
    dump_regs(cpu);
}

static bool run_alu(const std::string& op, Byte a, Byte b, AluResult& r) {
    if (op == "add") r = alu_add(a, b);
    else if (op == "sub") r = alu_sub(a, b);
    else if (op == "and") r = alu_and(a, b);
    else if (op == "or")  r = alu_or(a, b);
    else if (op == "xor") r = alu_xor(a, b);
    else if (op == "not") r = alu_not(a);
    else if (op == "shl") r = alu_shl(a);
    else if (op == "shr") r = alu_shr(a);
    else if (op == "inc") r = alu_inc(a, false);
    else if (op == "dec") r = alu_dec(a, false);
    else return false;
    return true;
}

int main() {
#ifdef _WIN32
#include <windows.h>
#endif
    Memory mem;
    CPU cpu;
    cpu.mem = &mem;

    const bool interactive = EMBER_ISATTY(EMBER_STDIN_FD) != 0;
    std::string line;
    while (true) {
        if (interactive) std::cout << "ember> " << std::flush;
        if (!std::getline(std::cin, line)) break;

        std::istringstream words(line);
        std::string cmd;
        if (!(words >> cmd)) continue;

        if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "set") {
            std::string a, w;
            long addr = 0, v = 0;
            if (!(words >> a) || !parse_number(a, addr) || addr < 0 || addr >= MEM_SIZE) {
                std::cout << "usage: set <addr 0..0xFFF> <byte> [byte...]\n";
                continue;
            }
            int count = 0;
            bool ok = true;
            while (words >> w) {
                if (!parse_number(w, v) || v < 0 || v > 255 || addr + count >= MEM_SIZE) {
                    ok = false;
                    break;
                }
                mem_set(mem, (std::uint16_t)(addr + count), (Byte)v);
                ++count;
            }
            if (!ok || count == 0) std::cout << "usage: set <addr 0..0xFFF> <byte 0..255> [byte...]\n";
        } else if (cmd == "get") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr) || addr < 0 || addr >= MEM_SIZE) {
                std::cout << "usage: get <addr 0..0xFFF>\n";
                continue;
            }
            print_hex(mem_get(mem, (std::uint16_t)addr), 2);
            std::cout << '\n';
        } else if (cmd == "dump") {
            std::string a, l;
            long addr = 0, len = 16;
            if (!(words >> a) || !parse_number(a, addr) || addr < 0 || addr >= MEM_SIZE ||
                ((words >> l) && (!parse_number(l, len) || len <= 0))) {
                std::cout << "usage: dump <addr> [len]\n";
                continue;
            }
            for (long i = 0; i < len && addr + i < MEM_SIZE; ++i) {
                if (i % 16 == 0) {
                    if (i != 0) std::cout << '\n';
                    print_hex((unsigned)(addr + i), 4);
                    std::cout << ':';
                }
                std::cout << ' ';
                print_hex(mem_get(mem, (std::uint16_t)(addr + i)), 2);
            }
            std::cout << '\n';
        } else if (cmd == "reg") {
            std::string r, v;
            long value = 0;
            if (!(words >> r) || !(words >> v) || !parse_number(v, value) || value < 0 ||
                value > 255) {
                std::cout << "usage: reg <a|b> <0..255>\n";
            } else if (r == "a") {
                cpu.a = (Byte)value;
            } else if (r == "b") {
                cpu.b = (Byte)value;
            } else {
                std::cout << "unknown register: " << r << '\n';
            }
        } else if (cmd == "regs") {
            dump_regs(cpu);
        } else if (cmd == "step") {
            std::string n;
            long count = 1;
            if ((words >> n) && (!parse_number(n, count) || count <= 0)) {
                std::cout << "usage: step [n]\n";
                continue;
            }
            for (long i = 0; i < count; ++i) {
                do_step(cpu);
                if (cpu.halted) {
                    if (i + 1 < count) std::cout << "halted\n";
                    break;
                }
            }
        } else if (cmd == "alu") {
            std::string op, x, y;
            long a = 0, b = 0;
            bool binary = false;
            if (!(words >> op) || !(words >> x) || !parse_number(x, a) || a < 0 || a > 255) {
                std::cout << "usage: alu <op> <a> [b]\n";
                continue;
            }
            binary = (op == "add" || op == "sub" || op == "and" || op == "or" || op == "xor");
            if (binary && (!(words >> y) || !parse_number(y, b) || b < 0 || b > 255)) {
                std::cout << "usage: alu " << op << " <a 0..255> <b 0..255>\n";
                continue;
            }
            AluResult r;
            if (!run_alu(op, (Byte)a, (Byte)b, r)) {
                std::cout << "unknown alu op: " << op << '\n';
                continue;
            }
            std::cout << "result=" << (int)r.value << "  Z=" << r.f.z << " N=" << r.f.n
                      << " C=" << r.f.c << '\n';
        } else {
            std::cout << "unknown command: " << cmd << " (try: help)\n";
        }
    }
    return 0;
}
