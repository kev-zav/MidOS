#include <iostream>
#include "PhysicalMemory.h"
#include "MemoryManager.h"
#include "CPU.h"

using namespace std;

int main() {
    cout << "MidOS - Module 1 Starting..." << endl;
    cout << "-------------------------------" << endl;
    
    // Create physical memory (10KB for now)
    PhysicalMemory* physMem = new PhysicalMemory(10000);
    cout << "Physical memory created: " << physMem->getSize() << " bytes" << endl;
    
    // Create memory manager
    MemoryManager* memMgr = new MemoryManager(physMem);
    cout << "Memory manager created" << endl;
    
    // Create CPU
    CPU* cpu = new CPU(memMgr);
    cout << "CPU created" << endl;
    
    // Test: Write a simple instruction to memory
    // Let's write an "incr r1" instruction
    // For now just some bytes
    memMgr->write(0, 1);      // Opcode 1 (we'll say this is incr)
    memMgr->writeInt(1, 1);   // Arg1: register 1
    memMgr->writeInt(5, 0);   // Arg2: unused
    
    cout << "\nWritten test instruction to memory" << endl;
    
    // Set up CPU initial state
    cpu->setIP(0);           // Start at address 0
    cpu->setRegister(1, 5);  // Put 5 in register 1
    
    cout << "Register 1 before: " << cpu->getRegister(1) << endl;
    
    // Execute one instruction
    cout << "\nExecuting instruction..." << endl;
    cpu->executeInstruction();
    
    cout << "\nClock ticks: " << cpu->getClockTicks() << endl;
    cout << "Instruction Pointer: " << cpu->getIP() << endl;
    
    // Cleanup
    delete cpu;
    delete memMgr;
    delete physMem;
    
    cout << "\n================================" << endl;
    cout << "Test complete!" << endl;
    
    return 0;
}