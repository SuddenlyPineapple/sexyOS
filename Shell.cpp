#include "MemoryManager.h"
#include "Planist.h"
#include "FileManager.h"
#include "Interpreter.h"
#include "Processes.h"
#include "Pipe.h"
#include "Shell.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <filesystem>
#include <windows.h>

using namespace std;

enum concol {
	pink = 13,
	white = 15
};

inline void set_color(int textcol, int backcol) {
	textcol %= 16; backcol %= 16;
	const unsigned short wAttributes = (static_cast<unsigned>(backcol) << 4) | static_cast<unsigned>(textcol);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wAttributes);
}

void sound_bootup() {
	PlaySound(TEXT("Startup.wav"), nullptr, SND_ASYNC);
}

Shell::Shell() {
	this->status = true;
	this->parsed.clear();
	this->line.clear();

	tree.init();
}

//Metody pracy shella
void Shell::boot() {
	set_color(white, pink);
	sound_bootup();
	logo();
	loop();
}

void Shell::logo() {
	system("cls");
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                  `:ossss+-       .+syys+.                                " << std::endl;
	std::cout << "                                                                .yNo.   .hMh`   .dd-  `/M/                                " << std::endl;
	std::cout << "            `:oso/     `:oso.    `/o/     -oo- -oo-      ++/   sMd.      `MMy   dMo     :                                 " << std::endl;
	std::cout << "            +N/..hh   .ym+-yMm   -hsNMs  /ddys`soyMN`    :MM:  yMN`        NMm   oMMh:                                    " << std::endl;
	std::cout << "            mMy`     /Nd` `hMs   .  -NMoho`   `  .MM/   `dMd  :MMo        .MMy    .oNMd/                                  " << std::endl;
	std::cout << "            .hMN+   :MMo+yds-        oMM+         NMy  -mMM:  oMM/        yMN.      `oNMh                                 " << std::endl;
	std::cout << "              :NM+  sMM:.`   `     .sysMN-        yMm`+dmMd   :MMo       sMd.         /MN                                 " << std::endl;
	std::cout << "          +o:.-hd.  :NMy::/os+ `yydd:  yMN+so     :MMNy:MM:    +NN/`  .+md/   .ys:. `:hh.                                 " << std::endl;
	std::cout << "         .+oso/.     .+ss+:`   -so-     /s+.       :+. hMh      `/ossso/`     ./oyyso:`                                   " << std::endl;
	std::cout << "                                                      :Mm`                                                                " << std::endl;
	std::cout << "                                                 `   :my`                                                                 " << std::endl;
	std::cout << "                                               `sNmhy+.                                                                   " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                                        ..----. .. --::.                  " << std::endl;
	std::cout << "                                                                                 .--::::::////:----::/+oso:               " << std::endl;
	std::cout << "                                                                          .-::////////:::::/:/+:-:/::::/+hy:              " << std::endl;
	std::cout << "                                                                      -:////////:-..`    ``-/:.-+o+/:///+hho-             " << std::endl;
	std::cout << "                                                                   -/////////-.            -:`-:/:-:oossshhy:             " << std::endl;
	std::cout << "                                                                  -///+++/-`              ./`.//://o+/oyhdhy/             " << std::endl;
	std::cout << "                                                                   .//++o:`               //`-:://+s+:+ydhhh+             " << std::endl;
	std::cout << "                                                                    `:/++++:.            `/-``-:/+o++oyhdhhho             " << std::endl;
	std::cout << "                                                                      .://+++/-`         `:`---:/osshdhddhyho             " << std::endl;
	std::cout << "                                                                        .:///++/::-.`    --.://+oshdddddhhyhs             " << std::endl;
	std::cout << "                                                                         `.:::/+++o+//:-..--://+osoosyhdhhhhs             " << std::endl;
	std::cout << "                                                                            .-:::/+o+++/--/.-:://---//+syhhhs             " << std::endl;
	std::cout << "                                                                               .-:://:/:..-:-......-://++shhy:            " << std::endl;
	std::cout << "                                                                                 ``--/:...-..--.-/:::///+ooyy/            " << std::endl;
	std::cout << "                                                                                     -.`...-----:+o+////+ooss+-           " << std::endl;
	std::cout << "                                                                                      ``.-:::::::/oyo+/+++osso--          " << std::endl;
	std::cout << "                                                                                  `.-:::://////////+so++++ooss/--         " << std::endl;
	std::cout << "                                                                                 -://////::://///////oo+++++oos/--        " << std::endl;
	std::cout << "                                                                                `-:///////////////////oso++++oos+--       " << std::endl;
	std::cout << "                                                                                ``-://////////////////+syo+++ooos/--      " << std::endl;
	std::cout << "                                                               .::-.`             `..:::////:::/:::////+ohhs+++ooss/-.    " << std::endl;
	std::cout << "                                                             .:///////-.`          ``.---::::::::////////hhhyo++ooss/-.   " << std::endl;
	std::cout << "                                                           .:////+++++///:.`         `:..-::::/::::://////yysooooooss:-.  " << std::endl;
	std::cout << "                                                        `.://:::///+++o+++//:-`       .:.-::::::::::://++oo+:+oooossso--  " << std::endl;
	std::cout << "                                                      `.://:--://///::::://+++/:.`   `-:---:::::::://///+ooooooossssss:-  " << std::endl;
	std::cout << "                                                    `.:/:--://++++++//////::-::::/:-`.:::::///::::://+++oooosssssss+:-    " << std::endl;
	std::cout << "                                                ` -/.-:::///++++++++++++++++++////:::///oo+////////+++ooooosssss+/:-      " << std::endl;
	std::cout << "                                              `-//://////+++++ooo+++++++++++++++:/+/++/+ooooo++/++ooosssyso-.-:           " << std::endl;
	std::cout << "                                           ` -://////+++++++oo+/--://++++++++++//+/oo+++oooo++oosyssssoo++/.              " << std::endl;
	std::cout << "                                        `-://///++++++++++++/:`     `.-:/+++++++++ooossssoosso+++++/::////:               " << std::endl;
	std::cout << "                                  .-://////++++++++++++//-.`           `-/++++++++++++++++++o+:::://///// :               " << std::endl;
	std::cout << "                                `-::///////+++oooooo++//:-                .:/++oooooooo+++++ooo+//////////:`              " << std::endl;
	std::cout << "                             :-////+oo++++++ooo+/--```                      `.-:/+ooooooooooooooss+///////::`             " << std::endl;
	std::cout << "                        `:://+ooossssoo,,``                                     -:+oooosssssssssso//////:::  `            " << std::endl;
	std::cout << "                     `:/+++++o,,,`````                                               -:+oosssssssyyyys+////::``           " << std::endl;
	std::cout << "                    `:/+++`````                                                          :/ossyyyyyyyyyyyo+/.``           " << std::endl;
	std::cout << "                 ,`:/+`                                                                     ```/////////::-               " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;

} //Wyswietlanie loga systemu

