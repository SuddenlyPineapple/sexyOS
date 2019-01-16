/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 04/01/19
*/

/*
 * Aby ³atwiej nawigowaæ po moim kodzie polecam z³o¿yæ wszystko
 * Skrót: CTRL + M + A
 */

#include "FileManager.h"
#include "Planista.h"
#include "Procesy.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>

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
	os << time.tm_hour << ':' << std::setfill('0') << std::setw(2) << time.tm_min
		<< ' ' << std::setfill('0') << std::setw(2) << time.tm_mday << '.'
		<< std::setfill('0') << std::setw(2) << time.tm_mon << '.' << time.tm_year;
	return os;
}
bool operator == (const tm& time1, const tm& time2) {
	return time1.tm_hour == time2.tm_hour && time1.tm_isdst == time2.tm_isdst && time1.tm_mday == time2.tm_mday && time1
		.tm_min == time2.tm_min && time1.tm_mon == time2.tm_mon && time1.tm_sec == time2.tm_sec && time1.tm_wday ==
		time2.tm_wday && time1.tm_yday == time2.tm_yday && time1.tm_year == time2.tm_year;
}



//----------------------- System Plików ---------------------

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

unsigned FileManager::FileSystem::get_free_inode_id() {
	for (u_int i = 0; i < inodeBitVector.size(); i++) {
		if (!inodeBitVector[i]) { return i; }
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



//-------------------------- I-wêzê³ ------------------------

FileManager::Inode::Inode() : creationTime(), modificationTime(), sem(nullptr, 99) {
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
	//flagOpen = false;
}



//--------------------------- Dysk --------------------------

FileManager::Disk::Disk() {
	//Zape³nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	space.fill(0);
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
	//Zapisywanie 0, jeœli dane nie wype³ni³y ostatniego bloku
	//for (; index <= end; index++) {
	//	space[index] = 0;
	//}
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
		if (space[index] != 0) { data += space[index]; }
	}
	return data;
}

const std::array<u_int, FileManager::BLOCK_SIZE / 2> FileManager::Disk::read_arr(const u_int& begin) const {
	const u_int end = begin + BLOCK_SIZE;		//Odczytywany jest jeden blok
	std::array<u_int, BLOCK_SIZE / 2> result{};	//Jedna liczba zajmuje 2 bajty
	std::string data;

	result.fill(-1);

	//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
	for (u_int index = begin; index < end; index++) {
		//Dodaj znak zapisany na dysku do danych
		if (space[index] != 0) { data += space[index]; }
	}

	std::string num;
	u_int arrIndex = 0;
	for (const char& c : data) {
		if (c == 0) { break; }
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



//------------------------- File IO -------------------------

void FileManager::FileIO::buffer_update(const int8_t& blockNumber) {
	if (blockNumber < BLOCK_INDEX_NUMBER) {
		buffer = disk->read_str(file->directBlocks[blockNumber] * BLOCK_SIZE);
	}
	else if (file->singleIndirectBlocks != -1) {
		std::array<u_int, BLOCK_SIZE / 2>blocks{};
		blocks.fill(-1);
		blocks = disk->read_arr(file->singleIndirectBlocks*BLOCK_SIZE);

		buffer = disk->read_str(blocks[blockNumber - BLOCK_INDEX_NUMBER] * BLOCK_SIZE);
	}
	else { buffer = '\0'; }
}

std::string FileManager::FileIO::read(const u_short_int& byteNumber) {
	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks{};

	indirectBlocks.fill(-1);

	auto blockIndex = static_cast<uint8_t>(floor(readPos / BLOCK_SIZE));
	if (blockIndex > BLOCK_INDEX_NUMBER + BLOCK_SIZE / 2) { return ""; }

	std::string data;
	u_short_int bytesRead = 0; //Zmienna zliczaj¹ca odczytane bajty
	uint8_t blockIndexPrev = -1; //Zmienna u¿ywana do sprawdzenia czy nale¿y zaktualizowaæ bufor

	while (bytesRead < byteNumber) {
		if (readPos >= file->realSize) { break; }
		if (blockIndex != blockIndexPrev) {
			this->buffer_update(blockIndex);
			blockIndexPrev = blockIndex;
		}

		data += this->buffer[readPos%BLOCK_SIZE];

		readPos++;
		bytesRead++;
		blockIndex = static_cast<int8_t>(floor(readPos / BLOCK_SIZE));
	}

	return data;
}

std::string FileManager::FileIO::read_all() {
	this->reset_read_pos();
	return read(file->realSize);
}

void FileManager::FileIO::write(const std::vector<std::string>& dataFragments, const int8_t& startIndex) const {
	u_int indexNumber = startIndex;
	u_int index;

	bool indirect = false;

	std::array<u_int, BLOCK_SIZE / 2>indirectBlocks{};
	indirectBlocks.fill(-1);
	if (file->singleIndirectBlocks != -1) {
		indirectBlocks = disk->read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
	}

	if (startIndex < BLOCK_INDEX_NUMBER) { index = file->directBlocks[startIndex]; }
	else { index = indirectBlocks[startIndex - BLOCK_INDEX_NUMBER]; }

	//Zapisuje wszystkie dane na dysku
	for (const auto& fileFragment : dataFragments) {
		if (index == -1) { break; }

		//Zapisuje fragment na dysku
		disk->write(index * BLOCK_SIZE, fileFragment);

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

const std::bitset<2> FileManager::FileIO::get_flags() const {
	std::bitset<2> flags;
	flags[READ_FLAG] = readFlag; flags[WRITE_FLAG] = writeFlag;
	return flags;
}



//-------------------- Podstawowe Metody --------------------

int FileManager::file_create(const std::string& name, const std::string& procName) {
	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }
		//Error2
		if (fileSystem.rootDirectory.size() >= INODE_NUMBER_LIMIT) {
			return FILE_ERROR_NO_INODES_LEFT;
		}
		//Error3
		if (check_if_name_used(name)) { return FILE_ERROR_NAME_USED; }
		//Error4
		if (name.size() > MAX_FILENAME_LENGTH) { return FILE_ERROR_NAME_TOO_LONG; }
	}

	//Czêœæ dzia³aj¹ca
	{
		const u_int inodeId = fileSystem.get_free_inode_id();
		//Dodanie pliku do katalogu g³ównego
		fileSystem.rootDirectory[name] = inodeId;
		fileSystem.inodeBitVector[inodeId] = true;
		fileSystem.inodeTable[inodeId].creationTime = get_current_time_and_date();
		fileSystem.inodeTable[inodeId].modificationTime = get_current_time_and_date();

		if (messages) { std::cout << "Stworzono plik o nazwie '" << name << "'.\n"; }
		return file_open(name, procName, FILE_OPEN_W_MODE);
	}
}

int FileManager::file_write(const std::string& name, const std::string& procName, const std::string& data) {
	Inode* inode;

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (data.size() > MAX_DATA_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }

		//Error3
		if (data.size() > BLOCK_INDEX_NUMBER * BLOCK_SIZE &&
			fileSystem.freeSpace < calculate_needed_blocks(data.size())*BLOCK_SIZE + BLOCK_SIZE) {
			return FILE_ERROR_DATA_TOO_BIG;
		}

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Error4
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
		inode = &fileSystem.inodeTable[fileIterator->second];

		//Error5
		if (!inode->opened) { return FILE_ERROR_NOT_OPENED; }

		//Error6
		if (accessedFiles.find(std::pair(name, procName)) == accessedFiles.end()) {
			return FILE_ERROR_NOT_OPENED;
		}

		//Error7
		if (!accessedFiles[std::pair(name, procName)].get_flags()[WRITE_FLAG]) { return FILE_ERROR_NOT_W_MODE; }

		//Error8
		if (data.size() > fileSystem.freeSpace - inode->blocksOccupied*BLOCK_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }
	}

	//Czêœæ dzia³aj¹ca
	PCB* proc = tree->find_proc(procName);
	proc->change_state(READY);
	p->Check();
	while (proc->state != RUNNING) {
		std::cout << "Czekanie na przydzielenie procesora . . . " << procName << "\n";
		p->Check();
	}
	file_deallocate(inode);
	file_write(inode, &accessedFiles[std::pair(name, procName)], data);
	if (messages) { std::cout << "Zapisano dane do pliku o nazwie '" << name << "'.\n"; }
	return FILE_ERROR_NONE;
}

int FileManager::file_append(const std::string& name, const std::string& procName, const std::string& data) {
	Inode* inode;

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (data.size() > BLOCK_INDEX_NUMBER * BLOCK_SIZE && fileSystem.freeSpace < calculate_needed_blocks(data.size())*BLOCK_SIZE + BLOCK_SIZE) {
			return FILE_ERROR_DATA_TOO_BIG;
		}

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Error3
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
		inode = &fileSystem.inodeTable[fileIterator->second];

		//Error4
		if (!inode->opened) { return FILE_ERROR_NOT_OPENED; }

		//Error5
		if (!accessedFiles[std::pair(name, procName)].get_flags()[WRITE_FLAG]) { return FILE_ERROR_NOT_W_MODE; }

		//Error6
		if (inode->realSize + data.size() > MAX_DATA_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }

		//Error7
		if (data.size() > fileSystem.freeSpace) { return FILE_ERROR_DATA_TOO_BIG; }
	}

	//Czêœæ dzia³aj¹ca
	PCB* proc = tree->find_proc(procName);
	while (proc->state != RUNNING) {
		if (detailedMessages) { std::cout << "Czekanie na przydzielenie procesora . . . (" << procName << ")\n"; }
		p->Check();
	}
	file_append(inode, &accessedFiles[std::pair(name, procName)], data);
	if (messages) { std::cout << "Wpisano dane do pliku o nazwie '" << name << "'.\n"; }
	return FILE_ERROR_NONE;
}

int FileManager::file_read(const std::string& name, const std::string& procName, const u_short_int& byteNumber, std::string& result) {
	//Iterator zwracany podczas przeszukiwania za plikiem o podanej nazwie
	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Czêœæ sprawdzaj¹ca --------------------
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

		//Error3
		if (accessedFiles.find(std::pair(name, procName)) == accessedFiles.end()) { return FILE_ERROR_NOT_OPENED; }

		//Error4
		if (!accessedFiles[std::pair(name, procName)].get_flags()[READ_FLAG]) { return FILE_ERROR_NOT_R_MODE; }
	}

	//Czêœæ dzia³aj¹ca ----------------------
	PCB* proc = tree->find_proc(procName);
	proc->change_state(READY);
	while (proc->state != RUNNING) {
		if (detailedMessages) { std::cout << "Czekanie na przydzielenie procesora . . . (" << procName << ", prio: " << proc->priority << ")\n"; }

		p->Check();
		p->displayPCBLists();
	}
	result = accessedFiles[std::pair(name, procName)].read(byteNumber);
	return FILE_ERROR_NONE;
}

