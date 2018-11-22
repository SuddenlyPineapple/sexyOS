/**
	SexyOS
	FileManager.h
	Przeznaczenie: Zawiera klasy Disk i FileManager oraz deklaracje metod i konstruktorów

	@author Tomasz Kiljañczyk
	@version 02/11/18
*/

#ifndef SEXYOS_FILEMANAGER_H
#define SEXYOS_FILEMANAGER_H

#define _CRT_SECURE_NO_WARNINGS

#include <ctime>
#include <string>
#include <array>
#include <bitset>
#include <vector>
#include <unordered_map>

/*
	To-do:
	- przerobiæ zapisywanie i odczytywanie z nazw na œcie¿ki plików
	- dodaæ zabezpieczenie do zapisywania i odczytywania danych tylko dla plików otwartych
*/

//Klasa zarz¹dcy przestrzeni¹ dyskow¹ i systemem plików
class FileManager {
private:
	using u_int = unsigned int;

	//--------------- Definicje sta³ych statycznych -------------
	static const size_t BLOCK_SIZE = 32;	   	    //Rozmiar bloku (bajty)
	static const size_t DISK_CAPACITY = 1024;       //Pojemnoœæ dysku (bajty)
	static const u_int BLOCK_INDEX_NUMBER = 3;	    //Wartoœæ oznaczaj¹ca d³ugoœæ pola blockDirect i bloków niebezpoœrednich
	static const u_int INODE_NUMBER_LIMIT = 32;     //Maksymalna iloœæ elementów w katalogu
	static const u_int MAX_FILENAME_LENGTH = 7;        //Maksymalna d³ugoœæ œcie¿ki
	static const bool BLOCK_FREE = false;           //Wartoœæ oznaczaj¹ca wolny blok
	static const bool BLOCK_OCCUPIED = !BLOCK_FREE; //Wartoœæ oznaczaj¹ca zajêty blok
	/**Maksymalny rozmiar pliku obliczony na podstawie maksymalnej iloœci indeksów*/
	static const size_t MAX_FILE_SIZE = (BLOCK_INDEX_NUMBER * 2) * BLOCK_SIZE;



	//---------------- Definicje struktur i klas ----------------

	//Klasa bloku indeksowego - przechowuje tablicê indeksów.
	class IndexBlock {
	private:
		//Tablica indeksów/bloków indeksowych
		std::array<u_int, BLOCK_INDEX_NUMBER> value;
	public:
		IndexBlock() = default;
		virtual ~IndexBlock() = default;

		const u_int size() const { return value.size(); }
		void clear();

		u_int& operator [] (const size_t& index);
		const u_int& operator [] (const size_t& index) const;
	};

	//Klasa i-wêz³a - zawiera podstawowe informacje o pliku
	class Inode {
	public:
		//Podstawowe informacje
		size_t blocksOccupied = 0;   //Iloœæ zajmowanych bloków
		size_t realSize = 0;         //Rzeczywisty ozmiar pliku
		std::array<u_int, BLOCK_INDEX_NUMBER> directBlocks; //Bezpoœrednie bloki
		IndexBlock singleIndirectBlocks; //Niebespoœredni blok indeksowy, zpisywany na dysku

		//Dodatkowe informacje
		tm creationTime = tm();	 //Czas i data utworzenia
		tm modificationTime = tm(); //Czas i data ostatniej modyfikacji pliku

		bool flagOpen = false; //Flaga otwarcia (true - plik otwarty, false - plik zamkniêty)

		Inode();

		/**
			Konstruktor domyœlny.
		*/
		explicit Inode(std::string type_) : directBlocks() {}

		virtual ~Inode() = default;

		void clear();
	};


	//Prosta klasa dysku (imitacja fizycznego) - przechowuje dane + system plików
	class Disk {
	public:
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
			@param data Dane typu string.
			@return void.
		*/
		void write(const u_int& begin, const std::string& data);

		/**
			Odczytuje dane zadanego typu (jeœli jest on zaimplementowany) w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const u_int& begin) const;
	} DISK;
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

		/**
			Konstruktor domyœlny. Wpisuje katalog g³ówny do tablicy iWêz³ów.
		*/
		FileSystem();

