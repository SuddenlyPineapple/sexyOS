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
	- zabezpieczenia + flagi w plikach
	- otwieranie i zamykanie pliku
	- zapisywanie plików z kodem asemblerowym (opcjonalne)
	- pliki executable (opcjonalne)
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
		Zamienia string na int. Do u¿ycia w przypadku odczytywania liczby z dysku.

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



	//---------------- Definicje struktur i klas ----------------

	//Klasa indeksu - przechowuje indeks bloku dyskowego.
	class Index {
	public:
		u_int value; //Wartoœæ indeksu

		Index() : value(NULL) {}
		explicit Index(const u_int& value) : value(value) {}
		virtual ~Index() = default;
	};

	//Klasa bloku indeksowego - przechowuje tablicê indeksów.
	class IndexBlock : public Index {
	private:
		//Tablica indeksów/bloków indeksowych
		std::array<std::shared_ptr<Index>, BLOCK_INDEX_NUMBER> block;
	public:
		IndexBlock() = default;
		virtual ~IndexBlock() = default;

		const u_int size() const { return block.size(); }
		void clear() { std::fill(block.begin(), block.end(), nullptr); }

		std::shared_ptr<Index>& operator [] (const size_t& index);
		const std::shared_ptr<Index>& operator [] (const size_t& index) const;
	};

	//Klasa i-wêz³a - zawiera podstawowe informacje o pliku
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
		explicit Inode(std::string type_) : type(std::move(type_)), creationTime(FileManager::GetCurrentTimeAndDate()) {}
		virtual ~Inode() = default;
	};

	//Klasa pliku dziedzicz¹ca po i-wêŸle - zawiera informacje specyficzne dla pliku
	class File : public Inode {
	public:
		//Podstawowe informacje
		size_t blocksOccupied;   //Iloœæ zajmowanych bloków
		size_t sizeOnDisk;       //Rozmiar pliku na dysku
		IndexBlock directBlocks; //Bezpoœrednie bloki (na koñcu 1 blok indeksowy 1-poziomu)

		//Dodatkowe informacje
		std::string creator;
		tm modificationTime; //Czas i data ostatniej modyfikacji pliku

		/**
			Flagi znaczenie:

			indeks 0 - flaga plik otwarty

			indeks 1 - flaga odczytu

			indeks 2 - flaga zapisu

			Domyœlnie plik jest zamkniêty i ma ustawione prawa odczytu i zapisu.
		 */
		std::bitset<3> flags;

		/**
			Konstruktor domyœlny.
		*/
		File() : Inode("FILE"), blocksOccupied(0), sizeOnDisk(0), modificationTime(creationTime) {}

		virtual ~File() = default;
	};

	//Klasa katalogu dziedzicz¹ po i-wêŸle - zawiera informacje specyficzne dla katalogu
	class Directory : public Inode {
	public:
		std::unordered_map<std::string, std::string> files; //Tablica hashowa Inode

		/**
			Konstruktor inicjalizuj¹cy nadrzêdny Inode typem "DIRECTORY".
		*/
		explicit Directory() : Inode("DIRECTORY") {}
		virtual ~Directory() = default;
	};

	//Prosta klasa dysku (imitacja fizycznego) - przechowuje dane + system plików
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
			FileSystem();
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
		void write(const u_int& begin, const std::string& data);

		/**
			Odczytuje dane zadanego typu (jeœli jest on zaimplementowany) w wskazanym przedziale.

			@param begin Indeks od którego dane maj¹ byæ odczytywane.
			@param end Indeks do którego dane maj¹ byæ odczytywane.
			@return zmienna zadanego typu.
		*/
		template<typename T>
		const T read(const u_int& begin) const;
	} DISK;



	//------------------- Definicje zmiennych -------------------
	bool messages = false; //Zmienna do w³¹czania/wy³¹czania powiadomieñ
	bool detailedMessages = false; //Zmienna do w³¹czania/wy³¹czania szczegó³owych powiadomieñ
	std::string currentDirectory; //Obecnie u¿ywany katalog
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
	bool FileCreate(const std::string& name);

	/**
		Zapisuje podane dane w danym pliku.

		@param name Nazwa pliku.
		@param data Dane do zapisu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool FileWriteData(const std::string& name, const std::string& data);

	/**
		Odczytuje dane z podanego pliku.

		@param name Nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	const std::string FileReadData(const std::string& name);

	/**
		Usuwa plik o podanej nazwie znajduj¹cy siê w obecnym katalogu.
		Plik jest wymazywany z tablicy i-wêz³ów oraz wektora bitowego.

		@param name Nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool FileDelete(const std::string& name);

	bool FileOpen(const std::string& name);

<<<<<<< HEAD
	bool FileClose(const std::string& path);
=======
	bool FileClose(const std::string& name);
>>>>>>> 3b567d58d87389659a9092de5f02974fba4e26d7

	/**
		Ustawia zestaw flag w pliku o podanej nazwie dla podanego u¿ytkownika.

		@param name Nazwa pliku.
		@param user U¿ytkownik do jakiego chcemy przypisaæ flagi.
		@param read Flaga odczytu.
		@param write Flaga zapisu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool FileSetFlags(const std::string& name, const std::string& user, const bool& read, const bool& write);

	/**
		Tworzy nowy katalog w obecnym katalogu.

		@param name Nazwa katalogu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryCreate(std::string name);

	/**
		Usuwa katalog o podanej nazwie.

		@param name Nazwa katalogu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryDelete(std::string name);

	/**
		Przechodzi z obecnego katalogu wskazanego katalogu.

		@param path œcie¿ka katalogu, do którego chcemy przejœæ.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryChange(std::string path);

	/**
		Przechodzi z obecnego katalogu do katalogu nadrzêdnego.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryUp();

	/**
		Przechodzi z obecnego katalogu do katalogu podrzêdnego o podanej nazwie.

		@param name Nazwa katalogu.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryDown(std::string name);



	//--------------------- Dodatkowe metody --------------------

	/**
		Formatuje dysk. Zeruje wektor bitowy, usuwa wszystkie i-wêz³y,
		tworzy nowy katalog g³ówny.

		@return void.
	*/
	void DiskFormat();

	/**
		Tworzy plik o podanej nazwie w obecnym katalogu i zapisuje w nim podane dane.

		@param name Nazwa pliku.
		@param data Dane typu string.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool FileCreate(const std::string& name, const std::string& data);

	bool FilePIDSet(const std::string& name, const u_int& pid);

	u_int FilePIDGet(const std::string& path);

	/**
		Zmienia nazwê katalogu (w obecnej œcie¿ce) o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param changeName Zmieniona nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool FileRename(const std::string& name, const std::string& changeName);

	/**
		Zmienia nazwê pliku (w obecnej œcie¿ce) o podanej nazwie.

		@param name Obecna nazwa pliku.
		@param changeName Zmieniona nazwa pliku.
		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DirectoryRename(std::string name, std::string changeName);

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
	void Messages(const bool& onOff);

	/**
		Zmienia zmienn¹ odpowiadaj¹c¹ za wyœwietlanie szczegó³owych komunikatów.
		false - komunikaty wy³¹czone.
		true - komunikaty w³¹czone.

		@param onOff Czy komunikaty maj¹ byæ w³¹czone.
		@return void.
	*/
	void DetailedMessages(const bool& onOff);

	/**
		Zwraca obecnie u¿ywan¹ œcie¿kê.

		@return Obecna œcie¿ka z odpowiednim formatowaniem.
	*/
	const std::string GetCurrentPath() const;



	//------------------ Metody do wyœwietlania -----------------

	/**
		Wyœwietla parametry systemu plików.

		@return void.
	*/
	void DisplayFileSystemParams() const;

	/**
		Wyœwietla informacje o wybranym katalogu.

		@return void.
	*/
	bool DisplayDirectoryInfo(std::string name);

	/**
		Wyœwietla informacje o pliku.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	bool DisplayFileInfo(const std::string& name);

	/**
		Wyœwietla strukturê katalogów.

		@return True, jeœli operacja siê uda³a lub false, jeœli operacja nie powiod³a siê.
	*/
	void DisplayDirectoryStructure();

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
		@return Prawda, jeœli nazwa u¿ywana, inaczej fa³sz.
	*/
	const bool CheckIfNameUsed(const std::string& directory, const std::string& name);

	/**
		Sprawdza czy jest miejsce na dane o zadaniej wielkoœci.

		@param dataSize Rozmiar danych, dla których bêdziemy sprawdzac miejsce.
		@return void.
	*/
	const bool CheckIfEnoughSpace(const u_int& dataSize) const;



	//-------------------- Metody Obliczaj¹ce -------------------

	/**
		Oblicza ile bloków zajmie podany string.

		@param dataSize D³ugoœæ danych, których rozmiar na dysku bêdzie obliczany.
		@return Iloœæ bloków jak¹ zajmie string.
	*/
	const u_int CalculateNeededBlocks(const size_t& dataSize) const;

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



	//--------------------- Metody Alokacji ---------------------

	/**
		Zmniejsza lub zwiêksza plik do podanego rozmiaru.

		@param file WskaŸnik na plik, którego rozmiar chcemy zmieniæ.
		@param neededBlocks Iloœæ bloków do alokacji.
		@return void.
	*/
	void FileTruncate(std::shared_ptr<File> file, const u_int& neededBlocks);

	/**
		Dodaje do pliku podane indeksy bloków.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param blocks Numery bloków do alokacji.
		@return void.
	*/
	void FileAddIndexes(const std::shared_ptr<File>& file, const std::vector<u_int>& blocks) const;

	/**
		Przeprowadza dealokacje danych pliku, czyli usuwa z pliku indeksy bloków
		oraz zmienia wartoœci w wektorze bitowym.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@return void.
	*/
	void FileDeallocate(const std::shared_ptr<File>& file);

	/**
		Alokuje przestrzeñ na podane bloki. Zmienia wartoœci w wektorze bitowym,
		aktualizuje wartoœæ zajmowanych bloków przez plik oraz wywo³uje funkcjê
		FileAddIndexes.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param blocks Numery bloków do alokacji.
		@return void.
	*/
	void FileAllocateBlocks(const std::shared_ptr<File>& file, const std::vector<u_int>& blocks);

	/**
		Obs³uguje proces zwiêkszania liczby zaalokowanych bloków na dane pliku.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param neededBlocks Liczba bloków do alokacji.
		@return void.
	*/
	void FileAllocationIncrease(std::shared_ptr<File>& file, const u_int& neededBlocks);

	/**
		Obs³uguje proces zwiêkszania liczby zaalokowanych bloków na dane pliku.

		@param file WskaŸnik na plik, do którego chcemy dodaæ indeksy.
		@param neededBlocks Liczba bloków do alokacji.
		@return void.
	*/
	void FileAllocationDecrease(const std::shared_ptr<File>& file, const u_int& neededBlocks);

	/**
	Znajduje nieu¿ywane bloki do zapisania pliku bez dopasowania do luk w blokach

	@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
	@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocksFragmented(u_int blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku metod¹ best-fit.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocksBestFit(const u_int& blockNumber);

	/*
		Znajduje nieu¿ywane bloki do zapisania pliku. Najpierw uruchamia funkcjê
		dzia³aj¹c¹ metod¹ best-fit, jeœli funkcja nie znajdzie dopasowania to
		uruchamiana jest funkcja znajduj¹c¹ pierwsze jakiekolwiek wolne bloki i
		wprowadzaj¹ca fragmentacjê danych.

		@param blockNumber Liczba bloków na jak¹ szukamy miejsca do alokacji.
		@return Zestaw indeksów bloków mo¿liwych do zaalokowania.
	*/
	const std::vector<u_int> FindUnallocatedBlocks(const u_int& blockNumber);



	//----------------------- Metody Inne -----------------------

	/**
		Zapisuje wektor fragmentów File.data na dysku.

		@param file WskaŸnik na plik którego dane bêd¹ zapisane na dysku.
		@param data Dane do zapisania na dysku.
		@return void.
	*/
	void FileWriteData(std::shared_ptr<File>& file, const std::string& data);

	/**
		Wczytuje dane pliku z dysku.

		@param file WskaŸnik na plik którego dane maj¹ byæ wczytane z dysku.
		@return Dane pliku w postaci string.
	*/
	const std::string FileReadData(const std::shared_ptr<File>& file) const;

	/**
		Usuwa wskazany plik.

		@param file WskaŸnik pliku do usuniêcia.
		@return void.
	*/
	void FileDelete(std::shared_ptr<File>& file);

	/**
		Usuwa ca³¹ strukturê katalogów.

		@param directory Katalog do usuniêcia.
		@return Rozmiar podanego katalogu.
	*/
	void DirectoryDeleteStructure(std::shared_ptr<Directory>& directory);

	/**
		Aktualizuje œcie¿ki w ca³ej strukturze katalogów.

		@param directory WskaŸnik na katalog, którego œcie¿ki chcemy zauktualizowaæ.
		@return Rozmiar podanego katalogu.
	*/
	void DirectoryRenameStructure(std::shared_ptr<Directory>& directory);

	/**
		Wyœwietla rekurencyjnie katalog i jego podkatalogi.

		@param directory Katalog szczytowy do wyœwietlenia.
		@param level Poziom obecnego katalogu w hierarchii katalogów.
		@return void.
	*/
	void DisplayDirectory(const std::shared_ptr<Directory>& directory, u_int level);

	/**
		Usuwa i-wêze³ z tablicy i-wêz³ów.

		@param inode WskaŸnik na i-wêze³ do usuniêcia.
		@return void.
	*/
	void InodeTableRemove(const std::shared_ptr<Inode>& inode);

	/**
		Zwraca œcie¿kê podanego katalogu.

		@param directory WskaŸnik na katalog którego œciê¿kê chcemy otrzymaæ.
		@return Œcie¿ka podanego katalogu.
	*/
	const std::string GetPath(const std::shared_ptr<Directory>& directory);

	/**
		Zwraca œcie¿kê katologu nadrzêdnego wzglêdem obecnego katalogu.

		@return Œcie¿ka katalogu nadrzêdnego.
	*/
	const std::string GetCurrentDirectoryParent() const;

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
		Zmienia wartoœæ w wektorze bitowym i zarz¹dza polem freeSpace
		w strukturze FileSystem.

		@param block Indeks bloku, którego wartoœæ w wektorze bitowym bêdzie zmieniona.
		@param value Wartoœæ do przypisania do wskazanego bloku ( BLOCK_FREE lub BLOCK_OCCUPIED)
		@return void.
	*/
	void ChangeBitVectorValue(const u_int& block, const bool& value);

	/**
		Dzieli string na fragmenty o rozmiarze BLOCK_SIZE.

		@param data String do podzielenia na fragmenty.
		@return Wektor fragmentów string.
	*/
	const std::vector<std::string> DataToDataFragments(const std::string& data) const;
};

static FileManager fileManager;

#endif //SEXYOS_FILEMANAGER_H