int FileManager::file_read_all(const std::string& name, const std::string& procName, std::string& result) {
	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
	}

	//Czêœæ dzia³aj¹ca
	return file_read(name, procName, fileSystem.inodeTable[fileSystem.rootDirectory[name]].realSize, result);
}

int FileManager::file_delete(const std::string& name, const std::string& procName) {
	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

		//Error3
		if (accessedFiles.find(std::pair(name, procName)) != accessedFiles.end()) { return FILE_ERROR_OPENED; }
	}

	//Czêœæ dzia³aj¹ca
	{
		Inode* inode = &fileSystem.inodeTable[fileIterator->second];

		file_deallocate(inode);
		fileSystem.inodeTable[fileIterator->second].clear();

		//Usuñ wpis o pliku z obecnego katalogu
		fileSystem.rootDirectory.erase(fileIterator);

		if (messages) { std::cout << "Usunieto plik o nazwie '" << name << "'.\n"; }
		return FILE_ERROR_NONE;
	}
}

int FileManager::file_open(const std::string& name, const std::string& procName, const unsigned int& mode) {
	Inode* inode;
	std::bitset<2>mode_(mode);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_DATA_TOO_BIG; }

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(name);

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

		inode = &fileSystem.inodeTable[fileIterator->second];

		//Error3
		if (accessedFiles.find(std::pair(name, procName)) != accessedFiles.end()) {}
		else if (inode->sem.is_blocked()) {
			tree->find_proc(procName)->change_state(WAITING);
			p->Check();
			return FILE_ERROR_SYNC;
		}
	}

	//Czêœæ dzia³aj¹ca
	{
		if (accessedFiles.find(std::pair(name, procName)) != accessedFiles.end()) {
			if (const int result = file_close(name, procName) != FILE_ERROR_NONE) { return result; };
		}

		if (!inode->opened) {
			int semVal = 0;
			if (mode == FILE_OPEN_R_MODE) { semVal = 2; } //Tak ma³o, ¿eby mo¿na pokazaæ, ¿e dzia³a dla dwóch a potem nie
			else if (mode == FILE_OPEN_W_MODE) { semVal = 1; }
			inode->sem = Semaphore(p, semVal);
		}

		inode->sem.Wait(nullptr); //Obni¿enie wartoœci semafora
		inode->opened = true;

		accessedFiles[std::pair(name, procName)] = FileIO(&disk, inode, mode_);


		if (messages) {
			std::cout << "Otwarto plik o nazwie '" << name << "' w trybie" << (mode_[1] || mode_[0] ? " " : "")
				<< (mode_[0] ? "R" : "") << (mode_[1] ? "W" : "") << " przez proces " << procName << ".\n";
		}
		return FILE_ERROR_NONE;
	}
}

