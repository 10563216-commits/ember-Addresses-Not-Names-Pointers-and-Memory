#include <iostream>
#include <sstream>
#include <vector>

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

static void put(Memory& m, std::uint16_t at, const std::vector<Byte>& bytes) {
    for (std::size_t i = 0; i < bytes.size(); ++i) mem_set(m, (std::uint16_t)(at + i), bytes[i]);
}

struct Quiet {
    std::ostringstream sink;
    std::streambuf* old;
    Quiet() : old(std::cout.rdbuf(sink.rdbuf())) {}
    ~Quiet() { std::cout.rdbuf(old); }
};

static void test_set16_get16() {
    Memory m;
    CHECK(set16(m, 0, 0x1234));
    CHECK(mem_get(m, 0) == 0x34);
    CHECK(mem_get(m, 1) == 0x12);
    CHECK(get16(m, 0) == 0x1234);

    for (unsigned v = 0; v <= 0xFFFF; ++v) {
        CHECK(set16(m, 100, (std::uint16_t)v));
        CHECK(get16(m, 100) == v);
        CHECK(mem_get(m, 100) == (Byte)(v & 0xFF));
        CHECK(mem_get(m, 101) == (Byte)(v >> 8));
    }

    CHECK(set16(m, MEM_SIZE - 2, 0xBEEF));
    CHECK(mem_get(m, MEM_SIZE - 2) == 0xEF && mem_get(m, MEM_SIZE - 1) == 0xBE);

    Byte last_before = mem_get(m, MEM_SIZE - 1);
    Byte first_before = mem_get(m, 0);
    CHECK(!set16(m, MEM_SIZE - 1, 0x1234));
    CHECK(mem_get(m, MEM_SIZE - 1) == last_before);
    CHECK(mem_get(m, 0) == first_before);
    CHECK(!set16(m, MEM_SIZE, 0x1));
    CHECK(!set16(m, (std::size_t)-1, 0x1));
}

static void test_isa_sizes() {
    CHECK(instr_size(OP_HALT) == 1 && instr_size(OP_NOP) == 1);
    CHECK(instr_size(OP_OUT) == 1 && instr_size(OP_OUTN) == 1);
    CHECK(instr_size(OP_ADD) == 1 && instr_size(OP_DEC) == 1);
    CHECK(instr_size(OP_LOADI_A) == 2 && instr_size(OP_LOADI_B) == 2);
    CHECK(instr_size(OP_LOAD_A) == 3 && instr_size(OP_LOAD_B) == 3);
    CHECK(instr_size(OP_STORE_A) == 3 && instr_size(OP_STORE_B) == 3);
    CHECK(instr_size(OP_MOV_AB) == 1 && instr_size(OP_MOV_BA) == 1);
    CHECK(instr_size(OP_LOADH) == 3);
    CHECK(instr_size(OP_LOAD_AH) == 1 && instr_size(OP_STORE_HA) == 1);
    CHECK(instr_size(OP_INCH) == 1 && instr_size(OP_DECH) == 1);
    CHECK(OP_OUT == 0x02 && OP_OUTN == 0x03);
    CHECK(OP_LOADI_A == 0x20 && OP_LOADI_B == 0x21 && OP_LOAD_A == 0x22 && OP_LOAD_B == 0x23);
    CHECK(OP_STORE_A == 0x24 && OP_STORE_B == 0x25 && OP_MOV_AB == 0x26 && OP_MOV_BA == 0x27);
    CHECK(OP_LOADH == 0x28 && OP_LOAD_AH == 0x29 && OP_STORE_HA == 0x2A);
    CHECK(OP_INCH == 0x2B && OP_DECH == 0x2C);
}

static void test_loadi_load_store() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    put(m, 0, {0x20, 0x41, 0x21, 0x07, 0x24, 0x00, 0x08, 0x25, 0x01, 0x08,
               0x22, 0x01, 0x08, 0x23, 0x00, 0x08, 0x00});
    step(cpu);
    CHECK(cpu.a == 0x41 && cpu.pc == 2);
    step(cpu);
    CHECK(cpu.b == 7 && cpu.pc == 4);
    step(cpu);
    CHECK(mem_get(m, 0x800) == 0x41 && cpu.pc == 7);
    step(cpu);
    CHECK(mem_get(m, 0x801) == 7 && cpu.pc == 10);
    step(cpu);
    CHECK(cpu.a == 7 && cpu.pc == 13);
    step(cpu);
    CHECK(cpu.b == 0x41 && cpu.pc == 16);
    step(cpu);
    CHECK(cpu.halted && cpu.fault.empty());
}

