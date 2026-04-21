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
    memMgr->setScheduler(scheduler);

    CPU* cpu = new CPU(memMgr, scheduler);

    // Load idle process automatically with lowest priority and quantum of 5
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

    delete cpu;
    delete scheduler;
    delete memMgr;
    delete physMem;

    return 0;
}