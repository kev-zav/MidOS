#include "Scheduler.h"
#include "CPU.h"
#include <iostream>
#include <algorithm>

Scheduler::Scheduler(MemoryManager* mm) {
    memoryManager = mm;
    currentProcess = nullptr;
    nextProcessId = 1;
}

Scheduler::~Scheduler() {
    for (PCB* p : processes) {
        delete p;
    }
}

PCB* Scheduler::createProcess(Program& program, int priority) {
    const int PAGE_SIZE = 256;
    
    PCB* pcb = new PCB(nextProcessId++);
    pcb->priority = priority;
    pcb->state = ProcessState::Ready;
    
    // Set memory sizes
    pcb->codeSize = program.getSize();
    pcb->dataSize = 512;
    pcb->stackSize = 512;
    pcb->heapStart = 0;
    pcb->heapEnd = 512;
    
    // Calculate pages needed
    int codePagesNeeded = (pcb->codeSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int dataPagesNeeded = (pcb->dataSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int heapPagesNeeded = (pcb->heapEnd + PAGE_SIZE - 1) / PAGE_SIZE;
    int stackPagesNeeded = (pcb->stackSize + PAGE_SIZE - 1) / PAGE_SIZE;
    
    int currentVirtualPage = 0;
    
    // Allocate CODE pages
    for (int i = 0; i < codePagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (physPage == -1) {
            std::cerr << "Error: Out of memory for process " << pcb->processId << std::endl;
            delete pcb;
            return nullptr;
        }
        // Expand page table if needed
        if (currentVirtualPage + i >= pcb->pageTable.size()) {
            pcb->pageTable.resize(currentVirtualPage + i + 1, -1);
        }
        pcb->pageTable[currentVirtualPage + i] = physPage;
    }
    
    // Load bytecode into memory using process page table
    memoryManager->setPageTable(&pcb->pageTable);
    const std::vector<uint8_t>& bytecode = program.getBytecode();
    for (size_t i = 0; i < bytecode.size(); i++) {
        memoryManager->write(i, bytecode[i]);
    }
    currentVirtualPage += codePagesNeeded;
    
    // Allocate DATA pages
    int dataStart = currentVirtualPage * PAGE_SIZE;
    for (int i = 0; i < dataPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (currentVirtualPage + i >= pcb->pageTable.size()) {
            pcb->pageTable.resize(currentVirtualPage + i + 1, -1);
        }
        pcb->pageTable[currentVirtualPage + i] = physPage;
    }
    currentVirtualPage += dataPagesNeeded;
    
    // Allocate HEAP pages
    pcb->heapStart = currentVirtualPage * PAGE_SIZE;
    for (int i = 0; i < heapPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (currentVirtualPage + i >= pcb->pageTable.size()) {
            pcb->pageTable.resize(currentVirtualPage + i + 1, -1);
        }
        pcb->pageTable[currentVirtualPage + i] = physPage;
    }
    pcb->heapEnd = pcb->heapStart + 512;
    currentVirtualPage += heapPagesNeeded;
    
    // Allocate STACK pages
    int stackStart = currentVirtualPage * PAGE_SIZE;
    int stackTop = stackStart + pcb->stackSize;
    for (int i = 0; i < stackPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (currentVirtualPage + i >= pcb->pageTable.size()) {
            pcb->pageTable.resize(currentVirtualPage + i + 1, -1);
        }
        pcb->pageTable[currentVirtualPage + i] = physPage;
    }
    
    pcb->processMemorySize = (currentVirtualPage + stackPagesNeeded) * PAGE_SIZE;
    
    // Set initial register values
    pcb->registers[11] = 0;            // IP = 0
    pcb->registers[12] = pcb->processId;  // PID
    pcb->registers[13] = stackTop;     // SP
    pcb->registers[14] = dataStart;    // GP (global data pointer)
    
    processes.push_back(pcb);
    
    std::cout << "Process " << pcb->processId << " created with priority " << priority << std::endl;
    
    return pcb;
}

PCB* Scheduler::findNextProcess() {
    PCB* best = nullptr;
    
    for (PCB* p : processes) {
        if (p->state == ProcessState::Ready) {
            if (best == nullptr || p->priority > best->priority) {
                best = p;
            }
        }
    }
    
    return best;
}

void Scheduler::saveContext(CPU* cpu) {
    if (currentProcess == nullptr) return;
    
    for (int i = 1; i <= 14; i++) {
        currentProcess->registers[i] = cpu->getRegister(i);
    }
    currentProcess->signFlag = cpu->getSignFlag();
    currentProcess->zeroFlag = cpu->getZeroFlag();
}

void Scheduler::loadContext(CPU* cpu) {
    if (currentProcess == nullptr) return;
    
    for (int i = 1; i <= 14; i++) {
        cpu->setRegister(i, currentProcess->registers[i]);
    }
    cpu->setSignFlag(currentProcess->signFlag);
    cpu->setZeroFlag(currentProcess->zeroFlag);
    
    memoryManager->setPageTable(&currentProcess->pageTable);
}

void Scheduler::contextSwitch(CPU* cpu) {
    // Save current process state
    if (currentProcess != nullptr && currentProcess->state == ProcessState::Running) {
        saveContext(cpu);
        currentProcess->state = ProcessState::Ready;
        currentProcess->contextSwitches++;
    }
    
    // Find next process
    PCB* next = findNextProcess();
    
    if (next == nullptr) {
        currentProcess = nullptr;
        return;
    }
    
    currentProcess = next;
    currentProcess->state = ProcessState::Running;
    currentProcess->resetQuantum();
    
    loadContext(cpu);
}

void Scheduler::sleepCurrentProcess(int cycles) {
    if (currentProcess == nullptr) return;
    
    currentProcess->sleepCounter = cycles;
    currentProcess->state = ProcessState::WaitingSleep;
}

void Scheduler::terminateCurrentProcess() {
    if (currentProcess == nullptr) return;
    
    std::cout << "\nProcess " << currentProcess->processId << " terminated." << std::endl;
    std::cout << "  Clock cycles: " << currentProcess->clockCycles << std::endl;
    std::cout << "  Context switches: " << currentProcess->contextSwitches << std::endl;
    
    // Free pages
    for (int i = 0; i < currentProcess->pageTable.size(); i++) {
        if (currentProcess->pageTable[i] != -1) {
            memoryManager->freePage(currentProcess->pageTable[i]);
        }
    }
    
    currentProcess->state = ProcessState::Terminated;
    currentProcess = nullptr;
}

void Scheduler::blockCurrentProcess(ProcessState reason) {
    if (currentProcess == nullptr) return;
    currentProcess->state = reason;
}

void Scheduler::unblockProcess(int processId) {
    for (PCB* p : processes) {
        if (p->processId == processId) {
            p->state = ProcessState::Ready;
            break;
        }
    }
}

void Scheduler::tick() {
    if (currentProcess != nullptr && currentProcess->state == ProcessState::Running) {
        currentProcess->clockCycles++;
        currentProcess->remainingQuantum--;
    }
    
    updateSleepingProcesses();
}

void Scheduler::updateSleepingProcesses() {
    for (PCB* p : processes) {
        if (p->state == ProcessState::WaitingSleep) {
            p->sleepCounter--;
            if (p->sleepCounter <= 0) {
                p->state = ProcessState::Ready;
            }
        }
    }
}

PCB* Scheduler::getCurrentProcess() {
    return currentProcess;
}

bool Scheduler::hasRunningProcess() {
    return currentProcess != nullptr && currentProcess->state == ProcessState::Running;
}

bool Scheduler::hasReadyProcess() {
    for (PCB* p : processes) {
        if (p->state == ProcessState::Ready) {
            return true;
        }
    }
    return false;
}

bool Scheduler::allProcessesTerminated() {
    for (PCB* p : processes) {
        if (p->state != ProcessState::Terminated) {
            return false;
        }
    }
    return true;
}

void Scheduler::printStatistics() {
    std::cout << "\n=== Final Statistics ===" << std::endl;
    for (PCB* p : processes) {
        std::cout << "Process " << p->processId << ":" << std::endl;
        std::cout << "  State: " << static_cast<int>(p->state) << std::endl;
        std::cout << "  Clock cycles: " << p->clockCycles << std::endl;
        std::cout << "  Context switches: " << p->contextSwitches << std::endl;
    }
    std::cout << "========================\n" << std::endl;
}