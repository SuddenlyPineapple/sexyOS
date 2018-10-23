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
	currentDirectory = &DISK.FAT.rootDirectory;
}

//-------------------- Podstawowe Metody --------------------
void FileManager::CreateFile(const std::string &name, const std::string &data) {
	const unsigned int fileSize = CalculateNeededBlocks(data)*BLOCK_SIZE;
	if (CheckIfEnoughSpace(fileSize) && CheckIfNameUnused(*currentDirectory, name)) {
		File file = File(name);
		file.size = fileSize;

		const std::vector<unsigned int> blocks = FindUnallocatedBlocks(file.size / BLOCK_SIZE);
		for (unsigned int i = 0; i < blocks.size() - 1; i++) {
			DISK.FAT.FileAllocationTable[blocks[i]] = blocks[i + 1];
		}
		file.FATindex = blocks[0];
		currentDirectory->files[file.name] = file;
		WriteFile(file, data);
		std::cout << "Stworzono plik o nazwie '" << file.name << "' w œcie¿ce '" << GetCurrentPath() << "'.\n";
		return;
	}
	if (!CheckIfEnoughSpace(fileSize)) {
		std::cout << "Za ma³o miejsca!\n";
	}
	if (!CheckIfNameUnused(*currentDirectory, name)) {
		std::cout << "Nazwa pliku '" << name << "' ju¿ zajêta!\n";
	}

}

const std::string FileManager::OpenFile(const unsigned int &id) {
	return DISK.read<std::string>(0 * 8, 4 * 8 - 1);
}

void FileManager::DeleteFile(const std::string &name) {
	auto fileIterator = currentDirectory->files.find(name);

	if (fileIterator != currentDirectory->files.end()) {
		unsigned int temp, index = fileIterator->second.FATindex;
		while (index != NULL) {
			temp = DISK.FAT.FileAllocationTable[index];
			DISK.FAT.bitVector[index] = 0;
			DISK.FAT.FileAllocationTable[index] = NULL;
			index = temp;
		}
		currentDirectory->files.erase(fileIterator);

		std::cout << "Usuniêto plik o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n";
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::CreateDirectory(const std::string &name) {
	if (currentDirectory->subDirectories.find(name) == currentDirectory->subDirectories.end()) {
		currentDirectory->subDirectories[name] = Directory(name);
		currentDirectory->subDirectories[name].parentDirectory = &(*currentDirectory);
		std::cout << "Stworzono katalog o nazwie '" << currentDirectory->subDirectories[name].name
			<< "' w œcie¿ce '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Nazwa katalogu '" << name << "' zajêta!\n"; }
}

void FileManager::CurrentDirectoryUp() {
	if (currentDirectory->parentDirectory != NULL) {
		currentDirectory = currentDirectory->parentDirectory;
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Jesteœ w katalogu g³ównym!\n"; }
}

void FileManager::CurrentDirectoryDown(const std::string &name) {
	if (currentDirectory->subDirectories.find(name) != currentDirectory->subDirectories.end()) {
		currentDirectory = &(currentDirectory->subDirectories.find(name)->second);
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Brak katalogu o podanej nazwie!\n"; }
}

//--------------------- Dodatkowe metody --------------------

void FileManager::CurrentDirectoryRoot() {
	while (currentDirectory->parentDirectory != NULL) {
		CurrentDirectoryUp();
	}
}

//------------------ Metody do wyœwietlania -----------------
void FileManager::DisplayDirectoryStructure() {
	DisplayDirectory(DISK.FAT.rootDirectory, 1);
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
	for (unsigned int i = 0; i < DISK.FAT.FileAllocationTable.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << std::setfill('0') << std::setw(3) << (DISK.FAT.FileAllocationTable[i] != NULL ? std::to_string(DISK.FAT.FileAllocationTable[i]) : "NUL")
			<< (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayBitVector() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << DISK.FAT.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
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
const std::string FileManager::GetCurrentPath() {
	std::vector<std::string>directoriesNames;
	std::string path;
	Directory* tempDir = currentDirectory;
	while (tempDir != NULL) {
		directoriesNames.push_back(tempDir->name);
		tempDir = tempDir->parentDirectory;
	}
	for (auto i = directoriesNames.rbegin(); i < directoriesNames.rend(); i++) {
		path += "/" + *i;
	}
	return path;
}
const bool FileManager::CheckIfNameUnused(const Directory &directory, const std::string &name) {
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		if (i->first == name) { return false; }
	}
	return true;
}

const bool FileManager::CheckIfEnoughSpace(const unsigned int &dataSize) {
	if (dataSize > DISK.FAT.freeSpace) { return false; }
	else { return true; }
}

void FileManager::ChangeBitVectorValue(const unsigned int &block, const bool &value) {
	if (value == 1) { DISK.FAT.freeSpace -= BLOCK_SIZE; }
	else if (value == 0) { DISK.FAT.freeSpace += BLOCK_SIZE; }
	DISK.FAT.bitVector[block] = value;
}

void FileManager::WriteFile(const File &file, const std::string &data) {
	const std::vector<std::string>fileFragments = DataToDataFragments(data);
	unsigned int index = file.FATindex;

	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		DISK.write(index * BLOCK_SIZE, (index + 1) * BLOCK_SIZE - 1, fileFragments[i]);
		ChangeBitVectorValue(index, 1);
		index = DISK.FAT.FileAllocationTable[index];
	}
}

const std::vector<std::string> FileManager::DataToDataFragments(const std::string &str) {
	std::vector<std::string>fileFragments;
	unsigned int substrBegin = 0;

	for (unsigned int i = 0; i < CalculateNeededBlocks(str); i++) {
		substrBegin = i * BLOCK_SIZE - 1 * (i - i);
		fileFragments.push_back(str.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

const unsigned int FileManager::CalculateNeededBlocks(const std::string &data) {
	return (int)ceil((double)data.size() / (double)BLOCK_SIZE);
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocks(unsigned int blockCount) {
	std::vector<unsigned int> blockList;
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		if (DISK.FAT.bitVector[i] == 0) {
			blockList.push_back(i);
			blockCount--;
			if (blockCount == 0) { break; }
		}
	}
	blockList.push_back(NULL);
	return blockList;
}
