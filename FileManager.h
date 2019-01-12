/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasê FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 04/01/19
*/

/*
 * Aby ³atwiej nawigowaæ po moim kodzie polecam z³o¿yæ wszystko
 * Skrót: CTRL + M + A
 */

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#define _CRT_SECURE_NO_WARNINGS

#include "Semaphores.hpp"
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <map>

 /*
	 TODO:
	 - dodaæ semafory
	 - dorobiæ stuff z ³adowaniem bufora do RAMu przy odczycie (opcjonalne)
 */

 //Do u¿ywania przy funkcji open (nazwy mówi¹ same za siebie)
#define FILE_OPEN_R_MODE  1 //01
#define FILE_OPEN_W_MODE  2 //10
#define FILE_OPEN_RW_MODE 3 //11

//Do u¿ycia przy obs³udze b³êdów
#define FILE_ERROR_NONE				0
#define FILE_ERROR_EMPTY_NAME		1
#define FILE_ERROR_NAME_TOO_LONG	2
#define FILE_ERROR_NAME_USED		3
#define FILE_ERROR_NO_INODES_LEFT	4
#define FILE_ERROR_DATA_TOO_BIG		5
#define FILE_ERROR_NOT_FOUND		6
#define FILE_ERROR_NOT_OPENED		7
#define FILE_ERROR_OPENED			8
#define FILE_ERROR_NOT_R_MODE		9
#define FILE_ERROR_NOT_W_MODE		10

//Klasa zarz¹dcy przestrzeni¹ dyskow¹ i systemem plików
class FileManager {
private:
	//--------------------------- Aliasy ------------------------
	using u_int = unsigned int;
	using u_short_int = unsigned short int;



	//--------------- Definicje sta³ych statycznych -------------

	static const uint8_t BLOCK_SIZE = 32;	   	   //Rozmiar bloku (bajty)
	static const u_short_int DISK_CAPACITY = 1024; //Pojemnoœæ dysku (bajty)
	static const uint8_t BLOCK_INDEX_NUMBER = 3;   //Wartoœæ oznaczaj¹ca d³ugoœæ pola blockDirect
	static const uint8_t INODE_NUMBER_LIMIT = 32;  //Maksymalna iloœæ elementów w katalogu
	static const uint8_t MAX_FILENAME_LENGTH = 16; //Maksymalna d³ugoœæ œcie¿ki

	static const bool BLOCK_FREE = false;           //Wartoœæ oznaczaj¹ca wolny blok
	static const bool BLOCK_OCCUPIED = !BLOCK_FREE; //Wartoœæ oznaczaj¹ca zajêty blok

	//Maksymalny rozmiar danych
	static const u_short_int MAX_DATA_SIZE = (BLOCK_INDEX_NUMBER + BLOCK_SIZE / 2)*BLOCK_SIZE;

	//Maksymalny rozmiar pliku (wliczony blok indeksowy)
	static const u_short_int MAX_FILE_SIZE = MAX_DATA_SIZE + BLOCK_SIZE;



	//---------------- Definicje struktur i klas ----------------

	//Klasa i-wêz³a - zawiera podstawowe informacje o pliku
	struct Inode {
		//Podstawowe informacje
		uint8_t blocksOccupied = 0;  //Iloœæ zajmowanych bloków
		u_short_int realSize = 0;    //Rzeczywisty rozmiar pliku (rozmiar danych)
		std::array<u_int, BLOCK_INDEX_NUMBER> directBlocks;	//Bezpoœrednie indeksy
		u_int singleIndirectBlocks; //Indeks bloku indeksowego, zpisywanego na dysku

		//Dodatkowe informacje
		tm creationTime = tm();		//Czas i data utworzenia
		tm modificationTime = tm(); //Czas i data ostatniej modyfikacji pliku

		//Synchronizacja
		Semaphore sem;

		Inode();

		virtual ~Inode() = default;

		void clear();
	};

	struct Disk {
		//Tablica reprezentuj¹ca przestrzeñ dyskow¹ (jeden indeks - jeden bajt)
		std::array<char, DISK_CAPACITY> space;

		//----------------------- Konstruktor -----------------------
		Disk();

		//-------------------------- Metody -------------------------
		void write(const u_short_int& begin, const std::string& data);
		void write(const u_short_int& begin, const std::array<u_int, BLOCK_SIZE / 2>& data);

		const std::string read_str(const u_int& begin) const;
		const std::array<u_int, BLOCK_SIZE / 2> read_arr(const u_int& begin) const;
	} disk; //Struktura dysku
	struct FileSystem {
		u_int freeSpace{ DISK_CAPACITY }; //Zawiera informacje o iloœci wolnego miejsca na dysku (bajty)

