#include "CPU.h"
#include "Scheduler.h"
#include <iostream>

// Constructor - initializes CPU state
CPU::CPU(MemoryManager* mm, Scheduler* sched) {
    memoryManager = mm;
    scheduler = sched;
    
    for (int i = 0; i < 15; i++) {
        registers[i] = 0;
    }
    
    signFlag = false;
    zeroFlag = false;
    clockTicks = 0;
    running = false;
}

// Get value from register (r1-r14)
int32_t CPU::getRegister(int regNum) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return 0;
    }
    return registers[regNum];
}

// Set value in register (r1-r14)
void CPU::setRegister(int regNum, int32_t value) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return;
    }
    registers[regNum] = value;
}

// Get instruction pointer
int32_t CPU::getIP() {
    return registers[IP];
}

// Set instruction pointer
void CPU::setIP(int32_t value) {
    registers[IP] = value;
}

// Get stack pointer
int32_t CPU::getSP() {
    return registers[SP];
}

// Set stack pointer
void CPU::setSP(int32_t value) {
    registers[SP] = value;
}

// Get sign flag
bool CPU::getSignFlag() {
    return signFlag;
}

// Set sign flag
void CPU::setSignFlag(bool value) {
    signFlag = value;
}

// Get zero flag
bool CPU::getZeroFlag() {
    return zeroFlag;
}

// Set zero flag
void CPU::setZeroFlag(bool value) {
    zeroFlag = value;
}

// Get clock tick count
int CPU::getClockTicks() {
    return clockTicks;
}

// Increment clock by one tick
void CPU::incrementClock() {
    clockTicks++;
    scheduler->tick();
}

// Push value onto stack
void CPU::push(int32_t value) {
    registers[SP] -= 4;
    memoryManager->writeInt(registers[SP], value);
}

// Pop value from stack
int32_t CPU::pop() {
    int32_t value = memoryManager->readInt(registers[SP]);
    registers[SP] += 4;
    return value;
}

// Check if context switch is needed
bool CPU::needsContextSwitch() {
    PCB* current = scheduler->getCurrentProcess();
    if (current == nullptr) return true;
    if (current->state != ProcessState::Running) return true;
    if (current->remainingQuantum <= 0) return true;
    return false;
}

// Main execution loop - runs until all processes done
void CPU::run() {
    running = true;
    
    while (running) {
        if (needsContextSwitch()) {
            if (scheduler->allProcessesTerminated()) {
                running = false;
                break;
            }
            
            if (!scheduler->hasReadyProcess()) {
                scheduler->updateSleepingProcesses();
                clockTicks++;
                continue;
            }
            
            scheduler->contextSwitch(this);
        }
        
        if (scheduler->getCurrentProcess() == nullptr) {
            continue;
        }
        
        executeInstruction();
    }
}

