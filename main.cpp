#include <iostream>
#include "PhysicalMemory.h"
#include "MemoryManager.h"
#include "CPU.h"
#include "Program.h"
#include "Scheduler.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Please provide at least one program file." << endl;
        return 1;
    }
    
    cout << "MidOS - Module 4" << endl;
    cout << "=================" << endl;
    
    // Create physical memory (64KB)
    int totalMemory = 65536;
    PhysicalMemory* physMem = new PhysicalMemory(totalMemory);
    MemoryManager* memMgr = new MemoryManager(physMem);
    
    // Initialize shared memory
    memMgr->initSharedMemory();
    
    Scheduler* scheduler = new Scheduler(memMgr);
    CPU* cpu = new CPU(memMgr, scheduler);
    
    // Load each program as a process
    for (int i = 1; i < argc; i++) {
        Program program;
        if (!program.loadFromFile(argv[i])) {
            cerr << "Failed to load program: " << argv[i] << endl;
            continue;
        }
        
        // Priority based on load order (first = highest)
        int priority = 32 - i;
        scheduler->createProcess(program, priority);
    }
    
    cout << "\nRunning processes...\n" << endl;
    
    cpu->run();
    
    scheduler->printStatistics();
    
    cout << "All processes completed." << endl;
    cout << "Total clock cycles: " << cpu->getClockTicks() << endl;
    
    // Cleanup
    delete cpu;
    delete scheduler;
    delete memMgr;
    delete physMem;
    
    return 0;
}