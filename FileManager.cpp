/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 02/11/18
*/

#include "FileManager.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <utility>

//--------------------------- Aliasy ------------------------
using u_int = unsigned int;

//------------------------- Operatory -----------------------
std::ostream& operator << (std::ostream& os, const std::vector<std::string>& strVec) {
	for (const std::string& str : strVec) {
		os << str << '\n';
	}
	return os;
}
std::ostream& operator << (std::ostream& os, const tm& time) {
	os << time.tm_hour << ':' << std::setfill('0') << std::setw(2) << time.tm_min << ' ' << std::setfill('0') << std::setw(2) << time.tm_mday << '.'
		<< std::setfill('0') << std::setw(2) << time.tm_mon << '.' << time.tm_year;
	return os;
}
bool operator == (const tm& time1, const tm& time2) {
	if (time1.tm_hour == time2.tm_hour  &&
		time1.tm_isdst == time2.tm_isdst &&
		time1.tm_mday == time2.tm_mday  &&
		time1.tm_min == time2.tm_min   &&
		time1.tm_mon == time2.tm_mon   &&
		time1.tm_sec == time2.tm_sec   &&
		time1.tm_wday == time2.tm_wday  &&
		time1.tm_yday == time2.tm_yday  &&
		time1.tm_year == time2.tm_year) {
		return true;
	}
	else { return false; };
}
bool FileManager::Directory::operator==(const Directory& dir) const
{
	if (/*this->parentDirectory == dir.parentDirectory &&*/
		this->files == dir.files &&
		this->creationTime == dir.creationTime &&
		this->type == dir.type /*&&
		this->creator == dir.creator*/) {
		return true;
	}
	else { return false; }
}



//------------------------ Serializer -----------------------
const std::string Serializer::IntToString(unsigned int input) {
	std::string result;
	while (input != 0) {
		if (input % 255 == 0) {
			result.push_back(char(255));
			input -= 255;
		}
		else {
			result.push_back(input % 255);
			input -= input % 255;
		}
	}
	return result;
}
const u_int Serializer::StringToInt(const std::string& input) {
	u_int result = 0;
	for (const char& c : input) {
		result += std::bitset<8>(c).to_ulong();
	}
	return result;
}



//--------------------------- Dysk --------------------------

FileManager::Disk::FileSystem::FileSystem()
{
	InodeTable[rootDirectory] = std::make_shared<Directory>();
	for (u_int i = 0; i < bitVector.size(); i++) {
		bitVector[i] = BLOCK_FREE;
	}
}

FileManager::Disk::Disk() {
	//Zape³nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	fill(space.begin(), space.end(), NULL);
}

std::shared_ptr<FileManager::Index>& FileManager::IndexBlock::operator[](const size_t& index) {
	return this->block[index];
}

const std::shared_ptr<FileManager::Index>& FileManager::IndexBlock::operator[](const size_t& index) const {
	return this->block[index];
}

void FileManager::Disk::write(const u_int& begin, const u_int& end, const std::string& data) {
	//Indeks który bêdzie s³u¿y³ do wskazywania na komórki pamiêci
	u_int index = begin;
	//Iterowanie po danych typu string i zapisywanie znaków na dysku
	for (u_int i = 0; i < data.size() && i <= end - begin; i++) {
		space[index] = data[i];
		index++;
	}
	//Zapisywanie NULL, jeœli dane nie wype³ni³y ostatniego bloku
	for (; index <= end; index++) {
		space[index] = NULL;
	}
}

template<typename T>
const T FileManager::Disk::read(const u_int& begin, const u_int& end) const {
	T data;
	if (typeid(T) == typeid(std::string)) {
		//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
		for (u_int index = begin; index <= end; index++) {
			//Dodaj znak zapisany na dysku do danych
			data += space[index];
		}
	}
	return data;
}



//----------------------- FileManager -----------------------

FileManager::FileManager() {
	currentDirectory = DISK.FileSystem.rootDirectory;
}



//-------------------- Podstawowe Metody --------------------

bool FileManager::FileCreate(const std::string& name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		std::vector<std::string> errorDescriptions;
		bool error = false;
		//Error1
		if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }
		//Error2
		if (DISK.FileSystem.InodeTable.size() >= INODE_NUMBER_LIMIT) {
			errorDescriptions.emplace_back(
				"Osi¹gniêto limit elementów w systemie plików!"); error = true;
		}
		//Error3
		if (CheckIfNameUsed(currentDirectory, name)) { errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true; }
		//Error4
		if (name.size() + GetCurrentPathLength() > MAX_PATH_LENGTH) { errorDescriptions.emplace_back("Œcie¿ka za d³uga!"); error = true; }
		if (error) { throw errorDescriptions; }

		const std::shared_ptr<File> file = std::make_shared<File>();

		//Dodanie pliku do obecnego katalogu
		DISK.FileSystem.InodeTable[GetCurrentPath() + name] = file;
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files[name] = GetCurrentPath() + name;

		if (messages) { std::cout << "Stworzono plik o nazwie '" << name << "' w œcie¿ce '" << GetCurrentPath() << "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
	catch (const std::vector<std::string>& descriptions) {
		std::cout << descriptions;
		return false;
	}
}

