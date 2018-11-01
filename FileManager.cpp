/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 01/11/18
*/

#include "FileManager.h"
#include <algorithm>
#include <iomanip>
#include <iostream>

const std::string Serializer::IntToString(unsigned int input) {
	std::string result;

	while (input != 0)
	{
		if (input % 255 == 0)
		{
			result.push_back(char(255));
			input -= 255;
		}
		else
		{
			result.push_back(input % 255);
			input -= input % 255;
		}
	}

	return result;
}
const unsigned Serializer::StringToInt(const std::string& input){
	unsigned int result = 0;
	for (const char& c : input)
	{
		result += std::bitset<8>(c).to_ulong();
	}
	return result;
}

using u_int = unsigned int;

//Operator do wyœwietlania czasu z dat¹
std::ostream& operator << (std::ostream &os, const tm &time) {
	os << time.tm_hour << ':' << time.tm_min << ' ' << time.tm_mday << '.' << time.tm_mon << '.' << time.tm_year;
	return os;
}

//--------------------------- Dysk --------------------------

FileManager::Disk::Disk() {
	//Zape³nanie naszego dysku zerowymi bajtami (symbolizuje pusty dysk)
	fill(space.begin(), space.end(), NULL);
}



std::shared_ptr<FileManager::Index>& FileManager::IndexBlock::operator[](const size_t &index){
	return this->value[index];
}

const std::shared_ptr<FileManager::Index>& FileManager::IndexBlock::operator[](const size_t &index) const {
	return this->value[index];
}

