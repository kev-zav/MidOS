//***************************************************************************************************
// Kevin Zavala
// Z2045582
// CSCI 480
//
// Reads asm files line by line, strips comments, parses 
// opcodes and arguments, and encodes each instruction
// as 9 bytes into a bytecode array for the CPU to execute.
//***************************************************************************************************
#include "Program.h"
#include "Opcode.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

Program::Program() {
}

const std::vector<uint8_t>& Program::getBytecode() const {
    return bytecode;
}

int Program::getSize() const {
    return bytecode.size();
}

std::string Program::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string Program::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> Program::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        token = trim(token);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

bool Program::isNumber(const std::string& str) {
    if (str.empty()) return false;
    size_t start = (str[0] == '-' || str[0] == '+') ? 1 : 0;
    for (size_t i = start; i < str.length(); i++) {
        if (!isdigit(str[i])) return false;
    }
    return true;
}

bool Program::isRegister(const std::string& str) {
    if (str.length() < 2) return false;
    if (str[0] != 'r' && str[0] != 'R') return false;
    std::string numPart = str.substr(1);
    return isNumber(numPart);
}

int Program::parseOperand(const std::string& operand) {
    std::string op = trim(operand);
    
    if (op[0] == '#') {
        return std::stoi(op.substr(1));
    }
    
    if (op[0] == '@') {
        return static_cast<int>(op[1]);
    }
    
    if (isRegister(op)) {
        return std::stoi(op.substr(1));
    }
    
    return std::stoi(op);
}

uint8_t Program::getOpcodeValue(const std::string& mnemonic) {
    std::string upper = toUpper(mnemonic);
    
    if (upper == "NOOP") return static_cast<uint8_t>(Opcode::NOOP);
    if (upper == "INCR") return static_cast<uint8_t>(Opcode::INCR);
    if (upper == "ADDI") return static_cast<uint8_t>(Opcode::ADDI);
    if (upper == "ADDR") return static_cast<uint8_t>(Opcode::ADDR);
    if (upper == "PUSHR") return static_cast<uint8_t>(Opcode::PUSHR);
    if (upper == "PUSHI") return static_cast<uint8_t>(Opcode::PUSHI);
    if (upper == "MOVI") return static_cast<uint8_t>(Opcode::MOVI);
    if (upper == "MOVR") return static_cast<uint8_t>(Opcode::MOVR);
    if (upper == "MOVMR") return static_cast<uint8_t>(Opcode::MOVMR);
    if (upper == "MOVRM") return static_cast<uint8_t>(Opcode::MOVRM);
    if (upper == "MOVMM") return static_cast<uint8_t>(Opcode::MOVMM);
    if (upper == "PRINTR") return static_cast<uint8_t>(Opcode::PRINTR);
    if (upper == "PRINTM") return static_cast<uint8_t>(Opcode::PRINTM);
    if (upper == "PRINTCR") return static_cast<uint8_t>(Opcode::PRINTCR);
    if (upper == "PRINTCM") return static_cast<uint8_t>(Opcode::PRINTCM);
    if (upper == "JMP") return static_cast<uint8_t>(Opcode::JMP);
    if (upper == "JMPI") return static_cast<uint8_t>(Opcode::JMPI);
    if (upper == "JMPA") return static_cast<uint8_t>(Opcode::JMPA);
    if (upper == "JMPE") return static_cast<uint8_t>(Opcode::JMPE);
    if (upper == "JMPNE") return static_cast<uint8_t>(Opcode::JMPNE);
    if (upper == "JMPLT") return static_cast<uint8_t>(Opcode::JMPLT);
    if (upper == "JLT") return static_cast<uint8_t>(Opcode::JLT);
    if (upper == "JLTI") return static_cast<uint8_t>(Opcode::JLTI);
    if (upper == "JLTA") return static_cast<uint8_t>(Opcode::JLTA);
    if (upper == "JMPGT") return static_cast<uint8_t>(Opcode::JMPGT);
    if (upper == "JGT") return static_cast<uint8_t>(Opcode::JGT);
    if (upper == "JGTI") return static_cast<uint8_t>(Opcode::JGTI);
    if (upper == "JGTA") return static_cast<uint8_t>(Opcode::JGTA);
    if (upper == "JE") return static_cast<uint8_t>(Opcode::JE);
    if (upper == "JEI") return static_cast<uint8_t>(Opcode::JEI);
    if (upper == "JEA") return static_cast<uint8_t>(Opcode::JEA);
    if (upper == "CALL") return static_cast<uint8_t>(Opcode::CALL);
    if (upper == "CALLM") return static_cast<uint8_t>(Opcode::CALLM);
    if (upper == "RET") return static_cast<uint8_t>(Opcode::RET);
    if (upper == "CMPI") return static_cast<uint8_t>(Opcode::CMPI);
    if (upper == "CMPR") return static_cast<uint8_t>(Opcode::CMPR);
    if (upper == "SUB") return static_cast<uint8_t>(Opcode::SUB);
    if (upper == "MUL") return static_cast<uint8_t>(Opcode::MUL);
    if (upper == "DIV") return static_cast<uint8_t>(Opcode::DIV);
    if (upper == "MOD") return static_cast<uint8_t>(Opcode::MOD);
    if (upper == "AND") return static_cast<uint8_t>(Opcode::AND);
    if (upper == "OR") return static_cast<uint8_t>(Opcode::OR);
    if (upper == "POPR") return static_cast<uint8_t>(Opcode::POPR);
    if (upper == "POPM") return static_cast<uint8_t>(Opcode::POPM);
    if (upper == "EXIT") return static_cast<uint8_t>(Opcode::EXIT);
    if (upper == "SLEEP") return static_cast<uint8_t>(Opcode::SLEEP);
    if (upper == "SETPRIORITY") return static_cast<uint8_t>(Opcode::SETPRIORITY);
    if (upper == "SETPRIORITYI") return static_cast<uint8_t>(Opcode::SETPRIORITYI);
    if (upper == "INPUT") return static_cast<uint8_t>(Opcode::INPUT);
    if (upper == "INPUTC") return static_cast<uint8_t>(Opcode::INPUTC);
    if (upper == "MAPSHAREDMEM") return static_cast<uint8_t>(Opcode::MAP_SHARED_MEM);
    if (upper == "ACQUIRELOCK") return static_cast<uint8_t>(Opcode::ACQUIRE_LOCK);
    if (upper == "ACQUIRELOCKI") return static_cast<uint8_t>(Opcode::ACQUIRE_LOCK_I);
    if (upper == "RELEASELOCK") return static_cast<uint8_t>(Opcode::RELEASE_LOCK);
    if (upper == "RELEASELOCKI") return static_cast<uint8_t>(Opcode::RELEASE_LOCK_I);
    if (upper == "SIGNALEVENT") return static_cast<uint8_t>(Opcode::SIGNAL_EVENT);
    if (upper == "SIGNALEVENTI") return static_cast<uint8_t>(Opcode::SIGNAL_EVENT_I);
    if (upper == "WAITEVENT") return static_cast<uint8_t>(Opcode::WAIT_EVENT);
    if (upper == "WAITEVENTI") return static_cast<uint8_t>(Opcode::WAIT_EVENT_I);
    if (upper == "TERMINATEPROCESS") return static_cast<uint8_t>(Opcode::TERMINATE_PROCESS);
    
    // Heap allocation
    if (upper == "ALLOC") return static_cast<uint8_t>(Opcode::ALLOC);
    if (upper == "FREEMEMORY") return static_cast<uint8_t>(Opcode::FREE_MEMORY);
    
    std::cerr << "Error: Unknown opcode '" << mnemonic << "'" << std::endl;
    return static_cast<uint8_t>(Opcode::INVALID);
}

