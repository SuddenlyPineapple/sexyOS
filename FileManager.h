/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 01/11/18
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

#include<iostream>

/*
	To-do:
	- iNode
	- otwieranie i zamykanie pliku
	- plik flagi
	- defragmentator
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
	using u_int = unsigned int;

private:
	//--------------- Definicje sta³ych statycznych -------------
	static const size_t BLOCK_SIZE = 32;				 //Rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024;        //Pojemnoœæ dysku (bajty)
	static const size_t BLOCK_INDEX_NUMBER = 3;		 //Wartoœæ oznaczaj¹ca d³ugoœæ pola blockDirect i bloków niebezpoœrednich
	static const size_t TOTAL_FILE_NUMBER_LIMIT = 32; //Maksymalna iloœæ elementów w katalogu
	static const size_t BLOCK_DIRECT_INDEX = BLOCK_INDEX_NUMBER - 1; //Wartoœæ oznaczaj¹ca iloœæ indeksów w polu directBlocks
	static const bool BLOCK_FREE = false;            //Wartoœæ oznaczaj¹ca wolny blok
	static const bool BLOCK_OCCUPIED = !BLOCK_FREE;  //Wartoœæ oznaczaj¹ca zajêty blok
	static const size_t MAX_PATH_LENGTH = 32;		 //Maksymalna d³ugoœæ œcie¿ki
	//Maksymalny rozmiar pliku obliczony na podstawie maksymalnej iloœci indeksów
	static const size_t MAX_FILE_SIZE = (BLOCK_DIRECT_INDEX + BLOCK_INDEX_NUMBER) * BLOCK_SIZE; 

	//--------------------- Definicje sta³ych -------------------

	//---------------- Definicje struktur i klas ----------------

	class Index {
	public:
		u_int value; //Wartoœæ indeksu

		Index() : value(NULL) {}
		explicit Index(const u_int &value) : value(value){}
		virtual ~Index() = default;
	};
	class IndexBlock : public Index {
	public:
		//Tablica indeksów/bloków indeksowych
		std::array<std::shared_ptr<Index>, BLOCK_INDEX_NUMBER> value;

		IndexBlock() = default;
		virtual ~IndexBlock() = default;

		std::shared_ptr<Index>& operator [] (const size_t &index);
		const std::shared_ptr<Index>& operator [] (const size_t &index) const;
	};

	//Struktura I-wêz³a
	class iNode{
	public:
		//Podstawowe informacje
		std::string type;

		//Dodatkowe informacje
		tm creationTime;	 //Czas i data utworzenia
		std::string creator; //Nazwa u¿ytkownika, który utworzy³ iNode

		/**
			Konstruktor domyœlny.
		*/
		iNode() = default;
	};

	class File : public iNode {
	public:
		//Podstawowe informacje
		size_t blocksOccupied; //Iloœæ zajmowanych bloków
		size_t sizeOnDisk; //Rozmiar pliku na dysku
		IndexBlock directBlocks; //Bezpoœrednie bloki (na koñcu 1 blok indeksowy 1-poziomu)

		//Dodatkowe informacje
		tm modificationTime; //Czas i data ostatniej modyfikacji pliku

		/**
			Konstruktor domyœlny.
		*/
		File() = default;
		virtual ~File() = default;
	};

	//Struktura katalogu
	class Directory : iNode {
	public:
		std::string name;  //Nazwa katalogu
		tm creationTime;   //Czas i data utworzenia katalogu

		std::unordered_map<std::string, File> files; //Tablica hashowa plików w katalogu
		std::unordered_map<std::string, Directory>subDirectories; //Tablica hashowa podkatalogów
		Directory* parentDirectory; //WskaŸnik na katalog nadrzêdny

		/**
			Konstruktor domyœlny.
		*/
		Directory() = default;

		/**
			Konstruktor inicjalizuj¹cy pole name i parentDirectory podanymi zmiennymi.

			@param name_ Nazwa pliku.
			@param parentDirectory_ WskaŸnik na katalog utworzenia
		*/
		Directory(std::string name_, Directory* parentDirectory_) : name(std::move(name_)), creationTime(),
		                                                            parentDirectory(parentDirectory_){}
		virtual ~Directory() = default;
	};

	class Disk {
	public:
		struct FileSystem {
			u_int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o iloœci wolnego miejsca na dysku (bajty)

			//Wektor bitowy bloków (0 - wolny blok, 1 - zajêty blok)
			std::bitset<DISK_CAPACITY / BLOCK_SIZE> bitVector;

			std::array<iNode, TOTAL_FILE_NUMBER_LIMIT>iNodeTable;

			Directory rootDirectory{ Directory("root", nullptr) }; //Katalog g³ówny

			/**
				Konstruktor domyœlny. Wykonuje zape³nienie tablicy FileSystem wartoœci¹ -1.
			*/
			FileSystem() = default;
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
			Zapisuje dane (u_int) pod wskazanym indeksem.

			@param index Indeks na którym zapisana zostanie liczba.
			@param data Liczba typu u_int.
			@return void.
		*/
		void write(const u_int &index, const u_int &data);

		/**
			Odczytuje dane zadanego typu (jeœli jest on zaimplementowany) w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@param end Indeks do którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const u_int &begin, const u_int &end);
	} DISK; //Prosta klasa dysku (imitacja fizycznego)

	//------------------- Definicje zmiennych -------------------
	u_int fileNumber = 0;  //Licznik plików w systemie (katalogi to te¿ pliki)
	bool messages = false; //Zmienna do w³¹czania/wy³¹czania powiadomieñ
	Directory* currentDirectory; //Obecnie u¿ytkowany katalog

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Przypisuje do obecnego katalogu katalog g³ówny.
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------
	/**
		Tworzy plik o podanej nazwie i danych w obecnym katalogu.

		@param name Nazwa pliku
		@param data Dane typu string.
		@return void.
	*/
	void FileCreate(const std::string &name, const std::string &data);

	//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!
	/**
		Funkcja niedokoñczona.

		@param name Nazwa pliku.
		@return Tymczasowo zwraca dane wczytane z dysku.
	*/
	const std::string FileOpen(const std::string &name);
	//!!!!!!!!!! NIEDOKOÑCZONE !!!!!!!!!!

	/**
		Wczytuje dane pliku z dysku.

		@param file Plik, którego dane maj¹ byæ wczytane.
		@return Dane pliku w postaci string.
	*/
	const std::string FileGetData(const File &file);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.
		Plik jest wymazywany z tablicy FileSystem oraz wektora bitowego.

		@param name Nazwa pliku.
		@return void.
	*/
	void FileDelete(const std::string &name);

	/**
		Zmniejsza plik do podanego rozmiaru. Podany rozmiar
		musi byæ mniejszy od rozmiaru pliku o conajmniej jedn¹
		jednostkê alokacji

		@param name Nazwa pliku.
		@param size Rozmiar do którego plik ma byæ zmniejszony.
		@return void.
	*/
	void FileTruncate(const std::string &name, const u_int &size);

	/**
		Tworzy nowy katalog w obecnym katalogu.

		@param name Nazwa katalogu.
		@return void.
	*/
	void DirectoryCreate(const std::string &name);

	/**
		Usuwa katalog o podanej nazwie.

		@param name Nazwa katalogu.
		@return void.
	*/
	void DirectoryDelete(const std::string &name);


	/**
		Przechodzi z obecnego katalogu do katalogu nadrzêdnego.

		@return void.
	*/
	void DirectoryUp();

	/**
		Przechodzi z obecnego katalogu do katalogu podrzêdnego o podanej nazwie

		@param name Nazwa katalogu.
		@return void.
	*/
	void DirectoryDown(const std::string &name);

	//--------------------- Dodatkowe metody --------------------
	/**
		Zmienia nazwê pliku o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param changeName Zmieniona nazwa pliku.
		@return void.
	*/
	void FileRename(const std::string &name, const std::string &changeName) const;
	
	/**
		Przechodzi z obecnego katalogu do katalogu g³ównego.

		@return void.
	*/
	void DirectoryRoot();

	//------------------ Metody do wyœwietlania -----------------
	/**
		Zmienia zmienn¹ odpowiadaj¹c¹ za wyœwietlanie komunikatów.
		false - komunikaty wy³¹czone.
		true - komunikaty w³¹czone.

		@param onOff Czy komunikaty maj¹ byæ w³¹czone.
		@return void.
	*/
	void Messages(const bool &onOff);

	void DisplayFileSystemParams() const;

	/**
		Wyœwietla informacje o wybranym katalogu.

		@return void.
	*/
	void DisplayDirectoryInfo(const std::string &name) const;

	/**
		Wyœwietla informacje o pliku.

		@return void.
	*/
	void DisplayFileInfo(const std::string &name);

	/**
		Wyœwietla strukturê katalogów.

		@return void.
	*/
	void DisplayDirectoryStructure() const;
	/**
		Wyœwietla rekurencyjnie katalog i jego podkatalogi.

		@param directory Katalog szczytowy do wyœwietlenia.
		@param level Poziom obecnego katalogu w hierarchii katalogów.
		@return void.
	*/
	static void DisplayDirectory(const Directory &directory, u_int level);

	/**
		Wyœwietla zawartoœæ dysku w formie binarnej.

		@return void.
	*/
	void DisplayDiskContentBinary();

	/**
		Wyœwietla zawartoœæ dysku w znaków.

		@return void.
	*/
	void DisplayDiskContentChar();

	//**
	//	Wyœwietla tablicê alokacji plików (FileSystem).
	//
	//	@return void.
	//*/
	//void DisplayFileAllocationTable();

	/**
		Wyœwietla wektor bitowy.

		@return void.
	*/
	void DisplayBitVector();

	/**
		Wyœwietla plik podzielony na fragmenty.

		@return void.
	*/
	static void DisplayFileFragments(const std::vector<std::string> &fileFragments);

private:
	//-------------------- Metody Pomocnicze --------------------
	/**
		Usuwa ca³¹ strukturê katalogów.

		@param directory Katalog do usuniêcia.
		@return Rozmiar podanego katalogu.
	*/
	void DirectoryDeleteStructure(Directory &directory);

	/**
		Usuwa wskazany plik.

		@param file Plik do usuniêcia.
		@return void.
	*/
	void FileDelete(File&file);

	/**
		Zwraca rozmiar podanego katalogu.

		@return Rozmiar podanego katalogu.
	*/
	static const size_t CalculateDirectorySize(const Directory &directory);

	/**
		Zwraca rzeczywisty rozmiar podanego katalogu.

		@return Rzeczywisty rozmiar podanego katalogu.
	*/
	static const size_t CalculateDirectorySizeOnDisk(const Directory &directory);

	/**
		Zwraca liczbê folderów (katalogów) w danym katalogu i podkatalogach.

		@return Liczba folderów.
	*/
	static const u_int CalculateDirectoryFolderNumber(const Directory &directory);

	/**
		Zwraca liczbê plików w danym katalogu i podkatalogach.

		@return Liczba plików.
	*/
	static const u_int CalculateDirectoryFileNumber(const Directory &directory);

	/**
		Zwraca œcie¿kê przekazanego folderu

		@param directory Katalog, którego œciê¿kê chcemy otrzymaæ.
		@return Obecna œcie¿ka z odpowiednim formatowaniem.
	*/
	static const std::string GetPath(const Directory &directory);

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
		Sprawdza czy nazwa pliku jest u¿ywana w danym katalogu.

		@param directory Katalog, w którym sprawdzana jest nazwa pliku.
		@param name Nazwa pliku
		@return Prawda, jeœli nazwa nieu¿ywana, inaczej fa³sz.
	*/
	static const bool CheckIfNameUnused(const Directory &directory, const std::string &name);

	/**
		Sprawdza czy jest miejsce na dane o zadaniej wielkoœci.

		@param dataSize Rozmiar danych, dla których bêdziemy sprawdzac miejsce.
		@return void.
	*/
	const bool CheckIfEnoughSpace(const u_int &dataSize) const;

	/**
		Zmienia wartoœæ w wektorze bitowym i zarz¹ pole freeSpace
		w strukturze FileSystem.

		@param block Indeks bloku, którego wartoœæ w wektorze bitowym bêdzie zmieniana.
		@param value Wartoœæ do przypisania do wskazanego bloku (0 - wolny, 1 - zajêty)
		@return void.
	*/
	void ChangeBitVectorValue(const u_int &block, const bool &value);

	/**
		Zapisuje wektor fragmentów File.data na dysku.

		@param file Plik, którego dane bêd¹ zapisane na dysku.
		@param data Dane do zapisania na dysku.
		@return void.
	*/
	void FileWrite(const File &file, const std::string &data);

	/**
		Dzieli string na fragmenty o rozmiarze BLOCK_SIZE.

		@param data String do podzielenia na fradmenty.
		@return Wektor fragmentów string.
	*/
	const std::vector<std::string> DataToDataFragments(const std::string &data) const;

	/**
		Oblicza ile bloków zajmie podany string.

		@param data String, którego rozmiar na dysku, bêdzie obliczany.
		@return Iloœæ bloków jak¹ zajmie string.
	*/
	const u_int CalculateNeededBlocks(const std::string &data) const;

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
