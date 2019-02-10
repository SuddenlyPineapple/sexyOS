/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
*/

/*
 * Aby ³atwiej nawigowaæ po moim kodzie polecam z³o¿yæ wszystko
 * Skrót: CTRL + M + L
 */

#include "FileManager.h"
#include "Planist.h"
#include "Processes.h"
#include <iomanip>
#include <iostream>

using namespace std;

FileManager fm;

//--------------------------- Aliasy ------------------------
using u_int = unsigned int;
using u_short_int = unsigned short int;
using u_char = unsigned char;



//------------------------- Operatory -----------------------
ostream& operator << (ostream& os, const vector<string>& strVec) {
	for (const string& str : strVec) {
		os << str << '\n';
	}
	return os;
}
ostream& operator << (ostream& os, const tm& time) {
	os << time.tm_hour << ':' << setfill('0') << setw(2) << time.tm_min
		<< ' ' << setfill('0') << setw(2) << time.tm_mday << '.'
		<< setfill('0') << setw(2) << time.tm_mon << '.' << time.tm_year;
	return os;
}
bool operator == (const tm& time1, const tm& time2) {
	return time1.tm_hour == time2.tm_hour && time1.tm_isdst == time2.tm_isdst && time1.tm_mday == time2.tm_mday && time1
		.tm_min == time2.tm_min && time1.tm_mon == time2.tm_mon && time1.tm_sec == time2.tm_sec && time1.tm_wday ==
		time2.tm_wday && time1.tm_yday == time2.tm_yday && time1.tm_year == time2.tm_year;
}



//-------------------- Podstawowe Metody --------------------

int FileManager::file_create(const string& fileName, const string& procName) {
	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		if (!check_if_name_used(fileName)) {
			//Error2
			if (fileSystem.rootDirectory.size() >= INODE_NUMBER_LIMIT) {
				return FILE_ERROR_NO_INODES_LEFT;
			}
		}

		//Sync test
		if (check_if_name_used(fileName)) {
			Inode* file = &fileSystem.inodeTable[fileSystem.rootDirectory[fileName]];
			if (file->sem.is_blocked() && procName.empty()) {
				return FILE_ERROR_SYNC;
			}
			else if (file->sem.is_blocked()) {
				file->sem.wait(tree.find(procName));
				return FILE_ERROR_SYNC;
			}
		}
	}

	//Czêœæ dzia³aj¹ca
	{
		if (!check_if_name_used(fileName)) {
			const u_int inodeId = fileSystem.get_free_inode_id();
			//Dodanie pliku do katalogu g³ównego
			fileSystem.rootDirectory[fileName] = inodeId;
			fileSystem.inodeBitVector[inodeId] = true;
		}

		Inode* file = &fileSystem.inodeTable[fileSystem.rootDirectory[fileName]];
		file->creationTime = get_current_time_and_date();
		file->modificationTime = get_current_time_and_date();

		if (messages) { cout << "Stworzono plik o nazwie '" << fileName << "'.\n"; }
		return file_open(fileName, procName, FILE_OPEN_W_MODE);
	}
}

//Zabezpieczone
int FileManager::file_write(const string& fileName, const string& procName, const string& data) {
	Inode* inode;

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (data.size() > MAX_DATA_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }

		//Error3
		if (data.size() > BLOCK_INDEX_NUMBER * BLOCK_SIZE &&
			fileSystem.freeSpace < calculate_needed_blocks(data.size())*BLOCK_SIZE + BLOCK_SIZE) {
			return FILE_ERROR_DATA_TOO_BIG;
		}

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(fileName);

		//Error4
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
		inode = &fileSystem.inodeTable[fileIterator->second];


		//Error5
		if (!inode->opened) { return FILE_ERROR_NOT_OPENED; }

		//Error6
		if (accessedFiles.find(pair(fileName, procName)) == accessedFiles.end()) {
			return FILE_ERROR_NOT_OPENED;
		}

		//Error7
		if (!accessedFiles[pair(fileName, procName)].get_flags()[WRITE_FLAG]) { return FILE_ERROR_NOT_W_MODE; }

		//Error8
		if (data.size() > fileSystem.freeSpace - inode->blocksOccupied*BLOCK_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }
	}

	//Czêœæ dzia³aj¹ca
	file_deallocate(inode);
	file_write(inode, &accessedFiles[pair(fileName, procName)], data);
	if (messages) { cout << "Zapisano dane do pliku o nazwie '" << fileName << "'.\n"; }
	return FILE_ERROR_NONE;
}