// Execute single instruction at current IP
void CPU::executeInstruction() {
    Opcode opcode = static_cast<Opcode>(memoryManager->read(registers[IP]));
    incrementClock();
    
    switch(opcode) {
        case Opcode::NOOP:
            registers[IP]++;
            break;
            
        case Opcode::EXIT:
            scheduler->terminateCurrentProcess();
            break;
            
        case Opcode::INCR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value = getRegister(regNum);
            value++;
            setRegister(regNum, value);
            
            zeroFlag = (value == 0);
            signFlag = (value < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOVI: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value = memoryManager->readInt(registers[IP] + 5);
            setRegister(regNum, value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOVR: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t srcReg = memoryManager->readInt(registers[IP] + 5);
            setRegister(destReg, getRegister(srcReg));
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOVMR: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t srcReg = memoryManager->readInt(registers[IP] + 5);
            int32_t address = getRegister(srcReg);
            int32_t value = memoryManager->readInt(address);
            setRegister(destReg, value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOVRM: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t srcReg = memoryManager->readInt(registers[IP] + 5);
            int32_t address = getRegister(destReg);
            int32_t value = getRegister(srcReg);
            memoryManager->writeInt(address, value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOVMM: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t srcReg = memoryManager->readInt(registers[IP] + 5);
            int32_t srcAddress = getRegister(srcReg);
            int32_t destAddress = getRegister(destReg);
            int32_t value = memoryManager->readInt(srcAddress);
            memoryManager->writeInt(destAddress, value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::ADDI: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value = memoryManager->readInt(registers[IP] + 5);
            int32_t result = getRegister(regNum) + value;
            setRegister(regNum, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::ADDR: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t result = getRegister(srcReg1) + getRegister(srcReg2);
            setRegister(destReg, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::SUB: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t result = getRegister(srcReg1) - getRegister(srcReg2);
            setRegister(destReg, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MUL: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t result = getRegister(srcReg1) * getRegister(srcReg2);
            setRegister(destReg, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::DIV: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t divisor = getRegister(srcReg2);
            if (divisor == 0) {
                std::cerr << "Error: Division by zero at IP " << registers[IP] << std::endl;
                scheduler->terminateCurrentProcess();
            } else {
                int32_t result = getRegister(srcReg1) / divisor;
                setRegister(destReg, result);
                
                zeroFlag = (result == 0);
                signFlag = (result < 0);
            }
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MOD: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t divisor = getRegister(srcReg2);
            if (divisor == 0) {
                std::cerr << "Error: Modulo by zero at IP " << registers[IP] << std::endl;
                scheduler->terminateCurrentProcess();
            } else {
                int32_t result = getRegister(srcReg1) % divisor;
                setRegister(destReg, result);
                
                zeroFlag = (result == 0);
                signFlag = (result < 0);
            }
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::AND: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t result = getRegister(srcReg1) & getRegister(srcReg2);
            setRegister(destReg, result);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::OR: {
            int32_t destReg = memoryManager->readInt(registers[IP] + 1);
            int32_t arg2 = memoryManager->readInt(registers[IP] + 5);
            int32_t srcReg1 = arg2 & 0xFFFF;
            int32_t srcReg2 = (arg2 >> 16) & 0xFFFF;
            
            int32_t result = getRegister(srcReg1) | getRegister(srcReg2);
            setRegister(destReg, result);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value = getRegister(regNum);
            std::cout << value << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTM: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t address = getRegister(regNum);
            int32_t value = memoryManager->readInt(address);
            std::cout << value << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTCR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value = getRegister(regNum);
            char c = static_cast<char>(value);
            std::cout << c << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTCM: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t address = getRegister(regNum);
            int32_t value = memoryManager->readInt(address);
            char c = static_cast<char>(value);
            std::cout << c << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::JMP: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            registers[IP] = address;
            break;
        }
        
        case Opcode::JMPI: {
            int32_t offset = memoryManager->readInt(registers[IP] + 1);
            registers[IP] += offset;
            break;
        }
        
        case Opcode::JMPA: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            registers[IP] = address;
            break;
        }
        
        case Opcode::JMPE: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JMPNE: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (!zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JLT: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t offset = getRegister(regNum);
            if (signFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JLTI: {
            int32_t offset = memoryManager->readInt(registers[IP] + 1);
            if (signFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JLTA: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (signFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JGT: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t offset = getRegister(regNum);
            if (!signFlag && !zeroFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JGTI: {
            int32_t offset = memoryManager->readInt(registers[IP] + 1);
            if (!signFlag && !zeroFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JGTA: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (!signFlag && !zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JE: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t offset = getRegister(regNum);
            if (zeroFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JEI: {
            int32_t offset = memoryManager->readInt(registers[IP] + 1);
            if (zeroFlag) {
                registers[IP] += offset;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::JEA: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 9;
            }
            break;
        }
        
        case Opcode::CMPI: {
            int32_t reg = memoryManager->readInt(registers[IP] + 1);
            int32_t value = memoryManager->readInt(registers[IP] + 5);
            
            int32_t result = getRegister(reg) - value;
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::CMPR: {
            int32_t reg1 = memoryManager->readInt(registers[IP] + 1);
            int32_t reg2 = memoryManager->readInt(registers[IP] + 5);
            
            int32_t result = getRegister(reg1) - getRegister(reg2);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PUSHR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            push(getRegister(regNum));
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PUSHI: {
            int32_t value = memoryManager->readInt(registers[IP] + 1);
            push(value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::POPR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            setRegister(regNum, pop());
            registers[IP] += 9;
            break;
        }
        
        case Opcode::POPM: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t address = getRegister(regNum);
            int32_t value = pop();
            memoryManager->writeInt(address, value);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::CALL: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            push(registers[IP] + 9);
            registers[IP] = address;
            break;
        }
        
        case Opcode::CALLM: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t address = memoryManager->readInt(getRegister(regNum));
            push(registers[IP] + 9);
            registers[IP] = address;
            break;
        }
        
        case Opcode::RET: {
            registers[IP] = pop();
            break;
        }
        
        case Opcode::INPUT: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t value;
            std::cin >> value;
            setRegister(regNum, value);
            registers[IP] += 9;
            scheduler->saveContext(this);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->state = ProcessState::Ready;
                current->contextSwitches++;
            }
            scheduler->contextSwitch(this);
            break;
        }
        
        case Opcode::INPUTC: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            char c;
            std::cin >> c;
            setRegister(regNum, static_cast<int32_t>(c));
            registers[IP] += 9;
            scheduler->saveContext(this);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->state = ProcessState::Ready;
                current->contextSwitches++;
            }
            scheduler->contextSwitch(this);
            break;
        }
        
        case Opcode::SLEEP: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t cycles = getRegister(regNum);
            registers[IP] += 9;
            scheduler->sleepCurrentProcess(cycles);
            scheduler->contextSwitch(this);
            break;
        }
        
        case Opcode::SETPRIORITY: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t priority = getRegister(regNum);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->priority = priority;
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::SETPRIORITYI: {
            int32_t priority = memoryManager->readInt(registers[IP] + 1);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->priority = priority;
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::MAP_SHARED_MEM: {
            int32_t regionReg = memoryManager->readInt(registers[IP] + 1);
            int32_t destReg = memoryManager->readInt(registers[IP] + 5);
            int32_t regionId = getRegister(regionReg);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                int32_t addr = memoryManager->mapSharedMemory(regionId, &current->pageTable);
                setRegister(destReg, addr);
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::ACQUIRE_LOCK: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t lockId = getRegister(regNum);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                if (!scheduler->acquireLock(lockId, current->processId)) {
                    current->state = ProcessState::WaitingLock;
                    current->waitingOnLock = lockId;
                    registers[IP] += 9;
                    scheduler->contextSwitch(this);
                    break;
                }
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::ACQUIRE_LOCK_I: {
            int32_t lockId = memoryManager->readInt(registers[IP] + 1);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                if (!scheduler->acquireLock(lockId, current->processId)) {
                    current->state = ProcessState::WaitingLock;
                    current->waitingOnLock = lockId;
                    registers[IP] += 9;
                    scheduler->contextSwitch(this);
                    break;
                }
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::RELEASE_LOCK: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t lockId = getRegister(regNum);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                scheduler->releaseLock(lockId, current->processId);
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::RELEASE_LOCK_I: {
            int32_t lockId = memoryManager->readInt(registers[IP] + 1);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                scheduler->releaseLock(lockId, current->processId);
            }
            registers[IP] += 9;
            break;
        }
        
        case Opcode::SIGNAL_EVENT: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t eventId = getRegister(regNum);
            scheduler->signalEvent(eventId);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::SIGNAL_EVENT_I: {
            int32_t eventId = memoryManager->readInt(registers[IP] + 1);
            scheduler->signalEvent(eventId);
            registers[IP] += 9;
            break;
        }
        
        case Opcode::WAIT_EVENT: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            int32_t eventId = getRegister(regNum);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->waitingOnEvent = eventId;
            }
            scheduler->waitEvent(eventId);
            registers[IP] += 9;
            if (current != nullptr && current->state == ProcessState::WaitingEvent) {
                scheduler->contextSwitch(this);
            }
            break;
        }
        
        case Opcode::WAIT_EVENT_I: {
            int32_t eventId = memoryManager->readInt(registers[IP] + 1);
            PCB* current = scheduler->getCurrentProcess();
            if (current != nullptr) {
                current->waitingOnEvent = eventId;
            }
            scheduler->waitEvent(eventId);
            registers[IP] += 9;
            if (current != nullptr && current->state == ProcessState::WaitingEvent) {
                scheduler->contextSwitch(this);
            }
            break;
        }
        
        default:
            std::cerr << "Error: Unimplemented opcode " << static_cast<int>(opcode) 
                      << " at address " << registers[IP] << std::endl;
            scheduler->terminateCurrentProcess();
            break;
    }
}

// Stop CPU execution
void CPU::stop() {
    running = false;
}