		//Wektor bitowy bloków (domyœlnie: 0 - wolny blok, 1 - zajêty blok)
		std::bitset<DISK_CAPACITY / BLOCK_SIZE> bitVector;

		/**
		 Tablica i-wêz³ów
		 */
		std::array<Inode, INODE_NUMBER_LIMIT> inodeTable;
		//Pomocnicza tablica 'zajêtoœci' i-wêz³ów (1 - zajêty, 0 - wolny).
		std::bitset<INODE_NUMBER_LIMIT> inodeBitVector;
		std::unordered_map<std::string, u_int> rootDirectory;

		FileSystem();

		const u_int get_free_inode_id();

		void reset();
	} fileSystem; //System plików

	class FileIO {
	private:
#define READ_FLAG 0
#define WRITE_FLAG 1

		std::string buffer;
		u_short_int readPos = 0;
		Disk* disk;
		Inode* file;

		bool readFlag;
		bool writeFlag;

	public:
		FileIO() : disk(nullptr), file(nullptr), readFlag(false), writeFlag(false) {}
		FileIO(Disk* disk, Inode* inode, const std::bitset<2>& mode) : disk(disk), file(inode),
			readFlag(mode[READ_FLAG]), writeFlag(mode[WRITE_FLAG]) {}

		void buffer_update(const int8_t& blockNumber);

		std::string read(const u_short_int& byteNumber);
		std::string read_all();
		void reset_read_pos() { readPos = 0; }

		void write(const std::vector<std::string>& dataFragments, const int8_t& startIndex) const;

		const std::bitset<2> get_flags() const;
	};



	//------------------- Definicje zmiennych -------------------
	bool messages = false; //Zmienna do w³¹czania/wy³¹czania powiadomieñ
	bool detailedMessages = false; //Zmienna do w³¹czania/wy³¹czania szczegó³owych powiadomieñ
	//std::unordered_map<std::string, u_int> usedFiles;

	//Mapa dostêpu dla poszczególnych plików i procesów
	//Klucz   - para nazwa pliku, nazwa procesu
	//Wartoœæ - semafor przypisany danemu procesowi
	std::map<std::pair<std::string, std::string>, FileIO> accessedFiles;

	//Mapa semaforów dla poszczególnych procesów
	//Klucz   - para nazwa pliku, nazwa procesu
	//Wartoœæ - semafor przypisany danemu procesowi
	std::map<std::pair<std::string, std::string>, Semaphore> fileSemaphores;


public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Przypisuje do obecnego katalogu katalog g³ówny.
	*/
	explicit FileManager() = default;



	//-------------------- Podstawowe Metody --------------------
	/**
		Tworzy plik o podanej nazwie w obecnym katalogu.\n
		Po stworzeniu plik jest otwarty w trybie do zapisu.

		@param name Nazwa pliku.
		@param procName Nazwa procesu tworz¹cego.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_create(const std::string& name, const std::string& procName);

	/**
		Zapisuje podane dane w danym pliku usuwaj¹c poprzedni¹ zawartoœæ.

		@param name Nazwa pliku.
		@param data Dane do zapisu.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_write(const std::string& name, const std::string& procName, const std::string& data);

	/**
		Dopisuje podane dane na koniec pliku.

		@param name Nazwa pliku.
		@param data Dane do zapisu.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_append(const std::string& name, const std::string& procName, const std::string& data);

	/**
		Odczytuje podan¹ liczbê bajtów z pliku. Po odczycie przesuwa siê wskaŸnik odczytu.\n
		Aby zresetowaæ wskaŸnik odczytu nale¿y ponownie otworzyæ plik.

		@param name Nazwa pliku.
		@param byteNumber Iloœæ bajtów do odczytu.
		@param result Miejsce do zapisania odczytanych danych.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_read(const std::string& name, const std::string& procName, const u_short_int& byteNumber, std::string& result);

	/**
		Odczytuje ca³e dane z pliku.

		@param name Nazwa pliku.
		@param result Miejsca do zapisania odczytanych danych.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_read_all(const std::string& name, const std::string& procName, std::string& result);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.\n
		Plik jest wymazywany z katalogu g³ównego oraz wektora bitowego.

		@param name Nazwa pliku.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_delete(const std::string& name, const std::string& procName);

	/**
		Otwiera plik z podanym trybem dostêpu:
		- R (read) - do odczytu
		- W (write) - do zapisu
		- RW (read/write) - do odczytu i zapisu

		@param name Nazwa pliku.
		@param procName Nazwa procesu tworz¹cego.
		@param mode Tryb dostêpu do pliku.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_open(const std::string& name, const std::string& procName, const unsigned int& mode);

	/**
		Zamyka plik o podanej nazwie.

		@param name Nazwa pliku.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_close(const std::string& name, const std::string& procName);



	//--------------------- Dodatkowe metody --------------------

	/**
		Formatuje dysk. Zeruje wektor bitowy, usuwa wszystkie i-wêz³y,
		tworzy nowy katalog g³ówny.

		@return void.
	*/
	bool disk_format();

