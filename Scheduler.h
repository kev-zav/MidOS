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
    
    // Locks: -1 = free, otherwise = process ID that holds it
    static const int NUM_LOCKS = 10;
    int locks[NUM_LOCKS];
    
    // Events: false = non-signaled, true = signaled
    static const int NUM_EVENTS = 10;
    bool events[NUM_EVENTS];
    
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
    void terminateProcess(int targetPid);
    void blockCurrentProcess(ProcessState reason);
    void unblockProcess(int processId);
    
    void tick();
    void updateSleepingProcesses();
    
    PCB* getCurrentProcess();
    bool hasRunningProcess();
    bool hasReadyProcess();
    bool allProcessesTerminated();
    
    void printStatistics();
    
    // Locks
    bool acquireLock(int lockId, int processId);
    bool releaseLock(int lockId, int processId);
    void releaseAllLocks(int processId);
    
    // Events
    void signalEvent(int eventId);
    void waitEvent(int eventId);

    // Heap allocation
    int allocateHeap(PCB* process, int size);
    void freeHeap(PCB* process, int address);

    //Virtual memory
    void recordPageFault();
};

#endif