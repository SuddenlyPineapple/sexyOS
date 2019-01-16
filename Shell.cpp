#include "Shell.h"
#include <iostream>
#include <string>
#include <algorithm>


//Metody pracy shella
void Shell::boot() //Funckja startująca pętlę shella
{
	tree.fork(new PCB("shell", 1));
	p.AddProces(tree.find_proc("shell"));
	logo();
	//PlaySound(TEXT("Startup.wav"), NULL, SND_ALIAS);
	loop();
}

void Shell::logo() {

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

void Shell::loop() //Pętla shella
{
	do {
		read_line();
		execute();

		//Czyszczenie
		line.clear();
		parsed.resize(0);

	} while (status == true);

}

void Shell::read_line() //Odczyt surowych danych
{
	cout << "$ ";
	getline(cin, line);
	parse();
}

void Shell::parse() //Parsowanie
{
	transform(line.begin(), line.end(), line.begin(), ::tolower);
	parsed.resize(0);
	line = line + ' ';
	string pom;
	for (int i = 0; i < line.size(); i++) {
		if (line[i] != ' ') {
			pom += line[i];
		}
		else { //Jeśli spacja
			parsed.push_back(pom);
			pom.clear();
		}
	}
}

void Shell::execute() {

	if (parsed[0] == "go")						//Nastepny krok pracy krokowej
	{
		go();
	}

	if (parsed[0] == "help")					//Wyswietalnie listy poleceń
	{
		help();
	}

	else if (parsed[0] == "exit")				//Kończenie pracy
	{
		exit();
	}

	else if (parsed[0] == "cp")					//Tworzenie procesu
	{
		cp();
	}

	else if (parsed[0] == "lp")					//Lista PCB wszystkich procesów
	{
		lp();
	}

	else if (parsed[0] == "lt")					//Drzewo procesow
	{
		lt();
	}

	else if (parsed[0] == "dp")					//Usuwanie procesu
	{
		dp();
	}

	else if (parsed[0] == "ls")					//Listowanie katalogu
	{
		ls();
	}

	else if (parsed[0] == "cf")					//Utworzenie pliku
	{
		cf();
	}

	else if (parsed[0] == "df")					//Usunięcie pliku
	{
        df();
	}

	else if (parsed[0] == "ld")					//Listowanie zawartości wskazanego bloku dyskowego
	{
        ld();
	}

	else if (parsed[0] == "rf")					//Zmiana nazwy pliku
	{
		rf();
	}

	else if (parsed[0] == "wf")					//Zapisywanie do pliku
	{
		wf();
	}

	else if (parsed[0] == "fo")					//Otwarcie pliku
	{
		fo();
	}

	else if (parsed[0] == "fc")					//Zamkniecie pliku
	{
		fc();
	}

	else if (parsed[0] == "dmem")					//Wyswietlenie pamieci
	{
		dmem();
	}

	else if(parsed[0]==""){
		go();
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

//Commands functions
//Metody interpretera;
void Shell::go(){
		cout << "Nastepny krok" << endl;
		//inter.exe;
}
//Metody shella
void Shell::help() //Wyświetlenie listy poleceń
{
	cout << "cp [name] [program]          Tworzenie procesu				                  " << endl;
	cout << "lp                           Lista PCB wszystkich procesów                   " << endl;
	cout << "ls                           Listowanie katalogu                             " << endl;
	cout << "cf [name]                    Utworzenie pliku                                " << endl;
	cout << "df [name]                    Usuniecie pliku                                 " << endl;
	cout << "ld [block_number]            Listowanie zawartosci wskazanego bloku dyskowego" << endl;
	cout << "exit                         Konczenie pracy                                 " << endl;
}
void Shell::exit() //Kończenie pracy
{
	status = false;
}
//Metody zarzadzania procesami
void Shell::cp() //Tworzenie procesu
{
	if(parsed[1] == "shell" || parsed[1] == "systemd"){
		cout << "Nie można stworzyć procesu " << parsed[1] << ".\n";
		return;
	}
	 else if (parsed.size() == 3)
	{
		tree.fork(new PCB(parsed[1], 2), parsed[2], 128);
		p.AddProces(tree.find_proc(parsed[1]));
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;

}

void Shell::lp() //Lista PCB wszystkich procesów
{
	if (parsed.size() == 1) 
	{
		tree.display_tree();
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::lt() {
	//Drzewo procesow
}

void Shell::dp() {
	if(parsed[1] == "shell" || parsed[1] == "systemd") {
		std::cout << "Odmowa dostepu!" << endl;
		return;
	}
	else {
		PCB* tempProc = tree.find_proc(parsed[1]);
		if(tempProc != nullptr) {
			tree.exit(tempProc->PID);
		}
		else {
			std::cout << "Nie znaleziono procesu!\n";
		}
	}
}
//Metody dyskowe
void Shell::ls() //Listowanie katalogu
{
	if (parsed.size() == 1) 
	{
		fm.display_root_directory();
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::cf() //Utworzenie pliku
{
	if (parsed.size() == 2)
	{
		fm.file_create(parsed[1], "shell");
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::df() //Usunięcie pliku
{
	if (parsed.size() == 2) 
	{
		fm.file_delete(parsed[1], "shell");
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::ld() //Listowanie zawartości wskazanego bloku dyskowego
{
	if (parsed.size() == 1)
	{
		fm.display_root_directory();
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::rf() {
	if (parsed.size() == 3)
	{
		//fm.
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::wf() {
	if (parsed.size() == 3)
	{
		fm.file_write(parsed[1], "shell", parsed[2]);
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::fo() {
	if (parsed.size() == 3)
	{
		unsigned int arg;
		if(parsed[2]=="-r") arg = FILE_OPEN_R_MODE;
		if(parsed[2] =="-w") arg = FILE_OPEN_W_MODE;
		fm.file_open(parsed[1], "shell", arg);
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}

void Shell::fc() {
	if (parsed.size() == 2)
	{
		fm.file_close(parsed[1], "shell");
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}
//Metody pamieci
void Shell::dmem() {
	if (parsed.size() == 1)
	{
		mm.showMem();
	}
	else
		cout << "Nie rozpoznano polecenia! Wpisz \"help\" by wyswietlic pomoc" << endl;
}




