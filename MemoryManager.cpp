// Created by Wojciech Kasperski on 15-Oct-18.
#include "MemoryManager.h"
#include "Procesy.h"

//------------- Konstruktory i destruktory  --------------
MemoryManager::Page::Page(std::string data) {
    while (data.size() < 16) data += " "; // Uzupełnianie argumentu spacjami, jeśli jest za mały
    for (int i = 0; i < 16; i++) // Przepisywanie argumentu do stronicy
        this->data[i] = data[i];
}

MemoryManager::Page::Page() = default;

PageTableData::PageTableData(bool bit, int frame) : bit(bit), frame(frame) {}

PageTableData::PageTableData(){
    this->bit = false;
    this->frame = -1;
};

MemoryManager::FrameData::FrameData(bool isFree, int PID, int pageID, std::vector<PageTableData> *pageList) : isFree(isFree), PID(PID),pageID(pageID), pageList(pageList) {}

MemoryManager::MemoryManager(){
    for (int i = 0; i < 16; i++)
        Frames.emplace_back(FrameData(true, -1, -1, nullptr));
};

MemoryManager::~MemoryManager() = default;

//------------- Funkcje do wyświetlania bieżących stanów pamięci oraz pracy krokowej  --------------
void MemoryManager::Page::print() {
    for(auto &x: data){
        std::cout << x;
    }
    std::cout << std::endl;
}

void MemoryManager::showMem() {
    for(int i = 0; i < 256; i++){
        if(i%16 == 0 && i!=0)
            std::cout << "\nFrame no." << i/16 << ": ";
        else if(i%16 == 0)
            std::cout << "Frame no." << i/16 << ": ";
        RAM[i] != ' ' ? std::cout << RAM[i] : std::cout << '-';
    }
}

void MemoryManager::showMem(int begin, int bytes) {
    if (begin + bytes > 256) {
        std::cout << "Error: Number of bytes to display has excced amount of memory!";
    } else {
        std::cout << "Displaying physical memory from cell " << begin << " to " << begin + bytes << ":" << std::endl;
        for (int i = begin; i < begin + bytes; i++) {
            if (i != 0 && i % 16 == 0) std::cout << "\n";
            RAM[i] != ' ' ? std::cout << RAM[i] : std::cout << '-';
        }
    }
}

void MemoryManager::showPageFile() {
    for(auto &process : PageFile){
        std::cout << "PID: " << process.first << "\n";
        for (auto &page: process.second){
            page.print();
        }
    }
}

void MemoryManager::showPageTable(std::vector<PageTableData> *pageList) {
    std::cout << "PAGE\t | \tFRAME \t | \tBIT \n";
    int i = 0;
    for(auto pageListRecord : *pageList){
        std::cout << i++ << "\t\t" << pageListRecord.frame << "\t\t" << pageListRecord.bit << "\n";
    }
}

void MemoryManager::showStack() {
    for(auto frame:Stack){
        std::cout << frame << " ";
    }
    std::cout << std::endl;
}

void MemoryManager::showFrames() {
    std::cout << "\t\tFREE \tPAGE \tPID " << std::endl;
    int i = 0;
    for(auto &frame : Frames){
        std::cout << "Frame no." << i++ << ":\t" << frame.isFree << "\t" << frame.pageID << "\t" << frame.PID << "\n";
    }
}

//------------- Funkcje użytkowe MemoryManagera  --------------
void MemoryManager::memoryInit() {
    for (char &cell : RAM) {
        cell = ' ';
    }
    std::string data = "JP 0";
    std::vector<Page> pageVector{ Page(data) };
    PageFile.insert(std::make_pair(1,pageVector));
}

void MemoryManager::stackUpdate(int frameID) {
    if(frameID > 15) return;

    for(auto it = Stack.begin(); it!= Stack.end(); it++){
        if(*it == frameID){
            Stack.erase(it);
            break;
        }
    }

    Stack.push_front(frameID);
}

std::vector<PageTableData> *MemoryManager::createPageList(int mem, int PID) {
    double pages = ceil((double)mem/16);
    auto *pageList = new std::vector<PageTableData>();

    for(int i = 0; i < pages; i++){
        pageList->push_back(PageTableData(false, 0));
    }

    loadToMemory(PageFile[PID][0], 0, PID, pageList);
    return pageList;
}

int MemoryManager::seekFreeFrame() {
    int seekedFrame = -1;
    for(int i = 0; i < Frames.size(); i++){
        if(Frames[i].isFree){
            seekedFrame = i;
            break;
        }
    }
    return seekedFrame;
}

void MemoryManager::kill(int PID) {
    for(int i = 0; i < Frames.size(); i++){
        if(Frames[i].PID == PID) {
            for(int j = i * 16; j < i * 16 + 16; j++)
                RAM[j] = ' ';
            Frames[i].isFree = true;
            Frames[i].pageID = -1;
            Frames[i].PID = -1;
            PageFile.erase(PID);
        }
    }
}

