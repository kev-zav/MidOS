//***********************************************
// Kevin Zavala
// Z2045582
//
// Defines one entry in a process's page table.
// Maps a virtual page to a physical page in RAM
//*********************************************
#ifndef PAGEENTRY_H
#define PAGEENTRY_H

// Represents one entry in a virtual page table.
struct PageEntry {
    int physicalPage;   // Which physical page this maps to,, -1 = not in memory.
    bool isValid;       // Is it currently in RAM
    bool isDirty;       // Has this page been written to since it was loaded
    int lastUsed;       // Clock tick when this page was last accessed
    int diskOffset;     // Where this page's data lives in the swap file
    int processId;      // Which process owns this page

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