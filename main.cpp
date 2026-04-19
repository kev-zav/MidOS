#include <iostream>
#include "PhysicalMemory.h"
#include "MemoryManager.h"
#include "CPU.h"
#include "Program.h"
#include "Scheduler.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cout << "Usage: MidOS <memory size> <program1.txt> ..." << endl;
        return 1;
    }
    
    cout << "MidOS" << endl;
    cout << "=================" << endl;
    
    int totalMemory = std::stoi(argv[1]);
    PhysicalMemory* physMem = new PhysicalMemory(totalMemory);
    MemoryManager* memMgr = new MemoryManager(physMem);
    
    memMgr->initSharedMemory();
    
    Scheduler* scheduler = new Scheduler(memMgr);
    CPU* cpu = new CPU(memMgr, scheduler);
    
    for (int i = 2; i < argc; i++) {
        Program program;
        if (!program.loadFromFile(argv[i])) {
            cerr << "Failed to load program: " << argv[i] << endl;
            continue;
        }
        int priority = 32 - (i - 1);
        scheduler->createProcess(program, priority);
    }
    
    cout << "\nRunning processes...\n" << endl;
    
    cpu->run();
    
    scheduler->printStatistics();
    
    cout << "All processes completed." << endl;
    cout << "Total clock cycles: " << cpu->getClockTicks() << endl;
    
    delete cpu;
    delete scheduler;
    delete memMgr;
    delete physMem;
    
    return 0;
}