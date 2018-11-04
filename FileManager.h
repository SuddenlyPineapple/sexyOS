/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 02/11/18
*/

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#include <ctime>
#include <string>
#include <array>
#include <bitset>
#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>

/*
	To-do:
	- otwieranie i zamykanie pliku (zale¿y czy mój zakres zadania)
	- plik flagi
	- defragmentator (bardzo opcjonalne)
	- zapisywanie plików z kodem asemblerowym
	- pliki executable
*/

//Klasa serializera (konwertowanie danych)
class Serializer {
public:
	/**
		Zamienia int na string, gdzie liczba przekonwertowywana
		jest na znaki, aby zajmowa³a mniej miejsca na dysku.

		@param input Liczba jak¹ chcemy zamieniæ na string.
		@return Liczba ca³kowita przekonwertowana na string.
	*/
	static const std::string IntToString(unsigned int input);

	/**
		Zamienia string na int.

		@param input Tekst jaki chcemy zamieniæ na liczbê.
		@return Liczba powsta³a po zsumowaniu znaków.
	*/
	static const unsigned int StringToInt(const std::string& input);
};

//Klasa zarz¹dcy przestrzeni¹ dyskow¹ i systemem plików
class FileManager {
private:
	using u_int = unsigned int;

	//--------------- Definicje sta³ych statycznych -------------
	static const size_t BLOCK_SIZE = 32;	   	    //Rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024;       //Pojemnoœæ dysku (bajty)
	static const u_int BLOCK_INDEX_NUMBER = 3;	    //Wartoœæ oznaczaj¹ca d³ugoœæ pola blockDirect i bloków niebezpoœrednich
	static const u_int INODE_NUMBER_LIMIT = 32;     //Maksymalna iloœæ elementów w katalogu
	static const u_int MAX_PATH_LENGTH = 32;        //Maksymalna d³ugoœæ œcie¿ki
	static const bool BLOCK_FREE = false;           //Wartoœæ oznaczaj¹ca wolny blok
	static const bool BLOCK_OCCUPIED = !BLOCK_FREE; //Wartoœæ oznaczaj¹ca zajêty blok
	/**Wartoœæ oznaczaj¹ca iloœæ indeksów w polu directBlocks*/
	static const u_int BLOCK_DIRECT_INDEX_NUMBER = BLOCK_INDEX_NUMBER - 1;
	/**Maksymalny rozmiar pliku obliczony na podstawie maksymalnej iloœci indeksów*/
	static const size_t MAX_FILE_SIZE = (BLOCK_DIRECT_INDEX_NUMBER + (BLOCK_INDEX_NUMBER - BLOCK_DIRECT_INDEX_NUMBER)*BLOCK_INDEX_NUMBER) * BLOCK_SIZE;



	//--------------------- Definicje sta³ych -------------------



	//---------------- Definicje struktur i klas ----------------

	//Klasa indeksu
	class Index {
	public:
		u_int value; //Wartoœæ indeksu

		Index() : value(NULL) {}
		explicit Index(const u_int &value) : value(value) {}
		virtual ~Index() = default;
	};

	//Klasa bloku indeksowego
	class IndexBlock : public Index {
	private:
		//Tablica indeksów/bloków indeksowych
		std::array<std::shared_ptr<Index>, BLOCK_INDEX_NUMBER> block;
	public:
		IndexBlock() = default;
		virtual ~IndexBlock() = default;

		const u_int size() const { return block.size(); }
		void clear() { std::fill(block.begin(), block.end(), nullptr); }

		std::shared_ptr<Index>& operator [] (const size_t &index);
		const std::shared_ptr<Index>& operator [] (const size_t &index) const;
	};

	//Klasa i-wêz³a
	class Inode {
	public:
		//Podstawowe informacje
		std::string type;

		//Dodatkowe informacje
		tm creationTime;	 //Czas i data utworzenia
		//std::string creator; //Nazwa u¿ytkownika, który utworzy³ Inode

		/**
			Konstruktor domyœlny.
		*/
		explicit Inode(std::string type_) : type(std::move(type_)), creationTime() {};
		virtual ~Inode() = default;
	};