//Zabezpieczone
int FileManager::file_append(const string& fileName, const string& procName, const string& data) {
	Inode* inode;

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (data.size() > BLOCK_INDEX_NUMBER * BLOCK_SIZE && fileSystem.freeSpace < calculate_needed_blocks(data.size())*BLOCK_SIZE + BLOCK_SIZE) {
			return FILE_ERROR_DATA_TOO_BIG;
		}

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(fileName);

		//Error3
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
		inode = &fileSystem.inodeTable[fileIterator->second];

		//Error4
		if (!inode->opened) { return FILE_ERROR_NOT_OPENED; }

		//Error5
		if (!accessedFiles[pair(fileName, procName)].get_flags()[WRITE_FLAG]) { return FILE_ERROR_NOT_W_MODE; }

		//Error6
		if (inode->realSize + data.size() > MAX_DATA_SIZE) { return FILE_ERROR_DATA_TOO_BIG; }

		//Error7
		if (data.size() > fileSystem.freeSpace) { return FILE_ERROR_DATA_TOO_BIG; }
	}

	//Czêœæ dzia³aj¹ca
	file_append(inode, &accessedFiles[pair(fileName, procName)], data);
	if (messages) { cout << "Wpisano dane do pliku o nazwie '" << fileName << "'.\n"; }
	return FILE_ERROR_NONE;
}

//Zabezpieczone
int FileManager::file_read(const string& fileName, const string& procName, const u_short_int& byteNumber, string& result) {
	//Iterator zwracany podczas przeszukiwania za plikiem o podanej nazwie
	const auto fileIterator = fileSystem.rootDirectory.find(fileName);

	//Czêœæ sprawdzaj¹ca --------------------
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

		//Error3
		if (accessedFiles.find(pair(fileName, procName)) == accessedFiles.end()) { return FILE_ERROR_NOT_OPENED; }

		//Error4
		if (!accessedFiles[pair(fileName, procName)].get_flags()[READ_FLAG]) { return FILE_ERROR_NOT_R_MODE; }
	}

	//Czêœæ dzia³aj¹ca ----------------------
	result = accessedFiles[pair(fileName, procName)].read(byteNumber);
	return FILE_ERROR_NONE;
}

//Zabezpieczone
int FileManager::file_read_all(const string& fileName, const string& procName, string& result) {
	const auto fileIterator = fileSystem.rootDirectory.find(fileName);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
	}

	//Czêœæ dzia³aj¹ca
	return file_read(fileName, procName, fileSystem.inodeTable[fileSystem.rootDirectory[fileName]].realSize, result);
}

//Zabezpieczone
int FileManager::file_delete(const string& fileName, const string& procName) {
	const auto fileIterator = fileSystem.rootDirectory.find(fileName);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

		//Error3
		if (file_accessing_proc_count(fileName) == 1 && accessedFiles.find(pair(fileName, procName)) != accessedFiles.end()) {}
		else if (file_accessing_proc_count(fileName) == 0) {}
		else if (accessedFiles.find(pair(fileName, procName)) != accessedFiles.end()) { return FILE_ERROR_OPENED; }
	}

	//Czêœæ dzia0³aj¹ca
	{
		if (fileSystem.rootDirectory.find(fileName) != fileSystem.rootDirectory.end()) {
			Inode* inode = &fileSystem.inodeTable[fileIterator->second];

			file_deallocate(inode);
			fileSystem.inodeTable[fileIterator->second].clear();

			//Usuñ wpis o pliku z obecnego katalogu
			fileSystem.rootDirectory.erase(fileName);

			if (messages) { cout << "Usunieto plik o nazwie '" << fileName << "'.\n"; }
		}
		return FILE_ERROR_NONE;
	}
}

