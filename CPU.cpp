#include "CPU.h"
#include <iostream>

// Constructor
CPU::CPU(MemoryManager* mm) {
    memoryManager = mm;
    
    // Initialize all registers to 0
    for (int i = 0; i < 15; i++) {
        registers[i] = 0;
    }
    
    // Initialize flags
    signFlag = false;
    zeroFlag = false;
    
    // Initialize clock
    clockTicks = 0;
    
    // Not running yet
    running = false;
}

// Get register value
int32_t CPU::getRegister(int regNum) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return 0;
    }
    return registers[regNum];
}

// Set register value
void CPU::setRegister(int regNum, int32_t value) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return;
    }
    registers[regNum] = value;
}

// Get Instruction Pointer
int32_t CPU::getIP() {
    return registers[IP];
}

// Set Instruction Pointer
void CPU::setIP(int32_t value) {
    registers[IP] = value;
}

// Get Stack Pointer
int32_t CPU::getSP() {
    return registers[SP];
}

// Set Stack Pointer
void CPU::setSP(int32_t value) {
    registers[SP] = value;
}

// Get Sign Flag
bool CPU::getSignFlag() {
    return signFlag;
}

// Set Sign Flag
void CPU::setSignFlag(bool value) {
    signFlag = value;
}

// Get Zero Flag
bool CPU::getZeroFlag() {
    return zeroFlag;
}

// Set Zero Flag
void CPU::setZeroFlag(bool value) {
    zeroFlag = value;
}

// Get clock ticks
int CPU::getClockTicks() {
    return clockTicks;
}

// Increment clock
void CPU::incrementClock() {
    clockTicks++;
}

// Push value onto stack
void CPU::push(int32_t value) {
    registers[SP] -= 4;  // Move stack pointer down by 4 bytes
    memoryManager->writeInt(registers[SP], value);
}

// Pop value from stack
int32_t CPU::pop() {
    int32_t value = memoryManager->readInt(registers[SP]);
    registers[SP] += 4;  // Move stack pointer up by 4 bytes
    return value;
}

// Main execution loop
void CPU::run() {
    running = true;
    
    while (running) {
        executeInstruction();
        incrementClock();
    }
}

// Execute one instruction
void CPU::executeInstruction() {
    // Get the opcode at current IP
    uint8_t opcode = memoryManager->read(registers[IP]);
    
    // Get the two arguments (each 4 bytes)
    int32_t arg1 = memoryManager->readInt(registers[IP] + 1);
    int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
    
    // Move IP to next instruction (9 bytes total)
    registers[IP] += 9;
    
    // TODO: Decode and execute based on opcode
    // For now, just print what we found
    std::cout << "Opcode: " << (int)opcode << " Arg1: " << arg1 << " Arg2: " << arg2 << std::endl;
    
    // Temporary: stop after one instruction
    running = false;
}

// Stop execution
void CPU::stop() {
    running = false;
}