//
// Created by Wojciech Kasperski on 15-Oct-18.
//

#ifndef SEXYOS_MEMORYMANAGER_H
#define SEXYOS_MEMORYMANAGER_H

#include <list>
#include <string>
//#include "Processes.h"

/* Tablica stronic */
struct PageTable {
    bool bit;  //Wartość bool'owska sprawdzająca zajętość tablicy w pamięci
    int frame; //Numer ramki w której znajduje się stronica

    PageTable(bool bit, int frame);
};

class MemoryManager {
private:
    char RAM[256]; //Pamięć Fizyczna Komputera [256 bajtów]

    //Struktura Stronicy
    struct Page {
        char data[16]; //Dane stronicy

        Page(){};
        Page(std::string data);
    };

public:

    MemoryManager();
    virtual ~MemoryManager();
};


#endif //SEXYOS_MEMORYMANAGER_H