//Zabezpieczone
int FileManager::file_open(const string& fileName, const string& procName, const unsigned int& mode) {
	Inode* file;
	bitset<2>mode_(mode);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_DATA_TOO_BIG; }

		//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
		const auto fileIterator = fileSystem.rootDirectory.find(fileName);

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
		file = &fileSystem.inodeTable[fileIterator->second];

		//ErrorShell
		if (file->sem.is_blocked() && procName.empty()) { return FILE_ERROR_SYNC; }
		//ErrorSync
		else if (file_accessing_proc_count(fileName) >= 1 && mode == FILE_OPEN_W_MODE) {
			file->sem.wait(tree.find(procName));
			return FILE_ERROR_SYNC;
		}
		else if (file_accessing_proc_count(fileName) >= 2 && mode == FILE_OPEN_R_MODE) {
			file->sem.wait(tree.find(procName));
			return FILE_ERROR_SYNC;
		}
	}

	//Czêœæ dzia³aj¹ca
	{
		if (accessedFiles.find(pair(fileName, procName)) != accessedFiles.end()) {
			accessedFiles.erase(pair(fileName, procName));
			file->sem.set_value(file->sem.get_value() + 1);
		}

		if (file_accessing_proc_count(fileName) == 0) {
			file->opened = false;
		}

		if (mode == FILE_OPEN_W_MODE && file->sem.get_value() >= 1 ||
			mode == FILE_OPEN_R_MODE && file->sem.get_value() >= 2) {
			file->sem = Semaphore();
		}

		if (!file->opened) {
			int semVal = 0;
			if (mode == FILE_OPEN_R_MODE) { semVal = 2; } //Tak ma³o, ¿eby mo¿na pokazaæ, ¿e dzia³a dla dwóch a potem nie
			else if (mode == FILE_OPEN_W_MODE) { semVal = 1; }
			if (mode == FILE_OPEN_W_MODE && file->sem.get_value() >= 1 ||
				mode == FILE_OPEN_R_MODE && file->sem.get_value() >= 2) {
				file->sem = Semaphore(semVal);
			}
		}

		file->opened = true;

		accessedFiles[pair(fileName, procName)] = FileIO(&disk, file, mode_);
		file->sem.wait(tree.find(procName));

		if (messages) {
			cout << "Otwarto plik o nazwie '" << fileName << "' w trybie" << (mode_[1] || mode_[0] ? " " : "")
				<< (mode_[0] ? "R" : "") << (mode_[1] ? "W" : "") << " przez proces " << procName << ".\n";
		}
		return FILE_ERROR_NONE;
	}
}

//Zabezpieczone
int FileManager::file_close(const string& fileName, const string& procName) {
	const auto fileIterator = fileSystem.rootDirectory.find(fileName);

	//Czêœæ sprawdzaj¹ca
	{
		//Error1
		if (fileName.empty()) { return FILE_ERROR_EMPTY_NAME; }

		//Error2
		if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }
	}

	//Czêœæ dzia³aj¹ca
	{
		accessedFiles.erase(pair(fileName, procName));
		Inode* file = &fileSystem.inodeTable[fileIterator->second];
		//Podwy¿szenie wartoœci semafora
		file->sem.signal();


		if (file_accessing_proc_count(fileName) == 0) {
			file->opened = false;
		}

		if (messages) { cout << "Zamknieto plik o nazwie '" << fileName << "' przez proces " << procName << ".\n"; }
		return FILE_ERROR_NONE;
	}
}

bool FileManager::file_exists(const std::string& fileName) {
	if (fileSystem.rootDirectory.find(fileName) != fileSystem.rootDirectory.end()) { return true; }
	else { return false; }
}


//--------------------- Dodatkowe Metody --------------------

int FileManager::file_close_all(const string& procName) {
	vector<pair<string, string>> fileNames;
	for (const auto& elem : accessedFiles) { fileNames.push_back(elem.first); }
	for (const pair<string, string>& fileName : fileNames) {
		if (fileName.second == procName) {
			if (const int result = file_close(fileName.first, fileName.second) != 0) { return result; }
		}
	}

	return FILE_ERROR_NONE;
}

