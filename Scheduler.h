#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <string>
#include "PCB.h"
#include "MemoryManager.h"
#include "Program.h"

class CPU;

class Scheduler {
private:
    std::vector<PCB*> processes;
    PCB* currentProcess;
    int nextProcessId;
    MemoryManager* memoryManager;
    
    PCB* findNextProcess();
    
public:
    Scheduler(MemoryManager* mm);
    ~Scheduler();
    
    PCB* createProcess(Program& program, int priority = 16);
    
    void saveContext(CPU* cpu);
    void loadContext(CPU* cpu);
    void contextSwitch(CPU* cpu);
    
    void sleepCurrentProcess(int cycles);
    void terminateCurrentProcess();
    void blockCurrentProcess(ProcessState reason);
    void unblockProcess(int processId);
    
    void tick();
    void updateSleepingProcesses();
    
    PCB* getCurrentProcess();
    bool hasRunningProcess();
    bool hasReadyProcess();
    bool allProcessesTerminated();
    
    void printStatistics();
};

#endif