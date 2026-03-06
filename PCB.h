#ifndef PCB_H
#define PCB_H

#include <vector>
#include <cstdint>

enum class ProcessState {
    New,
    Ready,
    Running,
    WaitingSleep,
    WaitingLock,
    WaitingEvent,
    Terminated
};

class PCB {
public:
    // Process identification
    int processId;
    
    // Memory sizes
    int codeSize;
    int stackSize;
    int dataSize;
    int heapStart;
    int heapEnd;
    int processMemorySize;
    
    // CPU state
    int32_t registers[15];
    bool signFlag;
    bool zeroFlag;
    
    // Scheduling
    ProcessState state;
    int priority;           
    int timeQuantum;        
    int remainingQuantum;   
    
    // Statistics
    int clockCycles; 
    int contextSwitches;   
    int sleepCounter;      
    
    // Memory management
    std::vector<int> pageTable; 
    
    // Constructor
    PCB(int id);
    
    // Reset quantum
    void resetQuantum();
};

#endif