	//Klasa pliku dziedzicz¹ca po i-wêŸle
	class File : public Inode {
	public:
		//Podstawowe informacje
		size_t blocksOccupied;   //Iloœæ zajmowanych bloków
		size_t sizeOnDisk;       //Rozmiar pliku na dysku
		IndexBlock directBlocks; //Bezpoœrednie bloki (na koñcu 1 blok indeksowy 1-poziomu)

		//Dodatkowe informacje
		tm modificationTime; //Czas i data ostatniej modyfikacji pliku

		/**
			Konstruktor domyœlny.
		*/
		File() : Inode("FILE"), blocksOccupied(0), sizeOnDisk(0), modificationTime() {}

		virtual ~File() = default;
	};

	//Klasa katalogu dziedzicz¹ po i-wêŸle
	class Directory : public Inode {
	public:
		std::unordered_map<std::string, std::string> Inodes; //Tablica hashowa Inode
		//std::string parentDirectory; //WskaŸnik na katalog nadrzêdny

		/**
			Konstruktor inicjalizuj¹cy parentDirectory podan¹ zmiennymi.

			@param parentDirectory_ WskaŸnik na katalog utworzenia
		*/
		explicit Directory(std::string parentDirectory_)
			: Inode("DIRECTORY")/*, parentDirectory(std::move(parentDirectory_))*/ {}
		virtual ~Directory() = default;

		bool operator == (const Directory &dir) const;
	};

	//Prosta klasa dysku (imitacja fizycznego)
	class Disk {
	public:
		struct FileSystem {
			u_int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o iloœci wolnego miejsca na dysku (bajty)

			//Wektor bitowy bloków (0 - wolny blok, 1 - zajêty blok)
			std::bitset<DISK_CAPACITY / BLOCK_SIZE> bitVector;

			/**
			 Mapa Inode.
			 */
			std::unordered_map<std::string, std::shared_ptr<Inode>> InodeTable;

			std::string rootDirectory = "root/"; //Katalog g³ówny

			/**
				Konstruktor domyœlny. Wpisuje katalog g³ówny do tablicy iWêz³ów.
			*/
			FileSystem() { InodeTable[rootDirectory] = std::make_shared<Directory>(""); }
		} FileSystem; //System plików FileSystem

		//Tablica reprezentuj¹ca przestrzeñ dyskow¹ (jeden indeks - jeden bajt)
		std::array<char, DISK_CAPACITY> space;

		//----------------------- Konstruktor -----------------------
		/**
			Konstruktor domyœlny. Wykonuje zape³nienie przestrzeni dyskowej wartoœci¹ NULL.
		*/
		Disk();

		//-------------------------- Metody -------------------------
		/**
			Zapisuje dane (string) na dysku od indeksu 'begin' do indeksu 'end' w³¹cznie.

			@param begin Indeks od którego dane maj¹ byæ zapisywane.
			@param end Indeks na którym zapisywanie danych ma siê zakoñczyæ.
			@param data Dane typu string.
			@return void.
		*/
		void write(const u_int &begin, const u_int &end, const std::string &data);

		/**
			Odczytuje dane zadanego typu (jeœli jest on zaimplementowany) w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@param end Indeks do którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const u_int &begin, const u_int &end) const;
	} DISK;

	//------------------- Definicje zmiennych -------------------
	//u_int fileNumber = 0;  //Licznik plików w systemie (katalogi to te¿ pliki)
	bool messages = false; //Zmienna do w³¹czania/wy³¹czania powiadomieñ
	bool detailedMessages = false; //Zmienna do w³¹czania/wy³¹czania szczegó³owych powiadomieñ
	std::string currentDirectory; //Obecnie u¿ywany katalog


public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Przypisuje do obecnego katalogu katalog g³ówny.
	*/
	FileManager();



	//-------------------- Podstawowe Metody --------------------
	/**
		Tworzy plik o podanej nazwie i danych w obecnym katalogu.

		@param name Nazwa pliku.
		@param data Dane typu string.
		@return void.
	*/
	bool FileCreate(const std::string &name, const std::string &data);

	//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!
	/**
		Funkcja niedokoñczona.

		@param name Nazwa pliku.
		@return Tymczasowo zwraca dane wczytane z dysku.
	*/
	//const std::string FileOpen(const std::string &name) const;
	//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!


	const std::string FileGetData(const std::string& file);

	bool FileSaveData(const std::string& name, const std::string& data);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.
		Plik jest wymazywany z tablicy FileSystem oraz wektora bitowego.

