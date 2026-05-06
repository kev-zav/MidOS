//***************************************************************************************************
// Kevin Zavala
// Z2045582
//
// Initializes a new PCB with default values. Every process gets one 
// PCB when it is created. Holds everything needed to pause and resume
// a process during context switches.
//*********************************************

#include "PCB.h"

PCB::PCB(int id) {
    processId = id;
    
    // Memory sizes
    codeSize = 0;
    stackSize = 0;
    dataSize = 0;
    heapStart = 0;
    heapEnd = 0;
    processMemorySize = 0;
    
    // CPU state
    for (int i = 0; i < 15; i++) {
        registers[i] = 0;
    }
    signFlag = false;
    zeroFlag = false;
    
    // Scheduling
    state = ProcessState::New;
    priority = 16;
    timeQuantum = 10;       
    remainingQuantum = 10;
    
    // Statistics
    clockCycles = 0;
    contextSwitches = 0;
    sleepCounter = 0;
    pageFaults = 0;
    
    // Waiting info
    waitingOnLock = 0;
    waitingOnEvent = 0;
}

void PCB::resetQuantum() {
    remainingQuantum = timeQuantum;
}