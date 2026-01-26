#include <iostream>
#include "PhysicalMemory.h"
#include "MemoryManager.h"
#include "CPU.h"
#include "Opcode.h"

using namespace std;

int main() {
    cout << "MidOS - Module 1 Test" << endl;
    cout << "---------------------" << endl;
    
    PhysicalMemory* physMem = new PhysicalMemory(10000);
    MemoryManager* memMgr = new MemoryManager(physMem);
    CPU* cpu = new CPU(memMgr);
    
    // Program: Load 10 into r1, load 5 into r2, add them, print result, exit
    
    int addr = 0;
    
    // MOVI r1, 10
    memMgr->write(addr++, static_cast<uint8_t>(Opcode::MOVI));
    memMgr->write(addr++, 1);  // r1
    memMgr->writeInt(addr, 10);
    addr += 4;
    
    // MOVI r2, 5
    memMgr->write(addr++, static_cast<uint8_t>(Opcode::MOVI));
    memMgr->write(addr++, 2);  // r2
    memMgr->writeInt(addr, 5);
    addr += 4;
    
    // ADDR r3, r1, r2  (r3 = r1 + r2)
    memMgr->write(addr++, static_cast<uint8_t>(Opcode::ADDR));
    memMgr->write(addr++, 3);  // dest: r3
    memMgr->write(addr++, 1);  // src1: r1
    memMgr->write(addr++, 2);  // src2: r2
    
    // PRINTR r3
    memMgr->write(addr++, static_cast<uint8_t>(Opcode::PRINTR));
    memMgr->write(addr++, 3);  // r3
    
    // EXIT
    memMgr->write(addr++, static_cast<uint8_t>(Opcode::EXIT));
    
    cout << "\nProgram loaded. Expected output: 15\n" << endl;
    
    // Set up CPU
    cpu->setIP(0);
    cpu->setSP(5000);  // Stack starts at address 5000
    
    // Run the program
    cout << "Running program..." << endl;
    cpu->run();
    
    cout << "\nProgram finished!" << endl;
    cout << "Clock cycles: " << cpu->getClockTicks() << endl;
    
    // Cleanup
    delete cpu;
    delete memMgr;
    delete physMem;
    
    return 0;
}