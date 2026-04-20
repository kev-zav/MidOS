#ifndef PAGEENTRY_H
#define PAGEENTRY_H

// Represents one entry in a virtual page table.
struct PageEntry {
    int physicalPage;   // Which physical page this maps to. -1 = not in memory.
    bool isValid;       // true = page is currently in physical memory
    bool isDirty;       // true = page has been written to since loaded (needs saving to disk)
    int lastUsed;       // Clock tick when this page was last accessed (for LRU eviction)
    int diskOffset;     // Where this page's data lives in the swap file (-1 = never swapped)
    int processId;      // Which process owns this page (needed for eviction bookkeeping)

    PageEntry() {
        physicalPage = -1;
        isValid = false;
        isDirty = false;
        lastUsed = 0;
        diskOffset = -1;
        processId = -1;
    }
};

#endif