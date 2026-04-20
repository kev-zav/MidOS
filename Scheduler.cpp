#include "Scheduler.h"
#include "CPU.h"
#include <iostream>
#include <algorithm>

Scheduler::Scheduler(MemoryManager* mm) {
    memoryManager = mm;
    currentProcess = nullptr;
    nextProcessId = 1;

    for (int i = 0; i < NUM_LOCKS; i++) locks[i] = -1;
    for (int i = 0; i < NUM_EVENTS; i++) events[i] = false;
}

Scheduler::~Scheduler() {
    for (PCB* p : processes) delete p;
}

PCB* Scheduler::createProcess(Program& program, int priority) {
    const int PAGE_SIZE = 256;

    PCB* pcb = new PCB(nextProcessId++);
    pcb->priority = priority;
    pcb->state = ProcessState::Ready;

    pcb->codeSize = program.getSize();
    pcb->dataSize = 512;
    pcb->stackSize = 512;
    int heapSize = 512;

    int codePagesNeeded  = (pcb->codeSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int dataPagesNeeded  = (pcb->dataSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int heapPagesNeeded  = (heapSize + PAGE_SIZE - 1) / PAGE_SIZE;
    int stackPagesNeeded = (pcb->stackSize + PAGE_SIZE - 1) / PAGE_SIZE;

    int totalVirtualPages = codePagesNeeded + dataPagesNeeded + heapPagesNeeded + stackPagesNeeded;
    pcb->pageTable.resize(totalVirtualPages);

    int currentVirtualPage = 0;

    // Code Pages
    for (int i = 0; i < codePagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (physPage == -1) {
            std::cerr << "Error: Out of memory for process " << pcb->processId << std::endl;
            delete pcb;
            return nullptr;
        }
        pcb->pageTable[currentVirtualPage + i].physicalPage = physPage;
        pcb->pageTable[currentVirtualPage + i].isValid = true;
        pcb->pageTable[currentVirtualPage + i].processId = pcb->processId;
    }

    // Load bytecode into virtual memory
    memoryManager->setPageTable(&pcb->pageTable);
    const std::vector<uint8_t>& bytecode = program.getBytecode();
    for (size_t i = 0; i < bytecode.size(); i++) {
        memoryManager->write(i, bytecode[i]);
    }
    currentVirtualPage += codePagesNeeded;

    // PhysicalMemory initializes all bytes to 0 in its constructor
    // so freshly allocated pages are already zeroed
    int dataStart = currentVirtualPage * PAGE_SIZE;
    for (int i = 0; i < dataPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (physPage == -1) {
            std::cerr << "Error: Out of memory (data) for process " << pcb->processId << std::endl;
            delete pcb;
            return nullptr;
        }
        pcb->pageTable[currentVirtualPage + i].physicalPage = physPage;
        pcb->pageTable[currentVirtualPage + i].isValid = true;
        pcb->pageTable[currentVirtualPage + i].processId = pcb->processId;
    }
    currentVirtualPage += dataPagesNeeded;

    // Heap Pages
    pcb->heapStart = currentVirtualPage * PAGE_SIZE;
    for (int i = 0; i < heapPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (physPage == -1) {
            std::cerr << "Error: Out of memory (heap) for process " << pcb->processId << std::endl;
            delete pcb;
            return nullptr;
        }
        pcb->pageTable[currentVirtualPage + i].physicalPage = physPage;
        pcb->pageTable[currentVirtualPage + i].isValid = true;
        pcb->pageTable[currentVirtualPage + i].processId = pcb->processId;
    }
    pcb->heapEnd = pcb->heapStart + heapSize;
    currentVirtualPage += heapPagesNeeded;

    // --- Stack pages ---
    int stackStart = currentVirtualPage * PAGE_SIZE;
    int stackTop = stackStart + pcb->stackSize - 4;
    for (int i = 0; i < stackPagesNeeded; i++) {
        int physPage = memoryManager->allocatePage();
        if (physPage == -1) {
            std::cerr << "Error: Out of memory (stack) for process " << pcb->processId << std::endl;
            delete pcb;
            return nullptr;
        }
        pcb->pageTable[currentVirtualPage + i].physicalPage = physPage;
        pcb->pageTable[currentVirtualPage + i].isValid = true;
        pcb->pageTable[currentVirtualPage + i].processId = pcb->processId;
    }

    pcb->processMemorySize = totalVirtualPages * PAGE_SIZE;

    // Set initial register state per spec
    pcb->registers[11] = 0;            // IP starts at 0
    pcb->registers[12] = pcb->processId;
    pcb->registers[13] = stackTop;     // SP at top of stack
    pcb->registers[14] = dataStart;    // r14 = start of global data

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
    for (int i = 1; i <= 14; i++) currentProcess->registers[i] = cpu->getRegister(i);
    currentProcess->signFlag = cpu->getSignFlag();
    currentProcess->zeroFlag = cpu->getZeroFlag();
}

void Scheduler::loadContext(CPU* cpu) {
    if (currentProcess == nullptr) return;
    for (int i = 1; i <= 14; i++) cpu->setRegister(i, currentProcess->registers[i]);
    cpu->setSignFlag(currentProcess->signFlag);
    cpu->setZeroFlag(currentProcess->zeroFlag);
    memoryManager->setPageTable(&currentProcess->pageTable);
}

void Scheduler::contextSwitch(CPU* cpu) {
    if (currentProcess != nullptr && currentProcess->state == ProcessState::Running) {
        saveContext(cpu);
        currentProcess->state = ProcessState::Ready;
        currentProcess->contextSwitches++;
    }

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

    // Print exit stats as required by spec
    std::cout << "\nProcess " << currentProcess->processId << " terminated." << std::endl;
    std::cout << "  Page faults:      " << currentProcess->pageFaults << std::endl;
    std::cout << "  Context switches: " << currentProcess->contextSwitches << std::endl;
    std::cout << "  Clock cycles:     " << currentProcess->clockCycles << std::endl;

    releaseAllLocks(currentProcess->processId);

    // Free all physical pages this process was using
    for (PageEntry& entry : currentProcess->pageTable) {
        if (entry.isValid && entry.physicalPage != -1) {
            memoryManager->freePage(entry.physicalPage);
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
            if (p->sleepCounter <= 0) p->state = ProcessState::Ready;
        }
    }
}

PCB* Scheduler::getCurrentProcess() { return currentProcess; }

bool Scheduler::hasRunningProcess() {
    return currentProcess != nullptr && currentProcess->state == ProcessState::Running;
}

bool Scheduler::hasReadyProcess() {
    for (PCB* p : processes)
        if (p->state == ProcessState::Ready) return true;
    return false;
}

bool Scheduler::allProcessesTerminated() {
    for (PCB* p : processes)
        if (p->state != ProcessState::Terminated) return false;
    return true;
}

void Scheduler::printStatistics() {
    std::cout << "\n=== Final Statistics ===" << std::endl;
    for (PCB* p : processes) {
        std::cout << "Process " << p->processId << ":" << std::endl;
        std::cout << "  State:            " << static_cast<int>(p->state) << std::endl;
        std::cout << "  Page faults:      " << p->pageFaults << std::endl;
        std::cout << "  Clock cycles:     " << p->clockCycles << std::endl;
        std::cout << "  Context switches: " << p->contextSwitches << std::endl;
    }
    std::cout << "========================\n" << std::endl;
}

bool Scheduler::acquireLock(int lockId, int processId) {
    if (lockId < 1 || lockId > NUM_LOCKS) return false;
    int index = lockId - 1;

    // Already held by this process - no-op, don't deadlock against yourself
    if (locks[index] == processId) return true;

    if (locks[index] == -1) {
        locks[index] = processId;
        return true;
    }

    PCB* holder = nullptr;
    PCB* waiter = nullptr;
    for (PCB* p : processes) {
        if (p->processId == locks[index]) holder = p;
        if (p->processId == processId) waiter = p;
    }
    if (holder && waiter && waiter->priority > holder->priority) {
        std::cout << "[Sched] Priority inversion: bumping process " << holder->processId
                  << " priority from " << holder->priority
                  << " to " << waiter->priority << std::endl;
        holder->priority = waiter->priority;
    }

    return false;
}

bool Scheduler::releaseLock(int lockId, int processId) {
    if (lockId < 1 || lockId > NUM_LOCKS) return false;
    int index = lockId - 1;
    if (locks[index] != processId) return false;

    locks[index] = -1;

    // Wake the HIGHEST PRIORITY process waiting on this lock
    PCB* best = nullptr;
    for (PCB* p : processes) {
        if (p->state == ProcessState::WaitingLock && p->waitingOnLock == lockId) {
            if (best == nullptr || p->priority > best->priority) {
                best = p;
            }
        }
    }
    if (best != nullptr) best->state = ProcessState::Ready;

    return true;
}

void Scheduler::releaseAllLocks(int processId) {
    for (int i = 0; i < NUM_LOCKS; i++) {
        if (locks[i] == processId) {
            locks[i] = -1;
            PCB* best = nullptr;
            for (PCB* p : processes) {
                if (p->state == ProcessState::WaitingLock && p->waitingOnLock == (i + 1)) {
                    if (best == nullptr || p->priority > best->priority) best = p;
                }
            }
            if (best) best->state = ProcessState::Ready;
        }
    }
}

void Scheduler::signalEvent(int eventId) {
    if (eventId < 1 || eventId > NUM_EVENTS) return;
    int index = eventId - 1;
    events[index] = true;
    for (PCB* p : processes) {
        if (p->state == ProcessState::WaitingEvent && p->waitingOnEvent == eventId) {
            p->state = ProcessState::Ready;
            events[index] = false;
        }
    }
}

void Scheduler::waitEvent(int eventId) {
    if (eventId < 1 || eventId > NUM_EVENTS) return;
    int index = eventId - 1;
    if (events[index]) {
        events[index] = false;
        return;
    }
    if (currentProcess != nullptr) {
        currentProcess->state = ProcessState::WaitingEvent;
        currentProcess->waitingOnEvent = eventId;
    }
}

int Scheduler::allocateHeap(PCB* process, int size) {
    if (size <= 0) return 0;
    for (HeapBlock& block : process->heapAllocations) {
        if (block.isFree && block.size >= size) {
            block.isFree = false;
            return block.startAddress;
        }
    }
    int nextFree = process->heapStart;
    for (HeapBlock& block : process->heapAllocations) {
        int blockEnd = block.startAddress + block.size;
        if (blockEnd > nextFree) nextFree = blockEnd;
    }
    if (nextFree + size > process->heapEnd) {
        std::cerr << "Error: Heap full for process " << process->processId << std::endl;
        return 0;
    }
    HeapBlock newBlock;
    newBlock.startAddress = nextFree;
    newBlock.size = size;
    newBlock.isFree = false;
    process->heapAllocations.push_back(newBlock);
    return nextFree;
}

void Scheduler::freeHeap(PCB* process, int address) {
    for (HeapBlock& block : process->heapAllocations) {
        if (block.startAddress == address && !block.isFree) {
            block.isFree = true;
            return;
        }
    }
    std::cerr << "Warning: Attempted to free invalid heap address "
              << address << " for process " << process->processId << std::endl;
}

void Scheduler::recordPageFault() {
    if (currentProcess != nullptr) {
        currentProcess->pageFaults++;
    }
}