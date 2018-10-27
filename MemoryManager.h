//
// Created by Wojciech Kasperski on 15-Oct-18.
//

#ifndef SEXYOS_MEMORYMANAGER_H
#define SEXYOS_MEMORYMANAGER_H

#include <list>
#include <string>
#include <vector>
//#include "Processes.h"

//------------- Tablica stronic procesu-------------
//Indeks stronic dla każdego procesu
struct PageTable {
    bool bit;  //Wartość bool'owska sprawdzająca zajętość tablicy w pamięci
    int frame; //Numer ramki w której znajduje się stronica

    PageTable(bool bit, int frame);
};

class MemoryManager {
    public: char RAM[256]; //Pamięć Fizyczna Komputera [256 bajtów]
    private:

        //------------- Struktura Pojedyńczej Stronicy w Pamięci -------------
        struct Page {
            char data[16]{}; //Dane stronicy

            Page();
            Page(std::string data);
        };

        //------------- Lista Ramek danego procesu -------------
        //Jest to struktura dodawana domyślnie do każdego procesu ułatwiająca odczytywanie wszystkich ramek jakimi dysponuje w danej chwili process
        struct FramesList {
            const int PID;
            const int PageNumber;
            std::vector<PageTable> *pageList;
        };

    public:

        //------------- Konstruktor  -------------
        MemoryManager();
        //------------- Destruktor  --------------
        virtual ~MemoryManager();
};


#endif //SEXYOS_MEMORYMANAGER_H