int FileManager::file_close(const std::string& name, const std::string& procName) {
	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
	}

	//Czêœæ dzia³aj¹ca
	{
		accessedFiles.erase(std::pair(name, procName));
		//Podwy¿szenie wartoœci semafora
		if (is_file_opened_write(name)) {
			if (fileSystem.inodeTable[fileIterator->second].sem.get_value() < 1) {
				fileSystem.inodeTable[fileIterator->second].sem.Signal(nullptr);
			}
		}
		else if (fileSystem.inodeTable[fileIterator->second].sem.get_value() < 2) {
			fileSystem.inodeTable[fileIterator->second].sem.Signal(nullptr);
		}
	}

	if (file_accessing_proc_count(name) == 0) {
		fileSystem.inodeTable[fileIterator->second].opened = false;
		fileSystem.inodeTable[fileIterator->second].sem = Semaphore(nullptr, 99);
	}

	if (messages) { std::cout << "Zamknieto plik o nazwie '" << name << "' przez proces " << procName << ".\n"; }
	return FILE_ERROR_NONE;
}



//--------------------- Dodatkowe Metody --------------------

int FileManager::file_create(const std::string& name, const std::string& procName, const std::string& data) {
	int result = file_create(name, procName);
	if (result != 0) { return result; }

	//Zapisanie danych na dysku
	result = file_write(name, procName, data);
	if (result != 0) {
		file_delete(name, procName);
	}
	return result;
}

