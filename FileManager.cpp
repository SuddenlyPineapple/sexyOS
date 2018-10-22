/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 22/10/18
*/

#include "FileManager.h"
#include<algorithm>
#include<iomanip>
#include<iostream>

//--------------------------- Dysk --------------------------
FileManager::Disk::Disk() {
	//Zape³nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	fill(space.begin(), space.end(), NULL);
}
void FileManager::Disk::write(const unsigned int &begin, const unsigned int &end, const std::string &data) {
	//Indeks który bêdzie s³u¿y³ do iterowania po pamiêci dysku
	unsigned int index = begin;
	//Iterowanie po danych typu string i zapisywanie znaków na dysku
	for (unsigned int i = 0; i < data.size() && i <= end - begin; i++) {
		space[index] = data[i];
		index++;
	}
	for (; index <= end; index++) {
		space[index] = NULL;
	}
}
void FileManager::Disk::write(const unsigned int &index, const unsigned int &data) {
	space[index] = data;
}
template<unsigned int size>
void FileManager::Disk::write(const unsigned int &begin, const std::bitset<size> &data) {
	unsigned int index = begin, tempInt = 0;
	for (unsigned int i = 0; i < size; i++) {
		tempInt += (unsigned int)pow(2, 7 - i % 8)*data[i];
		if (i % 8 == 7) {
			space[index] = char(tempInt);
			index++;
			tempInt = 0;
		}
	}
}
template<typename T>
const T FileManager::Disk::read(const unsigned int &begin, const unsigned int &end) {
	T data;
	if (typeid(T) == typeid(std::string)) {
		for (unsigned int index = begin; index <= end; index++) {
			data += space[index];
		}
	}
	return data;
}

//*Zarz¹danie plikami
//*Konstruktor
FileManager::FileManager() {
	for (unsigned int i = 0; i < (unsigned int)ceil(((double)clusterMap.size() / 8.0) / (double)BLOCK_SIZE); i++) {
		ChangeClusterMapValue(i, 1);
	}
}

//-------------------- Podstawowe Metody --------------------
void FileManager::CreateFile(const std::string &name, const unsigned int &id, const std::string &data) {
	File file = File(name, id, data);
	const unsigned int end = (int)ceil((double)file.data.size() / (double)BLOCK_SIZE) * BLOCK_SIZE - 1;

	//Tymczasowe, potem usun¹æ/zamieniæ
	const unsigned int firstBlock = (int)ceil(((double)clusterMap.size() / 8.0) / (double)BLOCK_SIZE);
	ChangeClusterMapValue(firstBlock, 1);
	WriteFileFAT(firstBlock, FileToFileFAT(file));
	//Tymczasowe, potem usun¹æ/zamieniæ

	//Tu dopisz kod
}
const std::string FileManager::OpenFile(const unsigned int &id) {
	return DISK.read<std::string>(0 * 8, 4 * 8 - 1);
}

//------------------ Metody do wyœwietlania -----------------
void FileManager::DisplayDiskContentBinary() {
	unsigned int index = 0;
	for (const char &c : DISK.space) {
		//bitset - tablica bitowa
		std::cout << std::bitset<8>(c) << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 16 == 15 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}
void FileManager::DisplayDiskContentChar() {
	unsigned int index = 0;
	for (const char &c : DISK.space) {
		if (c >= 0 && c <= 32) std::cout << "!";
		else std::cout << c;
		std::cout << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 64 == 63 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}
void FileManager::DisplayBlocks() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < clusterMap.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << clusterMap[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}
void FileManager::DisplayFileFAT(const std::vector<FileFAT> &fileFAT) {
	for (unsigned int i = 0; i < fileFAT.size(); i++) {
		std::cout << fileFAT[i].DATA << std::string(15 - fileFAT[i].DATA.size(), ' ') << " | " << (fileFAT[i].NEXT_FRAGM == NULL ? "NULL" : std::to_string(fileFAT[i].NEXT_FRAGM)) << '\n';
	}
}

//-------------------- Metody Pomocnicze --------------------
void FileManager::ChangeClusterMapValue(const unsigned int &block, const bool &value) {
	clusterMap[block] = value;
	DISK.write(0, clusterMap);
}
void FileManager::WriteFileFAT(const unsigned int &begin, const std::vector<FileFAT> &fileFAT) {
	ChangeClusterMapValue(begin, 1);
	DISK.write(begin*BLOCK_SIZE, begin*BLOCK_SIZE + BLOCK_SIZE - 2, fileFAT[0].DATA);
	DISK.write(begin*BLOCK_SIZE + BLOCK_SIZE - 1, fileFAT[0].NEXT_FRAGM * BLOCK_SIZE);

	for (unsigned int i = 1; i < fileFAT.size(); i++) {
		DISK.write(fileFAT[i - 1].NEXT_FRAGM * BLOCK_SIZE, fileFAT[i - 1].NEXT_FRAGM * BLOCK_SIZE + BLOCK_SIZE - 2, fileFAT[i].DATA);
		DISK.write(fileFAT[i - 1].NEXT_FRAGM * BLOCK_SIZE + BLOCK_SIZE - 1, fileFAT[i].NEXT_FRAGM * BLOCK_SIZE);
		ChangeClusterMapValue(fileFAT[i - 1].NEXT_FRAGM, 1);
	}
}
std::vector<FileManager::FileFAT> FileManager::FileToFileFAT(const File &file) {
	std::vector<FileFAT>fileFAT;
	std::vector<unsigned int> blockList = FindUnallocatedBlocks(CalculateNeededBlocks(file.data) - 1);
	unsigned int characterNumber = BLOCK_SIZE - 1;
	unsigned int substrBegin;

	for (unsigned int i = 0; i < blockList.size(); i++) {
		substrBegin = i * (BLOCK_SIZE - 1);
		if (characterNumber + substrBegin > file.data.size()) {
			characterNumber = file.data.size() - substrBegin;
		}
		fileFAT.push_back(FileFAT(file.data.substr(substrBegin, characterNumber), blockList[i]));
	}
	return fileFAT;
}

const unsigned int FileManager::CalculateNeededBlocks(const std::string &data) {
	return (int)ceil((double)data.size() / ((double)BLOCK_SIZE - 1));
}
std::vector<unsigned int> FileManager::FindUnallocatedBlocks(unsigned int blockCount) {
	std::vector<unsigned int> blockList;
	for (unsigned int i = 0; i < clusterMap.size(); i++) {
		if (clusterMap[i] == 0) {
			blockList.push_back(i);
			blockCount--;
			if (blockCount == 0) { break; }
		}
	}
	blockList.push_back(NULL);
	return blockList;
}