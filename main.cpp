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
    
    cout << "MidOS - Module 1 with Program Loader" << endl;
    cout << "=====================================" << endl;
    
    // Load program from file
    Program program;
    if (!program.loadFromFile(argv[1])) {
        cerr << "Failed to load program" << endl;
        return 1;
    }
    
    cout << "Program loaded: " << program.getSize() << " bytes" << endl;
    
    // Create memory and CPU
    PhysicalMemory* physMem = new PhysicalMemory(10000);
    MemoryManager* memMgr = new MemoryManager(physMem);
    CPU* cpu = new CPU(memMgr);
    
    // Load program bytecode into memory
    const vector<uint8_t>& bytecode = program.getBytecode();
    for (size_t i = 0; i < bytecode.size(); i++) {
        memMgr->write(i, bytecode[i]);
    }

    cpu->setIP(0);
    cpu->setSP(5000);
    
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