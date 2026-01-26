#include "CPU.h"
#include <iostream>

// Constructor
CPU::CPU(MemoryManager* mm) {
    memoryManager = mm;
    
    // Initialize all registers to 0
    for (int i = 0; i < 15; i++) {
        registers[i] = 0;
    }
    
    // Initialize flags
    signFlag = false;
    zeroFlag = false;
    
    // Initialize clock
    clockTicks = 0;
    
    // Not running yet
    running = false;
}

// Get register value
int32_t CPU::getRegister(int regNum) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return 0;
    }
    return registers[regNum];
}

// Set register value
void CPU::setRegister(int regNum, int32_t value) {
    if (regNum < 1 || regNum > 14) {
        std::cerr << "Error: Invalid register number " << regNum << std::endl;
        return;
    }
    registers[regNum] = value;
}

// Get Instruction Pointer
int32_t CPU::getIP() {
    return registers[IP];
}

// Set Instruction Pointer
void CPU::setIP(int32_t value) {
    registers[IP] = value;
}

// Get Stack Pointer
int32_t CPU::getSP() {
    return registers[SP];
}

// Set Stack Pointer
void CPU::setSP(int32_t value) {
    registers[SP] = value;
}

// Get Sign Flag
bool CPU::getSignFlag() {
    return signFlag;
}

// Set Sign Flag
void CPU::setSignFlag(bool value) {
    signFlag = value;
}

// Get Zero Flag
bool CPU::getZeroFlag() {
    return zeroFlag;
}

// Set Zero Flag
void CPU::setZeroFlag(bool value) {
    zeroFlag = value;
}

// Get clock ticks
int CPU::getClockTicks() {
    return clockTicks;
}

// Increment clock
void CPU::incrementClock() {
    clockTicks++;
}

// Push value onto stack
void CPU::push(int32_t value) {
    registers[SP] -= 4;  // Move stack pointer down by 4 bytes
    memoryManager->writeInt(registers[SP], value);
}

// Pop value from stack
int32_t CPU::pop() {
    int32_t value = memoryManager->readInt(registers[SP]);
    registers[SP] += 4;  // Move stack pointer up by 4 bytes
    return value;
}

void CPU::run() {
    running = true;
    
    while (running) {
        executeInstruction();
        incrementClock();
    }
}

