/**
	SexyOS
	FileManager.cpp
	Przeznaczenie: Zawiera definicje metod i konstruktorów dla klas z FileManager.h

	@author Tomasz Kiljañczyk
	@version 24/10/18
*/

#include "FileManager.h"
#include <algorithm>
#include <iomanip>

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

FileManager::Disk::FAT::FAT() {
	std::fill(FileAllocationTable.begin(), FileAllocationTable.end(), -1);
}

void FileManager::Disk::write(const unsigned int &begin, const unsigned int &end, const std::string &data) {
	//Indeks który bêdzie s³u¿y³ do wskazywania na komórki pamiêci
	unsigned int index = begin;
	//Iterowanie po danych typu string i zapisywanie znaków na dysku
	for (unsigned int i = 0; i < data.size() && i <= end - begin; i++) {
		space[index] = data[i];
		index++;
	}
	//Zapisywanie NULL, jeœli dane nie wype³ni³y ostatniego bloku
	for (; index <= end; index++) {
		space[index] = NULL;
	}
}

void FileManager::Disk::write(const unsigned int &index, const unsigned int &data) {
	//Zapisz liczbê pod danym indeksem
	space[index] = data;
}

template<typename T>
const T FileManager::Disk::read(const unsigned int &begin, const unsigned int &end) {
	//Dane
	T data;

	//Jeœli typ danych to string
	if (typeid(T) == typeid(std::string)) {
		//Odczytaj przestrzeñ dyskow¹ od indeksu begin do indeksu end
		for (unsigned int index = begin; index <= end; index++) {
			//Dodaj znak zapisany na dysku do danych
			data += space[index];
		}
	}

	return data;
}

//----------------------- FileManager  ----------------------

FileManager::FileManager() {
	//Przypisanie katalogu g³ównego do obecnego katalogu 
	currentDirectory = &DISK.FAT.rootDirectory;
}

//-------------------- Podstawowe Metody --------------------

