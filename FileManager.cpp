/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 13/12/18
*/

/*
 * Aby ³atwiej nawigowaæ po moim kodzie polecam z³o¿yæ wszystko
 * Skrót: CTRL + M + A
 */

#include "FileManager.h"
#include "Procesy.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <utility>

 //--------------------------- Aliasy ------------------------
using u_int = unsigned int;
using u_short_int = unsigned short int;
using u_char = unsigned char;



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



//--------------------------- Dysk --------------------------

FileManager::FileSystem::FileSystem() {
	//Zape³nienie tablicy i-wêz³ów pustymi i-wêz³ami
	for (u_int i = 0; i < INODE_NUMBER_LIMIT; i++) {
		inodeTable[i] = Inode();
	}
	//Wyzerowanie tablicy 'zajêtoœci' i-wêz³ów
	inodeBitVector.reset();
	for (u_int i = 0; i < bitVector.size(); i++) {
		bitVector[i] = BLOCK_FREE;
	}
}

const unsigned FileManager::FileSystem::get_free_inode_id() {
	for (u_int i = 0; i < inodeBitVector.size(); i++) {
		if (inodeBitVector[i] == false) { return i; }
	}
	return -1;
}

void FileManager::FileSystem::reset() {
	//Zape³nienie tablicy i-wêz³ów pustymi i-wêz³ami
	for (u_int i = 0; i < INODE_NUMBER_LIMIT; i++) {
		inodeTable[i] = Inode();
	}
	//Wyzerowanie tablicy 'zajêtoœci' i-wêz³ów
	inodeBitVector.reset();
	for (u_int i = 0; i < bitVector.size(); i++) {
		bitVector[i] = BLOCK_FREE;
	}
}

FileManager::Inode::Inode() : creationTime(), modificationTime() {
	//Wype³nienie indeksów bloków dyskowych wartoœci¹ -1 (pusty indeks)
	directBlocks.fill(-1);
	singleIndirectBlocks = -1;
}

void FileManager::Inode::clear() {
	blocksOccupied = 0;
	realSize = 0;
	directBlocks.fill(-1);
	singleIndirectBlocks = -1;
	creationTime = tm();
	modificationTime = tm();
	flagOpen = false;
}

FileManager::Disk::Disk() {
	//Zape³nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	space.fill(NULL);
}

