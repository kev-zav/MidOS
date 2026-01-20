#include "MemoryManager.h"

// Constructor
MemoryManager::MemoryManager(PhysicalMemory* pm) {
    physicalMemory = pm;
}

// Read 1 byte
uint8_t MemoryManager::read(int address) {
    return physicalMemory->read(address);
}

// Write 1 byte
void MemoryManager::write(int address, uint8_t value) {
    physicalMemory->write(address, value);
}

// Read 4 bytes as integer
int32_t MemoryManager::readInt(int address) {
    return physicalMemory->readInt(address);
}

// Write 4 bytes
void MemoryManager::writeInt(int address, int32_t value) {
    physicalMemory->writeInt(address, value);
}

// Get size
int MemoryManager::getSize() {
    return physicalMemory->getSize();
}