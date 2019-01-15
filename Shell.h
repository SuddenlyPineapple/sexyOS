#pragma once
#include <vector>
#include <string>

using namespace std;

class Shell {
private:
	bool status;														//Status uzależniający działanie pętli shella
	string line;														//Dane odczytane z bufora
	vector<string> parsed;												//Tablica z podzielonym poleceniem
	enum spis_funkcji {													//Lista poleceń
		HELP, CP, LP, LS, CF, DF, LD, EXIT
	};												
public:

	//Metody pracy shella
	void execute(/*Interpreter inter, ....., ProcessManager procmem*/); //Wykonywanie
	void parse();														//Parsowanie
	void read_line();													//Odczyt surowych danych
	void loop(/*Interpreter inter, ....., ProcessManager procmem*/);	//Pętla shella
	void boot();														//Funckja startująca pętlę shella
	void logo();														//Wyświetlanie loga systemu

	//Commands functions declarations

    //Metody shella
	void help();														//Wyswietalnie listy poleceń
    void exit();														//Kończenie pracy
    //Metody interpretera
    void go();															//Nastepny krok pracy krokowej
	//Metody zarzadzania procesami
	void cp(/*ProcessManager procmem*/);								//Tworzenie procesu
	void lp(/*ProcessManager procmem*/);								//Lista PCB wszystkich procesów
	void lt();                                                        //Drzewo procesow
	void dp();                                                        //Usuwanie procesu
	//Metody dyskowe
	void ls(/*HDD hdd*/);												//Listowanie katalogu
	void cf(/*HDD hdd*/);												//Utworzenie pliku
	void df(/*HDD hdd*/);												//Usunięcie pliku
	void ld(/*HDD hdd*/);												//Listowanie zawartości wskazanego bloku dyskowego
    void rf();                                                        //Zmiana nazwy pliku
    void wf();                                                        //Zapis do pliku
    void fo();                                                        //Otwarcie pliku
    void fc();                                                        //Zamkniecie pliku
    //Metody pamieci
    void dmem();                                                      //Wyswietlanie zawartosci pamieci







};


