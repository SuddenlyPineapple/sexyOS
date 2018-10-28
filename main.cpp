#include <iostream>
#include "MemoryManager.h"

int main() {
    MemoryManager memory;

    memory.showLRUStack();
    memory.stackUpdate(12);
    memory.stackUpdate(13);
    memory.stackUpdate(1);
    memory.stackUpdate(5);
    memory.stackUpdate(16);
    memory.showLRUStack();

    return 0;
}