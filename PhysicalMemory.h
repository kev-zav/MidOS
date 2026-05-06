//***************************************************************************************************
// Kevin Zavala
// Z2045582
//
// Simulates raw RAM as a byte array.
// Supports single byte and 4-byte integer
// reads and writes with bounds checking.
//***************************************************************************************************
#ifndef PHYSICALMEMORY_H
#define PHYSICALMEMORY_H

#include <vector>
#include <cstdint>

class PhysicalMemory {
private:
    std::vector<uint8_t> memory;  // The actual memory
    
public:
    PhysicalMemory(int size);  // Creates memory of given size
    
    uint8_t read(int address); 
    void write(int address, uint8_t value);
    
    int32_t readInt(int address);
    void writeInt(int address, int32_t value);
     
    int getSize();  // Get total memory size
};

#endif