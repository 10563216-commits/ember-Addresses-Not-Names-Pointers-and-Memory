#pragma once
#include <cstddef>
#include <cstdint>

using Byte = std::uint8_t;

const int MEM_SIZE = 4096;

struct Memory {
    Byte bytes[MEM_SIZE] = {};
};

Byte mem_get(const Memory& m, std::uint16_t addr);
void mem_set(Memory& m, std::uint16_t addr, Byte value);

const std::uint16_t CODE_LO = 0x000;
const std::uint16_t CODE_HI = 0x7FF;
const std::uint16_t DATA_LO = 0x800;
const std::uint16_t DATA_HI = 0x9FF;

std::uint16_t get16(const Memory& mem, std::size_t addr);

bool set16(Memory& mem, std::size_t addr, std::uint16_t value);