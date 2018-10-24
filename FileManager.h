/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 24/10/18
*/

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#include <math.h>
#include <time.h>
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <iostream>

/*
	Todo:
	- otwieranie i zamykanie pliku
	- plik flagi + dane utworzenia
	- defragmentator
	- zapisywanie plików z kodem asemblerowym
*/

//Klasa zarz¹dcy przestrzeni¹ dyskow¹ i systemem plików
class FileManager {
private:
	//--------------- Definicje sta³ych statycznych -------------
	static const unsigned int BLOCK_SIZE = 8; //Sta³y rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024; //Sta³a pojemnoœæ dysku (bajty)

	//---------------- Definicje struktur i klas ----------------

	//Struktura pliku
	struct File {
		//Podstawowe informacje
		std::string name;	   //Nazwa pliku
		unsigned int size;	   //Rozmiar pliku
		unsigned int FATindex; //Indeks pozycji pocz¹tku pliku w tablicy FAT

		//Dodatkowe informacje
		tm creationTime;	 //Czas i data utworzenia pliku
		tm modificationTime; //Czas i data ostatniej modyfikacji pliku
		std::string creator; //Nazwa u¿ytkownika, który utworzy³ plik

		/**
			Konstruktor domyœlny.
		*/
		File() {}

		/**
			Konstruktor inicjalizuj¹cy pola name podanymi zmiennymi.

			@param name_ Nazwa pliku.
		*/
		File(const std::string &name_) : name(name_) {};
	};

	//Struktura katalogu
	struct Directory {
		std::string name; //Nazwa katalogu
		std::unordered_map<std::string, File> files; //Tablica hashowa plików w katalogu
		std::unordered_map<std::string, Directory>subDirectories; //Tablica hashowa podkatalogów
		Directory* parentDirectory; //WskaŸnik na katalog nadrzêdny

		/**
			Konstruktor domyœlny.
		*/
		Directory() {}
		/**
			Konstruktor inicjalizuj¹cy pole name i parentDirectory podanymi zmiennymi.

			@param name_ Nazwa pliku.
			@param parentDirectory WskaŸnik na katalog utworzenia
		*/
		Directory(const std::string &name_, Directory* parentDirectory_) : name(name_), parentDirectory(parentDirectory_) {}
	};

	class Disk {
	public:
		struct FAT {
			unsigned int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o iloœci wolnego miejsca na dysku (bajty)

			//Wektor bitowy bloków (0 - wolny blok, 1 - zajêty blok)
			std::bitset<DISK_CAPACITY / BLOCK_SIZE> bitVector;

			/*
			Zawiera indeksy bloków dysku na dysku, na których znajduj¹ siê pofragmentowane dane pliku.
			Indeks odpowiada rzeczywistemu blokowi dyskowemu, a jego zawartoœci¹ jest indeks nastêpnego bloku lub NULL.
			*/
			std::array<unsigned int, DISK_CAPACITY / BLOCK_SIZE>FileAllocationTable = { NULL };

			Directory rootDirectory{ Directory("root", NULL) }; //Katalog g³ówny

		} FAT; //System plików FAT

		//Tablica reprezentuj¹ca przestrzeñ dyskow¹ (jeden indeks - jeden bajt)
		std::array<char, DISK_CAPACITY> space;

		//----------------------- Konstruktor -----------------------
		/**
			Konstruktor domyœlny. Wykonuje zape³nienie przestrzeni dyskowej wartoœci¹ NULL
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
		void write(const unsigned int &begin, const unsigned int &end, const std::string &data);

		/**
			Zapisuje dane (unsigned int) pod wskazanym indeksem.

			@param index Indeks na którym zapisana zostanie liczba.
			@param data Liczba typu unsigned int.
			@return void.
		*/
		void write(const unsigned int &index, const unsigned int &data);

		/**
			Odczytuje dane zadanego typu (jeœli jest on zaimplementowany) w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@param end Indeks do którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const unsigned int &begin, const unsigned int &end);
	} DISK; //Prosta klasa dysku (imitacja fizycznego)

	//------------------- Definicje zmiennych -------------------
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
	void CreateFile(const std::string &name, const std::string &data);

	//Otwiera plik
	/**
		Funkcja niedokoñczona.

		@param name Nazwa pliku.
		@return Tymczasowo zwraca dane wczytane z dysku.
	*/
	const std::string OpenFile(const std::string &name);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.
		Plik jest wymazywany z tablicy FAT oraz wektora bitowego.

		@param name Nazwa pliku.
		@return void.
	*/
	void DeleteFile(const std::string &name);

	/**
		Odcina koñcow¹ czêœæ pliku o zadanej iloœci bloków.
		Funkcja niedokoñczona.

		@param name Nazwa pliku.
		@param blockCount Liczba bloków do odciêcia.
		@return void.
	*/
	void TruncateFile(const std::string &name, const unsigned int &blockCount);