		@param name Nazwa pliku.
		@return void.
	*/
	bool FileDelete(const std::string &name);

	/**
		Tworzy nowy katalog w obecnym katalogu.

		@param name Nazwa katalogu.
		@return void.
	*/
	bool DirectoryCreate(const std::string &name);

	/**
		Usuwa katalog o podanej nazwie.

		@param name Nazwa katalogu.
		@return void.
	*/
	bool DirectoryDelete(const std::string &name);

	/**
		Przechodzi z obecnego katalogu do katalogu nadrzêdnego.

		@return void.
	*/
	bool DirectoryUp();

	/**
		Przechodzi z obecnego katalogu do katalogu podrzêdnego o podanej nazwie

		@param name Nazwa katalogu.
		@return void.
	*/
	bool DirectoryDown(const std::string &name);



	//--------------------- Dodatkowe metody --------------------

	/**
		Zmienia nazwê pliku o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param changeName Zmieniona nazwa pliku.
		@return void.
	*/
	bool FileRename(const std::string &name, const std::string &changeName);

	/**
		Przechodzi z obecnego katalogu do katalogu g³ównego.

		@return void.
	*/
	void DirectoryRoot();

	/**
	Zmienia zmienn¹ odpowiadaj¹c¹ za wyœwietlanie komunikatów.
	false - komunikaty wy³¹czone.
	true - komunikaty w³¹czone.

	@param onOff Czy komunikaty maj¹ byæ w³¹czone.
	@return void.
*/
	void Messages(const bool &onOff);

	void DetailedMessages(const bool &onOff);



	//------------------ Metody do wyœwietlania -----------------

	void DisplayFileSystemParams() const;

	/**
		Wyœwietla informacje o wybranym katalogu.

		@return void.
	*/
	bool DisplayDirectoryInfo(const std::string &name);

	/**
		Wyœwietla informacje o pliku.

		@return void.
	*/
	bool DisplayFileInfo(const std::string &name);

	/**
		Wyœwietla strukturê katalogów.

		@return void.
	*/
	void DisplayDirectoryStructure();
	/**
		Wyœwietla rekurencyjnie katalog i jego podkatalogi.

		@param directory Katalog szczytowy do wyœwietlenia.
		@param level Poziom obecnego katalogu w hierarchii katalogów.
		@return void.
	*/
	void DisplayDirectory(const std::shared_ptr<Directory>& directory, u_int level);

	/**
		Wyœwietla zawartoœæ dysku w formie binarnej.

		@return void.
	*/
	void DisplayDiskContentBinary();

	/**
		Wyœwietla zawartoœæ dysku jako znaki.
		'.' - puste pole.

		@return void.
	*/
	void DisplayDiskContentChar();

	/**
		Wyœwietla wektor bitowy.

		@return void.
	*/
	void DisplayBitVector();


private:
	//------------------- Metody Sprawdzaj¹ce -------------------

	/**
		Sprawdza czy nazwa pliku jest u¿ywana w danym katalogu.

		@param directory Katalog, w którym sprawdzana jest nazwa pliku.
		@param name Nazwa pliku
		@return Prawda, jeœli nazwa nieu¿ywana, inaczej fa³sz.
	*/
	const bool CheckIfNameUnused(const std::string& directory, const std::string& name);

	/**
		Sprawdza czy jest miejsce na dane o zadaniej wielkoœci.

		@param dataSize Rozmiar danych, dla których bêdziemy sprawdzac miejsce.
		@return void.
	*/
	const bool CheckIfEnoughSpace(const u_int& dataSize) const;



	//-------------------- Metody Pomocnicze --------------------

	const std::string GetCurrentDirectoryParent() const;

	/**
		Wczytuje dane pliku z dysku.

		@param file Plik, którego dane maj¹ byæ wczytane.
		@return Dane pliku w postaci string.
	*/
	const std::string FileGetData(const std::shared_ptr<File>& file) const;

	void FileAddIndexes(const std::shared_ptr<File>& file, const std::vector<u_int> &blocks) const;

	void FileAllocateBlocks(const std::shared_ptr<File>& file, const std::vector<u_int>& blocks);

	void FileAllocationIncrease(std::shared_ptr<File>& file, const u_int &neededBlocks);

	void FileAllocationDecrease(const std::shared_ptr<File>& file, const u_int &neededBlocks);

