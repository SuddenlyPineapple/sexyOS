// Created by Wojciech Kasperski on 15-Oct-18.

#include <iostream>
#include <cmath>
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

MemoryManager::FrameData::FrameData(bool isFree, int PID, int pageID, std::vector<PageTableData> *pageList) : isFree(isFree), PID(PID),pageID(pageID), pageList(pageList) {}

MemoryManager::MemoryManager(){
    for (int i = 0; i < 16; i++)
        Frames.emplace_back(FrameData(true, -1, -1, NULL));
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
    int Frame = -1;
    auto *pageList = new std::vector<PageTableData>();

    for(int i = 0; i < pages; i++){
        pageList->push_back(PageTableData(false, 0));
    }

    //Załadowanie pierwszej stronicy do pamięci fizycznej
    //TODO: uncomment this when function will be ready
    //loadtoMemory(PageFile.at(PID).at(0), 0, PID, pageList);

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
    //TODO: napisać ten szajs
    return 0;
}