int FileManager::file_close_all(const std::string& procName) {
	std::vector<std::pair<std::string, std::string>> fileNames;
	for (const auto& elem : accessedFiles) { fileNames.push_back(elem.first); }
	for (const std::pair<std::string, std::string>& fileName : fileNames) {
		if (fileName.second == procName) {
			if (const int result = file_close(fileName.first, fileName.second) != 0) { return result; }
		}
	}

	return FILE_ERROR_NONE;
}

int FileManager::file_close_all() {
	std::vector<std::pair<std::string, std::string>> fileNames;
	for (const auto& elem : accessedFiles) { fileNames.push_back(elem.first); }
	for (const std::pair<std::string, std::string>& fileName : fileNames) {
		if (const int result = file_close(fileName.first, fileName.second) != 0) { return result; }
		fileSystem.inodeTable[fileSystem.rootDirectory[fileName.first]].sem = Semaphore(nullptr, 99);
	}

	return FILE_ERROR_NONE;
}

void FileManager::set_messages(const bool& onOff) {
	messages = onOff;
}

void FileManager::set_detailed_messages(const bool&  onOff) {
	detailedMessages = onOff;
}



//------------------ Metody do wyœwietlania -----------------

void FileManager::display_file_system_params() {
	std::cout << "                      Disk capacity : " << DISK_CAPACITY << " Bytes\n";
	std::cout << "                         Block size : " << static_cast<int>(BLOCK_SIZE) << " Bytes\n";
	std::cout << "                      Max file size : " << MAX_FILE_SIZE << " Bytes\n";
	std::cout << "                      Max data size : " << MAX_DATA_SIZE << " Bytes\n";
	std::cout << "Max indexes in indirect index block : " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indexes\n";
	std::cout << "         Max direct indexes in file : " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indexes\n";
	std::cout << "                    Max file number : " << static_cast<int>(INODE_NUMBER_LIMIT) << " Files\n";
	std::cout << "                Max filename length : " << static_cast<int>(MAX_FILENAME_LENGTH) << " Characters\n";
}

