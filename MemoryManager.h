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
struct PageTableData {
    bool bit;  //Wartość bool'owska sprawdzająca zajętość tablicy w pamięci [Sprawdza, czy ramka znajduje się w pamięci RAM]
    int frame; //Numer ramki w której znajduje się stronica

    PageTableData(bool bit, int frame);
};

class MemoryManager {

//------------- Struktury używane przez MemoryManager'a oraz zmienne--------------
    public:
        char RAM[256]; //Pamięć Fizyczna Komputera [256 bajtów]
    private:
        //------------- Struktura Pojedyńczej Stronicy w Pamięci -------------
        struct Page {
            char data[16]{}; //Dane stronicy

            Page();
            Page(std::string data);

            void print();
        };

        //------------- Lista Ramek -------------
        //Struktura wykorzystywana do lepszego przeszukiwania pamięci ram i łatwiejszej wymiany stronic
        struct FrameData {
            bool isFree; //Czy ramka jest wolna (True == wolna, False == zajęta)
            int PID; //Numer Procesu
            int pageID; //Numer stronicy
            std::vector<PageTableData> *pageList; //Wskaźnik do tablicy stronic procesu, która znajduje się w PCB

            FrameData(bool isFree, int PID, int pageID, std::vector<PageTableData> *pageList);
        };

        //------------- Ramki załadowane w Pamięci Fizycznej [w pamięci RAM]-------------
        std::vector<FrameData> Frames;

        //------------- Plik stronicowania -------------
        // map < PID procesu, Stronice danego procesu>
        std::map<int, std::vector<Page>> PageFile;

        //------------- Stos ostatnio używanych ramek (Least Recently Used Stack) -------------
        //Stos dzięki, którem wiemy, która ramka jest najdłużej w pamięci i którą ramkę możemy zastąpić
        //Jako, że mamy 256B pamięci ram, a jedna ramka posiada 16B, to będziemy mieć łącznie 16 ramek [0-15]
        //Więcej: https://pl.wikipedia.org/wiki/Least_Recently_Used
        std::list<int> LRUStack {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};


//------------- Funkcje do wyświetlania bierzących stanów pamięci oraz pracy krokowej  --------------
    public:
        //Pokazuje zawartość pamięci operacyjnej [RAM]
        void showMem();

        //Pokazuje odpowiednie fragmenty pamięci [RAM]
        /* begin - miejsce w pamięci od którego ma być wyświetlona zawartość
         * bytes - ilość bitów do wyświetlenia
         */
        void showMem(int begin, int bytes);

        //Pokazuje zawartść pliku stronicowania
        void showPageFile();

        //Pokazuje zawartość tablicy wymiany processu
        /* pageList - wskaźnik na tablicę stronic procesu
         */
        void showPageTable(std::vector<PageTableData> *pageList);

        //Pokazuje Stos ostatnio używanych ramek (Od najmłodszych do najstarszych)
        void showLRUStack();

        void showFrames();


//------------- Funkcje użytkowe MemoryManagera  --------------

        //Tworzy process bezczynności systemu umieszczany w pamięci RAM przy starcie systemu
        void memoryInit();

        //Metoda ładująca program do pamięci - ładuje pierwsza stronicę programu do pamięci RAM
        /* path - ścieżka do programu na dysku twardym
         * mem -
         * PID - numer procesu
         */
        int loadProgram(std::string path, int mem, int PID);

        //Usuwa z pamięci dane wybranego procesu
        void kill(int PID);
    void stackUpdate(int frameID);
    private:
        //Zwraca adres pierwszej wolnej ramki w pamięci
        int seekFreeFrame();

        //Przesuwa ramkę podaną jako argument na początek stack'u ostatnio używanych (Least Recently Used - frames)
        /* frameID - numer ramki, którą chcemy przesunąć na początek stack'u
         */


        //Ładuje daną stronicę do pamięci RAM
        /*  page - stronica do załadowania
         *  pageID - numer stronicy
         *  PID - numer procesu
         *  *pageList - wskaźnik na tablicę stronic procesu
         */
        int LoadtoMemory(Page page, int pageID, int PID, std::vector<PageTableData> *pageList);
    public:
        //------------- Konstruktor  -------------
        MemoryManager();
        //------------- Destruktor  --------------
        ~MemoryManager();
};


#endif //SEXYOS_MEMORYMANAGER_H