void FileManager::Disk::write(const u_int &begin, const u_int &end, const std::string &data) {
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

void FileManager::Disk::write(const u_int &index, const u_int &data) {
	//Zapisz liczbê pod danym indeksem
	space[index] = data;
}

template<typename T>
const T FileManager::Disk::read(const u_int &begin, const u_int &end) {
	//Dane
	T data;

	//Jeœli typ danych to string
	if (typeid(T) == typeid(std::string)) {
		//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
		for (u_int index = begin; index <= end; index++) {
			//Dodaj znak zapisany na dysku do danych
			data += space[index];
		}
	}

	return data;
}

//----------------------- FileManager  ----------------------

FileManager::FileManager() {
	//Przypisanie katalogu g³ównego do obecnego katalogu 
	currentDirectory = &DISK.FileSystem.rootDirectory;
}

//-------------------- Podstawowe Metody --------------------

void FileManager::FileCreate(const std::string &name, const std::string &data) {
	//Rozmiar pliku obliczony na podstawie podanych danych
	const u_int fileSize = CalculateNeededBlocks(data)*BLOCK_SIZE;

	if (fileNumber < TOTAL_FILE_NUMBER_LIMIT) {
		//Jeœli plik siê zmieœci i nazwa nie u¿yta
		if (CheckIfEnoughSpace(fileSize) && data.size() <= MAX_FILE_SIZE && CheckIfNameUnused(*currentDirectory, name)) {
			//Jeœli œcie¿ka nie przekracza maksymalnej d³ugoœci
			if (name.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {
				fileNumber++;
				File file;
				file.blocksOccupied = fileSize/BLOCK_SIZE;
				file.sizeOnDisk = data.size();

				//Zapisywanie daty stworzenia pliku
				file.creationTime = GetCurrentTimeAndDate();
				file.modificationTime = file.creationTime;

				//Lista indeksów bloków, które zostan¹ zaalokowane na potrzeby pliku
				const std::vector<u_int> blocks = FindUnallocatedBlocks(file.blocksOccupied);

				//Wpisanie bloków do bezpoœredniego bloku indeksowego
				for (size_t i = 0; i < BLOCK_DIRECT_INDEX && i < blocks.size(); i++) {
					file.directBlocks[i] = std::make_shared<Index>(blocks[i]);
				}
				//Wpisanie bloków do 1-poziomowego bloku indeksowego
				if(blocks.size() > BLOCK_DIRECT_INDEX) {
					file.directBlocks[BLOCK_DIRECT_INDEX] = std::make_shared<IndexBlock>();
					for (size_t i = 0; i < BLOCK_INDEX_NUMBER && i < blocks.size() - BLOCK_DIRECT_INDEX; i++) {
						std::dynamic_pointer_cast<IndexBlock>(file.directBlocks[BLOCK_DIRECT_INDEX])->value[i] = std::make_shared<Index>(blocks[i + BLOCK_DIRECT_INDEX]);
					}
				}
				
				//Dodanie pliku do obecnego katalogu
				currentDirectory->files[name] = file;

				//Zapisanie danych pliku na dysku
				FileWrite(file, data);

				if (messages) { std::cout << "Stworzono plik o nazwie '" << name << "' w œcie¿ce '" << GetCurrentPath() << "'.\n"; }
				return;
			}
			else { std::cout << "Œcie¿ka za d³uga!\n"; }
		}
		//Jeœli plik siê nie mieœci
		if (!CheckIfEnoughSpace(fileSize)) {
			std::cout << "Za ma³o miejsca!\n";
		}
		//Jeœli plik jest wiêkszy ni¿ maksymalny dozwolony rozmiar
		if(data.size() > MAX_FILE_SIZE)
		{
			std::cout << "Rozmiar pliku wiêkszy ni¿ maksymalny dozwolony!\n";
		}
		//Jeœli nazwa u¿yta
		if (!CheckIfNameUnused(*currentDirectory, name)) {
			std::cout << "Nazwa pliku '" << name << "' ju¿ zajêta!\n";
		}
	}
	else {
		std::cout << "Osi¹gniêto limit elementów w œcie¿ce '" << GetCurrentPath() << "'!\n";
	}
}

//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!
const std::string FileManager::FileOpen(const std::string &name) {
	return DISK.read<std::string>(0 * 8, 4 * 8 - 1);
}
//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!

const std::string FileManager::FileGetData(const File &file) {
	//Dane
	std::string data;
	//Indeks do wczytywania danych z dysku
	size_t indexNumber = 0;
	std::shared_ptr<Index> index = file.directBlocks.value[indexNumber];

	//Dopóki nie natrafimy na koniec pliku
	while (index != nullptr) {
		std::cout << "index value: " << (index != nullptr ? std::to_string(index->value) : "nullptr") << '\n';

		//Dodaje do danych fragment pliku pod wskazanym indeksem
		data += DISK.read<std::string>(index->value * BLOCK_SIZE, (index->value + 1)*BLOCK_SIZE - 1);
		//Przypisuje indeksowi kolejny indeks w tablicy FileSystem
		indexNumber++;
		if(indexNumber < BLOCK_DIRECT_INDEX) {
			index = file.directBlocks[indexNumber];
		}
		else if (file.directBlocks[BLOCK_DIRECT_INDEX] != nullptr){
			index = (*std::dynamic_pointer_cast<IndexBlock>(file.directBlocks[BLOCK_DIRECT_INDEX]))[indexNumber - BLOCK_DIRECT_INDEX];
		}
		else { index = nullptr; }
	}
	return data;
}

void FileManager::FileDelete(const std::string &name) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);

	//Jeœli znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		FileDelete(fileIterator->second);
		//Usuñ wpis o pliku z obecnego katalogu
		currentDirectory->files.erase(fileIterator);

		if (messages) { std::cout << "Usuniêto plik o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n"; }
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::FileTruncate(const std::string &name, const u_int &size) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);
	//Jeœli znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		if (size <= (fileIterator->second.blocksOccupied - 1)*BLOCK_SIZE) {
			const u_int sizeToStart = u_int(ceil(double(size) / double(BLOCK_SIZE)))*BLOCK_SIZE;
			//Zmienna do analizowania, czy ju¿ mo¿na usuwaæ czêœæ pliku
			u_int currentSize = 0;

			unsigned int indexNumber = 0;
			std::shared_ptr<Index> index = fileIterator->second.directBlocks[indexNumber];

			//Dopóki indeks na coœ wskazuje
			while (index != nullptr) {
				currentSize += BLOCK_SIZE;
				if (indexNumber < BLOCK_DIRECT_INDEX) {
					index = fileIterator->second.directBlocks[indexNumber];
				}
				else if(fileIterator->second.directBlocks[BLOCK_DIRECT_INDEX] != nullptr) {
					index = (*std::dynamic_pointer_cast<IndexBlock>(fileIterator->second.directBlocks[BLOCK_DIRECT_INDEX]))[indexNumber - BLOCK_DIRECT_INDEX];
				}
				else { index = nullptr; }
				//Spisz kolejny indeks
				indexNumber++;

				//Jeœli obecny rozmiar przewy¿sza rozmiar potrzebny do rozpoczêcia usuwania
				//zacznij usuwaæ bloki
				if (currentSize > sizeToStart && index != nullptr) {
					//Zmniejszenie rozmiaru pliku
					fileIterator->second.blocksOccupied -= 1;
					//Po uciêciu rozmiar i rozmiar rzeczywisty bêd¹ takie same
					fileIterator->second.sizeOnDisk = fileIterator->second.blocksOccupied*BLOCK_SIZE;
					//Oznacz obecny indeks jako wolny
					DISK.FileSystem.bitVector[index->value] = BLOCK_FREE;
					//Obecny indeks w tablicy FileSystem wskazuje na nic
					if (indexNumber < BLOCK_DIRECT_INDEX) {
						fileIterator->second.directBlocks[BLOCK_DIRECT_INDEX] = nullptr;
					}
					else if (fileIterator->second.directBlocks[BLOCK_DIRECT_INDEX] != nullptr) {
						(*std::dynamic_pointer_cast<IndexBlock>(fileIterator->second.directBlocks[BLOCK_DIRECT_INDEX]))[indexNumber - BLOCK_DIRECT_INDEX] = nullptr;
					}
					
				}
			}
			if (messages) { std::cout << "Zmniejszono plik o nazwie '" << name << "' do rozmiaru " << fileIterator->second.blocksOccupied*BLOCK_SIZE << " Bajtów.\n"; }
		}
		else { std::cout << "Podano niepoprawny rozmiar!\n"; }
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DirectoryCreate(const std::string &name)
{
	if (fileNumber < TOTAL_FILE_NUMBER_LIMIT) {
		//Jeœli w katalogu nie istnieje podkatalog o podanej nazwie
		if (currentDirectory->subDirectories.find(name) == currentDirectory->subDirectories.end()) {
			//Jeœli œcie¿ka nie przekracza maksymalnej d³ugoœci
			if (name.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {
				fileNumber++;
				//Do podkatalogów obecnego katalogu dodaj nowy katalog o podanej nazwie
				currentDirectory->subDirectories[name] = Directory(name, &(*currentDirectory));
				//Zapisanie daty stworzenia katalogu
				currentDirectory->creationTime = GetCurrentTimeAndDate();
				if (messages) {
					std::cout << "Stworzono katalog o nazwie '" << currentDirectory->subDirectories[name].name
						<< "' w œcie¿ce '" << GetCurrentPath() << "'.\n";
				}
			}
			else { std::cout << "Œcie¿ka za d³uga!\n"; }
		}
		else { std::cout << "Nazwa katalogu '" << name << "' zajêta!\n"; }
	}
	else {
		std::cout << "Osi¹gniêto limit elementów w œcie¿ce '" << GetCurrentPath() << "'!\n";
	}
}

void FileManager::DirectoryDelete(const std::string &name) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za katalogiem o podanej nazwie
	auto directoryIterator = currentDirectory->subDirectories.find(name);

	//Jeœli znaleziono plik
	if (directoryIterator != currentDirectory->subDirectories.end()) {
		//Wywo³aj funkcjê usuwania katalogu wraz z jego zawartoœci¹
		DirectoryDeleteStructure(directoryIterator->second);
		//Usuñ wpis o katalogu z obecnego katalogu
		currentDirectory->subDirectories.erase(directoryIterator);
		if (messages) { std::cout << "Usuniêto katalog o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n"; }
	}
	else { std::cout << "Katalog o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DirectoryUp() {
	//Jeœli istnieje katalog nadrzêdny
	if (currentDirectory->parentDirectory != nullptr) {
		//Przejœcie do katalogu nadrzêdnego
		currentDirectory = currentDirectory->parentDirectory;
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Jesteœ w katalogu g³ównym!\n"; }
}

void FileManager::DirectoryDown(const std::string &name) {
	//Jeœli w obecnym katalogu znajduj¹ siê podkatalogi
	if (currentDirectory->subDirectories.find(name) != currentDirectory->subDirectories.end()) {
		//Przejœcie do katalogu o wskazanej nazwie
		currentDirectory = &(currentDirectory->subDirectories.find(name)->second);
		std::cout << "Obecna œcie¿ka to '" << GetCurrentPath() << "'.\n";
	}
	else { std::cout << "Brak katalogu o podanej nazwie!\n"; }
}

//--------------------- Dodatkowe metody --------------------

void FileManager::FileRename(const std::string &name, const std::string &changeName) const
{
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto file = currentDirectory->files.find(name);

	//Jeœli znaleziono plik
	if (file != currentDirectory->files.end()) {
		//Jeœli plik siê zmieœci i nazwa nie u¿yta
		if (CheckIfNameUnused(*currentDirectory, changeName)) {
			if (changeName.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {

				//Zapisywanie daty modyfikacji pliku
				file->second.modificationTime = GetCurrentTimeAndDate();

				//Lokowanie nowego klucza w tablicy hashowej i przypisanie do niego pliku
				currentDirectory->files[changeName] = file->second;
				//Usuniêcie starego klucza
				currentDirectory->files.erase(file);

				if (messages) { std::cout << "Zmieniono nazwê pliku '" << name << "' na '" << changeName << "'.\n"; }
				return;
			}
			else { std::cout << "Œcie¿ka za d³uga!\n"; }
		}
		else {
			std::cout << "Nazwa pliku '" << changeName << "' ju¿ zajêta!\n";
		}
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DirectoryRoot() {
	while (currentDirectory->parentDirectory != nullptr) {
		DirectoryUp();
	}
}

//------------------ Metody do wyœwietlania -----------------

void FileManager::Messages(const bool &onOff) {
	messages = onOff;
}

void FileManager::DisplayFileSystemParams() const {
	std::cout << "Disk capacity: " << DISK_CAPACITY << " Bytes\n";
	std::cout << "Block size: " << BLOCK_SIZE << " Bytes\n";
	std::cout << "Max file size: " << MAX_FILE_SIZE << " Bytes\n";
	std::cout << "Max indexes in block: " << BLOCK_INDEX_NUMBER << " Indexes\n";
	std::cout << "Max direct indexes in file: " << BLOCK_DIRECT_INDEX << " Indexes\n";
	std::cout << "Max file number: " << TOTAL_FILE_NUMBER_LIMIT << " Files\n";
	std::cout << "Max path length: " << MAX_PATH_LENGTH << " Characters\n";
	std::cout << "Number of files: " << fileNumber << " Files\n";
}

void FileManager::DisplayDirectoryInfo(const std::string &name) const
{
	const auto directoryIterator = currentDirectory->subDirectories.find(name);
	if (directoryIterator != currentDirectory->subDirectories.end()) {
		const Directory directory = directoryIterator->second;
		std::cout << "Name: " << directory.name << '\n';
		std::cout << "Size: " << CalculateDirectorySize(directory) << " Bytes\n";
		std::cout << "Size on disk: " << CalculateDirectorySize(directory) << " Bytes\n";
		std::cout << "Contains: " << CalculateDirectoryFileNumber(directory) << " Files, " << CalculateDirectoryFolderNumber(directory) << " Folders\n";
		std::cout << "Created: " << directory.creationTime << '\n';
	}
	else { std::cout << "Katalog o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DisplayFileInfo(const std::string &name) {
	const auto fileIterator = currentDirectory->files.find(name);
	if (fileIterator != currentDirectory->files.end()) {
		const File file = fileIterator->second;
		std::cout << "Name: " << name << '\n';
		std::cout << "Size: " << file.blocksOccupied*BLOCK_SIZE << " Bytes\n";
		std::cout << "Size on disk: " << file.sizeOnDisk << " Bytes\n";
		std::cout << "Created: " << file.creationTime << '\n';
		std::cout << "Modified: " << file.modificationTime << '\n';
		//std::cout << "Block indexes: " << file.FileSystemIndex << '\n';
		std::cout << "Saved data: " << FileGetData(file) << '\n';
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DisplayDirectoryStructure() const
{
	DisplayDirectory(DISK.FileSystem.rootDirectory, 1);
}
void FileManager::DisplayDirectory(const Directory &directory, u_int level)
{
	std::cout << std::string(level, ' ') << directory.name << "\\\n";
	for (auto i = directory.files.begin(); i != directory.files.end(); ++i) {
		std::cout << std::string(level + 1, ' ') << "- " << i->first << '\n';
	}
	level++;
	for (auto i = directory.subDirectories.begin(); i != directory.subDirectories.end(); ++i) {
		DisplayDirectory(i->second, level);
	}
}

void FileManager::DisplayDiskContentBinary() {
	u_int index = 0;
	for (const char &c : DISK.space) {
		//bitset - tablica bitowa
		std::cout << std::bitset<8>(c) << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 16 == 15 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayDiskContentChar() {
	u_int index = 0;
	for (const char &c : DISK.space) {
		if (c == ' ') { std::cout << ' '; }
		else if (c >= 0 && c <= 32) std::cout << ".";
		else std::cout << c;
		std::cout << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 32 == 31 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

//void FileManager::DisplayFileAllocationTable() {
//	u_int index = 0;
//	for (u_int i = 0; i < DISK.FileSystem.FileAllocationTable.size(); i++) {
//		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
//		std::cout << std::setfill('0') << std::setw(3) << (DISK.FileSystem.FileAllocationTable[i] != u_int(-1) ? std::to_string(DISK.FileSystem.FileAllocationTable[i]) : "NUL")
//			<< (index % 8 == 7 ? "\n" : " ");
//		index++;
//	}
//	std::cout << '\n';
//}

void FileManager::DisplayBitVector() {
	u_int index = 0;
	for (u_int i = 0; i < DISK.FileSystem.bitVector.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << DISK.FileSystem.bitVector[i] << (index % 8 == 7 ? "\n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayFileFragments(const std::vector<std::string> &fileFragments)
{
	for (const auto& fileFragment : fileFragments)
	{
		std::cout << fileFragment << std::string(BLOCK_SIZE - 1 - fileFragment.size(), ' ') << '\n';
	}
}

//-------------------- Metody Pomocnicze --------------------

void FileManager::DirectoryDeleteStructure(Directory &directory) {
	fileNumber--;
	//Usuwa wszystkie pliki z katalogu
	for (auto i = directory.files.begin(); i != directory.files.end(); ++i) {
		FileDelete(i->second);
		if (messages) { std::cout << "Usuniêto plik o nazwie '" << i->first << "' znajduj¹cy siê w œcie¿ce '" + GetPath(directory) + "'.\n"; }
	}
	//Czyœci listê plików w katalogu
	directory.files.clear();
	//Usuwa wszystkie katalogi w katalogu
	for (auto i = directory.subDirectories.begin(); i != directory.subDirectories.end(); ++i) {
		//Wywo³anie funkcji na podrzêdnym katalogu
		DirectoryDeleteStructure(i->second);
		if (messages) { std::cout << "Usuniêto katalog o nazwie '" << i->second.name << "' znajduj¹cy siê w œcie¿ce '" + GetPath(directory) + "'.\n"; }
	}
	//Czyœci listê katalogów w katalogu
	directory.subDirectories.clear();
}

void FileManager::FileDelete(File &file) {
	fileNumber--;
	//Obecny indeks
	u_int indexNumber = 0;
	std::shared_ptr<Index> index = file.directBlocks[indexNumber];
	//Dopóki indeks na coœ wskazuje
	while (index != nullptr) {
		//Oznacz obecny indeks jako wolny
		DISK.FileSystem.bitVector[index->value] = BLOCK_FREE;
		//Obecny indeks w tablicy FileSystem wskazuje na nic
		file.directBlocks[indexNumber] = nullptr;
		//Przypisz do obecnego indeksu kolejny indeks
		indexNumber++;
		if (indexNumber < BLOCK_DIRECT_INDEX) {
			index = file.directBlocks[indexNumber];
		}
		else if(file.directBlocks[BLOCK_DIRECT_INDEX] != nullptr){
			index = (*std::dynamic_pointer_cast<IndexBlock>(file.directBlocks[BLOCK_DIRECT_INDEX]))[indexNumber - BLOCK_DIRECT_INDEX];
		}
		else { index = nullptr; }
	}
}

const size_t FileManager::CalculateDirectorySize(const Directory &directory)
{
	//Rozmiar katalogu
	size_t size = 0;

	//Dodaje rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto &file : directory.files) {
		size += file.second.blocksOccupied*BLOCK_SIZE;
	}
	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
	for (const auto &dir : directory.subDirectories) {
		size += CalculateDirectorySize(dir.second);
	}

	return size;
}

const size_t FileManager::CalculateDirectorySizeOnDisk(const Directory &directory)
{
	//Rzeczywisty rozmiar katalogu
	size_t sizeOnDisk = 9;

	//Dodaje rzeczywisty rozmiar plików w katalogu do rozmiaru katalogu
	for (const std::pair<const std::string, File> &file : directory.files) {
		sizeOnDisk += file.second.sizeOnDisk;
	}
	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
	for (const std::pair<const std::string, Directory> &dir : directory.subDirectories) {
		sizeOnDisk += CalculateDirectorySize(dir.second);
	}

	return sizeOnDisk;
}

const u_int FileManager::CalculateDirectoryFolderNumber(const Directory &directory)
{
	//Iloœæ folderów w danym katalogu
	u_int folderNumber = 0;

	//Dodaje iloœæ folderów w tym folderze do zwracanej zmiennej
	folderNumber += directory.subDirectories.size();

	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
	for (const std::pair<const std::string, Directory> &dir : directory.subDirectories) {
		folderNumber += CalculateDirectoryFolderNumber(dir.second);
	}
	return folderNumber;
}

const u_int FileManager::CalculateDirectoryFileNumber(const Directory &directory)
{
	//Iloœæ plików w danym katalogu
	u_int filesNumber = 0;

	//Dodaje iloœæ plików w tym folderze do zwracanej zmiennej
	filesNumber += directory.files.size();

	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
	for (const std::pair<const std::string, Directory> &dir : directory.subDirectories) {
		filesNumber += CalculateDirectoryFolderNumber(dir.second);
	}
	return filesNumber;
}

const std::string FileManager::GetCurrentPath() const
{
	//Œcie¿ka
	std::string path;
	//Tymczasowa zmienna przechowuj¹ca wskaŸnik na katalog
	const Directory* tempDir = currentDirectory;
	//Dopóki nie doszliœmy do pustego katalogu
	while (tempDir != nullptr) {
		//Dodaj do œcie¿ki od przodu nazwê obecnego katalogu
		path.insert(0, "/" + tempDir->name);
		//Przypisanie tymczasowej zmiennej katalog wy¿szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return path;
}

const std::string FileManager::GetPath(const Directory &directory)
{
	std::string path;
	//Tymczasowa zmienna przechowuj¹ca wskaŸnik na katalog
	const Directory* tempDir = &directory;
	//Dopóki nie doszliœmy do pustego katalogu
	while (tempDir != nullptr) {
		//Dodaj do œcie¿ki od przodu nazwê obecnego katalogu
		path.insert(0, "/" + tempDir->name);
		//Przypisanie tymczasowej zmiennej katalog wy¿szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return path;
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

const size_t FileManager::GetCurrentPathLength() const
{
	//Œcie¿ka
	size_t length = 0;
	//Tymczasowa zmienna przechowuj¹ca wskaŸnik na katalog
	Directory* tempDir = currentDirectory;
	//Dopóki nie doszliœmy do pustego katalogu
	while (tempDir != nullptr) {
		//Dodaj do œcie¿ki od przodu nazwê obecnego katalogu
		length += tempDir->name.size();
		//Przypisanie tymczasowej zmiennej katalog wy¿szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return length;
}

const bool FileManager::CheckIfNameUnused(const Directory &directory, const std::string &name)
{
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	for (auto i = directory.files.begin(); i != directory.files.end(); ++i) {
		//Jeœli nazwa ta sama
		if (i->first == name) { return false; }
	}
	return true;
}

const bool FileManager::CheckIfEnoughSpace(const u_int &dataSize) const
{
	//Jeœli dane siê mieszcz¹
	if (dataSize <= DISK.FileSystem.freeSpace) { return true; }
	//Jeœli dane siê nie mieszcz¹
	return false;
}

void FileManager::ChangeBitVectorValue(const u_int &block, const bool &value) {
	//Jeœli wartoœæ zajêty to wolne miejsce - BLOCK_SIZE
	if (value == 1) { DISK.FileSystem.freeSpace -= BLOCK_SIZE; }
	//Jeœli wartoœæ wolny to wolne miejsce + BLOCK_SIZE
	else if (value == 0) { DISK.FileSystem.freeSpace += BLOCK_SIZE; }
	//Przypisanie blokowi podanej wartoœci
	DISK.FileSystem.bitVector[block] = value;
}

void FileManager::FileWrite(const File &file, const std::string &data) {
	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>fileFragments = DataToDataFragments(data);
	//Index pod którym maj¹ zapisywane byæ dane
	u_int indexNumber = 0;
	std::shared_ptr<Index> index = file.directBlocks[indexNumber];

	

	//Zapisuje wszystkie dane na dysku
	for (const auto& fileFragment : fileFragments)
	{
		//Zapisuje fragment na dysku
		DISK.write(index->value * BLOCK_SIZE, index->value * BLOCK_SIZE + fileFragment.size() - 1, fileFragment);
		//Zmienia wartoœæ bloku w wektorze bitowym na zajêty
		ChangeBitVectorValue(index->value, BLOCK_OCCUPIED);
		//Przypisuje do indeksu numer kolejnego bloku
		indexNumber++;
		if (indexNumber < BLOCK_DIRECT_INDEX) {
			index = file.directBlocks[indexNumber];
		}
		else if (file.directBlocks[BLOCK_DIRECT_INDEX] != nullptr){
			index = (*std::dynamic_pointer_cast<IndexBlock>(file.directBlocks[BLOCK_DIRECT_INDEX]))[indexNumber - BLOCK_DIRECT_INDEX];
		}
	}
}

const std::vector<std::string> FileManager::DataToDataFragments(const std::string &data) const
{
	//Tablica fragmentów podanych danych
	std::vector<std::string>fileFragments;

	//Przetworzenie ca³ych danych
	for (u_int i = 0; i < CalculateNeededBlocks(data); i++) {
		//Oblicza pocz¹tek kolejnej czêœci fragmentu danych.
		const u_int substrBegin = i * BLOCK_SIZE;
		//Dodaje do tablicy fragmentów kolejny fragment o d³ugoœci BLOCK_SIZE
		fileFragments.push_back(data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

const u_int FileManager::CalculateNeededBlocks(const std::string &data) const
{
	/*
	Przybli¿enie w górê rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa¿, jeœli zape³nia chocia¿ o jeden bajt
	wiêcej przy zajêtym bloku, to trzeba zaalokowaæ wtedy kolejny blok.
	*/
	return int(ceil(double(data.size()) / double(BLOCK_SIZE)));
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
	blockList.push_back(-1);
	return blockList;
}

const std::vector<u_int> FileManager::FindUnallocatedBlocksBestFit(const u_int &blockNumber) {
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

const std::vector<u_int> FileManager::FindUnallocatedBlocks(const u_int &blockNumber) {
	//Szuka bloków funkcj¹ z metod¹ best-fit
	std::vector<u_int> blockList = FindUnallocatedBlocksBestFit(blockNumber);

	//Jeœli funkcja z metod¹ best-fit nie znajdzie dopasowañ
	if (blockList.empty()) {
		//Szuka niezaalokowanych bloków, wybieraj¹c pierwsze wolne
		blockList = FindUnallocatedBlocksFragmented(blockNumber);
	}

	return blockList;
}