void FileManager::display_root_directory_info() {
	std::cout << "Name: " << "root" << '\n';
	std::cout << "Size: " << calculate_directory_size() << " Bytes\n";
	std::cout << "Size on disk: " << calculate_directory_size_on_disk() << " Bytes\n";
	std::cout << "Contains: " << fileSystem.rootDirectory.size() << " Files\n";
}

int FileManager::display_file_info(const std::string& name) {
	//Error1
	if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Error2
	if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

	auto file = &fileSystem.inodeTable[fileIterator->second];

	//Wyœwietlanie
	{
		std::cout << "         Name : " << name << '\n';
		std::cout << "I-node number : " << fileIterator->second << '\n';
		std::cout << "         Size : " << file->realSize << " Bytes\n";
		std::cout << " Size on disk : " << file->blocksOccupied*BLOCK_SIZE << " Bytes\n";
		std::cout << "      Created : " << file->creationTime << '\n';
		std::cout << "     Modified : " << file->modificationTime << "\n\n";

		std::cout << "Direct block indexes : ";
		for (const auto& elem : file->directBlocks) {
			if (elem != -1) { std::cout << elem << ' '; }
			else { std::cout << -1 << ' '; }
		}
		std::cout << '\n';

		std::cout << "Indirect block index : ";
		if (file->singleIndirectBlocks != -1)
		{
			std::cout << file->singleIndirectBlocks << ' ';
			std::cout << '\n';
			std::cout << "Indirect block indexes: ";
			std::array<u_int, BLOCK_SIZE / 2>blocks{};
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


		std::cout << "Saved data : ";
		FileIO tempIO(&disk, file, std::bitset<2>(3));
		std::cout << tempIO.read_all() << '\n';
	}

	return FILE_ERROR_NONE;
}

void FileManager::display_root_directory() {
	std::cout << std::string(1, ' ') << "root" << "\n";

	for (auto it = fileSystem.rootDirectory.begin(); it != fileSystem.rootDirectory.end(); ++it) {
		++it;
		std::cout << ' ';
		if (it != fileSystem.rootDirectory.end()) { std::cout << static_cast<u_char>(195); }
		else { std::cout << static_cast<u_char>(192); }
		--it;
		std::cout << std::string(2, static_cast<u_char>(196)) << " " << it->first << '\n';
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

bool FileManager::check_if_name_used(const std::string& name) {
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	return fileSystem.rootDirectory.find(name) != fileSystem.rootDirectory.end();
}

bool FileManager::check_if_enough_space(const u_int& dataSize) const {
	return dataSize <= fileSystem.freeSpace;
}



//-------------------- Metody Obliczaj¹ce -------------------

u_int FileManager::calculate_needed_blocks(const size_t& dataSize) {
	/*
	Przybli¿enie w górê rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa¿, jeœli zape³nia chocia¿ o jeden bajt
	wiêcej przy zajêtym bloku, to trzeba zaalokowaæ wtedy kolejny blok.
	*/
	return int(ceil(double(dataSize) / double(BLOCK_SIZE)));
}

size_t FileManager::calculate_directory_size_on_disk() {
	//Rozmiar katalogu
	size_t size = 0;

	//Dodaje rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto& element : fileSystem.rootDirectory) {
		size += fileSystem.inodeTable[element.second].blocksOccupied*BLOCK_SIZE;
	}
	return size;
}

size_t FileManager::calculate_directory_size() {
	//Rzeczywisty rozmiar katalogu
	size_t realSize = 0;

	//Dodaje rzeczywisty rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto& element : fileSystem.rootDirectory) {
		realSize += fileSystem.inodeTable[element.second].realSize;
	}
	return realSize;
}



//--------------------- Metody Alokacji ---------------------

void FileManager::file_truncate(Inode* file, const u_int& neededBlocks) {
	if (neededBlocks != file->blocksOccupied) {
		if (neededBlocks > file->blocksOccupied) { file_allocation_increase(file, neededBlocks); }
	}
}

void FileManager::file_add_indexes(Inode* file, const std::vector<u_int>& blocks) {
	if (file != nullptr) {
		if (!blocks.empty() && file->blocksOccupied * BLOCK_SIZE <= MAX_FILE_SIZE) {

			u_int blocksIndex = 0;
			//Wpisanie bloków do bezpoœredniego bloku indeksowego
			for (size_t i = 0; i < BLOCK_INDEX_NUMBER && blocksIndex < blocks.size(); i++) {
				if (file->directBlocks[i] == -1) {
					file->directBlocks[i] = blocks[blocksIndex];
					blocksIndex++;
				}
			}

			//Wpisanie bloków do 1-poziomowego bloku indeksowego
			if (file->blocksOccupied > BLOCK_INDEX_NUMBER) {
				const u_int index = find_unallocated_blocks(1)[0];

				file->blocksOccupied++;
				file->singleIndirectBlocks = index;

				std::array<u_int, BLOCK_SIZE / 2> indirectBlocks{};
				indirectBlocks.fill(-1);

				for (size_t i = 0; i < BLOCK_SIZE / 2 && blocksIndex < blocks.size(); i++) {
					if (indirectBlocks[i] == -1) {
						indirectBlocks[i] = blocks[blocksIndex];
						blocksIndex++;
					}
				}
				disk.write(index * BLOCK_SIZE, indirectBlocks);
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
	u_int index = -1;
	u_int lastBlockIndex = -1;

	if (file->blocksOccupied != 0) {
		//Jeœli indeksy zapisane s¹ tylko w bezpoœrednim bloku indeksowym 
		if (file->blocksOccupied <= BLOCK_INDEX_NUMBER) {
			for (int i = BLOCK_INDEX_NUMBER - 1; i >= 0 && index == -1; i--) {
				index = file->directBlocks[i];
				lastBlockIndex = index;
			}
		}
		else {
			std::array<u_int, BLOCK_SIZE / 2>indirectBlocks{};
			indirectBlocks.fill(-1);
			indirectBlocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);

			for (int i = BLOCK_SIZE / 2 - 1; i >= 0 && index == -1; i--) {
				index = indirectBlocks[i];
				lastBlockIndex = index;
			}
		}
		for (u_int i = lastBlockIndex + 1; i < lastBlockIndex + increaseBlocksNumber + 1; i++) {
			if (fileSystem.bitVector[i] == BLOCK_OCCUPIED) { fitsAfterLastIndex = false; break; }
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
		if (file->blocksOccupied > 0) { file_deallocate(file); }
		file_allocate_blocks(file, find_unallocated_blocks(neededBlocks));
	}
	if (detailedMessages) { std::cout << "Zwiekszono plik do rozmiaru " << file->blocksOccupied*BLOCK_SIZE << " Bajt.\n"; }
}

void FileManager::file_deallocate(Inode* file) {
	std::vector<u_int>freedBlocks; //Zmienna u¿yta do wyœwietlenia komunikatu

	if (file->directBlocks[0] != -1) {
		u_int index = 0;
		u_int indexNumber = 0;
		bool indirect = false;

		std::array<u_int, BLOCK_SIZE / 2>indirectBlocks{};
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
		if (!freedBlocks.empty()) {
			std::sort(freedBlocks.begin(), freedBlocks.end());
			std::cout << "Zwolniono bloki: ";
			for (u_int i = 0; i < freedBlocks.size(); i++) { std::cout << freedBlocks[i] << (i < freedBlocks.size() - 1 ? ", " : ""); }
			std::cout << '\n';
		}
	}
}

void FileManager::file_allocate_blocks(Inode* file, const std::vector<u_int>& blocks) {

	for (const auto& i : blocks) {
		change_bit_vector_value(i, BLOCK_OCCUPIED);
	}
	file->blocksOccupied += static_cast<int8_t>(blocks.size());

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

bool FileManager::is_file_opened_write(const std::string& fileName) {
	for (auto& elem : accessedFiles) {
		if (elem.first.first == fileName) {
			if (elem.second.get_flags()[1]) { return true; }
		}
	}
	return false;
}

int FileManager::file_accessing_proc_count(const std::string& fileName) {
	int result = 0;
	for (auto& elem : accessedFiles) {
		if (elem.first.first == fileName) { result++; }
	}
	return result;
}

std::string FileManager::get_file_data_block(Inode* file, const int8_t& indexNumber) const {
	std::string data;

	if (indexNumber < BLOCK_INDEX_NUMBER) {
		data = disk.read_str(file->directBlocks[indexNumber] * BLOCK_SIZE);
	}
	else if (file->singleIndirectBlocks != -1) {
		std::array<u_int, BLOCK_SIZE / 2>blocks{};
		blocks.fill(-1);
		blocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);

		data = disk.read_str(blocks[indexNumber - BLOCK_INDEX_NUMBER]);
	}
	return data;
}

void FileManager::file_write(Inode* file, FileIO* IO, const std::string& data) {
	file->modificationTime = get_current_time_and_date();

	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>dataFragments = fragmentate_data(data);

	//Alokowanie bloków dla pliku
	file_truncate(file, dataFragments.size());

	IO->write(dataFragments, 0);

	file->realSize = static_cast<u_short_int>(data.size());
}

void FileManager::file_append(Inode* file, FileIO* IO, const std::string& data) {
	file->modificationTime = get_current_time_and_date();
	std::string data_ = data;

	int8_t surplusBlock = 0;
	if (file->realSize % BLOCK_SIZE + data_.size() % BLOCK_SIZE > BLOCK_SIZE) { surplusBlock = 1; }

	auto lastBlockIndex = static_cast<int8_t>(floor(file->realSize / BLOCK_SIZE));
	if (file->realSize % BLOCK_SIZE != 0) {
		data_.insert(0, get_file_data_block(file, lastBlockIndex));
	}
	else { lastBlockIndex += surplusBlock; }

	const std::vector<std::string>dataFragments = fragmentate_data(data_);

	//Alokowanie bloków dla nowych danych
	file->blocksOccupied += static_cast<int8_t>(dataFragments.size()) - surplusBlock;
	file_allocate_blocks(file, find_unallocated_blocks(dataFragments.size() - surplusBlock));

	IO->write(dataFragments, lastBlockIndex);

	file->realSize += static_cast<u_short_int>(data.size());
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

const std::vector<std::string> FileManager::fragmentate_data(const std::string& data) {
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
