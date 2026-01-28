#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

class Program {
private:
    std::vector<uint8_t> bytecode;
    std::map<std::string, int> labels;
    
    // Helper methods for parsing
    std::string trim(const std::string& str);
    std::string toUpper(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    bool isNumber(const std::string& str);
    bool isRegister(const std::string& str);
    int parseOperand(const std::string& operand);
    uint8_t getOpcodeValue(const std::string& mnemonic);
    
public:
    Program();
    
    // Load program from assembly file
    bool loadFromFile(const std::string& filename);
    
    // Get the bytecode
    const std::vector<uint8_t>& getBytecode() const;
    
    // Get program size
    int getSize() const;
};

#endif