void CPU::executeInstruction() {
    Opcode opcode = static_cast<Opcode>(memoryManager->read(registers[IP]));
    
    incrementClock();
    
    switch(opcode) {
        case Opcode::NOOP:
            registers[IP]++;
            break;
            
        case Opcode::EXIT:
            // Stop the CPU
            running = false;
            break;
            
        case Opcode::INCR: {
            // Format: INCR <reg_num>
            int regNum = memoryManager->read(registers[IP] + 1);
            int32_t value = getRegister(regNum);
            value++;
            setRegister(regNum, value);
            
            zeroFlag = (value == 0);
            signFlag = (value < 0);
            
            registers[IP] += 2;
            break;
        }
        
        case Opcode::MOVI: {
            // Format: MOVI <reg_num> <value (4 bytes)>
            int regNum = memoryManager->read(registers[IP] + 1);
            int32_t value = memoryManager->readInt(registers[IP] + 2);
            setRegister(regNum, value);
            registers[IP] += 6;
            break;
        }
        
        case Opcode::MOVR: {
            // Format: MOVR <dest_reg> <src_reg>
            int destReg = memoryManager->read(registers[IP] + 1);
            int srcReg = memoryManager->read(registers[IP] + 2);
            setRegister(destReg, getRegister(srcReg));
            registers[IP] += 3;
            break;
        }
        
        case Opcode::ADDI: {
            // Format: ADDI <reg_num> <value (4 bytes)>
            int regNum = memoryManager->read(registers[IP] + 1);
            int32_t value = memoryManager->readInt(registers[IP] + 2);
            int32_t result = getRegister(regNum) + value;
            setRegister(regNum, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 6;
            break;
        }
        
        case Opcode::ADDR: {
            // Add two registers, store in destination
            // Format: ADDR <dest_reg> <src_reg1> <src_reg2>
            int destReg = memoryManager->read(registers[IP] + 1);
            int srcReg1 = memoryManager->read(registers[IP] + 2);
            int srcReg2 = memoryManager->read(registers[IP] + 3);
            
            int32_t result = getRegister(srcReg1) + getRegister(srcReg2);
            setRegister(destReg, result);
            
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 4;
            break;
        }
        
        case Opcode::SUB: {
            // Subtract: dest = src1 - src2
            // Format: SUB <dest_reg> <src_reg1> <src_reg2>
            int destReg = memoryManager->read(registers[IP] + 1);
            int srcReg1 = memoryManager->read(registers[IP] + 2);
            int srcReg2 = memoryManager->read(registers[IP] + 3);
            
            int32_t result = getRegister(srcReg1) - getRegister(srcReg2);
            setRegister(destReg, result);
            
            // Update flags
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 4;
            break;
        }
        
        case Opcode::MUL: {
            // Multiply: dest = src1 * src2
            // Format: MUL <dest_reg> <src_reg1> <src_reg2>
            int destReg = memoryManager->read(registers[IP] + 1);
            int srcReg1 = memoryManager->read(registers[IP] + 2);
            int srcReg2 = memoryManager->read(registers[IP] + 3);
            
            int32_t result = getRegister(srcReg1) * getRegister(srcReg2);
            setRegister(destReg, result);
            
            // Update flags
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 4;
            break;
        }
        
        case Opcode::DIV: {
            // Divide: dest = src1 / src2
            // Format: DIV <dest_reg> <src_reg1> <src_reg2>
            int destReg = memoryManager->read(registers[IP] + 1);
            int srcReg1 = memoryManager->read(registers[IP] + 2);
            int srcReg2 = memoryManager->read(registers[IP] + 3);
            
            int32_t divisor = getRegister(srcReg2);
            if (divisor == 0) {
                std::cerr << "Error: Division by zero at IP " << registers[IP] << std::endl;
                running = false;
            } else {
                int32_t result = getRegister(srcReg1) / divisor;
                setRegister(destReg, result);
                
                // Update flags
                zeroFlag = (result == 0);
                signFlag = (result < 0);
            }
            
            registers[IP] += 4;
            break;
        }
        
        case Opcode::PRINTR: {
            // Format: PRINTR <reg_num>
            int regNum = memoryManager->read(registers[IP] + 1);
            std::cout << getRegister(regNum) << std::endl;
            registers[IP] += 2;
            break;
        }
        
        case Opcode::PRINTM: {
            // Format: PRINTM <address (4 bytes)>
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            int32_t value = memoryManager->readInt(address);
            std::cout << value << std::endl;
            registers[IP] += 5;
            break;
        }
        
        case Opcode::JMP: {
            // Format: JMP <address (4 bytes)>
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            registers[IP] = address;
            break;
        }
        
        case Opcode::JMPE: {
            // Jump if equal (zero flag set)
            // Format: JMPE <address (4 bytes)>
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 5;
            }
            break;
        }
        
        case Opcode::JMPNE: {
            // Format: JMPNE <address (4 bytes)>
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            if (!zeroFlag) {
                registers[IP] = address;
            } else {
                registers[IP] += 5;
            }
            break;
        }
        
        case Opcode::CMP: {
            // Compare two registers (sets flags)
            // Format: CMP <reg1> <reg2>
            int reg1 = memoryManager->read(registers[IP] + 1);
            int reg2 = memoryManager->read(registers[IP] + 2);
            
            int32_t result = getRegister(reg1) - getRegister(reg2);
            
            // Update flags based on comparison
            zeroFlag = (result == 0);
            signFlag = (result < 0);
            
            registers[IP] += 3;
            break;
        }
        
        case Opcode::PUSHR: {
            // Push register to stack
            // Format: PUSHR <reg_num>
            int regNum = memoryManager->read(registers[IP] + 1);
            push(getRegister(regNum));
            registers[IP] += 2;
            break;
        }
        
        case Opcode::PUSHI: {
            // Push immediate value to stack
            // Format: PUSHI <value (4 bytes)>
            int32_t value = memoryManager->readInt(registers[IP] + 1);
            push(value);
            registers[IP] += 5;
            break;
        }
        
        case Opcode::POPR: {
            // Pop from stack to register
            int regNum = memoryManager->read(registers[IP] + 1);
            setRegister(regNum, pop());
            registers[IP] += 2;
            break;
        }
        
        case Opcode::CALL: {
            int32_t address = memoryManager->readInt(registers[IP] + 1);
            
            // Push return address (current IP + 5)
            push(registers[IP] + 5);
            
            registers[IP] = address;
            break;
        }
        
        case Opcode::RET: {
            // Return from function
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

void CPU::stop() {
    running = false;
}