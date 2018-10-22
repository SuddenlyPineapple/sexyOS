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
	static const size_t DISK_SIZE = 1024;	  //Sta³y rozmiar dysku (bajty)

	//---------------- Definicje struktur i klas ----------------
	class Disk {
	public:
		//Tablica reprezentuj¹ca przestrzeñ dyskow¹ (jeden indeks - jeden bajt)
		std::array<char, DISK_SIZE> space;

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
	//Struktura pliku (fizyczna)
	struct FileFAT {
		//-------------------- Definicje sta³ych --------------------
		const std::string DATA;		   //Dane typu string
		const unsigned int NEXT_FRAGM; //Wskazanie na kolejny fragment pliku
									   //w przestrzeni dyskowej

		//----------------------- Konstruktor -----------------------
		/**
			Konstruktor domyœlny. Inicjalizuje pola data i nextPart wartoœci¹ NULL.
		*/
		FileFAT() : DATA(NULL), NEXT_FRAGM(NULL) {}
		/**
			Konstruktor inicjalizuj¹cy pola data i nextPart podanymi zmiennymi.

			@param data_ Zmienna typu string do przechowania tekstu.
			@param nextPart_ Zmienna typu unsigned int bêd¹ca wskazaniem na indeks
			kolejnej czêœci pliku w przestrzeni dyskowej. J
		*/
		FileFAT(const std::string &data, const unsigned int nextFragm) : DATA(data), NEXT_FRAGM(nextFragm) {}
	};
	//Struktura pliku (logiczna)
	struct File {
		std::string name;  //Nazwa pliku
		unsigned int id;   //Numer identyfikacyjny pliku
		unsigned int size; //Rozmiar pliku
		std::string data;

		/**
			Konstruktor domyœlny.
		*/
		File() {}
		/**
			Konstruktor inicjalizuj¹cy pola name i id podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
		*/
		File(const std::string &name_, const unsigned int &id_) : name(name_), id(id_) {}
		/**
			Konstruktor inicjalizuj¹cy pola name, id i data podanymi zmiennymi.

			@param name_ Nazwa pliku
			@param id_ Numer identyfikacyjny pliku
			@param data_ Dane typu string zapisane w pliku
		*/
		File(const std::string &name_, const unsigned int &id_, const std::string &data_) : name(name_), id(id_), data(data_) {}
	};

	//------------------- Definicje zmiennych -------------------
	std::bitset<DISK_SIZE / BLOCK_SIZE> clusterMap; //Tablica bitowa bloków (0 - wolny, 1 - zajêty)

public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Rezerwuje bloki potrzebne do zapisania
		tablicy bitowej bloków oraz zapisuje t¹ tablicê na dysku
	*/
	FileManager();

	//-------------------- Podstawowe Metody --------------------
	//Tworzy plik
	void CreateFile(const std::string &name, const unsigned int &id);
	void CreateFile(const std::string &name, const unsigned int &id, const std::string &data);
	//Otwiera plik
	const std::string OpenFile(const unsigned int &id);
	//Usuwa plik (ca³kowicie wymazuje)
	void DeleteFile();
	//Usuwa plik (usuwa go z tablicy FAT)
	void TruncateFile();

	//------------------ Metody do wyœwietlania -----------------
	//Wyœwietla zawartoœæ dysku w formie binarnej
	void DisplayDiskContentBinary();
	//Wyœwietla zawartoœæ dysku w formie znaków
	void DisplayDiskContentChar();
	//Wyœwietla tablicê bloków
	void DisplayBlocks();
	//Wyœwietla fileFAT
	void DisplayFileFAT(const std::vector<FileFAT> &fileFAT);

private:
	//-------------------- Metody Pomocnicze --------------------
	//Zmienia wartoœæ w tablicy bitowej bloków i zapisuje zmianê na dysku
	void ChangeClusterMapValue(const unsigned int &block, const bool &value);

	//Zapisuje wektor FileFAT na dysku
	void WriteFileFAT(const unsigned int &begin, const std::vector<FileFAT> &fileFAT);
	//Konwertuje kompletny plik na formê do zapisania na dysku
	std::vector<FileFAT> FileToFileFAT(const File &file);
	//Oblicza ile bloków zajmie podany string (uwzglêdniaj¹c czêœæ FAT)
	const unsigned int CalculateNeededBlocks(const std::string &data);
	//Znajduje nieu¿ywane bloki do zapisania pliku;
	std::vector<unsigned int> FindUnallocatedBlocks(unsigned int blockCount);
};

#endif //SEXYOS_FILEMANAGER_H