void FileManager::Disk::write(const u_short_int& begin, const std::string& data) {
	const u_int end = begin + BLOCK_SIZE - 1;

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

void FileManager::Disk::write(const u_short_int& begin, const std::array<u_int, BLOCK_SIZE / 2>& data) {
	std::string result;
	for (size_t i = 0; i < BLOCK_SIZE / 2; i++) {
		std::string temp;
		if (data[i] != -1) {
			if (data[i] <= 9) {
				temp = "0" + std::to_string(data[i]);
			}
			else { temp = std::to_string(data[i]); }
			result += temp;
		}
		else { result += "-1"; }
	}
	write(begin, result);
}

const std::string FileManager::Disk::read_str(const u_int& begin) const {
	std::string data;
	const u_int end = begin + BLOCK_SIZE;
	//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
	for (u_int index = begin; index < end; index++) {
		//Dodaj znak zapisany na dysku do danych
		if (space[index] != NULL) { data += space[index]; }
	}
	return data;
}

const std::array<u_int, FileManager::BLOCK_SIZE / 2> FileManager::Disk::read_arr(const u_int& begin) const {
	const u_int end = begin + BLOCK_SIZE;
	std::string data;
	std::array<u_int, BLOCK_SIZE / 2> result;
	result.fill(-1);

	//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
	for (u_int index = begin; index < end; index++) {
		//Dodaj znak zapisany na dysku do danych
		if (space[index] != NULL) { data += space[index]; }
	}

	std::string num;
	u_int arrIndex = 0;
	for (const char& c : data) {
		if (c == NULL) { break; }
		num += c;
		if (num.length() == 2) {
			result[arrIndex] = std::stoi(num);
			num.clear();
			arrIndex++;
		}
		if (arrIndex == BLOCK_SIZE / 2) { break; }
	}
	return result;
}



//-------------------- Podstawowe Metody --------------------

bool FileManager::file_create(const std::string& name) {
	try {
		//Czêœæ sprawdzaj¹ca
		{
			//Error1
			if (name.empty()) { throw "Pusta nazwa!"; }
			std::vector<std::string> errorDescriptions{"B³¹d tworzenia pliku '"+name+"\'!"};
			bool error = false;
			//Error2
			if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }
			//Error3
			if (fileSystem.rootDirectory.size() >= INODE_NUMBER_LIMIT) {
				errorDescriptions.emplace_back(
					"Wykorzystano wszystkie i-wêz³y!"); error = true;
			}
			//Error4
			if (check_if_name_used(name)) { errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true; }
			//Error5
			if (name.size() > MAX_FILENAME_LENGTH) { errorDescriptions.emplace_back("Nazwa za d³uga!"); error = true; }
			if (error) { throw errorDescriptions; }
		}

		//Czêœæ dzia³aj¹ca
		{
			const u_int inodeId = fileSystem.get_free_inode_id();
			//Dodanie pliku do katalogu g³ównego
			fileSystem.rootDirectory[name] = inodeId;
			fileSystem.inodeBitVector[inodeId] = true;
			fileSystem.inodeTable[inodeId].creationTime = get_current_time_and_date();
			fileSystem.inodeTable[inodeId].modificationTime = get_current_time_and_date();

			if (messages) { std::cout << "Stworzono plik o nazwie '" << name << ".\n"; }
			return true;
		}
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

bool FileManager::file_write(const std::string& name, const std::string& data) {
	try {
		Inode* inode;
		//Czêœæ sprawdzaj¹ca
		{
			if (name.empty()) { throw "Pusta nazwa!"; }
			std::vector<std::string> errorDescriptions {"B³¹d zapisywania do pliku '" + name + "\'!"};
			bool error = false;

			//Error1
			if (data.size() > MAX_DATA_SIZE) {
				errorDescriptions.emplace_back("Podane dane przekraczaj¹ maksymalny rozmiar!"); error = true;
			}

			//Error2
			if (data.size() > BLOCK_INDEX_NUMBER * BLOCK_SIZE && fileSystem.freeSpace < calculate_needed_blocks(data.size())*BLOCK_SIZE + BLOCK_SIZE) {
				errorDescriptions.emplace_back("Brakuje bloków do zapisania podanych danych!"); error = true;
			}

			//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
			const auto fileIterator = fileSystem.rootDirectory.find(name);

			//Error3
			if (fileIterator == fileSystem.rootDirectory.end()) {
				errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony.");
				throw errorDescriptions;
			}
			inode = &fileSystem.inodeTable[fileIterator->second];

			//Error4
			if (data.size() > fileSystem.freeSpace - inode->blocksOccupied*BLOCK_SIZE) {
				errorDescriptions.emplace_back("Za ma³o miejsca na dysku!"); error = true;
			}
			if (error) { throw errorDescriptions; }
		}

		file_write(inode, data);
		if (messages) { std::cout << "Zapisano dane do pliku o nazwie '" << name << "'.\n"; }
		return true;
	}
	catch (const std::string& description) { std::cout << description << '\n'; return false; }
	catch (const std::vector<std::string>& descriptions) { std::cout << descriptions; return false; }
}

const bool FileManager::file_read(const std::string& name, const u_short_int& memoryAddress, const u_short_int& byteNumber) {
	try {
		//Iterator zwracany podczas przeszukiwania za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Czêœæ sprawdzaj¹ca --------------------
		{
			//Error1
			if (name.empty()) { throw "Pusta nazwa!"; }

			//Error2
			if (fileIterator == fileSystem.rootDirectory.end()) {
				throw("Plik o nazwie '" + name + "' nie znaleziony!");
			}
		}

		//Czêœæ dzia³aj¹ca ----------------------
		{
			Inode* inode = &fileSystem.inodeTable[fileIterator->second];
			bool finish = false; //Oznacza czy nale¿y zakoñczyæ odczyt
			std::string data;

			for (size_t i = 0; i < BLOCK_INDEX_NUMBER && inode->directBlocks[i] != -1 && !finish; i++) {
				data += disk.read_str(inode->directBlocks[i] * BLOCK_SIZE);
				if (data.size() == byteNumber) { finish = true; }
			}

			//Odczytywanie z dysku bloku indeksowego
			std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
			{
				indirectBlocks.fill(-1);
				if (inode->singleIndirectBlocks != -1) {
					indirectBlocks = disk.read_arr(inode->singleIndirectBlocks*BLOCK_SIZE);
				}
			}

			for (size_t i = 0; i < BLOCK_INDEX_NUMBER && indirectBlocks[i] != -1 && !finish; i++) {
				data += disk.read_str(indirectBlocks[i] * BLOCK_SIZE);
				if (data.size() == byteNumber) { finish = true; }
			}
		}
		return true;
	}
	catch (const std::string& description) { std::cout << description << '\n'; return false; }
}

const std::string FileManager::file_read_all(const std::string& name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		//Iterator zwracany podczas przeszukiwania za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Error1
		if (fileIterator == fileSystem.rootDirectory.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony!");
		}
		Inode* inode = &fileSystem.inodeTable[fileIterator->second];

		return file_read_all(inode);
	}
	catch (const std::string& description) { std::cout << description << '\n'; return ""; }
}

bool FileManager::file_delete(const std::string& name) {
	try {
		if (name.empty()) { throw "Pusta nazwa!"; }
		//Iterator zwracany podczas przeszukiwania katalogu g³ównego za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);
		//Error1
		if (fileIterator == fileSystem.rootDirectory.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony!");
		}

		Inode* inode = &fileSystem.inodeTable[fileIterator->second];

		file_deallocate(inode);
		fileSystem.inodeTable[fileIterator->second].clear();

		//Usuñ wpis o pliku z obecnego katalogu
		fileSystem.rootDirectory.erase(fileIterator);

		if (messages) { std::cout << "Usuniêto plik o nazwie '" << name << "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::file_open(const std::string & name) {
	try {
		//Error1
		if (name.empty()) { throw "Pusta nazwa!"; }
		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);
		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony!");
		}

		Inode* inode = &fileSystem.inodeTable[fileIterator->second];

		//Error3
		if (inode->flagOpen == true) { throw ("Plik o nazwie '" + name + "' jest ju¿ otwarty!"); }

		inode->flagOpen = true; //Ustawia flagê plik otwarty

		if (messages) { std::cout << "Otwarto plik o nazwie '" << name << "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

bool FileManager::file_close(const std::string & name) {
	try {
		//Error1
		if (name.empty()) { throw "Pusta nazwa!"; }

		const auto fileIterator = fileSystem.rootDirectory.find(name);
		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { throw("Plik o nazwie '" + name + "' nie znaleziony!"); }

		fileSystem.inodeTable[fileIterator->second].flagOpen = false; //Zeruje flagê plik otwarty
		usedFiles.erase(name);

		if (messages) { std::cout << "Zamkniêto plik o œcie¿ce '" << name << "'.\n"; }
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}



//--------------------- Dodatkowe Metody --------------------
bool FileManager::disk_format() {
	try {
		//Error1
		if (!usedFiles.empty()) { throw "Nie mo¿na sformatowaæ dysku gdy pliki s¹ u¿ywane!"; }

		fileSystem.reset();

		if (messages) { std::cout << "Sformatowano dysk!\n"; }
		return true;
	}
	catch (const std::string& description) { std::cout << description << '\n'; return false; }
}

bool FileManager::file_create(const std::string& name, const std::string& data) {
	if (!file_create(name)) { return false; }

	//Zapisanie danych na dysku
	if (!file_write(name, data)) {
		file_delete(name);
		return false;
	}
	else { return true; }
}

bool FileManager::file_rename(const std::string& name, const std::string& changeName) {
	try {
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Czêœæ sprawdzaj¹ca --------------------
		{
			std::vector<std::string> errorDescriptions;
			bool error = false;

			//Error1
			if (name.empty()) { errorDescriptions.emplace_back("Pusta nazwa!"); throw errorDescriptions; }
			//Error2
			if (changeName.empty()) { errorDescriptions.emplace_back("Pusta nowa nazwa!"); throw errorDescriptions; }
			//Error3
			if (check_if_name_used(changeName)) {
				errorDescriptions.push_back("Nazwa '" + name + "' jest ju¿ zajêta!"); error = true;
			}
			//Error4
			if (changeName.size() > MAX_FILENAME_LENGTH) {
				errorDescriptions.emplace_back("Nowa nazwa za d³uga!"); error = true;
			}

			//Error5
			if (fileIterator == fileSystem.rootDirectory.end()) {
				errorDescriptions.push_back("Plik o nazwie '" + name + "' nie znaleziony!");
				throw errorDescriptions;
			}
			if (error) { throw errorDescriptions; }
		}

		//Lokowanie nowego klucza w tablicy hashowej katalogu g³ównego i przypisanie do niego id i-wêz³a
		fileSystem.rootDirectory[changeName] = fileIterator->second;

		//Usuniêcie starego klucza z katalogu g³ównego
		fileSystem.rootDirectory.erase(fileIterator);

		if (messages) { std::cout << "Zmieniono nazwê pliku '" << name << "' na '" << changeName << "'.\n"; }

		return true;
	}
	catch (const std::vector<std::string>& descriptions) { std::cout << descriptions; return false; }
}

void FileManager::set_messages(const bool& onOff) {
	messages = onOff;
}

void FileManager::set_detailed_messages(const bool&  onOff) {
	detailedMessages = onOff;
}


//------------------ Metody do wyœwietlania -----------------

void FileManager::display_file_system_params() {
	std::cout << "disk capacity: " << DISK_CAPACITY << " Bytes\n";
	std::cout << "Block size: " << static_cast<int>(BLOCK_SIZE) << " Bytes\n";
	std::cout << "Max file size: " << MAX_FILE_SIZE << " Bytes\n";
	std::cout << "Max data size: " << MAX_DATA_SIZE << " Bytes\n";
	std::cout << "Max indexes in indirect index block: " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indexes\n";
	std::cout << "Max direct indexes in file: " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indexes\n";
	std::cout << "Max file number: " << static_cast<int>(INODE_NUMBER_LIMIT) << " Files\n";
	std::cout << "Max filename length: " << static_cast<int>(MAX_FILENAME_LENGTH) << " Characters\n";
}

void FileManager::display_root_directory_info() {
	std::cout << "Name: " << "root" << '\n';
	std::cout << "Size: " << calculate_directory_size() << " Bytes\n";
	std::cout << "Size on disk: " << calculate_directory_size_on_disk() << " Bytes\n";
	std::cout << "Contains: " << fileSystem.rootDirectory.size() << " Files\n";
}

bool FileManager::display_file_info(const std::string& name) {
	try {
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Error1
		if (fileIterator == fileSystem.rootDirectory.end()) {
			throw("Plik o nazwie '" + name + "' nie znaleziony!");
		}

		const auto file = &fileSystem.inodeTable[fileIterator->second];

		std::cout << "Name: " << name << '\n';
		std::cout << "Size: " << file->realSize << " Bytes\n";
		std::cout << "Size on disk: " << file->blocksOccupied*BLOCK_SIZE << " Bytes\n";
		std::cout << "Created: " << file->creationTime << '\n';
		std::cout << "Modified: " << file->modificationTime << '\n';
		std::cout << "Direct block indexes: ";
		for (const auto& elem : file->directBlocks) {
			if (elem != -1) { std::cout << elem << ' '; }
			else { std::cout << -1 << ' '; }
		}
		std::cout << '\n';

		std::cout << "Indirect block index: ";
		if (file->singleIndirectBlocks != -1)
		{
			std::cout << file->singleIndirectBlocks << ' ';
			std::cout << '\n';
			std::cout << "Indirect block indexes: ";
			std::array<u_int, BLOCK_SIZE / 2>blocks;
			blocks.fill(-1);
			blocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
			for (const u_int& block : blocks) {
				if (block != -1) {
					std::cout << block << ' ';
				}
				else { std::cout << -1 << ' '; }
			}
			std::cout << '\n';
		}
		else { std::cout << -1 << "\n"; }
		std::cout << "Saved data: " << file_read_all(file) << '\n';
		return true;
	}
	catch (const std::string& description) {
		std::cout << description << '\n';
		return false;
	}
}

void FileManager::display_root_directory() {
	std::cout << std::string(1, ' ') << "root/" << "\n";

	for (auto i = fileSystem.rootDirectory.begin(); i != fileSystem.rootDirectory.end(); ++i) {
		std::cout << std::string(2, ' ') << "- " << i->first << '\n';
	}
}

void FileManager::display_disk_content_char() {
	for (u_int i = 0; i < DISK_CAPACITY / BLOCK_SIZE; i++) {
		std::cout << std::setfill('0') << std::setw(2) << i << ".  ";
		for (u_int j = 0; j < BLOCK_SIZE; j++) {
			if (disk.space[i*BLOCK_SIZE + j] >= 0 && disk.space[i*BLOCK_SIZE + j] <= 32) { std::cout << "."; }
			else { std::cout << disk.space[i*BLOCK_SIZE + j]; }
		}
		if (i % 2 == 1) { std::cout << '\n'; }
		else { std::cout << "  "; }
	}
	std::cout << '\n';
}

void FileManager::display_bit_vector() {
	u_int index = 0;
	for (u_int i = 0; i < fileSystem.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << fileSystem.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}



//------------------- Metody Sprawdzaj¹ce -------------------

const bool FileManager::check_if_name_used(const std::string& name) {
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	if (fileSystem.rootDirectory.find(name) != fileSystem.rootDirectory.end()) {
		return true;
	}
	else { return false; }
}

const bool FileManager::check_if_enough_space(const u_int& dataSize) const {
	if (dataSize <= fileSystem.freeSpace) { return true; }
	return false;
}



//-------------------- Metody Obliczaj¹ce -------------------

const u_int FileManager::calculate_needed_blocks(const size_t& dataSize) const {
	/*
	Przybli¿enie w górê rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa¿, jeœli zape³nia chocia¿ o jeden bajt
	wiêcej przy zajêtym bloku, to trzeba zaalokowaæ wtedy kolejny blok.
	*/
	return int(ceil(double(dataSize) / double(BLOCK_SIZE)));
}

const size_t FileManager::calculate_directory_size_on_disk() {
	//Rozmiar katalogu
	size_t size = 0;

	//Dodaje rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto& element : fileSystem.rootDirectory) {
		size += fileSystem.inodeTable[element.second].blocksOccupied*BLOCK_SIZE;
	}
	return size;
}

const size_t FileManager::calculate_directory_size() {
	//Rzeczywisty rozmiar katalogu
	size_t realSize = 9;

	//Dodaje rzeczywisty rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto& element : fileSystem.rootDirectory) {
		realSize += fileSystem.inodeTable[element.second].realSize;
	}
	return realSize;
}



//--------------------- Metody Alokacji ---------------------

void FileManager::file_truncate(Inode* file, const u_int& neededBlocks) {
	if (neededBlocks != file->blocksOccupied) {
		//Jeœli nale¿y zmniejszyæ plik
		if (neededBlocks < file->blocksOccupied) { file_allocation_decrease(file, neededBlocks); }
		//Jeœli nale¿y zwiêkszyæ plik
		else if (neededBlocks > file->blocksOccupied) { file_allocation_increase(file, neededBlocks); }
	}
}

void FileManager::file_add_indexes(Inode* file, const std::vector<u_int>& blocks) {
	if (file != nullptr) {
		if (blocks.size() != size_t(0) && file->blocksOccupied * BLOCK_SIZE <= MAX_FILE_SIZE) {

			u_int blocksIndex = 0;
			//Wpisanie bloków do bezpoœredniego bloku indeksowego
			for (size_t i = 0; i < BLOCK_INDEX_NUMBER && blocksIndex < blocks.size(); i++) {
				file->directBlocks[i] = blocks[blocksIndex];
				blocksIndex++;
			}

			//Wpisanie bloków do 1-poziomowego bloku indeksowego
			if (blocks.size() > BLOCK_INDEX_NUMBER) {
				const u_int index = find_unallocated_blocks(1)[0];

				file->blocksOccupied++;
				file->singleIndirectBlocks = index;

				std::array<u_int, BLOCK_SIZE / 2> indirectBlocks;
				indirectBlocks.fill(-1);

				for (size_t i = 0; i < BLOCK_SIZE / 2 && blocksIndex < blocks.size(); i++) {
					indirectBlocks[i] = blocks[blocksIndex];
					blocksIndex++;
				}
				disk.write(index * 32, indirectBlocks);
				fileSystem.bitVector[index] = BLOCK_OCCUPIED;
				fileSystem.freeSpace -= BLOCK_SIZE;
				if (detailedMessages) { std::cout << "Stworzono blok indeksowy pod indeksem: " << index << '\n'; }
			}
		}
	}
}

void FileManager::file_allocation_increase(Inode* file, const u_int& neededBlocks) {
	const u_int increaseBlocksNumber = abs(int(neededBlocks - file->blocksOccupied));
	bool fitsAfterLastIndex = true;
	u_int index = 0;
	u_int lastBlockIndex = 0;

	if (file->blocksOccupied != 0) {
		//Jeœli indeksy zapisane s¹ tylko w bezpoœrednim bloku indeksowym 
		if (file->blocksOccupied <= BLOCK_INDEX_NUMBER) {
			for (u_int i = 0; i >= 0 && index == -1; i--) {
				lastBlockIndex = index;
				index = file->directBlocks[i];
				if (i == 0) { break; } //u_int po obni¿eniu zera przyjmuje wartoœæ wiêksz¹ od zera
			}
		}
		else {
			std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
			indirectBlocks.fill(-1);
			indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);

			for (u_int i = 0; i >= 0 && index == -1; i--) {
				lastBlockIndex = index;
				index = indirectBlocks[i];
			}
		}

		for (u_int i = lastBlockIndex + 1; i < lastBlockIndex + increaseBlocksNumber + 1; i++) {
			if (fileSystem.bitVector[i] == BLOCK_OCCUPIED) { fitsAfterLastIndex = false; break; }
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
		file_allocate_blocks(file, blocks);
	}
	else {
		if (file->blocksOccupied > 0) { file_deallocate(file); };
		file_allocate_blocks(file, find_unallocated_blocks(neededBlocks));
	}
	if (detailedMessages) { std::cout << "Zwiêkszono plik do rozmiaru " << file->blocksOccupied*BLOCK_SIZE << " Bajt.\n"; }
}

void FileManager::file_allocation_decrease(Inode* file, const u_int& neededBlocks) {
	const u_int sizeToStart = neededBlocks * BLOCK_SIZE;
	//Zmienna do analizowania, czy ju¿ mo¿na usuwaæ/dodawaæ bloki do pliku
	u_int currentSize = 0;

	u_int indexNumber = 0;
	u_int index = file->directBlocks[indexNumber];

	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
	indirectBlocks.fill(-1);
	if (file->blocksOccupied > BLOCK_INDEX_NUMBER) {
		indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
	}

	//Dopóki indeks na coœ wskazuje
	bool indirect = false;
	while (index != -1) {
		currentSize += BLOCK_SIZE;
		//Sprawdzenie czy mamy doczynienia z niebezpoœrednim blokiem indeksowym
		if (indexNumber == BLOCK_INDEX_NUMBER) {
			indirect = true;
			indexNumber = 0;
		}
		if (!indirect) {
			index = file->directBlocks[indexNumber];
		}
		else if (indirect) {
			index = indirectBlocks[indexNumber];
		}
		//Spisz kolejny indeks
		indexNumber++;

		//Jeœli obecny rozmiar przewy¿sza rozmiar potrzebny do rozpoczêcia usuwania
		//zacznij usuwaæ bloki
		if (currentSize > sizeToStart && index != -1) {
			//Zmniejszenie rozmiaru pliku
			file->blocksOccupied--;

			//Po uciêciu rozmiar i rozmiar rzeczywisty bêd¹ takie same
			file->realSize = file->blocksOccupied*BLOCK_SIZE;

			//Oznacz obecny indeks jako wolny
			change_bit_vector_value(index, BLOCK_FREE);

			//Obecny indeks w pliku wskazuje na nic
			if (!indirect) {
				file->directBlocks[index] = -1;
			}
			else if (indirect) {
				indirectBlocks[index] = -1;
			}
		}
	}

	if (detailedMessages) { std::cout << "Zmniejszono plik do rozmiaru " << file->blocksOccupied*BLOCK_SIZE << " Bajt.\n"; }
}

void FileManager::file_deallocate(Inode* file) {
	std::vector<u_int>freedBlocks; //Zmienna u¿yta do wyœwietlenia komunikatu

	if (file->directBlocks[0] != -1) {
		u_int index = 0;
		u_int indexNumber = 0;
		bool indirect = false;

		std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
		indirectBlocks.fill(-1);
		if (file->singleIndirectBlocks != -1) {
			indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
		}

		while (index != -1 && indexNumber < MAX_FILE_SIZE / BLOCK_SIZE) {
			if (indexNumber == BLOCK_INDEX_NUMBER) {
				indirect = true;
				indexNumber = 0;
			}
			if (!indirect) {
				index = file->directBlocks[indexNumber];
				file->directBlocks[indexNumber] = -1;
			}
			else if (indirect) {
				index = indirectBlocks[indexNumber];
			}
			else { index = -1; }

			if (index != -1) {
				freedBlocks.push_back(index);
				change_bit_vector_value(index, BLOCK_FREE);
				indexNumber++;
			}
		}
		file->directBlocks.fill(-1);
		file->blocksOccupied = 0;
	}
	if (detailedMessages) {
		std::sort(freedBlocks.begin(), freedBlocks.end());
		std::cout << "Zwolniono bloki: ";
		for (u_int i = 0; i < freedBlocks.size(); i++) { std::cout << freedBlocks[i] << (i < freedBlocks.size() - 1 ? ", " : ""); }
		std::cout << '\n';
	}
}

void FileManager::file_allocate_blocks(Inode* file, const std::vector<u_int>& blocks) {

	for (const auto& i : blocks) {
		change_bit_vector_value(i, BLOCK_OCCUPIED);
	}
	file->blocksOccupied += static_cast<u_char>(blocks.size());

	file_add_indexes(file, blocks);

	if (detailedMessages) {
		std::cout << "Zaalokowano bloki: ";
		for (u_int i = 0; i < blocks.size(); i++) { std::cout << blocks[i] << (i < blocks.size() - 1 ? ", " : ".\n"); }
	}
}

const std::vector<u_int> FileManager::find_unallocated_blocks_fragmented(u_int blockNumber) {
	//Lista wolnych bloków
	std::vector<u_int> blockList;

	//Szuka wolnych bloków
	for (u_int i = 0; i < fileSystem.bitVector.size(); i++) {
		//Jeœli blok wolny
		if (fileSystem.bitVector[i] == BLOCK_FREE) {
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

const std::vector<u_int> FileManager::find_unallocated_blocks_best_fit(const u_int& blockNumber) {
	//Lista indeksów bloków (dopasowanie)
	std::vector<u_int> blockList;
	//Najlepsze dopasowanie
	std::vector<u_int> bestBlockList(fileSystem.bitVector.size() + 1);

	//Szukanie wolnych bloków spe³niaj¹cych minimum miejsca
	for (u_int i = 0; i < fileSystem.bitVector.size(); i++) {
		if (fileSystem.bitVector[i] == BLOCK_FREE) {
			//Dodaj indeks bloku do listy bloków
			blockList.push_back(i);
		}
		else {
			//Jeœli uzyskana lista bloków jest wiêksza od iloœci bloków jak¹ chcemy uzyskaæ
			//to dodaj uzyskane dopasowanie do listy dopasowañ;
			if (blockList.size() >= blockNumber) {
				//Jeœli znalezione dopasowanie mniejsze ni¿ najlepsze dopasowanie
				if (blockList.size() < bestBlockList.size()) {
					bestBlockList = blockList;
					if (bestBlockList.size() == blockNumber) { break; }
				}
			}

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
		if (blockList.size() < bestBlockList.size()) {
			bestBlockList = blockList;
		}
	}

	//Jeœli znalezione najlepsze dopasowanie
	if (bestBlockList.size() < fileSystem.bitVector.size()) {
		//Odetnij nadmiarowe indeksy z dopasowania (jeœli wiêksze ni¿ potrzeba)
		bestBlockList.resize(blockNumber);
	}
	else { bestBlockList.resize(0); }

	return bestBlockList;
}

const std::vector<u_int> FileManager::find_unallocated_blocks(const u_int& blockNumber) {
	//Szuka bloków funkcj¹ z metod¹ best-fit
	std::vector<u_int> blockList = find_unallocated_blocks_best_fit(blockNumber);

	//Jeœli funkcja z metod¹ best-fit nie znajdzie dopasowañ
	if (blockList.empty()) {
		//Szuka niezaalokowanych bloków, wybieraj¹c pierwsze wolne
		blockList = find_unallocated_blocks_fragmented(blockNumber);
	}

	return blockList;
}



//----------------------- Metody Inne -----------------------

void FileManager::file_write(Inode* file, const std::string& data) {
	file->modificationTime = get_current_time_and_date();

	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>fileFragments = data_to_data_fragments(data);

	//Alokowanie bloków dla pliku
	file_truncate(file, fileFragments.size());

	file->realSize = static_cast<u_short_int>(data.size());

	//Indeks pod którym maj¹ zapisywane byæ dane
	u_int indexNumber = 0;
	u_int index = file->directBlocks[0];
	bool indirect = false;

	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
	indirectBlocks.fill(-1);
	if (file->singleIndirectBlocks != -1) {
		indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
	}

	//Zapisuje wszystkie dane na dysku
	for (const auto& fileFragment : fileFragments) {
		if (index == -1) { break; }
		//Zapisuje fragment na dysku
		disk.write(index * BLOCK_SIZE, fileFragment);
		//Przypisuje do indeksu numer kolejnego bloku
		indexNumber++;
		if (indexNumber == BLOCK_INDEX_NUMBER && !indirect) {
			indirect = true;
			indexNumber = 0;
		}
		if (!indirect) { index = file->directBlocks[indexNumber]; }
		else if (indirect) { index = indirectBlocks[indexNumber]; }
	}
}

const std::string FileManager::file_read_all(Inode* file) const {
	std::string data;

	for (size_t i = 0; i < BLOCK_INDEX_NUMBER && file->directBlocks[i] != -1; i++) {
		data += disk.read_str(file->directBlocks[i] * BLOCK_SIZE);
	}

	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
	indirectBlocks.fill(-1);
	if (file->singleIndirectBlocks != -1) {
		indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
	}

	for (size_t i = 0; i < BLOCK_INDEX_NUMBER && indirectBlocks[i] != -1; i++) {
		data += disk.read_str(indirectBlocks[i] * BLOCK_SIZE);
	}

	return data;
}

const tm FileManager::get_current_time_and_date() {
	time_t tt;
	time(&tt);
	tm timeAndDate = *localtime(&tt);
	timeAndDate.tm_year += 1900;
	timeAndDate.tm_mon += 1;
	return timeAndDate;
}

void FileManager::change_bit_vector_value(const u_int& block, const bool& value) {
	//Jeœli wartoœæ zajêty to wolne miejsce - BLOCK_SIZE
	if (value == 1) { fileSystem.freeSpace -= BLOCK_SIZE; }
	//Jeœli wartoœæ wolny to wolne miejsce + BLOCK_SIZE
	else if (value == 0) { fileSystem.freeSpace += BLOCK_SIZE; }
	//Przypisanie blokowi podanej wartoœci
	fileSystem.bitVector[block] = value;
}

const std::vector<std::string> FileManager::data_to_data_fragments(const std::string& data) const {
	//Tablica fragmentów podanych danych
	std::vector<std::string>fileFragments;

	//Przetworzenie ca³ych danych
	for (u_int i = 0; i < calculate_needed_blocks(data.size()); i++) {
		//Oblicza pocz¹tek kolejnej czêœci fragmentu danych.
		const u_int substrBegin = i * BLOCK_SIZE;
		//Dodaje do tablicy fragmentów kolejny fragment o d³ugoœci BLOCK_SIZE
		fileFragments.push_back(data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

void FileManager::FileReader::read(const u_short_int& byteNumber) {
	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks;
	indirectBlocks.fill(-1);
	if (file->singleIndirectBlocks != -1) {
		indirectBlocks = disk->read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
	}

	for (u_int i = 0; i < byteNumber; i++) {
		const u_int blockNumber = static_cast<u_int>(std::floor(posPointer / BLOCK_SIZE));
		if (blockNumber < BLOCK_INDEX_NUMBER) {
			buffer += disk->read_str(file->directBlocks[blockNumber]);
		}
		else {
			buffer += disk->read_str(file->directBlocks[blockNumber - BLOCK_INDEX_NUMBER]);
		}
		posPointer++;
		if (posPointer > MAX_DATA_SIZE) {
			resetPosPointer();
			break;
		}
	}
}
