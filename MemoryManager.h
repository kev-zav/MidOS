#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "PhysicalMemory.h"

class MemoryManager {
private:
    PhysicalMemory* physicalMemory;
    
public:
    MemoryManager(PhysicalMemory* pm);
    
    uint8_t read(int address);
    void write(int address, uint8_t value);
    
    int32_t readInt(int address);
    void writeInt(int address, int32_t value);
    
    int getSize();
};

#endif