void Shell::loop() {
	do {
		read_line();
		execute();

		//Czyszczenie
		line.clear();
		parsed.resize(0);

	} while (status == true);

}

void Shell::read_line() {
	cout << "$ ";
	getline(cin, line);
	parse();
}

void Shell::parse() {
	transform(line.begin(), line.end(), line.begin(), ::tolower);
	parsed.clear();
	line = line + ' ';
	string pom;
	for (const char& i : line) {
		if (i != ' ') {
			pom += i;
		}
		else { //Jeśli spacja
			parsed.push_back(pom);
			pom.clear();
		}
	}
}

void Shell::execute() {
	if (parsed[0] == "showregs") { showregs(); }
	else if (parsed[0] == "help") { help(); }
	else if (parsed[0] == "kill") { exit(); }
	else if (parsed[0] == "cls") { cls(); }
	else if (parsed[0] == "cp") { cp(); }
	else if (parsed[0] == "showpcblist") { showpcblist(); }
	else if (parsed[0] == "showtree") { showtree(); }
	else if (parsed[0] == "dp") { dp(); }
	else if (parsed[0] == "showpcb") { showpcb(); }
	else if (parsed[0] == "showpipe") { showpipe(); }
	else if (parsed[0] == "showroot") { showroot(); }
	else if (parsed[0] == "cf") { cf(); }
	else if (parsed[0] == "df") { df(); }
	else if (parsed[0] == "showblock") { showblock(); }
	else if (parsed[0] == "wf") { wf(); }
	else if (parsed[0] == "af") { wf(); }
	else if (parsed[0] == "fo") { fo(); }
	else if (parsed[0] == "fc") { fc(); }
	else if (parsed[0] == "finfo") { finfo(); }
	else if (parsed[0] == "dinfo") { dinfo(); }
	else if (parsed[0] == "showdisk") { showdisk(); }
	else if (parsed[0] == "fsysparam") { fsysparam(); }
	else if (parsed[0] == "bitvector") { bitvector(); }
	else if (parsed[0] == "showmem") { showmem(); }
	else if (parsed[0] == "showpagefile") { showpagefile(); }
	else if (parsed[0] == "showpagetable") { showpagetable(); }
	else if (parsed[0] == "showstack") { showstack(); }
	else if (parsed[0] == "showframes") { showframes(); }
	else if (parsed[0] == "ver") { ver(); }
	else if (parsed[0] == "thanks") { thanks(); }
	else if (parsed[0].empty() || parsed[0] == "go") { go(); }
	else { notRecognized(); }
}

