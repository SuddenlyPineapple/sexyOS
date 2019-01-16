#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "FileManager.h"
#include "Interpreter.h"
#include "MemoryManager.h"
#include "Procesy.h"
#include "Planista.h"
#include "pipe.h"

#pragma comment(lib, "Winmm.lib")

using namespace std;

class Shell {
private:
	bool status;														//Status uzależniający działanie pętli shella
	string line;														//Dane odczytane z bufora
	vector<string> parsed;												//Tablica z podzielonym poleceniem

	//Modules
	MemoryManager mm;
	Planista p;
	proc_tree tree;
	FileManager fm;
	Pipeline pipel;
	Interpreter inter;

public:
	Shell() : mm(), p(), pipel(nullptr), tree(&mm, &p, &pipel), fm(&p, &tree), inter(&fm, &mm, &tree, &pipel) {
		this->pipel.tree = &tree;
        this->status = true;
		this->parsed.resize(0);
		this->line.clear();
	}
	//Metody pracy shella
	void boot();														//Funckja startująca pętlę shella
	void logo();														//Wyświetlanie loga systemu
	void loop();														//Pętla shella
	void read_line();													//Odczyt surowych danych
	void parse();														//Parsowanie
	void execute(); 													//Wykonywanie
	//Commands functions declarations
	//Metody interpretera
	void go(); 													        //Nastepny krok pracy krokowej
	//Metody shella
    void ver();                                                       //Creditsy
	void help();														//Wyswietalnie listy poleceń
    void exit();														//Kończenie pracy
	void cls();															//Czyszczenie ekranu
    //Metody zarzadzania procesami
	void cp();															//Tworzenie procesu
	void lp();															//Lista PCB wszystkich procesów
	void lt();                                                        //Drzewo procesow
	void dp();                                                        //Usuwanie procesu
	//Metody dyskowe
	void ls();															//Listowanie katalogu
	void cf();															//Utworzenie pliku
	void df();															//Usunięcie pliku
	void ld();															//Listowanie zawartości wskazanego bloku dyskowego
    void wf();                                                        //Zapis do pliku
    void fo();                                                        //Otwarcie pliku
    void fc();                                                        //Zamkniecie pliku
    void finfo();
    void dinfo();
    void dskchar();
	void fsysparam();
	void bitvector();
    //Metody pamieci
    void showmem();                                                      //Wyswietlanie zawartosci pamieci
    void showpagefile();
    void showpagetable();
    void showstack();
    void showframes();

    //Specials
    void thanks();





	~Shell() {}
};