		const u_int get_free_inode_id();

		void reset();
	} FileSystem; //System plików FileSystem


	//------------------- Definicje zmiennych -------------------
	bool messages = false; //Zmienna do w³¹czania/wy³¹czania powiadomieñ
	bool detailedMessages = false; //Zmienna do w³¹czania/wy³¹czania szczegó³owych powiadomieñ
	std::unordered_map<std::string, u_int> usedFiles;


public:
	//----------------------- Konstruktor -----------------------
	/**
		Konstruktor domyœlny. Przypisuje do obecnego katalogu katalog g³ówny.
	*/
	FileManager();



	//-------------------- Podstawowe Metody --------------------
	/**
		Tworzy plik o podanej nazwie w obecnym katalogu.

		@param name Nazwa pliku.
		@return True, jeœli operacja siê uda³a i false, jeœli operacja nie powiod³a siê.
	*/
	bool file_create(const std::string& name);

	/**
		Zapisuje podane dane w danym pliku.

		@param name Nazwa pliku.
		@param data Dane do zapisu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool file_write(const std::string& name, const std::string& data);

	/**
		Odczytuje dane z podanego pliku.

		@param name Nazwa pliku.
		@return Wczytane dane.
	*/
	const std::string file_read_all(const std::string& name);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.
		Plik jest wymazywany z tablicy i-wêz³ów oraz wektora bitowego.

		@param name Nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool file_delete(const std::string& name);

	bool file_open(const std::string& name);

	bool file_close(const std::string& path);



	//--------------------- Dodatkowe metody --------------------

	/**
		Formatuje dysk. Zeruje wektor bitowy, usuwa wszystkie i-wêz³y,
		tworzy nowy katalog g³ówny.

		@return void.
	*/
	bool disk_format();

	/**
		Tworzy plik o podanej nazwie w obecnym katalogu i zapisuje w nim podane dane.

		@param name Nazwa pliku.
		@param data Dane typu string.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool file_create(const std::string& name, const std::string& data);

	/**
		Zmienia nazwê katalogu (w obecnej œcie¿ce) o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param changeName Zmieniona nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool file_rename(const std::string& name, const std::string& changeName);

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

	//------------------ Metody do wyœwietlania -----------------

	/**
		Wyœwietla parametry systemu plików.

		@return void.
	*/
	void display_file_system_params() const;

	/**
		Wyœwietla informacje o wybranym katalogu.

		@return void.
	*/
	void display_root_directory_info();

	/**
		Wyœwietla informacje o pliku.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool display_file_info(const std::string& name);

	/**
		Wyœwietla strukturê katalogów.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	void display_root_directory();

	/**
		Wyœwietla zawartoœæ dysku w formie binarnej.

		@return void.
	*/
	void display_disk_content_binary();

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

	/**
		Sprawdza czy nazwa pliku jest u¿ywana w katalogu g³ównym.

		@param name Nazwa pliku
		@return Prawda, jeœli nazwa u¿ywana, inaczej fa³sz.
	*/
	const bool check_if_name_used(const std::string& name);

	/**
		Sprawdza czy jest miejsce na dane o zadaniej wielkoœci.

		@param dataSize Rozmiar danych, dla których bêdziemy sprawdzac miejsce.
		@return void.
	*/
	const bool check_if_enough_space(const u_int& dataSize) const;



	//-------------------- Metody Obliczaj¹ce -------------------

	/**
		Oblicza ile bloków zajmie podany string.

		@param dataSize D³ugoœæ danych, których rozmiar na dysku bêdzie obliczany.
		@return Iloœæ bloków jak¹ zajmie string.
	*/
	const u_int calculate_needed_blocks(const size_t& dataSize) const;

	/**
		Zwraca rozmiar podanego katalogu.

		@return Rozmiar podanego katalogu.
	*/
	const size_t calculate_directory_size();