void Shell::notRecognized() {
	cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc\n\n";
	PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
}

//Metody interpretera
void Shell::go() {
	cout << "Nastepny krok\n";
	if (!planist.ReadyPCB.empty()) { //Sprawdza czy kolejka procesów READY nie jest pusta (powinien być zawsze conajmniej dummy)
		if (interpreter.execute_line(planist.ReadyPCB.front()->name) == -1) { //Wykonanie procesu, jeśli false to zakończył działanie
			const shared_ptr<PCB> tempProc = planist.ReadyPCB.front(); //Tymczasowe ściągnięte PCB
			cout << "Proces o nazwie \"" << tempProc->name << "\" zakonczyl swoje dzialanie\n\n";
			tree.kill(tempProc->name); //zabicie procesu
		}
		planist.check(); //aktualizacja planisty (kolejki procesów do wykonania i procesów czekających)
	}
}
void Shell::showregs() const {
	if (parsed.size() == 1) {
		interpreter.display_registers();
	}
	else { notRecognized(); }
}

//Metody shella
void Shell::ver() {
	logo();
	std::cout << "sexySO - Version 1.19.245 \n";
	std::cout << "<c> 2018 sexySO PUT Laboratory Group. All Rights Reserved. \n\n";
	std::cout << "Created by:\n";
	std::cout << "Tomasz Kiljanczyk \t File and directory management \n";
	std::cout << "Wojciech Kasperski \t RAM management \n";
	std::cout << "Michal Kalinowski \t Interface \n";
	std::cout << "Juliusz Horowski \t Process management \n";
	std::cout << "Marcin Jasinski \t Assembler commands and interpreter \n";
	std::cout << "Norbert Mlynarski \t CPU management \n";
	std::cout << "Krzysztof Kretkowski \t Inter-process communication \n";
	std::cout << "Aleksandra Laskowska \t Synchronization mechanisms \n";
	std::cout << "Alicja Gratkowska \t Virtual memory management \n\n\n";
}
void Shell::help()
{
	printf(R"EOF(

Metody interpretera
 go - Wykonanie kolejnej instrukcji

Metody shella
 ver  - Wersja systemu, prawa autorskie i autorzy
 help - Wyswietalnie listy poleceń
 kill - Konczenie pracy
 cls  - Czyszczenie ekranu

Metody zarzadzania procesami
 cp - Tworzenie procesu np.:
			CP [nazwa_procesu] [nazwa_pliku.txt]
			CP [nazwa_procesu] [nazwa_rodzica] [nazwa_pliku.txt]
 dp - Usuwanie procesu np. DP [nazwa_procesu]

Metody dyskowe
 cf - Utworzenie pliku np. CF [nazwa_pliku]
 df - Usuniecie pliku np. DF [nazwa_pliku]
 wf - Zapis do pliku np. WF [nazwa_pliku] [tresc]
 ap - Dopis do pliku np. AF [nazwa_pliku] [tresc]
 fo - Otwarcie pliku np. FO [nazwa_pliku] [parametr]
			parametry: -r(do odczytu) -w(do zapisu)
 fr - Odczyt danych z pliku np. [nazwa_pliku] [ilosc_bajtow_do_odczytu]
 fc - Zamkniecie pliku np. FC [nazwa_pliku]

Metody pracy krokowej
 finfo       - Wyswietla informacje o pliku np. FINFO [nazwa_pliku]
 dinfo       - Wyswietla informacje o katalogu
 showdisk    - Wyswietla zawartosc dysku jako znaki
 fsysparam   - Wyswietla parametry systemu plikow
 bitvector   - Wyswietla wektor bitowy
 showroot    - Listowanie katalogu
 showblock   - Listowanie zawartosci wskazanego bloku dyskowego, np. showblock [numer_bloku]
 showregs    - Wyswietlanie zawartosci rejestrow i licznika rozkazow
 showpipe    - Wyswietla wszystkie istniejace potoki
 showpcblist - Lista PCB wszystkich procesow
 showpcb     - Wyswietla informacje o PCB procesu, np. showpcb [nazwa_procesu]
 showtree    - Wyswietla drzewo procesow
 showmem     - Wyswietlanie zawartosci pamieci
 showpagefile  - Wyswietla plik stronicowania
 showpagetable - Wyswietla tablice wymiany stronic np. [nazwa_procesu]
 showstack   - Pokazuje kolejke FIFO wymiany stronic
 showframes  - Pokazuje ramki w pamieci RAM wraz ze szczegolami

Metody dodatkowe
 thanks - ;-)

)EOF");
}

