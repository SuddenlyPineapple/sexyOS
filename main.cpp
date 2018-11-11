#include <iostream>
#include "Procesy.h"
#include "MemoryManager.h"

int main() {
    MemoryManager memoryManager;
    memoryManager.memoryInit();
    //memoryManager.showFrames();

    auto test = memoryManager.createPageList(120,10);
    //memoryManager.showPageTable(test);

    //memoryManager.showMem();


    /*
    memoryManager.showLRUStack();
    memoryManager.stackUpdate(12);
    memoryManager.stackUpdate(13);
    memoryManager.stackUpdate(1);
    memoryManager.stackUpdate(5);
    memoryManager.stackUpdate(16);
    memoryManager.showStack();
    */
    return 0;
}