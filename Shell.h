#pragma once
#include <vector>
#include <string>

#pragma comment(lib, "Winmm.lib")

class Shell {
private:
	bool status;														//Status uzależniający działanie pętli shella
	std::string line;													//Dane odczytane z bufora
	std::vector<std::string> parsed;									//Tablica z podzielonym poleceniem

public:
	Shell();
	~Shell() = default;

	//Metody pracy shella
	void boot();			//Funckja startująca pętlę shella

private:
	static void logo();		//Wyświetlanie loga systemu
	void loop();			//Pętla główna shella
	void read_line();		//Odczyt surowych danych
	void parse();			//Parsowanie komendy
	void execute(); 		//Wykonywanie
	static void notRecognized();
	
	//Metody interpretera
	static void go();		//Następny krok pracy krokowej
	void showregs() const;		//Wyświetla stan rejestrów i licznik rozkazów
	
	//Metody shella
	static void ver();		//Creditsy
	static void help();		//Wyświetlanie listy poleceń
	void exit();			//Kończenie pracy systemu
	void cls() const;		//Czyszczenie konsoli
	
	//Metody zarządzania procesami
	void showpcb();		//Wyświetla informacje o procesie
	void cp();				//Tworzenie procesu
	void showpcblist() const;		//Lista PCB wszystkich procesów
	void showtree() const;		//Drzewo procesow
	void dp();				//Usuwanie procesu
	
	//Metody potoki
	static void showpipe();	//Wyświetla wszystkie istniejące potoki

	//Metody dyskowe
	void showroot() const;		//Listowanie katalogu
	void cf();				//Utworzenie pliku
	void df();				//Usunięcie pliku
	void showblock();				//Listowanie zawartości wskazanego bloku dyskowego
	void wf();				//Zapis do pliku
	void af();				//Dopis do pliku
	void fo();				//Otwarcie pliku
	void fr();				//Odczyt pliku
	void fc();				//Zamkniecie pliku
	void finfo();			//Wyświetla informacje o pliku
	void dinfo() const;		//Wyświetla informacje o katalogu głównym
	void showdisk() const;	//Wyświetla zawartość dysku jako znaki
	void fsysparam() const;	//Wyświetla parametry systemu plików
	void bitvector() const;	//Wyświetla wektor bitowy
	
	//Metody pamieci
	void showmem();				//Wyswietlanie zawartości RAM
	void showpagefile() const;	//Wyświetlanie pliku stronnicowania
	void showpagetable();		//Wyświetla tablicę wymiany stronnic
	void showstack() const;		//Pokazuje kolejkę FIFO wymiany stronnic
	void showframes() const;	//Pokazuje ramki w pamięci RAM wraz ze szczegółami

	//Easter Egg
	static void thanks();
};