static void test_flags_untouched() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    cpu.f.z = false; cpu.f.n = true; cpu.f.c = true;
    put(m, 0, {0x20, 0x00, 0x21, 0x00, 0x22, 0x00, 0x09, 0x26, 0x27, 0x28, 0x00, 0x09,
               0x29, 0x2A, 0x2B, 0x2C, 0x24, 0x00, 0x09});
    for (int i = 0; i < 11; ++i) step(cpu);
    CHECK(!cpu.f.z && cpu.f.n && cpu.f.c);
    CHECK(cpu.fault.empty());
}

static void test_mov() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    put(m, 0, {OP_MOV_AB, OP_MOV_BA});
    cpu.a = 1; cpu.b = 9;
    step(cpu);
    CHECK(cpu.a == 9 && cpu.b == 9 && cpu.pc == 1);
    cpu.a = 5;
    step(cpu);
    CHECK(cpu.b == 5 && cpu.pc == 2);
}

static void test_h_register() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    std::ostringstream out;
    cpu.out = &out;
    mem_set(m, 0x800, 65);
    mem_set(m, 0x801, 66);
    put(m, 0, {0x28, 0x00, 0x08, 0x29, 0x02, 0x2B, 0x29, 0x02, 0x00});
    step(cpu);
    CHECK(cpu.h == 0x0800 && cpu.pc == 3);
    step(cpu);
    CHECK(cpu.a == 65 && cpu.h == 0x0800 && cpu.pc == 4);
    step(cpu);
    CHECK(out.str() == "A");
    step(cpu);
    CHECK(cpu.h == 0x0801);
    step(cpu);
    step(cpu);
    CHECK(out.str() == "AB" && cpu.a == 66);
    step(cpu);
    CHECK(cpu.halted && cpu.h == 0x0801 && cpu.fault.empty());

    Memory m2;
    CPU c2;
    c2.mem = &m2;
    put(m2, 0, {0x28, 0x10, 0x09, 0x2A, 0x2C, 0x2A});
    c2.a = 0x77;
    for (int i = 0; i < 3; ++i) step(c2);
    CHECK(mem_get(m2, 0x910) == 0x77 && c2.h == 0x90F);
    step(c2);
    CHECK(mem_get(m2, 0x90F) == 0x77);

    Memory m3;
    CPU c3;
    c3.mem = &m3;
    put(m3, 0, {OP_INCH, OP_DECH});
    c3.h = 0xFFFF;
    step(c3);
    CHECK(c3.h == 0 && c3.fault.empty());
    step(c3);
    CHECK(c3.h == 0xFFFF && c3.fault.empty());
}

static void test_out() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    std::ostringstream out;
    cpu.out = &out;
    put(m, 0, {OP_OUTN, OP_OUT, OP_OUTN});
    cpu.a = 120;
    step(cpu);
    CHECK(out.str() == "120 ");
    cpu.a = 'Z';
    step(cpu);
    CHECK(out.str() == "120 Z");
    cpu.a = 0;
    step(cpu);
    CHECK(out.str() == "120 Z0 " && cpu.printed == 3);
    CHECK(cpu.pc == 3);

    Memory m2;
    CPU c2;
    c2.mem = &m2;
    put(m2, 0, {OP_OUT});
    c2.a = 'x';
    step(c2);
    CHECK(c2.pc == 1 && c2.fault.empty());
}