	/**
		Tworzy nowy katalog w obecnym katalogu.

		@param name Nazwa katalogu.
		@return void.
	*/
	void CreateDirectory(const std::string &name);


	/**
		Przechodzi z obecnego katalogu do katalogu nadrzêdnego.

		@param name Nazwa katalogu.
		@return void.
	*/
	void CurrentDirectoryUp();

	/**
		Przechodzi z obecnego katalogu do katalogu podrzêdnego o podanej nazwie

		@param name Nazwa katalogu.
		@return void.
	*/
	void CurrentDirectoryDown(const std::string &name);

	//--------------------- Dodatkowe metody --------------------

	/**
		Przechodzi z obecnego katalogu do katalogu g³ównego.

		@return void.
	*/
	void CurrentDirectoryRoot();

	//------------------ Metody do wyœwietlania -----------------

	/**
		Wyœwietla informacje o pliku.

		@return void.
	*/
	void DisplayFileInfo(const std::string &name);

	//Wyœwietla strukturê katalogów
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
	void DisplayDirectory(const Directory &directory, unsigned int level);

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

	/**
		Wyœwietla tablicê alokacji plików (FAT).

		@return void.
	*/
	void DisplayFileAllocationTable();

	/**
		Wyœwietla wektor bitowy.

		@return void.
	*/
	void DisplayBitVector();

	/**
		Wyœwietla plik podzielony na fragmenty.

		@return void.
	*/
	void DisplayFileFragments(const std::vector<std::string> &fileFragments);

private:
	//-------------------- Metody Pomocnicze --------------------

	/**
		Wczytuje dane pliku z dysku.

		@param file Plik, którego dane maj¹ byæ wczytane.
		@return Dane pliku w postaci string.
	*/
	const std::string GetFileData(const File &file);

	/**
		Zwraca obecnie u¿ywan¹ œcie¿kê.

		@return void.
	*/
	const std::string GetCurrentPath();

	/**
		Sprawdza czy nazwa pliku jest u¿ywana w danym katalogu.

		@param directory Katalog, w którym sprawdzana jest nazwa pliku.
		@param name Nazwa pliku
		@return Prawda, jeœli nazwa nieu¿ywana, inaczej fa³sz.
	*/
	const bool CheckIfNameUnused(const Directory &directory, const std::string &name);

	/**
		Sprawdza czy jest miejsce na dane o zadaniej wielkoœci.

		@param directory Katalog, w którym sprawdzana jest nazwa pliku.
		@param name Nazwa pliku
		@return void.
	*/
	const bool CheckIfEnoughSpace(const unsigned int &dataSize);

	/**
		Zmienia wartoœæ w wektorze bitowym i zarz¹ pole freeSpace
		w strukturze FAT.

		@param block Indeks bloku, którego wartoœæ w wektorze bitowym bêdzie zmieniana.
		@param value Wartoœæ do przypisania do wskazanego bloku (0 - wolny, 1 - zajêty)
		@return void.
	*/
	void ChangeBitVectorValue(const unsigned int &block, const bool &value);

	/**
		Zapisuje wektor fragmentów File.data na dysku.

		@param file Plik, którego dane bêd¹ zapisane na dysku.
		@param value Dane do zapisania na dysku.
		@return void.
	*/
	void WriteFile(const File &file, const std::string &data);

	/**
		Dzieli string na fragmenty o rozmiarze BLOCK_SIZE.

		@param data String do podzielenia na fradmenty.
		@return Wektor fragmentów string.
	*/
	const std::vector<std::string> DataToDataFragments(const std::string &data);

	/**
		Oblicza ile bloków zajmie podany string.

		@param data String, którego rozmiar na dysku, bêdzie obliczany.
		@return Iloœæ bloków jak¹ zajmie string.
	*/
	const unsigned int CalculateNeededBlocks(const std::string &data);

	/**
		Znajduje nieu¿ywane bloki do zapisania pliku bez dopasowania do luk w blokach

		@param blockCount Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<unsigned int> FindUnallocatedBlocksFragmented(unsigned int blockCount);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku metod¹ best-fit.
		
		@param blockCount Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<unsigned int> FindUnallocatedBlocksBestFit(const unsigned int &blockCount);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku. Najpierw uruchamia funkcjê
		dzia³aj¹c¹ metod¹ best-fit, jeœli funkcja nie znajdzie dopasowania do
		uruchamia funkcjê znajduj¹c¹ pierwsze jakiekolwiek wolne bloki i wprowadza
		fragmentacjê danych.

		@param blockCount Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Wektor indeksów bloków do zaalokowania.
	*/
	const std::vector<unsigned int> FindUnallocatedBlocks(const unsigned int &blockCount);

};

#endif //SEXYOS_FILEMANAGER_H
