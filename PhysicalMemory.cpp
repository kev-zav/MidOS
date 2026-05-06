//***************************************************************************************************
// Kevin Zavala
// Z2045582
//
// Simulates RAM as a byte array.
// Provides bounds-checked read and write for
// both single bytes and 4-byte integers.
//***************************************************************************************************
#include "PhysicalMemory.h"
#include <iostream>

// Creates memory of given size
PhysicalMemory::PhysicalMemory(int size) {
    memory.resize(size, 0);
}

// Read 1 byte from memory
uint8_t PhysicalMemory::read(int address) {
    if (address < 0 || address >= memory.size()) {
        std::cerr << "Error: Invalid memory access at address " << address << std::endl;
        return 0;
    }
    return memory[address];
}

// Write 1 byte to memory
void PhysicalMemory::write(int address, uint8_t value) {
    if (address < 0 || address >= memory.size()) {
        std::cerr << "Error: Invalid memory write at address " << address << std::endl;
        return;
    }
    memory[address] = value;
}

// Read 4 bytes as an integer
int32_t PhysicalMemory::readInt(int address) {
    if (address < 0 || address + 3 >= memory.size()) {
        std::cerr << "Error: Invalid memory access at address " << address << std::endl;
        return 0;
    }
    
    int32_t value = 0;
    value |= memory[address];
    value |= memory[address + 1] << 8;
    value |= memory[address + 2] << 16;
    value |= memory[address + 3] << 24;
    return value;
}

// Write 4 bytes from an integer
void PhysicalMemory::writeInt(int address, int32_t value) {
    if (address < 0 || address + 3 >= memory.size()) {
        std::cerr << "Error: Invalid memory write at address " << address << std::endl;
        return;
    }
    
    memory[address] = value & 0xFF;
    memory[address + 1] = (value >> 8) & 0xFF;
    memory[address + 2] = (value >> 16) & 0xFF;
    memory[address + 3] = (value >> 24) & 0xFF;
}

// Get total memory size
int PhysicalMemory::getSize() {
    return memory.size();
}