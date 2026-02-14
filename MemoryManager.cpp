#include "MemoryManager.h"
#include <iostream>

// Constructor
MemoryManager::MemoryManager(PhysicalMemory* pm) {
    physicalMemory = pm;
    totalPhysicalPages = pm->getSize() / PAGE_SIZE;
    freePages.resize(totalPhysicalPages, true);
    pageTable.clear();
}

int MemoryManager::translateAddress(int virtualAddress) {
    int virtualPage = (virtualAddress >> OFFSET_BITS) & PAGE_MASK;
    int offset = virtualAddress & OFFSET_MASK;

    if (virtualPage >= pageTable.size() || pageTable[virtualPage] == -1) {
        std::cerr << "Error: Page fault at virtual page " << virtualPage << std::endl;
        return -1;
    }

    int physicalPage = pageTable[virtualPage];
    return (physicalPage * PAGE_SIZE) + offset;
}

// Read 1 byte
uint8_t MemoryManager::read(int address) {
    int physical = translateAddress(address);
    if (physical == -1) return 0;
    return physicalMemory->read(physical);
}

// Write 1 byte
void MemoryManager::write(int address, uint8_t value) {
    int physical = translateAddress(address);
    if (physical == -1) return;
    physicalMemory->write(physical, value);
}

// Read 4 bytes as integer
int32_t MemoryManager::readInt(int address) {
    int physical = translateAddress(address);
    if (physical == -1) return 0;
    return physicalMemory->readInt(physical);
}

// Write 4 bytes
void MemoryManager::writeInt(int address, int32_t value) {
    int physical = translateAddress(address);
    if (physical == -1) return;
    physicalMemory->writeInt(physical, value);
}

// Get size
int MemoryManager::getSize() {
    return physicalMemory->getSize();
}

int MemoryManager::allocatePage() {
    for (int i = 0; i < totalPhysicalPages; i++) {
        if (freePages[i]) {
            freePages[i] = false;
            return i;
        }
    }
    return -1;
}

void MemoryManager::mapPage(int virtualPage, int physicalPage) {
    if (virtualPage >= pageTable.size()) {
        pageTable.resize(virtualPage + 1, -1);
    }
    pageTable[virtualPage] = physicalPage;
    if (physicalPage >= 0 && physicalPage < totalPhysicalPages) {
        freePages[physicalPage] = false;
    }
}

void MemoryManager::unmapPage(int virtualPage) {
    if (virtualPage < pageTable.size() && pageTable[virtualPage] != -1) {
        freePages[pageTable[virtualPage]] = true;
        pageTable[virtualPage] = -1;
    }
}

int MemoryManager::getPhysicalPage(int virtualPage) {
    if (virtualPage >= pageTable.size()) return -1;
    return pageTable[virtualPage];
}

int MemoryManager::getPageSize() {
    return PAGE_SIZE;
}

int MemoryManager::getTotalPages() {
    return totalPhysicalPages;
}

void MemoryManager::printPageTable() {
    std::cout << "\n=== Page Table ===" << std::endl;
    for (int i = 0; i < pageTable.size(); i++) {
        if (pageTable[i] != -1) {
            std::cout << "  Virtual " << i << " -> Physical " << pageTable[i] << std::endl;
        }
    }
    std::cout << "==================\n" << std::endl;
}