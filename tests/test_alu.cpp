// test_alu.cpp — самоперевірка ALU, decode() і step(). Запуск: ./build/test_alu
#include <iostream>

#include "alu.hpp"
#include "cpu.hpp"
#include "memory.hpp"

static int failures = 0;

#define CHECK(cond)                                                         \
    do {                                                                    \
        if (!(cond)) {                                                      \
            std::cout << "FAIL " << __FILE__ << ":" << __LINE__ << "  " #cond "\n"; \
            ++failures;                                                     \
        }                                                                   \
    } while (0)

static void check_flags(const AluResult& r, bool c) {
    CHECK(r.f.z == (r.value == 0));
    CHECK(r.f.n == ((r.value & 0x80) != 0));
    CHECK(r.f.c == c);
}

static void test_alu_exhaustive() {
    for (int a = 0; a < 256; ++a) {
        for (int b = 0; b < 256; ++b) {
            Byte A = (Byte)a, B = (Byte)b;

            AluResult add = alu_add(A, B);
            CHECK(add.value == (Byte)(a + b));
            check_flags(add, a + b > 255);

            AluResult sub = alu_sub(A, B);
            CHECK(sub.value == (Byte)(a - b));
            check_flags(sub, a < b);

            check_flags(alu_and(A, B), false);
            CHECK(alu_and(A, B).value == (Byte)(a & b));
            check_flags(alu_or(A, B), false);
            CHECK(alu_or(A, B).value == (Byte)(a | b));
            check_flags(alu_xor(A, B), false);
            CHECK(alu_xor(A, B).value == (Byte)(a ^ b));
        }
        Byte A = (Byte)a;
        CHECK(alu_not(A).value == (Byte)(255 - a));
        check_flags(alu_not(A), false);

        CHECK(alu_shl(A).value == (Byte)(a * 2));
        check_flags(alu_shl(A), a >= 128);
        CHECK(alu_shr(A).value == (Byte)(a / 2));
        check_flags(alu_shr(A), (a & 1) != 0);

        for (int c = 0; c < 2; ++c) {
            CHECK(alu_inc(A, c != 0).value == (Byte)(a + 1));
            check_flags(alu_inc(A, c != 0), c != 0);   // C не змінюється
            CHECK(alu_dec(A, c != 0).value == (Byte)(a - 1));
            check_flags(alu_dec(A, c != 0), c != 0);
        }
    }
}

static void test_worked_examples() {
    AluResult r = alu_add(200, 100);   // 300 = 256 + 44
    CHECK(r.value == 44 && !r.f.z && !r.f.n && r.f.c);
    r = alu_add(255, 1);               // загортання в нуль
    CHECK(r.value == 0 && r.f.z && !r.f.n && r.f.c);
    r = alu_sub(5, 10);                // позика: 5 - 10 = 251
    CHECK(r.value == 251 && !r.f.z && r.f.n && r.f.c);
    r = alu_shl(0x81);                 // біт 7 виїжджає в C
    CHECK(r.value == 0x02 && r.f.c);
    r = alu_and(7, 1);
    CHECK(r.value == 1 && !r.f.z && !r.f.c);
}

static void test_decode() {
    Decoded d = decode(0xA3);
    CHECK(d.group == 0xA && d.index == 0x3);
    d = decode(OP_ADD);
    CHECK(d.group == 0x1 && d.index == 0x0);
    d = decode(OP_HALT);
    CHECK(d.group == 0x0 && d.index == 0x0);
}

static void load(Memory& m, const Byte* prog, int n) {
    for (int i = 0; i < n; ++i) mem_set(m, (std::uint16_t)i, prog[i]);
}

static void test_step() {
    // ADD, HALT при A=200, B=100.
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    const Byte prog1[] = {OP_ADD, OP_HALT};
    load(m, prog1, 2);
    cpu.a = 200;
    cpu.b = 100;
    step(cpu);
    CHECK(cpu.a == 44 && cpu.f.c && !cpu.f.z && cpu.pc == 1 && !cpu.halted);
    step(cpu);
    CHECK(cpu.halted && cpu.pc == 2);
    step(cpu);   // після HALT відмовляє: PC не рухається
    CHECK(cpu.pc == 2);

    // AND, HALT при A=7, B=1 (M3 лаби).
    Memory m2;
    CPU c2;
    c2.mem = &m2;
    const Byte prog2[] = {OP_AND, OP_HALT};
    load(m2, prog2, 2);
    c2.a = 7;
    c2.b = 1;
    step(c2);
    CHECK(c2.a == 1 && !c2.f.z && !c2.f.n && !c2.f.c && c2.pc == 1);

    // INC не чіпає C: ADD виставляє C=1, INC лишає його.
    Memory m3;
    CPU c3;
    c3.mem = &m3;
    const Byte prog3[] = {OP_ADD, OP_INC, OP_NOP, OP_DEC, OP_HALT};
    load(m3, prog3, 5);
    c3.a = 200;
    c3.b = 100;
    step(c3);   // A=44, C=1
    step(c3);   // A=45, C лишається 1
    CHECK(c3.a == 45 && c3.f.c);
    step(c3);   // NOP: нічого не змінюється, крім PC
    CHECK(c3.a == 45 && c3.f.c && c3.pc == 3);
    step(c3);
    CHECK(c3.a == 44 && c3.f.c && c3.pc == 4);

    // Невідомий опкод поки що діє як NOP на один байт.
    Memory m4;
    CPU c4;
    c4.mem = &m4;
    mem_set(m4, 0, 0xEE);
    step(c4);
    CHECK(c4.pc == 1 && !c4.halted);
}

int main() {
    test_alu_exhaustive();
    test_worked_examples();
    test_decode();
    test_step();
    if (failures == 0) {
        std::cout << "all tests passed\n";
        return 0;
    }
    std::cout << failures << " check(s) failed\n";
    return 1;
}