int MemoryManager::loadProgram(std::string path, int mem, int PID) {
    double pagesAmount = ceil((double)mem/16);
    std::fstream file(path); //Plik na dysku
    std::string scrap; //Zmienna pomocnicza
    std::string program; //Program w jednej linii
    std::vector<Page> pageVector; //Wektor stronic do dodania

    if(!file.is_open()){
        std::cout << "Error: Can't open file! \n";
        return -1;
    }

    while(std::getline(file, scrap)){
        //Dodanie spacji zamiast końca linii
        if(!file.eof())
            scrap+= " ";
        program += scrap;
    }
    scrap.clear();

    //Dzielenie programu na stronice
    for (char i : program) {
        scrap+= i;
        //Tworzenie Stronicy
        if(scrap.size() == 16){
            pageVector.emplace_back(Page(scrap));
            scrap.clear();
        }
    }

    if(!scrap.empty())
        pageVector.emplace_back(Page(scrap));
    scrap.clear();

    if(pagesAmount * 16 < 16 * pageVector.size()){
        std::cout << "Error: Program has not assigned enough memory!\n";
        return -1;
    }


    //Sprawdzanie, czy program nie potrzebuje wiecej stronic w pamięci
    for(int i = pageVector.size(); i < pagesAmount; i++)
        pageVector.emplace_back(scrap);

    //Dodanie stronic do pliku wymiany
    PageFile.insert(std::make_pair(PID, pageVector));

    return 1;
}

int MemoryManager::loadToMemory(MemoryManager::Page page, int pageID, int PID, std::vector<PageTableData> *pageList) {
    int frame = seekFreeFrame();

    if(frame == -1)
        frame = insertPage(pageList, pageID, PID);

    //Przepisywanie stronicy do pamięci RAM
    int it = 0;
    int end = frame * 16 + 16;
    for(int i = frame * 16; i < end; i++)
        RAM[i] = page.data[it++];

    //Zmienianie bit'u w indeksie wymiany stronic
    pageList->at(pageID).bit = true;
    pageList->at(pageID).frame = frame;

    //Aktualizacja stosu używalności
    stackUpdate(frame);

    //Aktualizacja informacji o ramce
    Frames[frame].isFree = false;
    Frames[frame].pageID = pageID;
    Frames[frame].PID = PID;
    Frames[frame].pageList = pageList;

    return frame;
}

std::string MemoryManager::GET(PCB *process, int LADDR) {
    std::string response;
    bool reading = true;
    int Frame = -1;
    int PageID = LADDR/16;

    //przekroczenie zakres dla tego procesu
    if(process->pageList->size() <= PageID){
        std::cout << "Error: Exceeded memory range!";
        reading = false;
        response = "ERROR";
    }

    while(reading){
        PageID = LADDR/16; //Numer stronicy w pamięci

        if(process->pageList->size() <= PageID){
            reading = false;
            break;
        }

        //Sprawdza, czy stronica znajduje się w pamięci operacyjnej
        if(!process->pageList->at(PageID).bit)
            loadToMemory(PageFile[process->PID][PageID], PageID, process->PID, process->pageList);

        if(process->pageList->at(PageID).bit){
            Frame = process->pageList->at(PageID).frame;//Bieżąco używana ramka
            stackUpdate(Frame);//Ramka została użyta, więc trzeba zaktualizować stos
            if(RAM[Frame * 16 + LADDR - (16 * PageID)] == ' ') //Odczytywanie do napotkania spacji
                reading = false;
            else
                response+= RAM[Frame * 16 + LADDR - (16*PageID)];
        }
        LADDR++;
    }

    return response;
}

int MemoryManager::Write(PCB *process, int adress, std::string data) {
    if(data.size() == 0) return 1;

    if(adress + data.size() - 1 > process->pageList->size() * 16 - 1 || adress < 0){
        std::cout << "Error: Exceeded memory amount for this process! \n";
        return -1;
    }

    int pageID;
    for(int i = 0; i < data.size(); i++){
        pageID = (adress+i)/16;
        if(!process->pageList->at(pageID).bit)
            loadToMemory(PageFile[process->PID][pageID], pageID, process->PID, process->pageList);
        RAM[process->pageList->at(pageID).frame * 16 + adress + i - (16 * pageID)] = data[i];
        stackUpdate(process->pageList->at(pageID).frame);
    }
    return 1;
}

//TODO: ZROBIC FIFIO
int MemoryManager::insertPage(std::vector<PageTableData> *pageList, int pageID, int PID) {
    //*it numer ramki ktora jest ofiara
    auto it = Stack.end(); it--;
    int Frame = *it;
    // Przepisuje zawartosc z ramki ofiary do Pliku wymiany
    for (int i = Frame * 16; i < Frame * 16 + 16; i++) {
        PageFile[Frames[Frame].PID][Frames[Frame].pageID].data[i - (Frame * 16)] = RAM[i];
    }

    //Zmieniam wartosci w tablicy stronic ofiary
    Frames[Frame].pageList->at(Frames[Frame].pageID).bit = false;
    Frames[Frame].pageList->at(Frames[Frame].pageID).frame = -1;

    return Frame;
}