static void test_faults() {
    Quiet quiet;

    Memory m;
    CPU cpu;
    cpu.mem = &m;
    mem_set(m, 0, 0x55);
    put(m, 0, {0x22, 0x00, 0x10});
    cpu.a = 0x11;
    step(cpu);
    CHECK(cpu.halted && !cpu.fault.empty());
    CHECK(cpu.a == 0x11);
    CHECK(cpu.pc == 0);
    unsigned long steps = run(cpu);
    CHECK(steps == 0);


    Memory m2;
    CPU c2;
    c2.mem = &m2;
    put(m2, 0, {0x24, 0xFF, 0xFF});
    c2.a = 0x99;
    step(c2);
    CHECK(c2.halted && !c2.fault.empty());
    bool untouched = true;
    for (int i = 3; i < MEM_SIZE; ++i) untouched = untouched && mem_get(m2, (std::uint16_t)i) == 0;
    CHECK(untouched);

    Memory m3;
    CPU c3;
    c3.mem = &m3;
    put(m3, 0, {OP_LOAD_AH});
    c3.h = MEM_SIZE;
    step(c3);
    CHECK(c3.halted && !c3.fault.empty() && c3.pc == 0);

    Memory m4;
    CPU c4;
    c4.mem = &m4;
    put(m4, 0, {OP_STORE_HA});
    c4.h = 0xFFFF;
    step(c4);
    CHECK(c4.halted && !c4.fault.empty());

    Memory m5;
    CPU c5;
    c5.mem = &m5;
    mem_set(m5, 0xFFE, OP_LOADH);
    mem_set(m5, 0xFFF, 0x34);
    c5.pc = 0xFFE;
    step(c5);
    CHECK(c5.halted && !c5.fault.empty() && c5.pc == 0xFFE && c5.h == 0);

    Memory m6;
    CPU c6;
    c6.mem = &m6;
    mem_set(m6, 0xFFF, OP_LOADI_A);
    c6.pc = 0xFFF;
    step(c6);
    CHECK(c6.halted && !c6.fault.empty() && c6.a == 0);

    Memory m7;
    CPU c7;
    c7.mem = &m7;
    mem_set(m7, 0xFFF, OP_NOP);
    mem_set(m7, 0, OP_INC);
    c7.pc = 0xFFF;
    step(c7);
    CHECK(!c7.halted && c7.pc == 0x1000);
    step(c7);
    CHECK(c7.halted && !c7.fault.empty() && c7.a == 0);


    Memory m8;
    CPU c8;
    c8.mem = &m8;
    step(c8);
    CHECK(c8.halted && c8.fault.empty());
}

static void test_run_and_regions() {
    Memory m;
    CPU cpu;
    cpu.mem = &m;
    std::ostringstream out;
    cpu.out = &out;
    put(m, DATA_LO, {65, 66, 67, 0});
    put(m, CODE_LO, {OP_LOAD_A, 0x00, 0x08, OP_OUT, OP_HALT});
    unsigned long steps = run(cpu);
    CHECK(steps == 3 && out.str() == "A" && cpu.halted && cpu.fault.empty());
    CHECK(cpu.pc == 5);

    Memory m2;
    CPU c2;
    c2.mem = &m2;
    std::ostringstream out2;
    c2.out = &out2;
    put(m2, DATA_LO, {65, 66, 67, 0});
    put(m2, CODE_LO, {OP_LOADH, 0x00, 0x08, OP_LOAD_AH, OP_OUT, OP_HALT});
    steps = run(c2);
    CHECK(steps == 4 && out2.str() == "A");

    Quiet quiet;
    Memory m3;
    CPU c3;
    c3.mem = &m3;
    steps = run(c3);
    CHECK(steps == 1 && c3.fault.empty());
    Memory m4;
    CPU c4;
    c4.mem = &m4;
    for (int i = 0; i < MEM_SIZE; ++i) mem_set(m4, (std::uint16_t)i, OP_NOP);
    steps = run(c4);
    CHECK(steps == (unsigned long)MEM_SIZE + 1 && !c4.fault.empty());
}

static void test_disasm() {
    Memory m;
    put(m, 0, {0x28, 0x00, 0x08, 0x22, 0x34, 0x12, 0x20, 0x41, 0x2B});
    CHECK(disasm(m, 0) == "28 00 08  LOADH H, 0x0800");
    CHECK(disasm(m, 3) == "22 34 12  LOAD A, [0x1234]");
    CHECK(disasm(m, 6) == "20 41  LOADI A, 0x41");
    CHECK(disasm(m, 8) == "2b  INCH");
    mem_set(m, 0xFFF, OP_LOADH);
    CHECK(disasm(m, 0xFFF).find("??") != std::string::npos);
}

int main() {
    test_set16_get16();
    test_isa_sizes();
    test_loadi_load_store();
    test_flags_untouched();
    test_mov();
    test_h_register();
    test_out();
    test_faults();
    test_run_and_regions();
    test_disasm();
    if (failures == 0) {
        std::cout << "all tests passed\n";
        return 0;
    }
    std::cout << failures << " check(s) failed\n";
    return 1;
}