//Kończenie pracy systemu
void Shell::exit() {
	system("cls");
	ver();
	PlaySound(TEXT("Exit_Windows.wav"), nullptr, SND_SYNC);
	status = false;
}

void Shell::cls() const {
	if (parsed.size() == 1) {
		system("cls");
	}
	else { notRecognized(); }
}

void Shell::showpcb() {
	if (parsed.size() == 2) {
		shared_ptr<PCB>  tempProc = tree.find(parsed[1]);
		if (tempProc != nullptr) {
			tempProc->display();
			cout << "\n";
		}
	}
	else { notRecognized(); }
}

//Metody zarzadzania procesami

//Tworzenie procesu
void Shell::cp() {
	if (parsed.size() == 3) {
		if (!std::filesystem::exists(parsed[2])) { std::cout << "Nie znaleziono pliku!\n\n"; }
		else if (parsed[1] == "system_dummy") { cout << "Nie mozna stworzyc procesu o nazwie \"" << parsed[1] << "\"\n\n"; }
		else if (tree.find(parsed[1]) != nullptr) {
			cout << "Proces o nazwie \"" << parsed[1] << "\" juz istnieje!\n\n";
		}
		else {
			tree.fork(parsed[1], 1, parsed[2]);
			std::cout << "Stworzono proces \"" << parsed[1] << "\" wedlug programu z pliku \"" << parsed[2] << "\"\n\n";
		}
	}
	else if (parsed.size() == 4) {
		if (!std::filesystem::exists(parsed[3])) { std::cout << "Nie znaleziono pliku!\n\n"; }
		else if (parsed[1] == "system_dummy") { cout << "Nie mozna stworzyc procesu o nazwie \"" << parsed[1] << "\"\n\n"; }
		else if (tree.find(parsed[1]) != nullptr) {
			cout << "Proces o nazwie \"" << parsed[1] << "\" juz istnieje!\n\n";
		}
		else if (tree.find(parsed[2]) == nullptr) {
			cout << "Proces o nazwie \"" << parsed[2] << "\" nie istnieje!\n\n";
		}
		else {
			tree.fork(parsed[1], tree.find(parsed[2])->PID, parsed[3]);
			std::cout << "Stworzono proces \"" << parsed[1] << "\" wedlug programu z pliku \"" << parsed[3] << "\"\n\n";
		}
	}
	else { notRecognized(); }
}

