#include <iostream>
#include "Procesy.h"
#include "MemoryManager.h"
#include "Shell.h"

int main() {
	Shell shell;
	shell.boot();
    MemoryManager memoryManager;
    memoryManager.memoryInit();
    //memoryManager.showFrames();

    auto test = memoryManager.createPageList(16,1);
    //memoryManager.showPageTable(test);

    memoryManager.showMem();
    //memoryManager.showFrames();
    //memoryManager.showPageFile();

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