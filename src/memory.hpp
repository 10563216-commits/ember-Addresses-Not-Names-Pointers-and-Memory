// memory.hpp — коробка на 4 КБ із Lab 1 (мінімальна версія).
#pragma once
#include <cstdint>

using Byte = std::uint8_t;

const int MEM_SIZE = 4096;   // адреси 0x000..0xFFF

struct Memory {
    Byte bytes[MEM_SIZE] = {};
};

Byte mem_get(const Memory& m, std::uint16_t addr);
void mem_set(Memory& m, std::uint16_t addr, Byte value);
