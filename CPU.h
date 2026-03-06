#ifndef CPU_H
#define CPU_H

#include "MemoryManager.h"
#include "PCB.h"
#include <cstdint>
#include "Opcode.h"

class Scheduler;

class CPU {
private:
    // 14 registers (r1-r14)
    int32_t registers[15];  // Index 0 unused, r1-r14 use indices 1-14
    
    // Special register indices
    static const int IP = 11;   // Instruction Pointer
    static const int PID = 12;  // Process ID
    static const int SP = 13;   // Stack Pointer
    static const int GP = 14;   // Global Memory Pointer
    
    // Flags
    bool signFlag;
    bool zeroFlag;
    
    // Clock
    int clockTicks;
    
    // Memory
    MemoryManager* memoryManager;
    Scheduler* scheduler;
    
    // Running state
    bool running;
    
public:
    CPU(MemoryManager* mm, Scheduler* sched);
    
    // Register access
    int32_t getRegister(int regNum);
    void setRegister(int regNum, int32_t value);
    
    // Special registers
    int32_t getIP();
    void setIP(int32_t value);
    int32_t getSP();
    void setSP(int32_t value);
    
    // Flags
    bool getSignFlag();
    void setSignFlag(bool value);
    bool getZeroFlag();
    void setZeroFlag(bool value);
    
    // Clock
    int getClockTicks();
    void incrementClock();
    
    // Execution
    void run();  // Main execution loop
    void executeInstruction();  // Execute one instruction
    void stop();  // Stop execution
    bool needsContextSwitch();
    
    // Helper methods
    void push(int32_t value);  // Push to stack
    int32_t pop();  // Pop from stack
};

#endif