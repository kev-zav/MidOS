#ifndef PHYSICALMEMORY_H
#define PHYSICALMEMORY_H

#include <vector>
#include <cstdint>

class PhysicalMemory {
private:
    std::vector<uint8_t> memory;  // The actual memory (array of bytes)
    
public:
    PhysicalMemory(int size);  // Constructor - creates memory of given size
    
    uint8_t read(int address);              // Read 1 byte
    void write(int address, uint8_t value); // Write 1 byte
    
    int32_t readInt(int address);           // Read 4 bytes as an integer
    void writeInt(int address, int32_t value); // Write 4 bytes
     
    int getSize();  // Get total memory size
};

#endif