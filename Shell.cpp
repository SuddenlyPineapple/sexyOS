#include "Shell.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <ctype.h>
#include "MemoryManager.h"
#include "MemoryManager.cpp"
Shell::Shell()
{
	bool status;
	line.clear();
}

Shell::~Shell() {}

//Shell work functions

void Shell::execute(/*Interpreter inter,....., ProcessManager procmem*/) {

	if (parsed[0] == "help")					//Wyswietalnie listy poleceń
	{
		help();
	}

	else if (parsed[0] == "cp")					//Tworzenie procesu
	{
		cp(/*procmem*/);
	}

	else if (parsed[0] == "lp")					//Lista PCB wszystkich procesów
	{
		lp(/*procmem)*/);
	}

	else if (parsed[0] == "ls")					//Listowanie katalogu
	{
		ls(/*hdd)*/);
	}

	else if (parsed[0] == "cf")					//Utworzenie pliku
	{
		cf(/*hdd)*/);
	}

	else if (parsed[0] == "df")					//Usunięcie pliku
	{
		df(/*hdd)*/);
	}

	else if (parsed[0] == "ld")					//Listowanie zawartości wskazanego bloku dyskowego
	{
		ld(/*hdd)*/);
	}
	else if (parsed[0] == "exit")				//Kończenie pracy
	{
		exit();
	}
	else
		cout << "Command not found! Type \"help\" for more information" << endl;
}


void Shell::parse() //Parsowanie
{
	vector<string> parsed;
	parsed.resize(0);
	line = line + " ";
	int space_pos = -1;
	for (int i; i < line.size(); i++) {
		if (line[i] == ' ') {
			string pom = line.substr(space_pos + 1, i - 1);
			parsed.push_back(pom);
			space_pos = i;
		}
	}
	transform(parsed[0].begin(), parsed[0].end(), parsed[0].begin(), ::tolower);
	execute();
}

void Shell::read_line() //Odczyt surowych danych
{
	cout << "$ ";
	getline(cin, line);
	parse();
}

void Shell::loop(/*Interpreter inter,....., ProcessManager procmem*/) //Pętla shella
{
	do {
		read_line();
		execute(/*inter, ....., procmem*/);

		//Czyszczenie 
		line.clear();
		parsed.resize(0);

	} while (status);

}

void Shell::boot() //Funckja startująca pętlę shella
{
	logo();
	Interpreter inter;
	.
	.
	.
	.
	ProcessManager procmem;
	loop(/*inter,....,procmem*/);
}

//Commands functions

void Shell::help() //Wyświetlenie listy poleceń
{
	cout << "cp [name] [program]          Tworzenie procesu				                  " << endl;
	cout << "lp                           Lista PCB wszystkich procesów                   " << endl;
	cout << "ls                           Listowanie katalogu                             " << endl;
	cout << "cf [name]                    Utworzenie pliku                                " << endl;
	cout << "df [name]                    Usunięcie pliku                                 " << endl;
	cout << "ld [block_number]            Listowanie zawartości wskazanego bloku dyskowego" << endl;
	cout << "exit                         Kończenie pracy                                 " << endl;
}

void Shell::cp(/*ProcessManager procmem*/) //Tworzenie procesu
{
	if (parsed.size() == 3) 
	{
		procmem.create_process(parsed[1], parsed[2]);
		
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;

}

void Shell::lp(/*ProcessManager procmem*/) //Lista PCB wszystkich procesów
{
	if (parsed.size() == 1) 
	{
		procmem.display_pcbs();
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;
}

void Shell::ls(/*HDD hdd*/) //Listowanie katalogu
{
	if (parsed.size() == 1) 
	{
		hdd.display_catalog();
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;
}

void Shell::cf/*HDD hdd*/() //Utworzenie pliku
{
	if (parsed.size() == 2)
	{
		hdd.create_file(parsed[1]);
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;
}

void Shell::df(/*HDD hdd*/) //Usunięcie pliku
{
	if (parsed.size() == 2) 
	{
		hdd.delete_file(parsed[1]);
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;
}

void Shell::ld(/*HDD hdd*/) //Listowanie zawartości wskazanego bloku dyskowego
{
	if (parsed.size() == 2)
	{
		hdd.display_block(parsed[1]);
	}
	else cout << "Wrong command construction! Type \"help\" for more information" << endl;
}

void Shell::exit() //Kończenie pracy
{
	status = false;
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

}