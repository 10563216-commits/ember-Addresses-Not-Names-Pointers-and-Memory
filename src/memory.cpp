#include "memory.hpp"

// Адреса маскується до 12 біт, тож вихід за 0xFFF не псує пам'ять поза коробкою.
Byte mem_get(const Memory& m, std::uint16_t addr) {
    return m.bytes[addr & 0x0FFF];
}

void mem_set(Memory& m, std::uint16_t addr, Byte value) {
    m.bytes[addr & 0x0FFF] = value;
}

// ДАНО. Прочитати два байти як одне 16-бітне значення, молодший перший.
// Увага: перевірки меж тут немає — mem_get маскує адресу, тож addr + 1 == MEM_SIZE
// мовчки прочитає байт 0. Межі стереже той, хто викликає (step() у cpu.cpp).
std::uint16_t get16(const Memory& mem, std::size_t addr) {
    Byte lo = mem_get(mem, addr);
    Byte hi = mem_get(mem, addr + 1);
    return (std::uint16_t)(lo | (hi << 8));
}

// ВАШЕ. Дзеркало get16: молодший байт — за addr, старший — за addr + 1.
// Перевірка записана як addr >= MEM_SIZE - 1, а не addr + 1 >= MEM_SIZE:
// так вона не ламається, навіть якщо addr == SIZE_MAX (там addr + 1 загорнулося б у 0).
bool set16(Memory& mem, std::size_t addr, std::uint16_t value) {
    if (addr >= (std::size_t)MEM_SIZE - 1) return false;
    mem_set(mem, (std::uint16_t)addr,       (Byte)(value & 0xFF));   // молодший
    mem_set(mem, (std::uint16_t)(addr + 1), (Byte)(value >> 8));     // старший
    return true;
}
