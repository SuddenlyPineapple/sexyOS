// Created by Wojciech Kasperski on 15-Oct-18.

#ifndef SEXYOS_MEMORYMANAGER_H
#define SEXYOS_MEMORYMANAGER_H

#include <list>
#include <string>
#include <vector>
#include <map>
//#include "Processes.h"

//------------- Tablica stronic procesu-------------
//Indeks stronic dla każdego procesu, vector tej struktury znajduje się w PCB
struct PageTable {
    bool bit;  //Wartość bool'owska sprawdzająca zajętość tablicy w pamięci [Sprawdza, czy ramka znajduje się w pamięci RAM]
    int frame; //Numer ramki w której znajduje się stronica

    PageTable(bool bit, int frame);
};

class MemoryManager {
    public: char RAM[256]; //Pamięć Fizyczna Komputera [256 bajtów]
    private:
        //------------- Struktura Pojedyńczej Stronicy w Pamięci -------------
        struct PageFrame {
            char data[16]{}; //Dane stronicy

            PageFrame();
            PageFrame(std::string data);
        };

        //------------- Lista Ramek -------------
        //Jest to struktura dodawana domyślnie do każdego procesu ułatwiająca odczytywanie wszystkich ramek jakimi dysponuje w danej chwili process
        //Przechowywana jest w Pamięci RAM jako struktura, dzięki czemu możemy spradzić do jakiego procesu należy ramka, bez potrzeby zaglądania do PCB, w którym też jest *pageList
        struct ProcessFramesList {
            bool isFree;
            int PID;
            int PageNumber;
            std::vector<PageTable> *pageList;

            ProcessFramesList(bool isFree, int PID, int PageNumber, std::vector<PageTable> *pageList);
        };

        //------------- Ramki załadowane w Pamięci Fizycznej [w pamięci RAM]-------------
        std::vector<ProcessFramesList> Frames;

        //------------- Plik stronicowania -------------
        // map < PID procesu, Stronice danego procesu>
        std::map<int, std::vector<PageFrame>> PageFile;

        //------------- Stos ostatnio używanych ramek -------------
        //Stos dzięki, którem wiemy, która ramka jest najdłużej w pamięci i którą ramkę możemy zastąpić
        //Jako, że mamy 256B pamięci ram, a jedna ramka posiada 16B, to będziemy mieć łącznie 16 ramek [0-15]
        //Więcej: https://pl.wikipedia.org/wiki/Least_Recently_Used
        std::list<int> LeastRecentlyUsedStack{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

public:
        //------------- Konstruktor  -------------
        MemoryManager();
        //------------- Destruktor  --------------
        ~MemoryManager();
};


#endif //SEXYOS_MEMORYMANAGER_H
