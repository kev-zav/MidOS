#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "PhysicalMemory.h"
#include <vector>

class MemoryManager {
private:
    PhysicalMemory* physicalMemory;

    static const int PAGE_SIZE = 256;
    static const int OFFSET_BITS = 8;
    static const int OFFSET_MASK = 0xFF;
    static const int PAGE_MASK = 0xFFFFFF;
    
    //Page table
    std::vector<int>* pageTable;

    //Tracks which pages are free
    std::vector<bool> freePages;

    int totalPhysicalPages;

    //Translates virtual address to physical
    int translateAddress(int virtualAddress);

public:
    MemoryManager(PhysicalMemory* pm);
    
    uint8_t read(int address);
    void write(int address, uint8_t value);
    
    int32_t readInt(int address);
    void writeInt(int address, int32_t value);
    
    int getSize();

    //Page management
    int allocatePage();
    void mapPage(int virtualPage, int physicalPage);
    void unmapPage(int virtualPage);
    int getPhysicalPage(int virtualPage);

    int getPageSize();
    int getTotalPages();
    void setPageTable(std::vector<int>* pt);
    void freePage(int physicalPage);
    void printPageTable();
};

#endif