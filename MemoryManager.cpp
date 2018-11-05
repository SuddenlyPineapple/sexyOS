// Created by Wojciech Kasperski on 15-Oct-18.

#include <iostream>
#include "MemoryManager.h"

PageTableData::PageTableData(bool bit, int frame) : bit(bit), frame(frame) {}

MemoryManager::MemoryManager(){
    for (int i = 0; i < 16; i++)
        Frames.emplace_back(FrameData(1, -1, -1, NULL));
};

MemoryManager::~MemoryManager() = default;

MemoryManager::Page::Page(std::string data) {
    while (data.size() < 16) data += " "; // Uzupełnianie argumentu spacjami, jeśli jest za mały
    for (int i = 0; i < 16; i++) // Przepisywanie argumentu do stronicy
        this->data[i] = data[i];
}

MemoryManager::Page::Page() = default;

void MemoryManager::Page::print() {
    for(auto x: data){
        std::cout << x;
    }
    std::cout << std::endl;
}

MemoryManager::FrameData::FrameData(bool isFree, int PID, int pageID, std::vector<PageTableData> *pageList) : isFree(isFree), PID(PID),pageID(pageID), pageList(pageList) {}

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

void MemoryManager::showStack() {
    for(auto frame:Stack){
        std::cout << frame << " ";
    }
    std::cout << std::endl;
}