bool FileManager::FileSaveData(const std::string& name, const std::string& data) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		std::vector<std::string> errorDescriptions;
		bool error = false;
		//Error1
		if (data.size() > MAX_FILE_SIZE) { errorDescriptions.emplace_back("Podane dane przekraczaj¹ maksymalny rozmiar pliku!"); error = true; }

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);
		//Error2
		if (fileIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '"); throw errorDescriptions;
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[fileIterator->second];
		//Error3
		if (inode->type != "FILE") { errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '"); error = true; }

		std::shared_ptr<File> file = std::dynamic_pointer_cast<File>(inode);
		//Error4
		if (data.size() > DISK.FileSystem.freeSpace - file->blocksOccupied*BLOCK_SIZE) {
			errorDescriptions.
				emplace_back("Za ma³o miejsca na dysku!"); error = true;
		}
		if (error) { throw errorDescriptions; }

		FileSaveData(file, data);
		if (messages) { std::cout << "Zapisano dane do pliku o nazwie '" << name << "'.\n"; }
		return true;
	}
	catch(const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
	catch (const std::vector<std::string>& descriptions) {
		std::cout << descriptions;
		return false;
	}
}

const std::string FileManager::FileGetData(const std::string& name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);

		//Error1
		if (fileIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}
		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[fileIterator->second];
		//Error2
		if (inode->type != "FILE") { throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!"); }

		return FileGetData(std::dynamic_pointer_cast<File>(inode));
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return "";
	}
}

bool FileManager::FileDelete(const std::string& name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);
		//Error1
		if (fileIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[fileIterator->second];
		//Error2
		if (inode->type != "FILE") { throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!"); }

		std::shared_ptr<File> file = std::dynamic_pointer_cast<File>(inode);
		FileDelete(file);
		//Usuñ wpis o pliku z obecnego katalogu
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.erase(fileIterator);

		if (messages) { std::cout << "Usuniêto plik o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::DirectoryCreate(std::string name) {
	try {
		std::vector<std::string> errorDescriptions;
		bool error = false;
		//Error1
		if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }

		if (*(name.end() - 1) != '/') { name += '/'; }
		//Error2
		if (DISK.FileSystem.InodeTable.size() >= INODE_NUMBER_LIMIT) {
			errorDescriptions.emplace_back(
				"Osi¹gniêto limit elementów w systemie plików!"); error = true;
		}

		const auto directoryIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);
		//Error3
		if (directoryIterator != std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true;
		}

		//Error4
		if (name.size() + GetCurrentPathLength() > MAX_PATH_LENGTH) { errorDescriptions.emplace_back("Œcie¿ka za d³uga!"); error = true; }
		if (error) { throw errorDescriptions; }

		//Do iWêz³ów w obecnym katalogu dodaj nowy podkatalog
		DISK.FileSystem.InodeTable[GetCurrentPath() + name] = std::make_shared<Directory>();

		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files[name] = GetCurrentPath() + name;
		if (messages) { std::cout << "Stworzono katalog o nazwie '" << name << "' w œcie¿ce '" << GetCurrentPath() << "'.\n"; }

		return true;
	}
	catch (const std::vector<std::string>& descriptions) {
		std::cout << descriptions;
		return false;
	}
}