//Lista PCB wszystkich procesów
void Shell::showpcblist() const {
	if (parsed.size() == 1) { planist.display_PCB_lists(); }
	else { notRecognized(); }
}

void Shell::showtree() const {
	if (parsed.size() == 1) { tree.display(); }
	else { notRecognized(); }
}

void Shell::dp() {
	if (parsed[1] == "system_dummy") { std::cout << "Odmowa dostepu!\n\n"; }
	else {
		const shared_ptr<PCB>  tempProc = tree.find(parsed[1]);
		if (tempProc != nullptr) {
			tree.kill(tempProc->name);
			std::cout << "Usunieto proces o nazwie \"" << parsed[1] << "\"\n\n";
		}
		else {
			std::cout << "Nie znaleziono procesu o nazwie \"" << parsed[1] << "\"!\n\n";
			PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
		}
	}
}

void Shell::showpipe() {
	pipeline.display();
	cout << '\n';
}

//Metody dyskowe

//Listowanie katalogu głównego
void Shell::showroot() const {
	if (parsed.size() == 1) { fm.display_root_directory(); std::cout << '\n'; }
	else { notRecognized(); }
}

void Shell::cf() {
	if (parsed.size() == 2) {
		const int result = fm.file_create(parsed[1], "");
		if (result != 0) { cout << "Blad operacji: " << result << "!\n"; }
		else { std::cout << "Stworzono plik o nazwie \"" << parsed[1] << "\"\n\n"; }
	}
	else { notRecognized(); }
}

//Usunięcie pliku
void Shell::df() {
	if (parsed.size() == 2) {
		if (!fm.file_exists(parsed[1])) { cout << "Nie znaleziono pliku o nazwie \"" << parsed[1] << "\"!\n\n"; }
		else if (const int result = fm.file_delete(parsed[1], "") != 0) {
			cout << "Kod bledu: " << result << "!\n\n";
		}
		else { std::cout << "Usunieto plik o nazwie \"" << parsed[1] << "\"\n\n"; }
	}
	else { notRecognized(); }
}

//Listowanie zawartości wskazanego bloku dyskowego
void Shell::showblock() {
	if (parsed.size() == 2) {
		for (const char& c : parsed[1]) {
			if (c >= '0' || c <= '9') {}
			else { std::cout << "Zla liczba!\n"; return; }
		}
		fm.display_block_char(std::stoi(parsed[1]));
	}
	else { notRecognized(); }
}

void Shell::wf() {
	if (parsed.size() == 2) {
		string data;
		cout << "Dane do wprowadzenia: ";
		getline(cin, data);
		if (fm.file_write(parsed[1], "", data) != 0) {
			std::cout << "Operacja niepowiodla sie!\n";
		}
	}
	else { notRecognized(); }
}

void Shell::af() {
	if (parsed.size() == 2) {
		string data;
		cout << "Dane do dopisania: ";
		getline(cin, data);
		if (fm.file_append(parsed[1], "", data) != 0) {
			std::cout << "Operacja niepowiodla sie!\n";
		}
	}
	else { notRecognized(); }
}

void Shell::fo() {
	if (parsed.size() == 3) {
		unsigned int arg;
		if (parsed[2] == "-r") arg = FILE_OPEN_R_MODE;
		if (parsed[2] == "-w") arg = FILE_OPEN_W_MODE;
		fm.file_open(parsed[1], "", arg);
		cout << "Otwarto plik o nazwie \"" << parsed[1] << "\" w trybie " << (arg == 0 ? "W" : "R") << "\n\n";
	}
	else { notRecognized(); }
}

void Shell::fr() {
	if (parsed.size() == 3) {
		string data;

		if (fm.file_read(parsed[1], "", stoi(parsed[2]), data) != 0) {
			std::cout << "Operacja niepowiodla sie!\n\n";
			return;
		}
		cout << data << "\n\n";
	}
	else { notRecognized(); }
}

void Shell::fc() {
	if (parsed.size() == 2) {
		fm.file_close(parsed[1], "");
		cout << "Zamknieto plik o nazwie \"" << parsed[1] << "\"\n\n";
	}
	else { notRecognized(); }
}