int FileManager::file_close_all() {
	vector<pair<string, string>> fileNames;
	for (const auto& elem : accessedFiles) { fileNames.push_back(elem.first); }
	for (const pair<string, string>& fileName : fileNames) {
		if (const int result = file_close(fileName.first, fileName.second) != 0) { return result; }
		fileSystem.inodeTable[fileSystem.rootDirectory[fileName.first]].sem = Semaphore();
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
	cout << " |       Pojemnosc dysku : " << DISK_CAPACITY << " B\n";
	cout << " |         Rozmiar bloku : " << static_cast<int>(BLOCK_SIZE) << " B\n";
	cout << " |    Maks rozmiar pliku : " << MAX_FILE_SIZE << " B\n";
	cout << " |   Maks rozmiar danych : " << MAX_DATA_SIZE << " B\n";
	cout << " | Maks indeksow w bloku : " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indeksow\n";
	cout << " | Maks indeksow w pliku : " << static_cast<int>(BLOCK_INDEX_NUMBER) << " Indeksow\n";
	cout << " |     Maks ilosc plikow : " << static_cast<int>(INODE_NUMBER_LIMIT) << " Plikow\n";
}

void FileManager::display_root_directory_info() {
	cout << " |          Nazwa : " << "root" << '\n';
	cout << " | Rozmiar dancyh : " << calculate_directory_size() << " B\n";
	cout << " |        Rozmiar : " << calculate_directory_size_on_disk() << " B\n";
	cout << " |   Ilosc plikow : " << fileSystem.rootDirectory.size() << " Plikow\n";
}

int FileManager::display_file_info(const string& name) {
	//Error1
	if (name.empty()) { return FILE_ERROR_EMPTY_NAME; }

	const auto fileIterator = fileSystem.rootDirectory.find(name);

	//Error2
	if (fileIterator == fileSystem.rootDirectory.end()) { return FILE_ERROR_NOT_FOUND; }

	auto file = &fileSystem.inodeTable[fileIterator->second];

	//Wyœwietlanie
	{
		cout << " |           Nazwa : " << name << '\n';
		cout << " |   Numer I-wezla : " << fileIterator->second << '\n';
		cout << " |  Rozmiar danych : " << file->realSize << " Bytes\n";
		cout << " |         Rozmiar : " << file->blocksOccupied*BLOCK_SIZE << " Bytes\n";
		cout << " |       Stworzono : " << file->creationTime << '\n';
		cout << " |   Zmodyfikowano : " << file->modificationTime << "\n";
		cout << " |\n";
		cout << " | Indeksy w pliku : ";
		for (const auto& elem : file->directBlocks) {
			if (elem != -1) { cout << elem << ' '; }
			else { cout << -1 << ' '; }
		}
		cout << '\n';

		cout << " |    Indeks bloku : ";
		if (file->singleIndirectBlocks != -1) {
			cout << file->singleIndirectBlocks << " \n";
			cout << " | Indeksy w bloku : ";
			array<u_int, BLOCK_SIZE / 2>blocks{};
			blocks.fill(-1);
			blocks = disk.read_arr(file->singleIndirectBlocks*BLOCK_SIZE);
			for (const u_int& block : blocks) {
				if (block != -1) {
					cout << block << ' ';
				}
				else { cout << -1 << ' '; }
			}
			cout << '\n';
		}
		else { cout << -1 << "\n"; }

		cout << " |   Zapisane dane : ";
		FileIO tempIO(&disk, file, bitset<2>(3));
		cout << tempIO.read_all() << '\n';
	}

	return FILE_ERROR_NONE;
}

void FileManager::display_root_directory() {
	cout << string(1, ' ') << "root" << "\n";

	for (auto it = fileSystem.rootDirectory.begin(); it != fileSystem.rootDirectory.end(); ++it) {
		++it;
		cout << ' ';
		if (it != fileSystem.rootDirectory.end()) { cout << static_cast<u_char>(195); }
		else { cout << static_cast<u_char>(192); }
		--it;
		cout << string(2, static_cast<u_char>(196)) << " " << it->first << '\n';
	}
}

void FileManager::display_disk_content_char() {
	for (u_int i = 0; i < DISK_CAPACITY / BLOCK_SIZE; i++) {
		cout << setfill('0') << setw(2) << i << ".  ";
		for (u_int j = 0; j < BLOCK_SIZE; j++) {
			if (disk.space[i*BLOCK_SIZE + j] >= 0 && disk.space[i*BLOCK_SIZE + j] <= 32) { cout << "."; }
			else { cout << disk.space[i*BLOCK_SIZE + j]; }
		}
		if (i % 2 == 1) { cout << '\n'; }
		else { cout << "  "; }
	}
	cout << '\n';
}

void FileManager::display_bit_vector() {
	u_int index = 0;
	for (u_int i = 0; i < fileSystem.bitVector.size(); i++) {
		if (i % 8 == 0) { cout << setfill('0') << setw(2) << (index / 8) + 1 << ". "; }
		cout << fileSystem.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	cout << '\n';
}

void FileManager::display_block_char(const unsigned int& block) {
	int i = block * BLOCK_SIZE;
	if (block < fileSystem.bitVector.size()) {
		cout << setfill('0') << setw(2) << block << ". ";
		while (true) {
			if (disk.space[i] >= 0 && disk.space[i] <= 32) { cout << "."; }
			else { cout << disk.space[i]; }
			i++;
			if (i % BLOCK_SIZE == 0) { break; }
		}
	}
	cout << '\n';
}
