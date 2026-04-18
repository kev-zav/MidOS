#include "MemoryManager.h"
#include <iostream>

MemoryManager::MemoryManager(PhysicalMemory* pm) {
    physicalMemory = pm;
    totalPhysicalPages = pm->getSize() / PAGE_SIZE;
    freePages.resize(totalPhysicalPages, true);
    pageTable = nullptr;
}

int MemoryManager::translateAddress(int virtualAddress) {
    int virtualPage = (virtualAddress >> OFFSET_BITS) & PAGE_MASK;
    int offset = virtualAddress & OFFSET_MASK;

    if (pageTable == nullptr || virtualPage >= pageTable->size() || (*pageTable)[virtualPage] == -1) {
        std::cerr << "Error: Page fault at virtual page " << virtualPage << std::endl;
        return -1;
    }

    int physicalPage = (*pageTable)[virtualPage];
    return (physicalPage * PAGE_SIZE) + offset;
}

uint8_t MemoryManager::read(int address) {
    int physical = translateAddress(address);
    if (physical == -1) return 0;
    return physicalMemory->read(physical);
}

void MemoryManager::write(int address, uint8_t value) {
    int physical = translateAddress(address);
    if (physical == -1) return;
    physicalMemory->write(physical, value);
}

int32_t MemoryManager::readInt(int address) {
    int physical = translateAddress(address);
    if (physical == -1) return 0;
    return physicalMemory->readInt(physical);
}

void MemoryManager::writeInt(int address, int32_t value) {
    int physical = translateAddress(address);
    if (physical == -1) return;
    physicalMemory->writeInt(physical, value);
}

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
    if (pageTable == nullptr) return;
    if (virtualPage >= pageTable->size()) {
        pageTable->resize(virtualPage + 1, -1);
    }
    (*pageTable)[virtualPage] = physicalPage;
    if (physicalPage >= 0 && physicalPage < totalPhysicalPages) {
        freePages[physicalPage] = false;
    }
}

void MemoryManager::unmapPage(int virtualPage) {
    if (pageTable == nullptr) return;
    if (virtualPage < pageTable->size() && (*pageTable)[virtualPage] != -1) {
        freePages[(*pageTable)[virtualPage]] = true;
        (*pageTable)[virtualPage] = -1;
    }
}

int MemoryManager::getPhysicalPage(int virtualPage) {
    if (pageTable == nullptr || virtualPage >= pageTable->size()) return -1;
    return (*pageTable)[virtualPage];
}

int MemoryManager::getPageSize() {
    return PAGE_SIZE;
}

int MemoryManager::getTotalPages() {
    return totalPhysicalPages;
}

void MemoryManager::printPageTable() {
    std::cout << "\n=== Page Table ===" << std::endl;
    if (pageTable == nullptr) {
        std::cout << "  No page table set" << std::endl;
        std::cout << "==================\n" << std::endl;
        return;
    }
    for (int i = 0; i < pageTable->size(); i++) {
        if ((*pageTable)[i] != -1) {
            std::cout << "  Virtual " << i << " -> Physical " << (*pageTable)[i] << std::endl;
        }
    }
    std::cout << "==================\n" << std::endl;
}

void MemoryManager::setPageTable(std::vector<int>* pt) {
    pageTable = pt;
}

void MemoryManager::freePage(int physicalPage) {
    if (physicalPage >= 0 && physicalPage < totalPhysicalPages) {
        freePages[physicalPage] = true;
    }
}

void MemoryManager::initSharedMemory() {
    for (int i = 0; i < NUM_SHARED_PAGES; i++) {
        sharedMemoryPages[i] = allocatePage();
        std::cout << "Shared memory region " << (i + 1) << " at physical page " << sharedMemoryPages[i] << std::endl;
    }
}

int MemoryManager::mapSharedMemory(int regionId, std::vector<int>* processPageTable) {
    if (regionId < 1 || regionId > NUM_SHARED_PAGES) {
        std::cerr << "Error: Invalid shared memory region " << regionId << std::endl;
        return -1;
    }
    
    int physicalPage = sharedMemoryPages[regionId - 1];
    int virtualPage = processPageTable->size();
    processPageTable->resize(virtualPage + 1, -1);
    (*processPageTable)[virtualPage] = physicalPage;
    
    return virtualPage * PAGE_SIZE;
}

int MemoryManager::getSharedPagePhysical(int regionId) {
    if (regionId < 1 || regionId > NUM_SHARED_PAGES) {
        return -1;
    }
    return sharedMemoryPages[regionId - 1];
}