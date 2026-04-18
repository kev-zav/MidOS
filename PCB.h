#ifndef PCB_H
#define PCB_H

#include <vector>
#include <cstdint>

// Represents one chunk of heap memory for a process
struct HeapBlock {
    int startAddress;  // Virtual address where this block starts
    int size;          // How many bytes this block is
    bool isFree;       // true = available, false = in use
};

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
    
    // Waiting info
    int waitingOnLock;
    int waitingOnEvent;
    
    // Memory management
    std::vector<int> pageTable;

    // Heap allocation tracking, every alloc and free gets recorded here
    std::vector<HeapBlock> heapAllocations;
    
    // Constructor
    PCB(int id);
    
    // Reset quantum
    void resetQuantum();
};

#endif