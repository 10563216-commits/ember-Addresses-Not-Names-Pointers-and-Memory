#include "memory.hpp"

// Адреса маскується до 12 біт, тож вихід за 0xFFF не псує пам'ять поза коробкою.
Byte mem_get(const Memory& m, std::uint16_t addr) {
    return m.bytes[addr & 0x0FFF];
}

void mem_set(Memory& m, std::uint16_t addr, Byte value) {
    m.bytes[addr & 0x0FFF] = value;
}
