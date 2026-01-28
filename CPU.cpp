#include "CPU.h"
#include <iostream>

CPU::CPU(MemoryManager* mm) {
    memoryManager = mm;
    
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

void CPU::run() {
    running = true;
    
    while (running) {
        executeInstruction();
    }
}

// Execute single instruction at current IP
// All instructions are 9 bytes: opcode(1) + arg1(4) + arg2(4)
void CPU::executeInstruction() {
    Opcode opcode = static_cast<Opcode>(memoryManager->read(registers[IP]));
    incrementClock();
    
    switch(opcode) {
        case Opcode::NOOP:
            registers[IP]++;
            break;
            
        case Opcode::EXIT:
            running = false;
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
        
        // For 3-arg instructions: pack src1 and src2 into arg2 (lower/upper 16 bits)
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
                running = false;
            } else {
                int32_t result = getRegister(srcReg1) / divisor;
                setRegister(destReg, result);
                
                zeroFlag = (result == 0);
                signFlag = (result < 0);
            }
            
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTR: {
            int32_t regNum = memoryManager->readInt(registers[IP] + 1);
            std::cout << getRegister(regNum) << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::PRINTM: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            int32_t value = memoryManager->readInt(address);
            std::cout << value << std::endl;
            registers[IP] += 9;
            break;
        }
        
        case Opcode::JMP: {
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
        
        case Opcode::CMP: {
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
        
        case Opcode::CALL: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            push(registers[IP] + 9);
            registers[IP] = address;
            break;
        }
        
        case Opcode::RET: {
            registers[IP] = pop();
            break;
        }
        
        default:
            std::cerr << "Error: Unimplemented opcode " << static_cast<int>(opcode) 
                      << " at address " << registers[IP] << std::endl;
            running = false;
            break;
    }
}

// Stop CPU execution
void CPU::stop() {
    running = false;
}