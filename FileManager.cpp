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

//
//----------------- FileManager Konstruktor -----------------
FileManager::FileManager() {
	for (unsigned int i = 0; i < (unsigned int)ceil(((double)FAT.bitVector.size() / 8.0) / (double)BLOCK_SIZE); i++) {
		ChangeBlockMapValue(i, 1);
		FAT.freeSpace -= BLOCK_SIZE;
	}
	currentDirectory = &FAT.rootDirectory;
}

//-------------------- Podstawowe Metody --------------------
void FileManager::CreateFile(const std::string &name, const std::string &data) {
	const unsigned int fileSize = CalculateNeededBlocks(data)*BLOCK_SIZE;
	if (CheckIfEnoughSpace(fileSize) && CheckIfNameUnused(*currentDirectory, name)) {
		File file = File(name, data);
		file.size = fileSize;

		currentDirectory->files[file.name] = file;

		const std::vector<unsigned int> blocks = FindUnallocatedBlocks(file.size / BLOCK_SIZE);
		for (unsigned int i = 0; i < blocks.size() - 1; i++) {
			FAT.FileAllocationTable[blocks[i]] = blocks[i + 1];
		}
		file.FATindex = blocks[0];
		WriteFile(file);
		std::cout << "Stworzono plik o nazwie '" << file.name << "' w katalogu '" << currentDirectory->name << "'\n";
		return;
	}
	if (!CheckIfEnoughSpace(fileSize) ){
		std::cout << "Za ma³o miejsca!";
	}
	if (!CheckIfNameUnused(*currentDirectory, name)) {
		std::cout << "Nazwa pliku '" << name << "' ju¿ zajêta!";
	}

}

const std::string FileManager::OpenFile(const unsigned int &id) {
	return DISK.read<std::string>(0 * 8, 4 * 8 - 1);
}

void FileManager::CreateDirectory(const std::string &name) {
	if (currentDirectory->subDirectories.find(name) == currentDirectory->subDirectories.end()) {
		currentDirectory->subDirectories[name] = Directory(name);
		currentDirectory->subDirectories[name].parentDirectory = &(*currentDirectory);
		std::cout << "Stworzono katalog o nazwie '" << currentDirectory->subDirectories[name].name
			<< "' w katalogu '" << currentDirectory->name << "'\n";
	}
	else { std::cout << "Nazwa katalogu '" << name << "' zajêta!\n"; }
}

void FileManager::CurrentDirectoryUp() {
	if (currentDirectory->parentDirectory != NULL) {
		currentDirectory = currentDirectory->parentDirectory;
		std::cout << "Obecny katalog to '" << currentDirectory->name << "'\n";
	}
	else { std::cout << "Jesteœ w katalogu g³ównym!\n"; }
}

void FileManager::CurrentDirectoryDown(const std::string &name) {
	if (currentDirectory->subDirectories.find(name) != currentDirectory->subDirectories.end()) {
		currentDirectory = &(currentDirectory->subDirectories.find(name)->second);
		std::cout << "Obecny katalog to '" << currentDirectory->name << "'\n";
	}
	else { std::cout << "Brak katalogu o podanej nazwie!\n"; }
}

//------------------ Metody do wyœwietlania -----------------
void FileManager::DisplayDirectoryStructure() {
	DisplayDirectory(FAT.rootDirectory, 1);
}
void FileManager::DisplayDirectory(const Directory &directory, unsigned int level) {
	std::cout << std::string(level, ' ') << directory.name << "\\\n";
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		std::cout << std::string(level + 1, ' ') << "- " << i->first << '\n';
	}
	level++;
	for (auto i = directory.subDirectories.begin(); i != directory.subDirectories.end(); i++) {
		DisplayDirectory(i->second, level);
	}
}

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

void FileManager::DisplayFileAllocationTable() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < FAT.FileAllocationTable.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << std::setfill('0') << std::setw(3) << (FAT.FileAllocationTable[i] != NULL ? std::to_string(FAT.FileAllocationTable[i]) : "NUL")
			<< (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayBitVector() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < FAT.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << FAT.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayFileFragments(const std::vector<std::string> &fileFragments) {
	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		std::cout << fileFragments[i] << std::string(BLOCK_SIZE - 1 - fileFragments[i].size(), ' ') << '\n';
	}
}

//-------------------- Metody Pomocnicze --------------------
const bool FileManager::CheckIfNameUnused(const Directory &directory, const std::string &name) {
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		if (i->first == name) { return false; }
	}
	return true;
}

const bool FileManager::CheckIfEnoughSpace(const unsigned int &dataSize) {
	if (dataSize > FAT.freeSpace) { return false; }
	else { return true; }
}

void FileManager::ChangeBlockMapValue(const unsigned int &block, const bool &value) {
	if (value == 1) { FAT.freeSpace -= BLOCK_SIZE; }
	else if (value == 0) { FAT.freeSpace += BLOCK_SIZE; }
	FAT.bitVector[block] = value;
	DISK.write(0, FAT.bitVector);
}

void FileManager::WriteFile(const File &file) {
	const std::vector<std::string>fileFragments = FileToFileFragments(file);
	unsigned int index = file.FATindex;

	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		DISK.write(index * BLOCK_SIZE, (index + 1) * BLOCK_SIZE - 1, fileFragments[i]);
		ChangeBlockMapValue(index, 1);
		index = FAT.FileAllocationTable[index];
	}
}

const std::vector<std::string> FileManager::FileToFileFragments(const File &file) {
	std::vector<std::string>fileFragments;
	unsigned int substrBegin = 0;

	for (unsigned int i = 0; i < file.size / BLOCK_SIZE; i++) {
		substrBegin = i * BLOCK_SIZE - 1 * (i - i);
		fileFragments.push_back(file.data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

const unsigned int FileManager::CalculateNeededBlocks(const std::string &data) {
	return (int)ceil((double)data.size() / (double)BLOCK_SIZE);
}

std::vector<unsigned int> FileManager::FindUnallocatedBlocks(unsigned int blockCount) {
	std::vector<unsigned int> blockList;
	for (unsigned int i = 0; i < FAT.bitVector.size(); i++) {
		if (FAT.bitVector[i] == 0) {
			blockList.push_back(i);
			blockCount--;
			if (blockCount == 0) { break; }
		}
	}
	blockList.push_back(NULL);
	return blockList;
}