bool FileManager::DirectoryDelete(std::string name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		if (*(name.end() - 1) != '/') { name += '/'; }
		//Iterator zwracany podczas przeszukiwania obecnego katalogu za katalogiem o podanej nazwie
		const auto directoryIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);

		//Error1
		if (directoryIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[directoryIterator->second];
		//Error2
		if (inode->type != "DIRECTORY") {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		//Wywo³aj funkcjê usuwania katalogu wraz z jego zawartoœci¹
		DISK.FileSystem.InodeTable.erase(name);
		std::shared_ptr<Directory> directory = std::dynamic_pointer_cast<Directory>(inode);
		DirectoryDeleteStructure(directory);

		//Usuñ wpis o katalogu z obecnego katalogu
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.erase(directoryIterator);
		DISK.FileSystem.InodeTable.erase(GetCurrentPath() + name);

		if (messages) { std::cout << "Usuniêto katalog o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::DirectoryChange(std::string path) {
	try {
		if (path.empty()) { throw "Pusta œcie¿ka!"; }
		if (*(path.end() - 1) != '/') { path += '/'; }
		const auto inodeTableIterator = DISK.FileSystem.InodeTable.find(path);

		//Error1
		if (inodeTableIterator != DISK.FileSystem.InodeTable.end()) {
			throw("Katalog o œcie¿ce '" + path + "' nie znaleziony!");
		}
		//Error2
		if (inodeTableIterator->second->type == "DIRECTORY") {
			throw("Katalog o œcie¿ce '" + path + "' nie znaleziony!");
		}

		//Przejœcie do katalogu o wskazanej nazwie
		currentDirectory = inodeTableIterator->first;
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::DirectoryUp() {
	try {
		if (GetCurrentDirectoryParent().empty()) { throw std::string("Jesteœ w katalogu g³ównym!"); }
		//Przejœcie do katalogu nadrzêdnego
		else { currentDirectory = GetCurrentDirectoryParent(); }
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::DirectoryDown(std::string name) {
	try {
		if (*(name.end() - 1) != '/') { name += '/'; }

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za katalogiem o podanej nazwie
		const auto directoryIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);

		//Error1
		if (directoryIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[directoryIterator->second];
		//Error2
		if (inode->type != "DIRECTORY") {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		//Przejœcie do katalogu o wskazanej nazwie
		currentDirectory = GetCurrentPath() + directoryIterator->first;
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}



//--------------------- Dodatkowe metody --------------------

bool FileManager::FileCreate(const std::string& name, const std::string& data) {
	try {
		std::vector<std::string> errorDescriptions;
		bool error = false;
		//Error1
		if (data.size() > MAX_FILE_SIZE) {
			errorDescriptions.emplace_back(
				"Podane dane przekraczaj¹ maksymalny rozmiar pliku!"); error = true;
		}
		//Error2
		if (!CheckIfEnoughSpace(CalculateNeededBlocks(data.size())*BLOCK_SIZE)) {
			errorDescriptions.
				emplace_back("Za ma³o miejsca na dysku!"); error = true;
		}
		if (error) { throw errorDescriptions; }

		FileCreate(name);
		const std::shared_ptr<File> file = std::make_shared<File>();

		//Dodanie pliku do obecnego katalogu
		DISK.FileSystem.InodeTable[GetCurrentPath() + name] = file;
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files[name] = GetCurrentPath() + name;

		//Zapisanie danych pliku na dysku
		FileSaveData(name, data);

		return true;
	}
	catch (const std::vector<std::string>& descriptions)
	{
		std::cout << descriptions;
		return false;
	}
}

bool FileManager::DirectoryRename(std::string name, std::string changeName) {
	try {
		std::vector<std::string> errorDescriptions;
		bool error = false;

		//Error1
		if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }
		//Error2
		if (changeName.empty()) { errorDescriptions.emplace_back("Pusta nowa nazwa!"); throw errorDescriptions; }

		if (*(changeName.end() - 1) != '/') { changeName += '/'; }
		if (*(name.end() - 1) != '/') { name += '/'; }
		//Error3
		if (CheckIfNameUsed(currentDirectory, changeName)) {
			errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true;
		}
		//Error4
		if (changeName.size() + GetCurrentPathLength() > MAX_PATH_LENGTH) {
			errorDescriptions.emplace_back("Œcie¿ka za d³uga!"); error = true;
		}

		const auto directoryIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);
		//Error5
		if (directoryIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			errorDescriptions.push_back("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
			throw errorDescriptions;
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[directoryIterator->second];
		//Error6
		if (inode->type != "DIRECTORY") {
			errorDescriptions.push_back("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
			error = true;
		}
		if (error) { throw errorDescriptions; }

		//Lokowanie nowego klucza w tablicy hashowej i przypisanie do niego pliku
		DISK.FileSystem.InodeTable[currentDirectory + changeName] = std::dynamic_pointer_cast<Directory>(inode);
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files[changeName] = GetCurrentPath() + changeName;

		//Usuniêcie starego klucza
		DISK.FileSystem.InodeTable.erase(name);
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.erase(directoryIterator);

		if (messages) { std::cout << "Zmieniono nazwê katalogu '" << name << "' na '" << changeName << "'.\n"; }

		return true;
	}
	catch (const std::vector<std::string>& descriptions) {
		std::cout << descriptions;
		return false;
	}


}

bool FileManager::FileRename(const std::string& name, const std::string& changeName) {
	try {
		std::vector<std::string> errorDescriptions;
		bool error = false;

		//Error1
		if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }
		//Error2
		if (changeName.empty()) { errorDescriptions.emplace_back("Pusta nowa nazwa!"); throw errorDescriptions; }
		//Error3
		if (CheckIfNameUsed(currentDirectory, changeName)) {
			errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true;
		}
		//Error4
		if (changeName.size() + GetCurrentPathLength() > MAX_PATH_LENGTH) {
			errorDescriptions.emplace_back("Œcie¿ka za d³uga!"); error = true;
		}

		const auto fileIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);
		//Error5
		if (fileIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
			throw errorDescriptions;
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[fileIterator->second];
		//Error6
		if (inode->type != "FILE") {
			errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
			error = true;
		}
		if (error) { throw errorDescriptions; }


		const std::shared_ptr<File> file = std::dynamic_pointer_cast<File>(inode);

		//Lokowanie nowego klucza w tablicy hashowej i przypisanie do niego pliku
		DISK.FileSystem.InodeTable[currentDirectory + changeName] = file;
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files[changeName] = GetCurrentPath() + changeName;

		//Usuniêcie starego klucza
		DISK.FileSystem.InodeTable.erase(currentDirectory + name);
		std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.erase(fileIterator);

		if (messages) { std::cout << "Zmieniono nazwê pliku '" << name << "' na '" << changeName << "'.\n"; }

		return true;
	}

	catch (const std::vector<std::string>& descriptions) {
		std::cout << descriptions;
		return false;
	}
}

void FileManager::DirectoryRoot() {
	currentDirectory = DISK.FileSystem.rootDirectory;
	std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
}

void FileManager::Messages(const bool& onOff) {
	messages = onOff;
}

void FileManager::DetailedMessages(const bool&  onOff) {
	detailedMessages = onOff;
}

const std::string FileManager::GetCurrentPath() const {
	return currentDirectory;
}



//------------------ Metody do wyœwietlania -----------------

void FileManager::DisplayFileSystemParams() const {
	std::cout << "Disk capacity: " << DISK_CAPACITY << " Bytes\n";
	std::cout << "Block size: " << BLOCK_SIZE << " Bytes\n";
	std::cout << "Max file size: " << MAX_FILE_SIZE << " Bytes\n";
	std::cout << "Max indexes in block: " << BLOCK_INDEX_NUMBER << " Indexes\n";
	std::cout << "Max direct indexes in file: " << BLOCK_DIRECT_INDEX_NUMBER << " Indexes\n";
	std::cout << "Max file number: " << INODE_NUMBER_LIMIT << " Files\n";
	std::cout << "Max path length: " << MAX_PATH_LENGTH << " Characters\n";
	std::cout << "Number of files: " << DISK.FileSystem.InodeTable.size() << " Files\n";
}

bool FileManager::DisplayDirectoryInfo(const std::string& name) {
	try {
		const auto directoryIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);

		//Error1
		if (directoryIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[directoryIterator->second];
		//Error2
		if (inode->type != "DIRECTORY") {
			throw("Katalog o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Directory> directory = std::dynamic_pointer_cast<Directory>(inode);
		std::cout << "Name: " << name << '\n';
		std::cout << "Size: " << CalculateDirectorySize(directory) << " Bytes\n";
		std::cout << "Size on disk: " << CalculateDirectorySizeOnDisk(directory) << " Bytes\n";
		std::cout << "Contains: " << CalculateDirectoryFileNumber(directory) << " Files, " << CalculateDirectoryFolderNumber(directory) << " Folders\n";
		std::cout << "Created: " << directory->creationTime << '\n';

		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}

}

bool FileManager::DisplayFileInfo(const std::string& name) {
	try {
		const auto fileIterator = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.find(name);


		//Error1
		if (fileIterator == std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[currentDirectory])->files.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[fileIterator->second];
		//Error2
		if (inode->type != "FILE") {
			throw("Plik o nazwie '" + name + "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!");
		}

		const std::shared_ptr<File> file = std::dynamic_pointer_cast<File>(inode);
		std::cout << "Name: " << name << '\n';
		std::cout << "Size: " << file->blocksOccupied*BLOCK_SIZE << " Bytes\n";
		std::cout << "Size on disk: " << file->sizeOnDisk << " Bytes\n";
		std::cout << "Created: " << file->creationTime << '\n';
		std::cout << "Modified: " << file->modificationTime << '\n';
		std::cout << "Saved data: " << FileGetData(file) << '\n';

		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

void FileManager::DisplayDirectoryStructure() {
	const u_int level = 0;
	std::cout << std::string(level + 1, ' ') << DISK.FileSystem.rootDirectory << "\n";
	DisplayDirectory(std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[DISK.FileSystem.rootDirectory]), 0);
}

void FileManager::DisplayDiskContentBinary() {
	u_int index = 0;
	for (const char& c : DISK.space) {
		std::cout << std::bitset<8>(c) << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 16 == 15 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayDiskContentChar() {
	u_int index = 0;
	for (const char& c : DISK.space) {
		if (c == ' ') { std::cout << ' '; }
		else if (c >= 0 && c <= 32) std::cout << ".";
		else std::cout << c;
		std::cout << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 32 == 31 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayBitVector() {
	u_int index = 0;
	for (u_int i = 0; i < DISK.FileSystem.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << DISK.FileSystem.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}



//------------------- Metody Sprawdzaj¹ce -------------------

const bool FileManager::CheckIfNameUsed(const std::string& directory, const std::string& name) {
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	std::shared_ptr<Directory> dir = std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[directory]);
	for (auto i = dir->files.begin(); i != dir->files.end(); ++i) {
		//Jeœli nazwa ta sama
		if (i->first == name) { return true; }
	}
	return false;
}

const bool FileManager::CheckIfEnoughSpace(const u_int& dataSize) const {
	if (dataSize <= DISK.FileSystem.freeSpace) { return true; }
	return false;
}



//-------------------- Metody Obliczaj¹ce -------------------

const u_int FileManager::CalculateNeededBlocks(const size_t& dataSize) const {
	/*
	Przybli¿enie w górê rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa¿, jeœli zape³nia chocia¿ o jeden bajt
	wiêcej przy zajêtym bloku, to trzeba zaalokowaæ wtedy kolejny blok.
	*/
	return int(ceil(double(dataSize) / double(BLOCK_SIZE)));
}

const size_t FileManager::CalculateDirectorySize(const std::shared_ptr<Directory>& directory) {
	if (directory != nullptr) {
		//Rozmiar katalogu
		size_t size = 0;

		//Dodaje rozmiar plików w katalogu do rozmiaru katalogu
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "FILE") {
				size += std::dynamic_pointer_cast<File>(inode)->blocksOccupied*BLOCK_SIZE;
			}
		}
		//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "DIRECTORY") {
				size += CalculateDirectorySize(std::dynamic_pointer_cast<Directory>(inode));
			}
		}
		return size;
	}
	else { return 0; }
}

const size_t FileManager::CalculateDirectorySizeOnDisk(const std::shared_ptr<Directory>& directory)
{
	if (directory != nullptr) {
		//Rzeczywisty rozmiar katalogu
		size_t sizeOnDisk = 9;

		//Dodaje rzeczywisty rozmiar plików w katalogu do rozmiaru katalogu
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "FILE") {
				sizeOnDisk += std::dynamic_pointer_cast<File>(inode)->sizeOnDisk;
			}
		}
		//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "DIRECTORY") {
				sizeOnDisk += CalculateDirectorySize(std::dynamic_pointer_cast<Directory>(inode));
			}
		}
		return sizeOnDisk;
	}
	else { return 0; }
}

const u_int FileManager::CalculateDirectoryFolderNumber(const std::shared_ptr<Directory>& directory) {
	if (directory != nullptr) {
		//Iloœæ folderów w danym katalogu
		u_int folderNumber = 0;

		/**
		 Dodaje iloœæ folderów w tym katalogu do zwracanej zmiennej
		 Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
		 */
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "DIRECTORY") {
				folderNumber += CalculateDirectoryFolderNumber(std::dynamic_pointer_cast<Directory>(inode));
				folderNumber += 1;
			}
		}
		return folderNumber;
	}
	else { return 0; }
}

const u_int FileManager::CalculateDirectoryFileNumber(const std::shared_ptr<Directory>& directory) {
	if (directory != nullptr) {
		//Iloœæ folderów w danym katalogu
		u_int fileNumber = 0;

		/**
		 Dodaje iloœæ plików w tym katalogu do zwracanej zmiennej
		 Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
		 */
		for (const auto& element : directory->files) {
			const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[element.second];
			if (inode->type == "FILE") {
				fileNumber += CalculateDirectoryFileNumber(std::dynamic_pointer_cast<Directory>(inode));
				fileNumber += 1;
			}
		}
		return fileNumber;
	}
	else { return 0; }
}



//--------------------- Metody Alokacji ---------------------

void FileManager::FileTruncate(std::shared_ptr<File> file, const u_int& neededBlocks) {
	if (neededBlocks != file->blocksOccupied) {
		//Jeœli nale¿y zmniejszyæ plik
		if (neededBlocks < file->blocksOccupied) { FileAllocationDecrease(file, neededBlocks); }
		//Jeœli nale¿y zwiêkszyæ plik
		else if (neededBlocks > file->blocksOccupied) { FileAllocationIncrease(file, neededBlocks); }
	}
}

void FileManager::FileAddIndexes(const std::shared_ptr<File>& file, const std::vector<u_int>& blocks) const {
	if (file != nullptr) {
		if (blocks.size() != size_t(0) && file->blocksOccupied * BLOCK_SIZE <= MAX_FILE_SIZE) {

			u_int blocksIndex = 0;
			//Wpisanie bloków do bezpoœredniego bloku indeksowego
			for (size_t i = 0; i < BLOCK_DIRECT_INDEX_NUMBER && i < blocks.size(); i++) {
				if (file->directBlocks[i] == nullptr) {
					file->directBlocks[i] = std::make_shared<Index>(blocks[blocksIndex]);
					blocksIndex++;
				}
			}

			//Wpisanie bloków do 1-poziomowego bloku indeksowego
			if (blocksIndex < blocks.size()) {
				for (size_t i = BLOCK_DIRECT_INDEX_NUMBER; i < BLOCK_INDEX_NUMBER; i++) {
					if (file->directBlocks[i] == nullptr) {
						file->directBlocks[i] = std::make_shared<IndexBlock>();
					}
					for (size_t j = 0; j < BLOCK_INDEX_NUMBER && blocksIndex < blocks.size(); j++) {
						if ((*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[i]))[j] == nullptr) {
							(*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[i]))[j] = std::make_shared<Index>(blocks[blocksIndex]);
							blocksIndex++;
						}
					}
				}
			}
		}
	}
}

void FileManager::FileAllocationIncrease(std::shared_ptr<File>& file, const u_int& neededBlocks) {
	const u_int increaseBlocksNumber = abs(int(neededBlocks - file->blocksOccupied));
	bool fitsAfterLastIndex = true;
	std::shared_ptr<Index> tempIndex;
	const std::shared_ptr<IndexBlock> tempIndexBlock = std::dynamic_pointer_cast<IndexBlock>(tempIndex);
	u_int lastBlockIndex = 0;

	if (file->blocksOccupied != 0) {
		//Jeœli indeksy zapisane s¹ tylko w bloku ideksowym 
		if (file->blocksOccupied <= BLOCK_INDEX_NUMBER) {
			for (u_int i = BLOCK_INDEX_NUMBER - BLOCK_DIRECT_INDEX_NUMBER; i >= 0 && tempIndex == nullptr; i--) {
				tempIndex = file->directBlocks[i];
				if (i == 0) { break; } //u_int po obni¿eniu zera przyjmuje wartoœæ wiêksz¹ od zera
			}
		}
		else {
			for (u_int i = file->directBlocks.size() - 1; i > BLOCK_DIRECT_INDEX_NUMBER && tempIndex == nullptr; i--) {
				tempIndex = file->directBlocks[i];
			}
			for (u_int i = 0; i < BLOCK_INDEX_NUMBER && tempIndex != nullptr; i++) {
				lastBlockIndex = tempIndex->value;
				tempIndex = (*tempIndexBlock)[i];
			}
		}

		for (u_int i = lastBlockIndex + 1; i < lastBlockIndex + increaseBlocksNumber + 1; i++) {
			if (DISK.FileSystem.bitVector[i] == BLOCK_OCCUPIED) { fitsAfterLastIndex = false; break; }
			else if (i == increaseBlocksNumber) { break; }
		}
		lastBlockIndex++;
	}
	else {
		fitsAfterLastIndex = false;
	}

	if (fitsAfterLastIndex) {
		std::vector<u_int>blocks;
		for (u_int i = lastBlockIndex; i < lastBlockIndex + increaseBlocksNumber; i++) {
			blocks.push_back(i);
		}
		FileAllocateBlocks(file, blocks);
	}
	else {
		if (file->blocksOccupied > 0) { FileDeallocate(file); };
		FileAllocateBlocks(file, FindUnallocatedBlocks(neededBlocks));
	}
	if (detailedMessages) { std::cout << "Zwiêkszono plik do rozmiaru " << file->blocksOccupied*BLOCK_SIZE << " Bajtów.\n"; }
}

void FileManager::FileAllocationDecrease(const std::shared_ptr<File>& file, const u_int& neededBlocks) {
	const u_int sizeToStart = neededBlocks * BLOCK_SIZE;
	//Zmienna do analizowania, czy ju¿ mo¿na usuwaæ/dodawaæ bloki do pliku
	u_int currentSize = 0;

	u_int indexNumber = 0;
	std::shared_ptr<Index> index = file->directBlocks[indexNumber];

	//Dopóki indeks na coœ wskazuje
	while (index != nullptr) {
		currentSize += BLOCK_SIZE;
		if (indexNumber < BLOCK_DIRECT_INDEX_NUMBER) {
			index = file->directBlocks[indexNumber];
		}
		else if (file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER] != nullptr) {
			index = (*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[indexNumber - BLOCK_DIRECT_INDEX_NUMBER];
		}
		else { index = nullptr; }
		//Spisz kolejny indeks
		indexNumber++;

		//Jeœli obecny rozmiar przewy¿sza rozmiar potrzebny do rozpoczêcia usuwania
		//zacznij usuwaæ bloki
		if (currentSize > sizeToStart && index != nullptr) {
			//Zmniejszenie rozmiaru pliku
			file->blocksOccupied--;
			//Po uciêciu rozmiar i rozmiar rzeczywisty bêd¹ takie same
			file->sizeOnDisk = file->blocksOccupied*BLOCK_SIZE;
			//Oznacz obecny indeks jako wolny
			ChangeBitVectorValue(index->value, BLOCK_FREE);
			//Obecny indeks w tablicy FileSystem wskazuje na nic
			if (indexNumber < BLOCK_DIRECT_INDEX_NUMBER) {
				file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER] = nullptr;
			}
			else if (file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER] != nullptr) {
				(*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[indexNumber - BLOCK_DIRECT_INDEX_NUMBER] = nullptr;
			}
		}
	}
	if (file->blocksOccupied == BLOCK_INDEX_NUMBER) {
		file->blocksOccupied--;
		file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER] = nullptr;
	}
	if (detailedMessages) { std::cout << "Zmniejszono plik do rozmiaru " << file->blocksOccupied*BLOCK_SIZE << " Bajtów.\n"; }
}

void FileManager::FileDeallocate(const std::shared_ptr<File>& file) {
	std::vector<u_int>freedBlocks; //Zmienna u¿yta do wyœwietlenia komunikatu

	if (file->directBlocks[0] != nullptr) {
		std::shared_ptr<Index> index = std::make_shared<Index>();
		u_int indexNumber = 0;
		while (indexNumber < BLOCK_INDEX_NUMBER && index != nullptr && indexNumber < MAX_FILE_SIZE / BLOCK_SIZE) {
			if (indexNumber < BLOCK_DIRECT_INDEX_NUMBER) {
				index = file->directBlocks[indexNumber];
				file->directBlocks[indexNumber] = nullptr;
			}
			else if (file->directBlocks[indexNumber] != nullptr) {
				index = (*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[indexNumber - BLOCK_DIRECT_INDEX_NUMBER];
				(*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[indexNumber - BLOCK_DIRECT_INDEX_NUMBER] = nullptr;
			}
			else { index = nullptr; }

			if (index != nullptr) {
				freedBlocks.push_back(index->value);
				ChangeBitVectorValue(index->value, BLOCK_FREE);
				indexNumber++;
			}
		}
		file->directBlocks.clear();
		file->blocksOccupied = 0;
	}
	if (detailedMessages) {
		std::sort(freedBlocks.begin(), freedBlocks.end());
		std::cout << "Zwolniono bloki: ";
		for (u_int i = 0; i < freedBlocks.size(); i++) { std::cout << freedBlocks[i] << (i < freedBlocks.size() - 1 ? ", " : ".\n"); }
	}
}

void FileManager::FileAllocateBlocks(const std::shared_ptr<File>& file, const std::vector<u_int>& blocks) {

	for (const auto& i : blocks) {
		ChangeBitVectorValue(i, BLOCK_OCCUPIED);
	}
	file->blocksOccupied += blocks.size();

	FileAddIndexes(file, blocks);

	if (detailedMessages) {
		std::cout << "Zaalokowano bloki: ";
		for (u_int i = 0; i < blocks.size(); i++) { std::cout << blocks[i] << (i < blocks.size() - 1 ? ", " : ".\n"); }
	}
}

const std::vector<u_int> FileManager::FindUnallocatedBlocksFragmented(u_int blockNumber) {
	//Lista wolnych bloków
	std::vector<u_int> blockList;

	//Szuka wolnych bloków
	for (u_int i = 0; i < DISK.FileSystem.bitVector.size(); i++) {
		//Jeœli blok wolny
		if (DISK.FileSystem.bitVector[i] == BLOCK_FREE) {
			//Dodaje indeks bloku
			blockList.push_back(i);
			//Potrzeba teraz jeden blok mniej
			blockNumber--;
			//Jeœli potrzeba 0 bloków, przerwij
			if (blockNumber == 0) { break; }
		}
	}
	return blockList;
}

const std::vector<u_int> FileManager::FindUnallocatedBlocksBestFit(const u_int& blockNumber) {
	//Lista indeksów bloków (dopasowanie)
	std::vector<u_int> blockList;
	//Najlepsze dopasowanie
	std::vector<u_int> bestBlockList(DISK.FileSystem.bitVector.size() + 1);

	//Szukanie wolnych bloków spe³niaj¹cych minimum miejsca
	for (u_int i = 0; i < DISK.FileSystem.bitVector.size(); i++) {
		//Jeœli blok wolny
		if (DISK.FileSystem.bitVector[i] == BLOCK_FREE) {
			//Dodaj indeks bloku do listy bloków
			blockList.push_back(i);
		}
		//Jeœli blok zajêty
		else {
			//Jeœli uzyskana lista bloków jest wiêksza od iloœci bloków jak¹ chcemy uzyskaæ
			//to dodaj uzyskane dopasowanie do listy dopasowañ;
			if (blockList.size() >= blockNumber) {
				//Jeœli znalezione dopasowanie mniejsze ni¿ najlepsze dopasowanie
				if (blockList.size() < bestBlockList.size()) {
					//Przypisanie nowego najlepszego dopasowania
					bestBlockList = blockList;
					if (bestBlockList.size() == blockNumber) { break; }
				}
			}

			//Czyœci listê bloków, aby mo¿na przygotowaæ kolejne dopasowanie
			blockList.clear();
		}
	}

	/*
	Jeœli zdarzy siê, ¿e ostatni blok w wektorze bitowym jest wolny, to
	ostatnie dopasownie nie zostanie dodane do listy dopasowañ, dlatego
	trzeba wykonañ poni¿szy kod. Jeœli ostatni blok w wektorze bitowym
	bêdzie zajêty to blockList bêdzie pusty i nie spie³ni warunku
	*/
	if (blockList.size() >= blockNumber) {
		//Jeœli blok wolny
		if (blockList.size() < bestBlockList.size()) {
			//Dodaj indeks bloku do listy bloków
			bestBlockList = blockList;
		}
	}

	//Jeœli znalezione najlepsze dopasowanie
	if (bestBlockList.size() < DISK.FileSystem.bitVector.size() + 1) {
		//Odetnij nadmiarowe indeksy z dopasowania (jeœli wiêksze ni¿ potrzeba)
		bestBlockList.resize(blockNumber);
	}
	//Inaczej zmniejsz dopasowanie do 0, ¿eby po zwróceniu wybrano inn¹ metodê
	else { bestBlockList.resize(0); }

	return bestBlockList;
}

const std::vector<u_int> FileManager::FindUnallocatedBlocks(const u_int& blockNumber) {
	//Szuka bloków funkcj¹ z metod¹ best-fit
	std::vector<u_int> blockList = FindUnallocatedBlocksBestFit(blockNumber);

	//Jeœli funkcja z metod¹ best-fit nie znajdzie dopasowañ
	if (blockList.empty()) {
		//Szuka niezaalokowanych bloków, wybieraj¹c pierwsze wolne
		blockList = FindUnallocatedBlocksFragmented(blockNumber);
	}

	return blockList;
}



//----------------------- Metody Inne -----------------------

void FileManager::FileSaveData(std::shared_ptr<File>& file, const std::string& data) {
	file->modificationTime = GetCurrentTimeAndDate();

	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>fileFragments = DataToDataFragments(data);

	//Alokowanie bloków dla pliku
	FileTruncate(file, fileFragments.size());

	file->sizeOnDisk = data.size();


	//Index pod którym maj¹ zapisywane byæ dane
	u_int indexNumber = 0;
	std::shared_ptr<Index> index = file->directBlocks[indexNumber];

	//Zapisuje wszystkie dane na dysku
	for (const auto& fileFragment : fileFragments) {
		//Zapisuje fragment na dysku
		DISK.write(index->value * BLOCK_SIZE, (index->value + 1) * BLOCK_SIZE - 1, fileFragment);
		//Przypisuje do indeksu numer kolejnego bloku
		indexNumber++;
		if (indexNumber < BLOCK_DIRECT_INDEX_NUMBER) {
			index = file->directBlocks[indexNumber];
		}
		else if (file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER] != nullptr) {
			index = (*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[(indexNumber - BLOCK_DIRECT_INDEX_NUMBER) % BLOCK_INDEX_NUMBER];
		}
	}
}

const std::string FileManager::FileGetData(const std::shared_ptr<File>& file) const {
	//Dane
	std::string data;
	//Indeks do wczytywania danych z dysku
	size_t indexNumber = 0;
	size_t indexInBlockNumber = 0;
	std::shared_ptr<Index> index = std::make_shared<Index>();

	//Dopóki nie natrafimy na koniec pliku
	while (index != nullptr && indexNumber) {
		if (indexNumber < BLOCK_DIRECT_INDEX_NUMBER) {
			index = file->directBlocks[indexNumber];
		}
		else if (file->directBlocks[indexNumber] != nullptr) {
			index = (*std::dynamic_pointer_cast<IndexBlock>(file->directBlocks[BLOCK_DIRECT_INDEX_NUMBER]))[(indexInBlockNumber) % BLOCK_INDEX_NUMBER];
			indexInBlockNumber++;
		}
		else { index = nullptr; }

		//Dodaje do danych fragment pliku pod wskazanym indeksem
		data += DISK.read<std::string>(index->value * BLOCK_SIZE, (index->value + 1)*BLOCK_SIZE - 1);
		//Przypisuje indeksowi kolejny indeks w tablicy FileSystem
		if (indexInBlockNumber % BLOCK_INDEX_NUMBER == 0) { indexNumber++; }
	}
	return data;
}

void FileManager::FileDelete(std::shared_ptr<File>& file) {
	if (file != nullptr) {
		FileDeallocate(file);
		for (auto it = DISK.FileSystem.InodeTable.begin(); it != DISK.FileSystem.InodeTable.end(); ++it) {
			if (it->second == file) { DISK.FileSystem.InodeTable.erase(it); break; }
		}
	}
}

void FileManager::DirectoryDeleteStructure(std::shared_ptr<Directory>& directory) {
	//Usuwa wszystkie pliki z katalogu
	for (auto it = directory->files.begin(); it != directory->files.end(); ++it) {
		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[it->second];
		if (inode->type == "FILE") {
			std::shared_ptr<File> file = std::dynamic_pointer_cast<File>(inode);
			FileDelete(file);
			if (messages) { std::cout << "Usuniêto plik o nazwie '" << it->first << "' znajduj¹cy siê w œcie¿ce '" + GetPath(directory) + "'.\n"; }
		}
	}

	//Usuwa wszystkie katalogi w katalogu
	for (auto it = directory->files.begin(); it != directory->files.end(); ++it) {
		const std::shared_ptr<Inode> inode = DISK.FileSystem.InodeTable[it->second];
		if (inode != nullptr) {
			if (inode->type == "DIRECTORY") {
				//Wywo³anie funkcji na podrzêdnym katalogu
				std::shared_ptr<Directory> dir = std::dynamic_pointer_cast<Directory>(inode);
				DirectoryDeleteStructure(dir);
				DISK.FileSystem.InodeTable.erase(GetPath(directory) + it->first + "/");
				if (messages) { std::cout << "Usuniêto katalog o nazwie '" << it->first << "' znajduj¹cy siê w œcie¿ce '" << GetPath(directory) << "'.\n"; }
			}
		}
	}
	//Czyœci listê iWêz³ów w katalogu
	directory->files.clear();
}

void FileManager::DisplayDirectory(const std::shared_ptr<Directory>& directory, u_int level) {
	if (directory != nullptr) {
		for (auto i = directory->files.begin(); i != directory->files.end(); ++i) {
			if (DISK.FileSystem.InodeTable[i->second]->type == "FILE") {
				std::cout << std::string(level + 2, ' ') << "- " << i->first << '\n';
			}
		}
		level++;
		for (auto i = directory->files.begin(); i != directory->files.end(); ++i) {
			if (DISK.FileSystem.InodeTable[i->second]->type == "DIRECTORY") {
				std::cout << std::string(level + 1, ' ') << i->first << "\n";
				DisplayDirectory(std::dynamic_pointer_cast<Directory>(DISK.FileSystem.InodeTable[i->second]), level);
			}
		}
	}
}

const std::string FileManager::GetPath(const std::shared_ptr<Directory>& directory) {
	for (auto it = DISK.FileSystem.InodeTable.begin(); it != DISK.FileSystem.InodeTable.end(); ++it) {
		if (it->second->type == "DIRECTORY") {
			if (*std::dynamic_pointer_cast<Directory>(it->second) == *directory) { return it->first; }
		}
	}
	return "";
}

const std::string FileManager::GetCurrentDirectoryParent() const {
	std::string result = currentDirectory;
	result.pop_back();
	for (u_int i = result.size() - 1; i >= 0; i--) {
		if (i == 0) { result = ""; break; }
		else if (result[i] == '/') { break; }
		else { result.pop_back(); }
	}
	return result;
}

const size_t FileManager::GetCurrentPathLength() const {
	std::string tempDir = currentDirectory;
	tempDir.erase(std::remove(tempDir.begin(), tempDir.end(), '/'));
	return tempDir.size();
}

const tm FileManager::GetCurrentTimeAndDate()
{
	time_t tt;
	time(&tt);
	tm timeAndDate = *localtime(&tt);
	timeAndDate.tm_year += 1900;
	timeAndDate.tm_mon += 1;
	return timeAndDate;
}

void FileManager::ChangeBitVectorValue(const u_int& block, const bool& value) {
	//Jeœli wartoœæ zajêty to wolne miejsce - BLOCK_SIZE
	if (value == 1) { DISK.FileSystem.freeSpace -= BLOCK_SIZE; }
	//Jeœli wartoœæ wolny to wolne miejsce + BLOCK_SIZE
	else if (value == 0) { DISK.FileSystem.freeSpace += BLOCK_SIZE; }
	//Przypisanie blokowi podanej wartoœci
	DISK.FileSystem.bitVector[block] = value;
}

const std::vector<std::string> FileManager::DataToDataFragments(const std::string& data) const {
	//Tablica fragmentów podanych danych
	std::vector<std::string>fileFragments;

	//Przetworzenie ca³ych danych
	for (u_int i = 0; i < CalculateNeededBlocks(data.size()); i++) {
		//Oblicza pocz¹tek kolejnej czêœci fragmentu danych.
		const u_int substrBegin = i * BLOCK_SIZE;
		//Dodaje do tablicy fragmentów kolejny fragment o d³ugoœci BLOCK_SIZE
		fileFragments.push_back(data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}
