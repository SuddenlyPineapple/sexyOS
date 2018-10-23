/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 22/10/18
*/

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#include <math.h>
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>

/*
	!!!UWAGA!!!
	Czêœæ funkcji jest zaimplementowana tylko po to, ¿eby
	testowaæ inne funkcje i maj¹ np sta³e lokalizacje bloków
	albo tylko tworz¹ obiekt z danymi i zapisuj¹
*/

class FileManager {
private:
	//--------------- Definicje sta³ych statycznych -------------
	static const unsigned int BLOCK_SIZE = 8; //Sta³y rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024;	  //Sta³a pojemnoœæ dysku (bajty)

	//---------------- Definicje struktur i klas ----------------
	class Disk {
	public:
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
			Zapisuje dane typu bitset od wskazanego indeksu.

			@param index Indeks od którego dane maj¹ byæ zapisywane.
			@param data Bitset o dowolnym.
			@return void.
		*/
		template<unsigned int size>
		void write(const unsigned int &begin, const std::bitset<size> &data);

		/**
			Odczytuje dane typu dowolnego w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@param end Indeks do którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const unsigned int &begin, const unsigned int &end);
	} DISK; //Prosta klasa dysku (fizycznego)
	//Struktura pliku
	struct FileFAT {
		std::string name;  //Nazwa pliku
		unsigned int size; //Rozmiar pliku
		std::vector<unsigned int>occupiedBlocks; //Bloki zajmowane przez plik
		std::string data;

		/**
			Konstruktor domyœlny.
		*/
		FileFAT() {}

		/**
			Konstruktor inicjalizuj¹cy pola name i id podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
		*/
		FileFAT(const std::string &name_) : name(name_) {};

		/**
			Konstruktor inicjalizuj¹cy pola name, id i data podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
			@param data_ Dane typu string zapisane w pliku
		*/
		FileFAT(const std::string &name_, const std::string &data_) : name(name_), data(data_) {}
	};
	//Struktura katalogu
	struct Directory {
		std::string name; //Nazwa katalogu
		std::unordered_map<std::string, FileFAT> FAT; //Tablica hashowa plików w katalogu
		std::unordered_map<std::string, Directory>subDirectories; //Tablica hashowa podkatalogów

		Directory() {}
		Directory(const std::string &name_) : name(name_) {}
	};

	//------------------- Definicje zmiennych -------------------
	std::bitset<DISK_CAPACITY / BLOCK_SIZE> blockMap; //Tablica bitowa bloków (0 - wolny, 1 - zajêty)
	Directory rootDirectory{ Directory("root") }; //Katalog g³ówny
	unsigned int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o iloœci wolnego miejsca na dysku (bajty)

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Rezerwuje bloki potrzebne do zapisania
		tablicy bitowej bloków oraz zapisuje t¹ tablicê na dysku
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------

	//Tworzy plik
	void CreateFile(const std::string &name);
	void CreateFile(const std::string &name, const std::string &data);

	//Otwiera plik
	const std::string OpenFile(const unsigned int &id);

	//Usuwa plik (ca³kowicie wymazuje)
	void DeleteFile();

	//Usuwa plik (usuwa go z tablicy FAT)
	void TruncateFile();

	//------------------ Metody do wyœwietlania -----------------

	//Wyœwietla strukturê katalogów
	void DisplayDirectoryStructure();

	//Wyœwietla zawartoœæ dysku w formie binarnej
	void DisplayDiskContentBinary();

	//Wyœwietla zawartoœæ dysku w formie znaków
	void DisplayDiskContentChar();

	//Wyœwietla tablicê bloków
	void DisplayBlocks();

	//Wyœwietla fragmenty pliku
	void DisplayFileFragments(const std::vector<std::string> &fileFragments);

private:
	//-------------------- Metody Pomocnicze --------------------

	//Sprawdza czy nazwa pliku jest u¿yta w danym katalogu
	const bool CheckIfNameUnused(const Directory &directory, const std::string &name);

	//Sprawdza czy jest miejsce na dane o zadaniej wielkoœci
	const bool CheckIfEnoughSpace(const unsigned int &dataSize);

	//Zmienia wartoœæ w tablicy bitowej bloków i zapisuje zmianê na dysku
	void ChangeBlockMapValue(const unsigned int &block, const bool &value);

	//Zapisuje wektor FileFAT na dysku
	void WriteFile(const FileFAT &file);

	//Konwertuje kompletny plik na formê do zapisania na dysku
	const std::vector<std::string> FileFATToFileFragments(const FileFAT &fileFAT);

	//Oblicza ile bloków zajmie podany string
	const unsigned int CalculateNeededBlocks(const std::string &data);

	//Znajduje nieu¿ywane bloki do zapisania pliku;
	std::vector<unsigned int> FindUnallocatedBlocks(unsigned int blockCount);
};

#endif //SEXYOS_FILEMANAGER_H
