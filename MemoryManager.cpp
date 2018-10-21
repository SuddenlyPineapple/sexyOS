//
// Created by Wojtek on 15-Oct-18.
//

#include "MemoryManager.h"

PageTable::PageTable(bool bit, int frame) : bit(bit), frame(frame) {}

MemoryManager::MemoryManager() = default;

MemoryManager::~MemoryManager() = default;

MemoryManager::Page::Page(std::string data) {
    while(data.size()<16){
        data+=" ";
    }
    for(int i = 0;i < 16; i++){
        this->data[i] = data[i];
    }
}

MemoryManager::Page::Page() {
    for(int i = 0;i < 16;i++){
        this->data[i] = data[i];
    }
}