void Shell::finfo() {
	if (parsed.size() == 2) {
		fm.display_file_info(parsed[1]);
		std::cout << "\n";
	}
	else {
		notRecognized();
		PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
	}
}

void Shell::dinfo() const {
	if (parsed.size() == 1) {
		fm.display_root_directory_info();
		std::cout << "\n";
	}
	else {
		notRecognized();
		PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
	}
}

void Shell::showdisk() const {
	if (parsed.size() == 1) { fm.display_disk_content_char(); }
	else { notRecognized(); }
}

void Shell::fsysparam() const {
	if (parsed.size() == 1) {
		FileManager::display_file_system_params();
		std::cout << "\n";
	}
	else { notRecognized(); }
}

void Shell::bitvector() const {
	if (parsed.size() == 1) { fm.display_bit_vector(); std::cout << "\n"; }
	else { notRecognized(); }
}

//Metody pamieci
void Shell::showmem() {
	if (parsed.size() == 1) { mm.show_memory(); std::cout << "\n"; }
	else if (parsed.size() == 3) {
		const int begin = stoi(parsed[1]);
		const int bytes = stoi(parsed[2]);
		mm.show_memory(begin, bytes);
		std::cout << "\n";
	}
	else {
		notRecognized();
		PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
	}
}

void Shell::showpagefile() const {
	if (parsed.size() == 1) { mm.show_page_file(); std::cout << "\n"; }
	else {
		notRecognized();
		PlaySound(TEXT("Critical_Stop.wav"), nullptr, SND_ASYNC);
	}
}

void Shell::showpagetable() {
	if (parsed.size() == 2) {
		if (parsed[1] == "shell") { cout << "Odmowa dostepu!\n"; }
		else {
			MemoryManager::show_page_table(tree.find(parsed[1])->pageList);
			std::cout << "\n";
		}
	}
	else { notRecognized(); }
}

void Shell::showstack() const {
	if (parsed.size() == 1) { mm.show_stack(); std::cout << "\n"; }
	else { notRecognized(); }
}

void Shell::showframes() const {
	if (parsed.size() == 1) { mm.show_frames(); std::cout << "\n"; }
	else { notRecognized(); }
}


