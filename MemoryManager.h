//
// Created by Wojciech Kasperski on 15-Oct-18.
//

#ifndef SEXYOS_MEMORYMANAGER_H
#define SEXYOS_MEMORYMANAGER_H

#include <list>
//#include "Processes.h"

/* Tablica stronic */
struct PageTable {
    bool bit;  //Wartość bool'owska sprawdzająca zajętość tablicy w pamięci
    int frame; //Numer ramki w której znajduje się stronica
    PageTable(bool bit, int frame);
};

class MemoryManager {
public:

    MemoryManager();
    virtual ~MemoryManager();
};


#endif //SEXYOS_MEMORYMANAGER_H