	/**
		Zwraca rzeczywisty rozmiar podanego katalogu.

		@return Rzeczywisty rozmiar podanego katalogu.
	*/
	const size_t calculate_directory_size_on_disk();



	//--------------------- Metody Alokacji ---------------------

	/**
		Zmniejsza lub zwiêksza plik do podanego rozmiaru.

		@param file WskaŸnik na plik, którego rozmiar chcemy zmieniæ.
		@param neededBlocks Iloœæ bloków do alokacji.
		@return void.
	*/
	void file_truncate(Inode* file, const u_int& neededBlocks);

	/**
		Dodaje do pliku podane indeksy bloków.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param blocks Numery bloków do alokacji.
		@return void.
	*/
	void file_add_indexes(Inode* file, const std::vector<u_int>& blocks) const;

	/**
		Przeprowadza dealokacje danych pliku, czyli usuwa z pliku indeksy bloków
		oraz zmienia wartoœci w wektorze bitowym.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@return void.
	*/
	void file_deallocate(Inode* file);

	/**
		Alokuje przestrzeñ na podane bloki. Zmienia wartoœci w wektorze bitowym,
		aktualizuje wartoœæ zajmowanych bloków przez plik oraz wywo³uje funkcjê
		file_add_indexes.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param blocks Numery bloków do alokacji.
		@return void.
	*/
	void file_allocate_blocks(Inode* file, const std::vector<u_int>& blocks);

	/**
		Obs³uguje proces zwiêkszania liczby zaalokowanych bloków na dane pliku.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param neededBlocks Liczba bloków do alokacji.
		@return void.
	*/
	void file_allocation_increase(Inode* file, const u_int& neededBlocks);

	/**
		Obs³uguje proces zwiêkszania liczby zaalokowanych bloków na dane pliku.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param neededBlocks Liczba bloków do alokacji.
		@return void.
	*/
	void file_allocation_decrease(Inode* file, const u_int& neededBlocks);

	/**
	Znajduje nieu¿ywane bloki do zapisania pliku bez dopasowania do luk w blokach

	@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
	@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> find_unallocated_blocks_fragmented(u_int blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku metod¹ best-fit.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> find_unallocated_blocks_best_fit(const u_int& blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku. Najpierw uruchamia funkcjê
		dzia³aj¹c¹ metod¹ best-fit, jeœli funkcja nie znajdzie dopasowania to
		uruchamiana jest funkcja znajduj¹c¹ pierwsze jakiekolwiek wolne bloki i
		wprowadzaj¹ca fragmentacjê danych.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> find_unallocated_blocks(const u_int& blockNumber);



	//----------------------- Metody Inne -----------------------

	/**
		Zapisuje wektor fragmentów File.data na dysku.

		@param file WskaŸnik na plik którego dane bêd¹ zapisane na dysku.
		@param data Dane do zapisania na dysku.
		@return void.
	*/
	void file_write(Inode* file, const std::string& data);

	/**
		Wczytuje dane pliku z dysku.

		@param file WskaŸnik na plik którego dane maj¹ byæ wczytane z dysku.
		@return Dane pliku w postaci string.
	*/
	const std::string file_read_all(Inode* file) const;

	/**
		Zwraca aktualny czas i datê.

		@return Czas i data.
	*/
	static const tm get_current_time_and_date();

	/**
		Zmienia wartoœæ w wektorze bitowym i zarz¹dza polem freeSpace
		w strukturze FileSystem.

		@param block Indeks bloku, którego wartoœæ w wektorze bitowym bêdzie zmieniona.
		@param value Wartoœæ do przypisania do wskazanego bloku ( BLOCK_FREE lub BLOCK_OCCUPIED)
		@return void.
	*/
	void change_bit_vector_value(const u_int& block, const bool& value);

	/**
		Dzieli string na fragmenty o rozmiarze BLOCK_SIZE.

		@param data String do podzielenia na fragmenty.
		@return Wektor fragmentów string.
	*/
	const std::vector<std::string> data_to_data_fragments(const std::string& data) const;
};

static FileManager fileManager;

#endif //SEXYOS_FILEMANAGER_H