//Easter egg
void Shell::thanks() {
	system("cls");
	cout << "\n"
		"                                                                               \n"
		"   //////// 	 //////  //////      //////   //    //////   ////  ////         \n"
		" ////        //    /// //    //    //   //  //     //     //    //             \n"
		"///    //// //    /// //     //   //////   //     /////   ////  ////           \n"
		"///    /// //    /// //     //   //    // //     //         //    //           \n"
		"/////////   ///////  ///////     //////   ///// //////  ///// /////            \n"
		"                                                                               \n"
		"                       ,,,/////#####(///,/%%%#)                                \n"
		"                     /*....                   .#&                              \n"
		"                  /*,...,....                  ..#&                            \n"
		"                (*,,,,...                       ..,#%&                         \n"
		"              (**,,....             ...,,,,....       #%                       \n"
		"             (*,,....  .         ..,,,,,,.......       .#%                     \n"
		"            (*,,,............ .,*/*,,.,..........        .%%                   \n"
		"           (***,,....,..... .*/***,,,.....,.......         (%                  \n"
		"          (*,*,,,,..,,...,,////*,,,,.....,,,...,,..         ,(#                \n"
		"         (//***,,.,,,,,*/////*,,,,,,,..,,,,,,,,,,,..         *(#               \n"
		"        (///*******////((//**,,,,,,,,,,,,,,,,,,,,,,,.         *(               \n"
		"       ((///(((#%###(((//*****,****,,,********,**,,,,...      /##              \n"
		"       (((//(#%###(((///***********,,**********//*,..,,....   .##              \n"
		"      ((((///(####((((//***************************,,,,.,....  /#              \n"
		"      ((/////(#####((((//****,****,,,**,,,,,,,,,,***,,,,.,...*,,*              \n"
		"      (((**/((######(((///**,,,,*******,,,,,,,,,******,,.*,.**(##))            \n"
		"      ((/*///((#####((((((////***/////#((#(###/#/##*****,##*#///((((           \n"
		"       (//((((####((/////#(((//**((##/%/((//*****,##(#*##./(//((/#(            \n"
		"        (/((((##%#(##(#/##%###*,#/#/%%&%&&&%#***/(***#...,##((((##             \n"
		"        ((((((####((##///%##%%#/#*#(((######(#/***/**/#(/*/##/(/(((            \n"
		"          #(#(((##(((#%&&%%%%%%%(**#(/////*****,,,,/,*#////(/*///(((           \n"
		"           #((###/(##%#########%(*,*#(****,,,,,,,,*,##*(//***,,*/((/           \n"
		"            ###((/(##(////(/((##/*,,,*/#,##,#,#,###/,*////***,,*#(             \n"
		"             #(/(####((/*///(#%/*,,,,,,,*(%%#(//**,***////(***/##              \n"
		"              %#%#/(#((/////##((/****//##(((###(//////////**(##                \n"
		"                #%%###((((#%%%%%%#(((#((///*/(###(///(////(##                  \n"
		"                ###%###((##%###%%%%###(/***//(###(((((//(/##                   \n"
		"                 (##%%####%%#######((((((###%%##(((((((/(/                     \n"
		"                  %%%%####%#%&&%%%%###((/##(////(//(((/(/*                     \n"
		"                   %%%%%#((###%%&&%%#%#///******///((//(*/                     \n"
		"                    #%%#%#((((#####((((((//*******/(///(*/                     \n"
		"                      ####%#(((((((((((///********((/((///                     \n"
		"                      @@%##%##((//(////*********/(###(///*((                   \n"
		"           ##%###%%%&@@@@&%#%%#(//////*******//(##%#(/////(((%%%               \n"
		"      (###%%%##%%%%#%&&@@@&&%%%#((//////((((###%%##(//////(##%%%#(###          \n"
		"    #(##%%%%%%#%&&%#%&@&@@@&&&%%%%#####%%%%%%%%##((//(///(((%%%##%#######      \n"
		"####((#%%%%%%%%&%%#%&&&%&@@&&&&&&&&%%%%%%%%%###(///(/*/(((#%%%(#%####/(######  \n"
		"####((%%%%%%%%%&%#%&&%%&@&&&&&%%%%%%%%#######((((//*//(((#%%%&#%#////**##,,### \n"
		"###((#%%%%%%%%%%#%&&%%%&&&&&&%%%%%%%%%%%######(/////((((#%&&&%#(((/*****///####\n"
		"#####%%%%%%%%%%##%%%%%%&&%&@@&&%###(#######((////((**/(&&&%%###((//////((((( ##\n"
		"####%%%%%%%%%%##%%%%%%%%%%&&&&@&%%#(((((((//((((#%%%&&&&&&&%%##((//////(((#((##\n"
		"####%%%%%%%%%###%%%%##%%%%%%&%%@@##(((/////(&&&@@&&%%&&%%#####(((((((((((((((##\n"
		"###%%%%%%%%%##########%%%%##%%%%&&&((///(%&&@&&&&&%%%%%%######((((((((((((((###\n"
		"###%%%%%%#%###########%%%%%######%&&((%&&&&@&&&&%%%%%%%%%%###(((((/(((((/(((%##\n"
		"###%%%%#%%##############%%%########%%&&&&&&&&%%%%%%%%%#%%#####(((/////(((((((##\n"
		"###%%%#############################%(,/%%%%%%##%%%%%%########(((/////((((((((%#\n"
		"###%%########(#############((####%%%..,%#%%%%#%#############(((///(//(((((((###\n"
		"##%%%#(######(############(((((##%%%%%######################(//////(((((((((###\n"
		"##%%%##(####(############((((((#%%%%#######################((/////((((((((((##%\n"
		"##%%%##(((#(#############((((##%%%%#########(############(((///////((((((((####\n"
		"#%%%%%##((((#############((((##%%%####(####((#######(##(((((///////((((((((###%\n\n"; //Koniec cout
}
