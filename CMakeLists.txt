cmake_minimum_required(VERSION 3.16)
project(ember CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

if(MSVC)
  # Джерела в UTF-8; /W4 /WX — аналог -Wall -Wextra -Werror.
  add_compile_options(/utf-8 /W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Lab 3: cmake -S . -B build-asan -DEMBER_SANITIZE=ON — збірка з AddressSanitizer + UBSan.
# Потрібен компілятор із підтримкою санітайзерів (gcc/clang на Linux, clang64 в MSYS2, WSL);
# gcc із ucrt64/mingw64 їх зазвичай не має.
option(EMBER_SANITIZE "Збирати з -fsanitize=address,undefined" OFF)
if(EMBER_SANITIZE AND NOT MSVC)
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

set(CORE_SOURCES
  src/memory.cpp
  src/alu.cpp
  src/cpu.cpp
)

add_executable(ember src/main.cpp ${CORE_SOURCES})
target_include_directories(ember PRIVATE src)

# Самоперевірка ALU і step(): ./build/test_alu
add_executable(test_alu tests/test_alu.cpp ${CORE_SOURCES})
target_include_directories(test_alu PRIVATE src)

# Самоперевірка Lab 3 (get16/set16, група 0x2_, OUT, межі): ./build/test_mem
add_executable(test_mem tests/test_mem.cpp ${CORE_SOURCES})
target_include_directories(test_mem PRIVATE src)