bool Program::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }
    
    bytecode.clear();
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        line = trim(line);
        
        if (line.empty()) continue;
        
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) continue;
        
        std::string opcode = tokens[0];
        uint8_t opcodeValue = getOpcodeValue(opcode);
        
        if (opcodeValue == static_cast<uint8_t>(Opcode::INVALID)) {
            std::cerr << "Line " << lineNumber << ": Invalid opcode" << std::endl;
            return false;
        }
        
        bytecode.push_back(opcodeValue);
        
        int arg1 = 0, arg2 = 0;
        
        if (tokens.size() > 1) {
            std::string operands = "";
            for (size_t i = 1; i < tokens.size(); i++) {
                operands += tokens[i];
            }
            
            std::vector<std::string> args = split(operands, ',');
            
            if (args.size() > 0) {
                arg1 = parseOperand(args[0]);
            }
            if (args.size() > 1) {
                arg2 = parseOperand(args[1]);
            }
            if (args.size() > 2) {
                int arg3 = parseOperand(args[2]);
                arg2 = (arg2 & 0xFFFF) | ((arg3 & 0xFFFF) << 16);
            }
        }
        
        bytecode.push_back(arg1 & 0xFF);
        bytecode.push_back((arg1 >> 8) & 0xFF);
        bytecode.push_back((arg1 >> 16) & 0xFF);
        bytecode.push_back((arg1 >> 24) & 0xFF);
        
        bytecode.push_back(arg2 & 0xFF);
        bytecode.push_back((arg2 >> 8) & 0xFF);
        bytecode.push_back((arg2 >> 16) & 0xFF);
        bytecode.push_back((arg2 >> 24) & 0xFF);
    }
    
    file.close();
    return true;
}