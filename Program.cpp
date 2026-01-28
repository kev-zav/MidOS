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
    
    // Check for constant (#123)
    if (op[0] == '#') {
        return std::stoi(op.substr(1));
    }
    
    // Check for character (@a)
    if (op[0] == '@') {
        return static_cast<int>(op[1]);
    }
    
    // Check for register (r1)
    if (isRegister(op)) {
        return std::stoi(op.substr(1));
    }
    
    // Otherwise it's a direct number
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
    if (upper == "JMP") return static_cast<uint8_t>(Opcode::JMP);
    if (upper == "JMPE") return static_cast<uint8_t>(Opcode::JMPE);
    if (upper == "JMPNE") return static_cast<uint8_t>(Opcode::JMPNE);
    if (upper == "JMPLT") return static_cast<uint8_t>(Opcode::JMPLT);
    if (upper == "JMPGT") return static_cast<uint8_t>(Opcode::JMPGT);
    if (upper == "CALL") return static_cast<uint8_t>(Opcode::CALL);
    if (upper == "RET") return static_cast<uint8_t>(Opcode::RET);
    if (upper == "CMP") return static_cast<uint8_t>(Opcode::CMP);
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
    if (upper == "INPUT") return static_cast<uint8_t>(Opcode::INPUT);
    
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
        
        // Remove comments (everything after ;)
        size_t commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }
        
        // Trim whitespace
        line = trim(line);
        
        // Skip empty lines
        if (line.empty()) continue;
        
        // Split into tokens
        std::vector<std::string> tokens = split(line, ' ');
        if (tokens.empty()) continue;
        
        // First token is the opcode
        std::string opcode = tokens[0];
        uint8_t opcodeValue = getOpcodeValue(opcode);
        
        if (opcodeValue == static_cast<uint8_t>(Opcode::INVALID)) {
            std::cerr << "Line " << lineNumber << ": Invalid opcode" << std::endl;
            return false;
        }
        
        // Add opcode to bytecode
        bytecode.push_back(opcodeValue);
        
        // Parse operands
        int arg1 = 0, arg2 = 0;
        
        if (tokens.size() > 1) {
            // Combine all tokens after opcode
            std::string operands = "";
            for (size_t i = 1; i < tokens.size(); i++) {
                operands += tokens[i];
            }
            
            // Split by comma
            std::vector<std::string> args = split(operands, ',');
            
            if (args.size() > 0) {
                arg1 = parseOperand(args[0]);
            }
            if (args.size() > 1) {
                arg2 = parseOperand(args[1]);
            }
            // Handle 3-arg instructions (ADDR, SUB, MUL, DIV)
            // Pack arg2 and arg3 into a single 32-bit value
            if (args.size() > 2) {
                int arg3 = parseOperand(args[2]);
                arg2 = (arg2 & 0xFFFF) | ((arg3 & 0xFFFF) << 16);
            }
        }
        
        // Add arguments as 4-byte integers (little-endian)
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