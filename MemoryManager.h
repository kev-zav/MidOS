//***************************************************************
// Kevin Zavala
// Z2045582
//
// Handles virtual to physical address translation.
// page faults, LRU eviction and shared memory.
//***************************************************************
#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "PhysicalMemory.h"
#include "PageEntry.h"
#include <vector>
#include <string>
#include <fstream>

class Scheduler;  // forward declaration

class MemoryManager {
private:
    PhysicalMemory* physicalMemory;
    Scheduler* scheduler;  //  Records page faults on the current process

    static const int PAGE_SIZE = 256;
    static const int OFFSET_BITS = 8;
    static const int OFFSET_MASK = 0xFF;
    static const int PAGE_MASK = 0xFFFFFF;

    // The current process's virtual page table
    std::vector<PageEntry>* currentPageTable;

    // Tracks which physical pages are free
    std::vector<bool> freePhysicalPages;
    int totalPhysicalPages;

    // Global clock tick counter/used for LRU tracking
    int clockTick;

    // Swap file/pages evicted from physical memory are written here
    std::fstream swapFile;
    std::string swapFileName;
    int nextSwapOffset;
    bool invalidMemoryAccess;

    // Shared memory pages (2 regions)
    static const int NUM_SHARED_REGIONS = 10;
    static const int SHARED_REGION_SIZE = 1000;
    int sharedMemoryPages[NUM_SHARED_REGIONS];

    // Translate virtual address to physical
    // isWrite=true marks the page dirty
    int translateAddress(int virtualAddress, bool isWrite = false);

    // Find and evict the least recently used page to free up physical memory
    int evictPage();

    // Write/read one page to/from the swap file
    void writePageToSwap(int physicalPage, int swapOffset);
    void readPageFromSwap(int swapOffset, int physicalPage);

public:
    MemoryManager(PhysicalMemory* pm);
    ~MemoryManager();

    // Basic memory access
    uint8_t read(int address);
    void write(int address, uint8_t value);
    int32_t readInt(int address);
    void writeInt(int address, int32_t value);

    int getSize();

    // Page management
    int allocatePage();
    void mapPage(int virtualPage, int physicalPage);
    void unmapPage(int virtualPage);
    int getPhysicalPage(int virtualPage);
    int getPageSize();
    int getTotalPages();
    void setPageTable(std::vector<PageEntry>* pt);
    void freePage(int physicalPage);
    void printPageTable();
    bool wasInvalidAccess() { return invalidMemoryAccess; }
    void clearInvalidAccess() { invalidMemoryAccess = false; }
    void setScheduler(Scheduler* s) { scheduler = s; }

    // Handle a page fault/brings a page back from disk into physical memory
    void handlePageFault(int virtualPage, std::vector<PageEntry>* pageTable);

    // LRU clock
    void tickClock() { clockTick++; }
    int getClockTick() { return clockTick; }

    // Shared Memory 
    void initSharedMemory();
    int mapSharedMemory(int regionId, std::vector<PageEntry>* processPageTable);
    int getSharedPagePhysical(int regionId);
};

#endif