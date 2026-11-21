// memory.hpp — коробка на 4 КБ із Lab 1 (мінімальна версія).
#pragma once
#include <cstddef>
#include <cstdint>

using Byte = std::uint8_t;

const int MEM_SIZE = 4096;   // адреси 0x000..0xFFF

struct Memory {
    Byte bytes[MEM_SIZE] = {};
};

Byte mem_get(const Memory& m, std::uint16_t addr);
void mem_set(Memory& m, std::uint16_t addr, Byte value);

// Карта пам'яті (ISA.uk.md §2). Код і дані — в одній коробці.
const std::uint16_t CODE_LO = 0x000;
const std::uint16_t CODE_HI = 0x7FF;
const std::uint16_t DATA_LO = 0x800;
const std::uint16_t DATA_HI = 0x9FF;

// 16-бітний доступ, молодший байт уперед (little-endian, ISA.uk.md §4).
// get16 ДАНО в лабі; set16 — його дзеркало.
std::uint16_t get16(const Memory& mem, std::size_t addr);

// Записує `value` двома байтами: за addr — молодший, за addr + 1 — старший.
// Якщо addr + 1 вийшло б за MEM_SIZE — нічого не пише і повертає false
// (жодного напівзапису: або обидва байти, або жодного).
bool set16(Memory& mem, std::size_t addr, std::uint16_t value);
