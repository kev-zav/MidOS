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
    
    cout << "MidOS - Module 2" << endl;
    cout << "=================" << endl;
    
    Program program;
    if (!program.loadFromFile(argv[1])) {
        cerr << "Failed to load program" << endl;
        return 1;
    }
    
    const int PAGE_SIZE = 256;
    
    int codeSize = program.getSize();
    int globalDataSize = 512;
    int heapSize = 512;
    int stackSize = 512;
    
    int codePagesNeeded = (codeSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int globalDataPagesNeeded = (globalDataSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int heapPagesNeeded = (heapSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int stackPagesNeeded = (stackSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int totalPagesNeeded = codePagesNeeded + globalDataPagesNeeded + heapPagesNeeded + stackPagesNeeded;
    
    int totalMemory = (totalPagesNeeded + 4) * PAGE_SIZE;
    
    cout << "Program loaded: " << codeSize << " bytes" << endl;
    
    PhysicalMemory* physMem = new PhysicalMemory(totalMemory);
    MemoryManager* memMgr = new MemoryManager(physMem);
    CPU* cpu = new CPU(memMgr);
    
    const vector<uint8_t>& bytecode = program.getBytecode();
    int currentVirtualPage = 0;
    
    // Allocate and map CODE pages
    for (int i = 0; i < codePagesNeeded; i++) {
        int physPage = memMgr->allocatePage();
        memMgr->mapPage(currentVirtualPage + i, physPage);
    }
    
    // Write bytecode to virtual memory
    for (size_t i = 0; i < bytecode.size(); i++) {
        memMgr->write(i, bytecode[i]);
    }
    currentVirtualPage += codePagesNeeded;
    
    // Allocate and map GLOBAL DATA pages
    int globalDataStart = currentVirtualPage * PAGE_SIZE;
    for (int i = 0; i < globalDataPagesNeeded; i++) {
        int physPage = memMgr->allocatePage();
        memMgr->mapPage(currentVirtualPage + i, physPage);
    }
    currentVirtualPage += globalDataPagesNeeded;
    
    // Allocate and map HEAP pages
    for (int i = 0; i < heapPagesNeeded; i++) {
        int physPage = memMgr->allocatePage();
        memMgr->mapPage(currentVirtualPage + i, physPage);
    }
    currentVirtualPage += heapPagesNeeded;
    
    // Allocate and map STACK pages
    int stackStart = currentVirtualPage * PAGE_SIZE;
    int stackTop = stackStart + stackSize;
    for (int i = 0; i < stackPagesNeeded; i++) {
        int physPage = memMgr->allocatePage();
        memMgr->mapPage(currentVirtualPage + i, physPage);
    }
    
    memMgr->printPageTable();
    
    cpu->setIP(0);
    cpu->setRegister(12, 1);
    cpu->setSP(stackTop);
    cpu->setRegister(14, globalDataStart);
    
    cout << "\nRunning program...\n" << endl;
    
    cpu->run();
    
    cout << "\nProgram finished!" << endl;
    cout << "Clock cycles: " << cpu->getClockTicks() << endl;
    
    delete cpu;
    delete memMgr;
    delete physMem;
    
    return 0;
}