	/**
		Tworzy plik o podanej nazwie w obecnym katalogu i zapisuje w nim podane dane.
		Po stworzeniu plik jest otwarty w trybie do zapisu.

		@param name Nazwa pliku.
		@param procName Nazwa procesu tworz¹cego.
		@param data Dane typu string.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_create(const std::string& name, const std::string& procName, const std::string& data);

	/**
		Zmienia nazwê pliku o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param newName Zmieniona nazwa pliku.
		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_rename(const std::string& name, const std::string& procName, const std::string& newName);

	/**
		Zamyka wszystkie pliki.

		@return Kod b³êdu. 0 oznacza brak b³êdu.
	*/
	int file_close_all();

	/**
		Zmienia zmienn¹ odpowiadaj¹c¹ za wyœwietlanie komunikatów.
		false - komunikaty wy³¹czone.
		true - komunikaty w³¹czone.

		@param onOff Czy komunikaty maj¹ byæ w³¹czone.
		@return void.
	*/
	void set_messages(const bool& onOff);

	/**
		Zmienia zmienn¹ odpowiadaj¹c¹ za wyœwietlanie szczegó³owych komunikatów.
		false - komunikaty wy³¹czone.
		true - komunikaty w³¹czone.

		@param onOff Czy komunikaty maj¹ byæ w³¹czone.
		@return void.
	*/
	void set_detailed_messages(const bool& onOff);



	//----------------- Metody do synchronizacji ----------------

	/**
		Zwraca g³ówny semafor dla pliku.

		@param fileName Nazwa pliku.
		@param sem Zmienna do zapisania w niej semafora.
		@return Kod b³êdu.
	*/
	int file_get_semaphore(const std::string& fileName, Semaphore& sem);

	/**
		Zwraca semafor dla pliku przypisany do procesu.

		@param fileName Nazwa pliku.
		@param procName Nazwa procesu.
		@param sem Zmienna do zapisania w niej semafora.
		@return Semafor pliku dla danego procesu.
	*/
	int file_get_semaphore(const std::string& fileName, const std::string& procName, Semaphore& sem);



	//------------------ Metody do wyœwietlania -----------------

	/**
		Wyœwietla parametry systemu plików.

		@return void.
	*/
	static void display_file_system_params();

	/**
		Wyœwietla informacje o wybranym katalogu.

		@return void.
	*/
	void display_root_directory_info();

	/**
		Wyœwietla informacje o pliku.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	int display_file_info(const std::string& name);

	/**
		Wyœwietla strukturê katalogów.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	void display_root_directory();

	/**
		Wyœwietla zawartoœæ dysku jako znaki.
		'.' - puste pole.

		@return void.
	*/
	void display_disk_content_char();

	/**
		Wyœwietla wektor bitowy.

		@return void.
	*/
	void display_bit_vector();


private:
	//------------------- Metody Sprawdzaj¹ce -------------------

	const bool check_if_name_used(const std::string& name);

	const bool check_if_enough_space(const u_int& dataSize) const;



	//-------------------- Metody Obliczaj¹ce -------------------

	static const u_int calculate_needed_blocks(const size_t& dataSize);

	const size_t calculate_directory_size();

	const size_t calculate_directory_size_on_disk();



	//--------------------- Metody Alokacji ---------------------

	void file_truncate(Inode* file, const u_int& neededBlocks);

	void file_add_indexes(Inode* file, const std::vector<u_int>& blocks);

	void file_deallocate(Inode* file);

	void file_allocate_blocks(Inode* file, const std::vector<u_int>& blocks);

	void file_allocation_increase(Inode* file, const u_int& neededBlocks);

	const std::vector<u_int> find_unallocated_blocks_fragmented(u_int blockNumber);

	const std::vector<u_int> find_unallocated_blocks_best_fit(const u_int& blockNumber);

	const std::vector<u_int> find_unallocated_blocks(const u_int& blockNumber);



	//----------------------- Metody Inne -----------------------

	std::string get_file_data_block(Inode* file, const int8_t& indexNumber) const;

	void file_write(Inode* file, FileIO* IO, const std::string& data);

	void file_append(Inode* file, FileIO* IO, const std::string& data);

	static const tm get_current_time_and_date();

	void change_bit_vector_value(const u_int& block, const bool& value);

	static const std::vector<std::string> fragmentate_data(const std::string& data);
};

#endif //SEXYOS_FILEMANAGER_H