void FileManager::FileCreate(const std::string &name, const std::string &data) {
	//Rozmiar pliku obliczony na podstawie podanych danych
	const unsigned int fileSize = CalculateNeededBlocks(data)*BLOCK_SIZE;

	if (currentDirectory->files.size() + currentDirectory->subDirectories.size() < MAX_DIRECTORY_ELEMENTS) {
		//Jeœli plik siê zmieœci i nazwa nie u¿yta
		if (CheckIfEnoughSpace(fileSize) && CheckIfNameUnused(*currentDirectory, name)) {
			//Jeœli œcie¿ka nie przekracza maksymalnej d³ugoœci
			if (name.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {
				//Stwórz plik o podanej nazwie
				File file = File(name);
				//Zapisz w pliku jego rozmiar
				file.size = fileSize;
				//Zapisz w plik jego rzeczywisty rozmiar
				file.sizeOnDisk = data.size();

				//Zapisywanie daty stworzenia pliku
				file.creationTime = GetCurrentTimeAndDate();
				file.modificationTime = file.creationTime;

				//Lista indeksów bloków, które zostan¹ zaalokowane na potrzeby pliku
				const std::vector<unsigned int> blocks = FindUnallocatedBlocks(file.size / BLOCK_SIZE);

				//Wpisanie bloków do tablicy FAT
				for (unsigned int i = 0; i < blocks.size() - 1; i++) {
					DISK.FAT.FileAllocationTable[blocks[i]] = blocks[i + 1];
				}

				//Dodanie do pliku indeksu pierwszego bloku na którym jest zapisany
				file.FATindex = blocks[0];

				//Dodanie pliku do obecnego katalogu
				currentDirectory->files[file.name] = file;

				//Zapisanie danych pliku na dysku
				WriteFile(file, data);

				if (messages) { std::cout << "Stworzono plik o nazwie '" << file.name << "' w œcie¿ce '" << GetCurrentPath() << "'.\n"; }
				return;
			}
			else { std::cout << "Œcie¿ka za d³uga!\n"; }
		}
		//Jeœli plik siê nie mieœci
		if (!CheckIfEnoughSpace(fileSize)) {
			std::cout << "Za ma³o miejsca!\n";
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
	unsigned int index = file.FATindex;
	//Dopóki nie natrafimy na koniec pliku
	while (index != -1) {
		//Dodaje do danych fragment pliku pod wskazanym indeksem
		data += DISK.read<std::string>(index*BLOCK_SIZE, (index + 1)*BLOCK_SIZE - 1);
		//Przypisuje indeksowi kolejny indeks w tablicy FAT
		index = DISK.FAT.FileAllocationTable[index];
	}
	return data;
}

void FileManager::FileDelete(const std::string &name) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);

	//Jeœli znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		//Zmienna do tymczasowego przechowywania kolejnego indeksu
		unsigned int tempIndex;
		//Obecny indeks
		unsigned index = fileIterator->second.FATindex;
		//Dopóki indeks na coœ wskazuje
		while (index != -1) {
			//Spisz kolejny indeks
			tempIndex = DISK.FAT.FileAllocationTable[index];
			//Oznacz obecny indeks jako wolny
			DISK.FAT.bitVector[index] = 0;
			//Obecny indeks w tablicy FAT wskazuje na nic
			DISK.FAT.FileAllocationTable[index] = -1;
			//Przypisz do obecnego indeksu kolejny indeks
			index = tempIndex;
		}
		//Usuñ plik z obecnego katalogu
		currentDirectory->files.erase(fileIterator);

		if (messages) { std::cout << "Usuniêto plik o nazwie '" << name << "' znajduj¹cy siê w œcie¿ce '" + GetCurrentPath() + "'.\n"; }
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::FileTruncate(const std::string &name, const unsigned int &size) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto fileIterator = currentDirectory->files.find(name);
	//Jeœli znaleziono plik
	if (fileIterator != currentDirectory->files.end()) {
		if (size <= fileIterator->second.size - BLOCK_SIZE) {
			unsigned int sizeToStart = (unsigned int)ceil((double)size / (double)BLOCK_SIZE)*BLOCK_SIZE;
			//Zmienna do tymczasowego przechowywania kolejnego indeksu
			unsigned int tempIndex;
			//Zmienna do analizowania, ju¿ mo¿na usuwaæ czêœæ pliku
			unsigned int currentSize = 0;
			//Obecny indeks
			unsigned index = fileIterator->second.FATindex;
			//Dopóki indeks na coœ wskazuje
			while (index != -1) {
				//Zwiêksz obecny rozmiar o rozmiar jednostki alokacji
				currentSize += BLOCK_SIZE;
				//Spisz kolejny indeks
				tempIndex = DISK.FAT.FileAllocationTable[index];

				//Jeœli obecny rozmiar przewy¿sza rozmiar potrzebny do rozpoczêcia usuwania
				//zacznij usuwaæ bloki
				if (currentSize > sizeToStart) {
					//Zmniejszenie rozmiaru pliku
					fileIterator->second.size -= BLOCK_SIZE;
					//Po uciêciu rozmiar i rozmiar rzeczywisty bêd¹ takie same
					fileIterator->second.sizeOnDisk = fileIterator->second.size;
					//Oznacz obecny indeks jako wolny
					DISK.FAT.bitVector[index] = 0;
					//Obecny indeks w tablicy FAT wskazuje na nic
					DISK.FAT.FileAllocationTable[index] = -1;
				}
				//Przypisz do obecnego indeksu kolejny indeks
				index = tempIndex;
			}
			if (messages) { std::cout << "Zmniejszono plik o nazwie '" << name << "' do rozmiaru " << fileIterator->second.size << " Bajtów.\n"; }
		}
		else { std::cout << "Podano niepoprawny rozmiar!\n"; }
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DirectoryCreate(const std::string &name) {
	if (currentDirectory->files.size() + currentDirectory->subDirectories.size() < MAX_DIRECTORY_ELEMENTS) {
		//Jeœli w katalogu nie istnieje podkatalog o podanej nazwie
		if (currentDirectory->subDirectories.find(name) == currentDirectory->subDirectories.end()) {
			//Jeœli œcie¿ka nie przekracza maksymalnej d³ugoœci
			if (name.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {
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

void FileManager::DirectoryUp() {
	//Jeœli istnieje katalog nadrzêdny
	if (currentDirectory->parentDirectory != NULL) {
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

void FileManager::FileRename(const std::string &name, const std::string &changeName) {
	//Iterator zwracany podczas przeszukiwania obecnego katalogu za plikiem o podanej nazwie
	auto file = currentDirectory->files.find(name);

	//Jeœli znaleziono plik
	if (file != currentDirectory->files.end()) {
		//Jeœli plik siê zmieœci i nazwa nie u¿yta
		if (CheckIfNameUnused(*currentDirectory, changeName)) {
			if (changeName.size() + GetCurrentPathLength() < MAX_PATH_LENGTH) {

				//Zapisywanie daty modyfikacji pliku
				file->second.modificationTime = GetCurrentTimeAndDate();

				//Zmiana nazwy pliku
				file->second.name = changeName;

				//Lokowanie nowego klucza w tablicy hashowej i przypisanie do niego pliku
				currentDirectory->files[changeName] = file->second;
				//Usuniêcie starego klucza
				currentDirectory->files.erase(file);

				if (messages) { std::cout << "Zmieniono nazwê pliku '" << name << "' na '" << currentDirectory->files[changeName].name << "'.\n"; }
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
	while (currentDirectory->parentDirectory != NULL) {
		DirectoryUp();
	}
}

//------------------ Metody do wyœwietlania -----------------

void FileManager::Messages(const bool &onOff) {
	messages = onOff;
}

void FileManager::DisplayDirectoryInfo(const std::string &name) {
	auto directoryIterator = currentDirectory->subDirectories.find(name);
	if (directoryIterator != currentDirectory->subDirectories.end()) {
		const Directory directory = directoryIterator->second;
		std::cout << "Name: " << directory.name << '\n';
		std::cout << "Size: " << CalculateDirectorySize(directory) << " Bytes\n";
		std::cout << "Size on disk: " << CalculateDirectorySize(directory) << " Bytes\n";
		std::cout << "Contains: " << CalculateDirectoryFileCount(directory) << " Files, " << CalculateDirectoryFolderCount(directory) << " Folders\n";
		std::cout << "Created: " << directory.creationTime << '\n';
	}
	else { std::cout << "Katalog o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

void FileManager::DisplayFileInfo(const std::string &name) {
	auto fileIterator = currentDirectory->files.find(name);
	if (fileIterator != currentDirectory->files.end()) {
		const File file = fileIterator->second;
		std::cout << "Name: " << file.name << '\n';
		std::cout << "Size: " << file.size << " Bytes\n";
		std::cout << "Size on disk: " << file.sizeOnDisk << " Bytes\n";
		std::cout << "Created: " << file.creationTime << '\n';
		std::cout << "Modified: " << file.modificationTime << '\n';
		std::cout << "FAT index: " << file.FATindex << '\n';
		std::cout << "Saved data: " << FileGetData(file) << '\n';
	}
	else { std::cout << "Plik o nazwie '" << name << "' nie znaleziony w œcie¿ce '" + GetCurrentPath() + "'!\n"; }
}

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
		if (c == ' ') { std::cout << ' '; }
		else if (c >= 0 && c <= 32) std::cout << ".";
		else std::cout << c;
		std::cout << (index % BLOCK_SIZE == BLOCK_SIZE - 1 ? " , " : "") << (index % 32 == 31 ? " \n" : " ");
		index++;
	}
	std::cout << '\n';
}

void FileManager::DisplayFileAllocationTable() {
	unsigned int index = 0;
	for (unsigned int i = 0; i < DISK.FAT.FileAllocationTable.size(); i++) {
		if (i % 8 == 0) { std::cout << std::setfill('0') << std::setw(2) << (index / 8) + 1 << ". "; }
		std::cout << std::setfill('0') << std::setw(3) << (DISK.FAT.FileAllocationTable[i] != -1 ? std::to_string(DISK.FAT.FileAllocationTable[i]) : "NUL")
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

const size_t FileManager::CalculateDirectorySize(const Directory &directory) {
	//Rozmiar katalogu
	size_t size = 0;

	//Dodaje rozmiar plików w katalogu do rozmiaru katalogu
	for (const auto &file : directory.files) {
		size += file.second.size;
	}
	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
	for (const auto &dir : directory.subDirectories) {
		size += CalculateDirectorySize(dir.second);
	}

	return size;
}

const size_t FileManager::CalculateDirectorySizeOnDisk(const Directory &directory) {
	//Rzeczywisty rozmiar katalogu
	size_t sizeOnDisk = 9;

	//Dodaje rzeczywisty rozmiar plików w katalogu do rozmiaru katalogu
	for (const std::pair<std::string, File> &file : directory.files) {
		sizeOnDisk += file.second.sizeOnDisk;
	}
	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do rozmiaru
	for (const std::pair <std::string, Directory> &dir : directory.subDirectories) {
		sizeOnDisk += CalculateDirectorySize(dir.second);
	}

	return sizeOnDisk;
}

const unsigned int FileManager::CalculateDirectoryFolderCount(const Directory &directory) {
	//Iloœæ folderów w danym katalogu
	unsigned int folderCount = 0;

	//Dodaje iloœæ folderów w tym folderze do zwracanej zmiennej
	folderCount += directory.subDirectories.size();

	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
	for (const std::pair <std::string, Directory> &dir : directory.subDirectories) {
		folderCount += CalculateDirectoryFolderCount(dir.second);
	}
	return folderCount;
}

const unsigned int FileManager::CalculateDirectoryFileCount(const Directory &directory) {
	//Iloœæ plików w danym katalogu
	unsigned int filesCount = 0;

	//Dodaje iloœæ plików w tym folderze do zwracanej zmiennej
	filesCount += directory.files.size();

	//Przegl¹da katalogi i wywo³uje na nich obecn¹ funkcjê i dodaje zwrócon¹ wartoœæ do iloœci
	for (const std::pair <std::string, Directory> &dir : directory.subDirectories) {
		filesCount += CalculateDirectoryFolderCount(dir.second);
	}
	return filesCount;
}

const std::string FileManager::GetCurrentPath() {
	//Œcie¿ka
	std::string path;
	//Tymczasowa zmienna przechowuj¹ca wskaŸnik na katalog
	Directory* tempDir = currentDirectory;
	//Dopóki nie doszliœmy do pustego katalogu
	while (tempDir != NULL) {
		//Dodaj do œcie¿ki od przodu nazwê obecnego katalogu
		path.insert(0, "/" + tempDir->name);
		//Przypisanie tymczasowej zmiennej katalog wy¿szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return path;
}

const tm FileManager::GetCurrentTimeAndDate() {
	time_t tt;
	time(&tt);
	tm timeAndDate = *localtime(&tt);
	timeAndDate.tm_year += 1900;
	timeAndDate.tm_mon += 1;
	return timeAndDate;
}

const size_t FileManager::GetCurrentPathLength() {
	//Œcie¿ka
	size_t length = 0;
	//Tymczasowa zmienna przechowuj¹ca wskaŸnik na katalog
	Directory* tempDir = currentDirectory;
	//Dopóki nie doszliœmy do pustego katalogu
	while (tempDir != NULL) {
		//Dodaj do œcie¿ki od przodu nazwê obecnego katalogu
		length += tempDir->name.size();
		//Przypisanie tymczasowej zmiennej katalog wy¿szy w hierarchii
		tempDir = tempDir->parentDirectory;
	}
	return length;
}

const bool FileManager::CheckIfNameUnused(const Directory &directory, const std::string &name) {
	//Przeszukuje podany katalog za plikiem o tej samej nazwie
	for (auto i = directory.files.begin(); i != directory.files.end(); i++) {
		//Jeœli nazwa ta sama
		if (i->first == name) { return false; }
	}
	return true;
}

const bool FileManager::CheckIfEnoughSpace(const unsigned int &dataSize) {
	//Jeœli dane siê mieszcz¹
	if (dataSize <= DISK.FAT.freeSpace) { return true; }
	//Jeœli dane siê nie mieszcz¹
	else { return false; }
}

void FileManager::ChangeBitVectorValue(const unsigned int &block, const bool &value) {
	//Jeœli wartoœæ zajêty to wolne miejsce - BLOCK_SIZE
	if (value == 1) { DISK.FAT.freeSpace -= BLOCK_SIZE; }
	//Jeœli wartoœæ wolny to wolne miejsce + BLOCK_SIZE
	else if (value == 0) { DISK.FAT.freeSpace += BLOCK_SIZE; }
	//Przypisanie blokowi podanej wartoœci
	DISK.FAT.bitVector[block] = value;
}

void FileManager::WriteFile(const File &file, const std::string &data) {
	//Uzyskuje dane podzielone na fragmenty
	const std::vector<std::string>fileFragments = DataToDataFragments(data);
	//Index pod którym maj¹ zapisywane byæ dane
	unsigned int index = file.FATindex;

	//Zapisuje wszystkie dane na dysku
	for (unsigned int i = 0; i < fileFragments.size(); i++) {
		//Zapisuje fragment na dysku
		DISK.write(index * BLOCK_SIZE, index * BLOCK_SIZE + fileFragments[i].size() - 1, fileFragments[i]);
		//Zmienia wartoœæ bloku w wektorze bitowym na zajêty
		ChangeBitVectorValue(index, 1);
		//Przypisuje do indeksu numer kolejnego bloku
		index = DISK.FAT.FileAllocationTable[index];
	}
}

const std::vector<std::string> FileManager::DataToDataFragments(const std::string &data) {
	//Tablica fragmentów podanych danych
	std::vector<std::string>fileFragments;
	//Pocz¹tek czêœci danych, u¿ywany podczas dzielenia danych
	unsigned int substrBegin = 0;

	//Przetworzenie ca³ych danych
	for (unsigned int i = 0; i < CalculateNeededBlocks(data); i++) {
		//Oblicza pocz¹tek kolejnej czêœci fragmentu danych.
		substrBegin = i * BLOCK_SIZE;
		//Dodaje do tablicy fragmentów kolejny fragment o d³ugoœci BLOCK_SIZE
		fileFragments.push_back(data.substr(substrBegin, BLOCK_SIZE));
	}
	return fileFragments;
}

const unsigned int FileManager::CalculateNeededBlocks(const std::string &data) {
	/*
	Przybli¿enie w górê rozmiaru pliku przez rozmiar bloku.
	Jest tak, poniewa¿, jeœli zape³nia chocia¿ o jeden bajt
	wiêcej przy zajêtym bloku, to trzeba zaalokowaæ wtedy kolejny blok.
	*/
	return (int)ceil((double)data.size() / (double)BLOCK_SIZE);
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocksFragmented(unsigned int blockCount) {
	//Lista wolnych bloków
	std::vector<unsigned int> blockList;

	//Szuka wolnych bloków
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		//Jeœli blok wolny
		if (DISK.FAT.bitVector[i] == 0) {
			//Dodaje indeks bloku
			blockList.push_back(i);
			//Potrzeba teraz jeden blok mniej
			blockCount--;
			//Jeœli potrzeba 0 bloków, przerwij
			if (blockCount == 0) { break; }
		}
	}
	blockList.push_back(-1);
	return blockList;
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocksBestFit(const unsigned int &blockCount) {
	//Lista indeksów bloków (dopasowanie)
	std::vector<unsigned int> blockList;
	//Najlepsze dopasowanie
	std::vector<unsigned int> bestBlockList(DISK.FAT.bitVector.size() + 1);
	//Lista dopasowañ
	std::vector<std::vector<unsigned int>> blockLists;

	//Szukanie wolnych bloków spe³niaj¹cych minimum miejsca
	for (unsigned int i = 0; i < DISK.FAT.bitVector.size(); i++) {
		//Jeœli blok wolny
		if (DISK.FAT.bitVector[i] == 0) {
			//Dodaj indeks bloku do listy bloków
			blockList.push_back(i);
		}
		//Jeœli blok zajêty
		else {
			//Jeœli uzyskana lista bloków jest wiêksza od iloœci bloków jak¹ chcemy uzyskaæ
			//to dodaj uzyskane dopasowanie do listy dopasowañ;
			if (blockList.size() >= blockCount) { blockLists.push_back(blockList); }

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
	if (blockList.size() >= blockCount) { blockLists.push_back(blockList); }
	blockList.clear();

	//Jeœli znaleziono dopasowania (rozmiar > 0)
	if (blockLists.size() > 0) {
		//Szuka najlepszego dopasowania z znalezionych dopasowañ
		for (const std::vector<unsigned int> &v : blockLists) {
			//Jeœli obecne dopasowanie jest lepsze od poprzedniego
			if (v.size() < bestBlockList.size()) {
				bestBlockList = v;
			}
		}

		//Ucina najlepsze dopasowanie do iloœci bloków jakie chcemy zaalokowaæ
		bestBlockList.resize(blockCount);
	}

	return bestBlockList;
}

const std::vector<unsigned int> FileManager::FindUnallocatedBlocks(const unsigned int &blockCount) {
	//Szuka bloków funkcj¹ z metod¹ best-fit
	std::vector<unsigned int> blockList = FindUnallocatedBlocksBestFit(blockCount);

	//Jeœli funkcja z metod¹ best-fit nie znajdzie dopasowañ
	if (blockList.size() == 0) {
		//Szuka niezaalokowanych bloków, wybieraj¹c pierwsze wolne
		blockList = FindUnallocatedBlocksFragmented(blockCount);
	}

	//Dodaje -1, poniewa¿ przy zapisie w tablicy FAT ostatnia pozycja wskazuje na nic (czyli -1)
	blockList.push_back(-1);
	return blockList;
}
