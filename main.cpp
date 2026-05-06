//***************************************************************************
// Kevin Zavala
// Z2045582
// 
// Boots the OS, initializes memory, scheduler, and CPU,
// loads all programs, and starts execution.
//***************************************************************************
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

    // Create physical memory with the size passed on the command line
    int totalMemory = std::stoi(argv[1]);
    PhysicalMemory* physMem = new PhysicalMemory(totalMemory);

    // Create the memory manager and initialize the 10 shared memory regions
    MemoryManager* memMgr = new MemoryManager(physMem);
    memMgr->initSharedMemory();

    // Create the scheduler and connect it to the memory manager
    Scheduler* scheduler = new Scheduler(memMgr);
    memMgr->setScheduler(scheduler);

    // Create the CPU
    CPU* cpu = new CPU(memMgr, scheduler);

    // Load idle process automatically
    Program idleProgram;
    if (!idleProgram.loadFromFile("idle.txt")) {
        cerr << "Failed to load idle process" << endl;
        return 1;
    }
    PCB* idleProcess = scheduler->createProcess(idleProgram, 1);
    if (idleProcess != nullptr) {
        idleProcess->timeQuantum = 5;
        idleProcess->remainingQuantum = 5;
    }

    // Load user programs
    for (int i = 2; i < argc; i++) {
        Program program;
        if (!program.loadFromFile(argv[i])) {
            cerr << "Failed to load program: " << argv[i] << endl;
            continue;
        }
        int priority = 32 - (i - 2);
        scheduler->createProcess(program, priority);
    }

    cout << "\nRunning processes...\n" << endl;

    cpu->run();

    scheduler->printStatistics();

    cout << "All processes completed." << endl;
    cout << "Total clock cycles: " << cpu->getClockTicks() << endl;

    // Clean up
    delete cpu;
    delete scheduler;
    delete memMgr;
    delete physMem;

    return 0;
}