	void FileDeallocate(const std::shared_ptr<File>& file);

	/**
		Usuwa ca³¹ strukturê katalogów.

		@param directory Katalog do usuniêcia.
		@return Rozmiar podanego katalogu.
	*/
	void DirectoryDeleteStructure(std::shared_ptr<Directory>& directory);

	/**
		Usuwa wskazany plik.

		@param file Plik do usuniêcia.
		@return void.
	*/
	void FileDelete(std::shared_ptr<File>& file);

	/**
		Zwraca rozmiar podanego katalogu.

		@return Rozmiar podanego katalogu.
	*/
	const size_t CalculateDirectorySize(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca rzeczywisty rozmiar podanego katalogu.

		@return Rzeczywisty rozmiar podanego katalogu.
	*/
	const size_t CalculateDirectorySizeOnDisk(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca liczbê folderów (katalogów) w danym katalogu i podkatalogach.

		@return Liczba folderów.
	*/
	const u_int CalculateDirectoryFolderNumber(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca liczbê plików w danym katalogu i podkatalogach.

		@return Liczba plików.
	*/
	const u_int CalculateDirectoryFileNumber(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca œcie¿kê przekazanego folderu

		@param directory Katalog, którego œciê¿kê chcemy otrzymaæ.
		@return Obecna œcie¿ka z odpowiednim formatowaniem.
	*/
	const std::string GetPath(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca obecnie u¿ywan¹ œcie¿kê.

		@return Obecna œcie¿ka z odpowiednim formatowaniem.
	*/
	const std::string GetCurrentPath() const;

	/**
		Zwraca d³ugoœæ obecnej œcie¿ki.

		@return d³ugoœæ obecnej œcie¿ki.
	*/
	const size_t GetCurrentPathLength() const;

	/**
		Zwraca aktualny czas i datê.

		@return Czas i data.
	*/
	static const tm GetCurrentTimeAndDate();

	/**
		Zmienia wartoœæ w wektorze bitowym i zarz¹ pole freeSpace
		w strukturze FileSystem.

		@param block Indeks bloku, którego wartoœæ w wektorze bitowym bêdzie zmieniana.
		@param value Wartoœæ do przypisania do wskazanego bloku (0 - wolny, 1 - zajêty)
		@return void.
	*/
	void ChangeBitVectorValue(const u_int &block, const bool &value);


	/**
		Zmniejsza plik do podanego rozmiaru. Podany rozmiar
		musi byæ mniejszy od rozmiaru pliku o conajmniej jedn¹
		jednostkê alokacji

		@param file WskaŸnik na plik, którego rozmiar chcemy zmieniæ.
		@param neededBlocks Iloœæ bloków do alokacji.
		@return void.
	*/
	void FileTruncate(std::shared_ptr<File> file, const u_int &neededBlocks);

	/**
		Zapisuje wektor fragmentów File.data na dysku.

		@param file Plik, którego dane bêd¹ zapisane na dysku.
		@param data Dane do zapisania na dysku.
		@return void.
	*/
	void FileSaveData(std::shared_ptr<File> &file, const std::string &data);

	/**
		Dzieli string na fragmenty o rozmiarze BLOCK_SIZE.

		@param data String do podzielenia na fradmenty.
		@return Wektor fragmentów string.
	*/
	const std::vector<std::string> DataToDataFragments(const std::string &data) const;

	/**
		Oblicza ile bloków zajmie podany string.

		@param dataSize D³ugoœæ danych, których rozmiar na dysku bêdzie obliczany.
		@return Iloœæ bloków jak¹ zajmie string.
	*/
	const u_int CalculateNeededBlocks(const size_t &dataSize) const;

	/**
		Znajduje nieu¿ywane bloki do zapisania pliku bez dopasowania do luk w blokach

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocksFragmented(u_int blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku metod¹ best-fit.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocksBestFit(const u_int &blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku. Najpierw uruchamia funkcjê
		dzia³aj¹c¹ metod¹ best-fit, jeœli funkcja nie znajdzie dopasowania do
		uruchamia funkcjê znajduj¹c¹ pierwsze jakiekolwiek wolne bloki i wprowadza
		fragmentacjê danych.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocks(const u_int &blockNumber);

};

static FileManager fileManager;

#endif //SEXYOS_FILEMANAGER_H
