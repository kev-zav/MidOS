# MidOS
Operating systems project for CSCI480. A virtual OS running on a custom virtual machine built in C++.

## How to Compile
g++ -std=c++17 -o MidOS main.cpp CPU.cpp Scheduler.cpp MemoryManager.cpp PhysicalMemory.cpp PCB.cpp Program.cpp

## How to Run
./MidOS <memory size> <program1.asm> [program2.asm] ...

Examples:
./MidOS 65536 test_loop.asm
./MidOS 65536 test_loop.asm test_arithmetic.asm
./MidOS 65536 test_heap.asm

Note: idle.txt must be in the same directory as the executable.

## What's Implemented
- Module 1: Virtual CPU with 14 registers, ~60 opcodes, assembly file parser
- Module 2: Paged memory, virtual to physical address translation
- Module 3: Process management, priority scheduling, context switching, PCBs
- Module 4: Mutex locks with priority inversion, events, shared memory
- Module 5: Heap allocation with Alloc and FreeMemory
- Module 6: Virtual memory, LRU page eviction, swap file, page fault handling