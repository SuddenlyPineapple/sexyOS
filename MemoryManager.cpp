// Created by Wojciech Kasperski on 15-Oct-18.

#include <iostream>
#include "MemoryManager.h"

PageTableData::PageTableData(bool bit, int frame) : bit(bit), frame(frame) {}

MemoryManager::MemoryManager() = default;

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

MemoryManager::FrameData::FrameData(bool isFree, int PID, int PageNumber, std::vector<PageTableData> *pageList) : isFree(isFree), PID(PID),PageNumber(PageNumber), pageList(pageList) {}
