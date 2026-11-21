#include "memory.hpp"

Byte mem_get(const Memory& m, std::uint16_t addr) {
    return m.bytes[addr & 0x0FFF];
}

void mem_set(Memory& m, std::uint16_t addr, Byte value) {
    m.bytes[addr & 0x0FFF] = value;
}

std::uint16_t get16(const Memory& mem, std::size_t addr) {
    Byte lo = mem_get(mem, addr);
    Byte hi = mem_get(mem, addr + 1);
    return (std::uint16_t)(lo | (hi << 8));
}

bool set16(Memory& mem, std::size_t addr, std::uint16_t value) {
    if (addr >= (std::size_t)MEM_SIZE - 1) return false;

    mem_set(mem, (std::uint16_t)addr, (Byte)(value & 0xFF));
    mem_set(mem, (std::uint16_t)(addr + 1), (Byte)(value >> 8));

    return true;
}