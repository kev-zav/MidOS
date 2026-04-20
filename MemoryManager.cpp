#include "MemoryManager.h"
#include "Scheduler.h"
#include <iostream>

MemoryManager::MemoryManager(PhysicalMemory* pm) {
    physicalMemory = pm;
    scheduler = nullptr;
    totalPhysicalPages = pm->getSize() / PAGE_SIZE;
    freePhysicalPages.resize(totalPhysicalPages, true);
    currentPageTable = nullptr;
    clockTick = 0;
    nextSwapOffset = 0;

    // Open the swap file (creates it if it doesn't exist)
    swapFileName = "swap.bin";
    swapFile.open(swapFileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
    if (!swapFile.is_open()) {
        std::cerr << "Warning: Could not open swap file. Virtual memory will be limited." << std::endl;
    }
}

MemoryManager::~MemoryManager() {
    if (swapFile.is_open()) {
        swapFile.close();
    }
    std::remove(swapFileName.c_str());
}

// Address translation
int MemoryManager::translateAddress(int virtualAddress, bool isWrite) {
    int virtualPage = (virtualAddress >> OFFSET_BITS) & PAGE_MASK;
    int offset = virtualAddress & OFFSET_MASK;

    if (currentPageTable == nullptr) {
        std::cerr << "Error: No page table set for address " << virtualAddress << std::endl;
        return -1;
    }

    if (virtualPage >= (int)currentPageTable->size() || 
        (*currentPageTable)[virtualPage].physicalPage == -1) {
        std::cerr << "Error: Virtual page " << virtualPage 
                  << " is not mapped (invalid access)" << std::endl;
        return -1;
    }

    PageEntry& entry = (*currentPageTable)[virtualPage];

    // Page fault: page is mapped but not currently in physical memory
    if (!entry.isValid) {
        handlePageFault(virtualPage, currentPageTable);
        if (!entry.isValid) {
            std::cerr << "Error: Page fault could not be resolved for virtual page " 
                      << virtualPage << std::endl;
            return -1;
        }
    }

    // Update LRU timestamp on every access
    entry.lastUsed = clockTick;

    // If this is a write, mark the page dirty
    if (isWrite) {
        entry.isDirty = true;
    }

    return (entry.physicalPage * PAGE_SIZE) + offset;
}

uint8_t MemoryManager::read(int address) {
    int physical = translateAddress(address, false);
    if (physical == -1) return 0;
    return physicalMemory->read(physical);
}

void MemoryManager::write(int address, uint8_t value) {
    int physical = translateAddress(address, true);
    if (physical == -1) return;
    physicalMemory->write(physical, value);
}

int32_t MemoryManager::readInt(int address) {
    int physical = translateAddress(address, false);
    if (physical == -1) return 0;
    return physicalMemory->readInt(physical);
}

void MemoryManager::writeInt(int address, int32_t value) {
    int physical = translateAddress(address, true);
    if (physical == -1) return;
    physicalMemory->writeInt(physical, value);
}

int MemoryManager::getSize() {
    return physicalMemory->getSize();
}

// Page fault handler
void MemoryManager::handlePageFault(int virtualPage, std::vector<PageEntry>* pageTable) {
    PageEntry& entry = (*pageTable)[virtualPage];

    std::cout << "[VM] Page fault on virtual page " << virtualPage << std::endl;

    // Tell the scheduler to count this page fault for the current process
    if (scheduler != nullptr) {
        scheduler->recordPageFault();
    }

    // Find a free physical page
    int physPage = -1;
    for (int i = 0; i < totalPhysicalPages; i++) {
        if (freePhysicalPages[i]) {
            physPage = i;
            freePhysicalPages[i] = false;
            break;
        }
    }

    if (physPage == -1) {
        // No free pages - need to evict someone
        physPage = evictPage();
        if (physPage == -1) {
            std::cerr << "Error: Cannot resolve page fault - no memory available" << std::endl;
            return;
        }
    }

    // If the page was previously swapped out, read it back in
    if (entry.diskOffset != -1) {
        readPageFromSwap(entry.diskOffset, physPage);
        std::cout << "[VM] Swapped in virtual page " << virtualPage
                  << " from disk offset " << entry.diskOffset
                  << " to physical page " << physPage << std::endl;
    } else {
        // First time - zero it out
        for (int i = 0; i < PAGE_SIZE; i++) {
            physicalMemory->write(physPage * PAGE_SIZE + i, 0);
        }
    }

    entry.physicalPage = physPage;
    entry.isValid = true;
    entry.isDirty = false;
    entry.lastUsed = clockTick;
}

int MemoryManager::evictPage() {
    int lruPhysPage = -1;
    int oldestTick = INT32_MAX;
    PageEntry* victimEntry = nullptr;

    if (currentPageTable == nullptr) return -1;

    for (PageEntry& entry : *currentPageTable) {
        if (entry.isValid && entry.physicalPage != -1) {
            // Don't evict shared memory pages
            bool isShared = false;
            for (int i = 0; i < NUM_SHARED_PAGES; i++) {
                if (entry.physicalPage == sharedMemoryPages[i]) {
                    isShared = true;
                    break;
                }
            }
            if (isShared) continue;

            if (entry.lastUsed < oldestTick) {
                oldestTick = entry.lastUsed;
                lruPhysPage = entry.physicalPage;
                victimEntry = &entry;
            }
        }
    }

    if (lruPhysPage == -1 || victimEntry == nullptr) return -1;

    // If dirty save to disk/If clean just discard it.
    if (victimEntry->isDirty) {
        if (victimEntry->diskOffset == -1) {
            victimEntry->diskOffset = nextSwapOffset++;
        }
        writePageToSwap(lruPhysPage, victimEntry->diskOffset);
        std::cout << "[VM] Evicted dirty physical page " << lruPhysPage
                  << " to disk offset " << victimEntry->diskOffset << std::endl;
    } else {
        std::cout << "[VM] Evicted clean physical page " << lruPhysPage
                  << " (no disk write needed)" << std::endl;
    }

    victimEntry->isValid = false;
    victimEntry->physicalPage = -1;
    freePhysicalPages[lruPhysPage] = true;

    return lruPhysPage;
}

void MemoryManager::writePageToSwap(int physicalPage, int swapOffset) {
    if (!swapFile.is_open()) return;
    swapFile.seekp(swapOffset * PAGE_SIZE, std::ios::beg);
    for (int i = 0; i < PAGE_SIZE; i++) {
        uint8_t byte = physicalMemory->read(physicalPage * PAGE_SIZE + i);
        swapFile.write(reinterpret_cast<char*>(&byte), 1);
    }
    swapFile.flush();
}

void MemoryManager::readPageFromSwap(int swapOffset, int physicalPage) {
    if (!swapFile.is_open()) return;
    swapFile.seekg(swapOffset * PAGE_SIZE, std::ios::beg);
    for (int i = 0; i < PAGE_SIZE; i++) {
        uint8_t byte = 0;
        swapFile.read(reinterpret_cast<char*>(&byte), 1);
        physicalMemory->write(physicalPage * PAGE_SIZE + i, byte);
    }
}

// Page allocation and mapping
int MemoryManager::allocatePage() {
    for (int i = 0; i < totalPhysicalPages; i++) {
        if (freePhysicalPages[i]) {
            freePhysicalPages[i] = false;
            return i;
        }
    }
    std::cout << "[VM] Physical memory full - evicting LRU page..." << std::endl;
    return evictPage();
}

void MemoryManager::mapPage(int virtualPage, int physicalPage) {
    if (currentPageTable == nullptr) return;
    if (virtualPage >= (int)currentPageTable->size()) {
        currentPageTable->resize(virtualPage + 1);
    }
    PageEntry& entry = (*currentPageTable)[virtualPage];
    entry.physicalPage = physicalPage;
    entry.isValid = true;
    entry.isDirty = false;
    entry.lastUsed = clockTick;

    if (physicalPage >= 0 && physicalPage < totalPhysicalPages) {
        freePhysicalPages[physicalPage] = false;
    }
}

void MemoryManager::unmapPage(int virtualPage) {
    if (currentPageTable == nullptr) return;
    if (virtualPage < (int)currentPageTable->size()) {
        PageEntry& entry = (*currentPageTable)[virtualPage];
        if (entry.physicalPage != -1) {
            freePhysicalPages[entry.physicalPage] = true;
        }
        entry = PageEntry();  // reset to defaults
    }
}

int MemoryManager::getPhysicalPage(int virtualPage) {
    if (currentPageTable == nullptr || 
        virtualPage >= (int)currentPageTable->size()) return -1;
    return (*currentPageTable)[virtualPage].physicalPage;
}

int MemoryManager::getPageSize() { return PAGE_SIZE; }
int MemoryManager::getTotalPages() { return totalPhysicalPages; }

void MemoryManager::setPageTable(std::vector<PageEntry>* pt) {
    currentPageTable = pt;
}

void MemoryManager::freePage(int physicalPage) {
    if (physicalPage >= 0 && physicalPage < totalPhysicalPages) {
        freePhysicalPages[physicalPage] = true;
    }
}

void MemoryManager::printPageTable() {
    std::cout << "\n=== Page Table ===" << std::endl;
    if (currentPageTable == nullptr) {
        std::cout << "  No page table set" << std::endl;
        std::cout << "==================\n" << std::endl;
        return;
    }
    for (int i = 0; i < (int)currentPageTable->size(); i++) {
        PageEntry& e = (*currentPageTable)[i];
        if (e.physicalPage != -1 || e.diskOffset != -1) {
            std::cout << "  VPage " << i
                      << " -> PPage " << e.physicalPage
                      << " | Valid:" << e.isValid
                      << " Dirty:" << e.isDirty
                      << " LastUsed:" << e.lastUsed
                      << " DiskOffset:" << e.diskOffset
                      << std::endl;
        }
    }
    std::cout << "==================\n" << std::endl;
}

// Shared memory
void MemoryManager::initSharedMemory() {
    for (int i = 0; i < NUM_SHARED_PAGES; i++) {
        sharedMemoryPages[i] = allocatePage();
        std::cout << "Shared memory region " << (i + 1)
                  << " at physical page " << sharedMemoryPages[i] << std::endl;
    }
}

int MemoryManager::mapSharedMemory(int regionId, std::vector<PageEntry>* processPageTable) {
    if (regionId < 1 || regionId > NUM_SHARED_PAGES) {
        std::cerr << "Error: Invalid shared memory region " << regionId << std::endl;
        return -1;
    }

    int physicalPage = sharedMemoryPages[regionId - 1];
    int virtualPage = processPageTable->size();
    processPageTable->resize(virtualPage + 1);

    PageEntry& entry = (*processPageTable)[virtualPage];
    entry.physicalPage = physicalPage;
    entry.isValid = true;
    entry.isDirty = false;
    entry.lastUsed = clockTick;
    entry.diskOffset = -1;

    return virtualPage * PAGE_SIZE;
}

int MemoryManager::getSharedPagePhysical(int regionId) {
    if (regionId < 1 || regionId > NUM_SHARED_PAGES) return -1;
    return sharedMemoryPages[regionId - 1];
}