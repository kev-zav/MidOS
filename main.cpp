#include <iostream>
#include "PhysicalMemory.h"
#include "MemoryManager.h"
#include "CPU.h"
#include "Program.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <program.asm>" << endl;
        return 1;
    }
    
    cout << "MidOS - Module 1" << endl;
    cout << "=================" << endl;
    
    // Load program from file
    Program program;
    if (!program.loadFromFile(argv[1])) {
        cerr << "Failed to load program" << endl;
        return 1;
    }
    
    // Memory region sizes
    int codeSize = program.getSize();
    int globalDataSize = 512;
    int heapSize = 512;
    int stackSize = 512;
    
    // Calculate total memory needed
    int totalMemory = codeSize + globalDataSize + heapSize + stackSize;
    
    // Calculate region starting addresses
    int codeStart = 0;
    int globalDataStart = codeSize;
    int heapStart = codeSize + globalDataSize;
    int stackTop = totalMemory;  // Stack starts at top, grows down
    
    cout << "Program loaded: " << codeSize << " bytes" << endl;
    cout << "\nMemory Layout:" << endl;
    cout << "  Code:        " << codeStart << " - " << (globalDataStart - 1) << endl;
    cout << "  Global Data: " << globalDataStart << " - " << (heapStart - 1) << endl;
    cout << "  Heap:        " << heapStart << " - " << (stackTop - stackSize - 1) << endl;
    cout << "  Stack:       " << stackTop << " (grows down)" << endl;
    cout << "  Total:       " << totalMemory << " bytes" << endl;
    
    // Create memory and CPU
    PhysicalMemory* physMem = new PhysicalMemory(totalMemory);
    MemoryManager* memMgr = new MemoryManager(physMem);
    CPU* cpu = new CPU(memMgr);
    
    // Load program bytecode into memory (CODE region)
    const vector<uint8_t>& bytecode = program.getBytecode();
    for (size_t i = 0; i < bytecode.size(); i++) {
        memMgr->write(i, bytecode[i]);
    }
    
    // Global data region is already initialized to 0 by PhysicalMemory
    
    // Set up CPU registers for process
    cpu->setIP(0);                          // r11 = Instruction Pointer (start of code)
    cpu->setRegister(12, 1);                // r12 = Process ID
    cpu->setSP(stackTop);                   // r13 = Stack Pointer (top of memory)
    cpu->setRegister(14, globalDataStart);  // r14 = Global Data start address
    
    cout << "\nInitial Registers:" << endl;
    cout << "  r11 (IP): " << cpu->getIP() << endl;
    cout << "  r12 (PID): " << cpu->getRegister(12) << endl;
    cout << "  r13 (SP): " << cpu->getSP() << endl;
    cout << "  r14 (GP): " << cpu->getRegister(14) << endl;
    
    cout << "\nRunning program...\n" << endl;
    
    cpu->run();
    
    cout << "\nProgram finished!" << endl;
    cout << "Clock cycles: " << cpu->getClockTicks() << endl;
    
    // Cleanup
    delete cpu;
    delete memMgr;
    delete physMem;
    
    return 0;
}