#include "stdafx.h"
#include "interpreter.h"
#include <iostream>
#include <string>
#include <fstream>


interpreter::interpreter() { A = B = C = D = 0; }

void interpreter::stan_rejestrow()
{
	std::cout << "\nA: " << A << "\nB: " << B << "\nC: " << C << "\nD: " << D <<"\nilosc rozkazow: "<<counter<< "\n\n";
}


// chyba bede musial zmienic niektore nazwy rozkazow w tych ifach, bo asembler to nie asembler najwyrazniej. bucholc mnie takich rozkazow nauczyl jakie mam teraz, a nie jakies dziwne 2literowe,nie wiadomo skad, no bo w sumie to o co chodzi, po co zmieniac nazwy rozkazow? Jest jeden asembler po co tak konfudowac ludzi? No Bambo to pewnie bedzie chcial te dziwne, bo on sam jest dziwny i w ogole co ja mam dac na tej prezentacji, no bo sory, ale on nie dal mi zadnych materialow. Nie wiem co mam jeszcze napisac, by bylo smiesznie, ze jest taki dlugi komentarz. Na co mam jewszcze narzekac? A niewazne, i tak juz jest chyba wystarczajaco dlugi, nie? no dobra, juz koncze. xoxo

void interpreter::wykonanie_programu(rozkazy r)
{
	int i=counter++;
	std::string rozkaz = r.rozkaz; // rozkaz bedzie z memory manager potem
	std::string rej = r.rej;

	if (rej.size() < 1)
		rej = "0";

	int *rej1;	rej1 = &A;
	int *rej2;	rej2 = rej1;
	int liczba=0;

	//zamiana na rozkaz i rejestr
	if (rej[1] == ',')
	{
		int temp = rej[0];
		rej.erase(rej.begin(), rej.begin() + 2);

		if (rej == "A")
			rej2 = &A;
		else if (rej == "B")
			rej2 = &B;
		else if (rej == "C")
			rej2 = &C;
		else if (rej == "D")
			rej2 = &D;
		else liczba = std::stoi(rej);


		rej = temp;
	}

	if (rej == "A")
		rej1 = &A;
	else if (rej == "B")
		rej1 = &B;
	else if (rej == "C")
		rej1 = &C;
	else if (rej == "D")
		rej1 = &D;
	else liczba = std::stoi(rej);







	if (rozkaz == "ADD")
	{
		if (liczba == 0)
			*rej1 += *rej2;
		else
			*rej1 += liczba;
	}
	else if (rozkaz == "SUB")
	{
		if (liczba == 0)
			*rej1 -= *rej2;
		else
			*rej1 -= liczba;
	}
	else if (rozkaz == "MUL")
	{
		if (liczba == 0)
			*rej1 *= *rej2;
		else
			*rej1 *= liczba;
	}
	else if (rozkaz == "DIV")
	{
		if (liczba == 0)
			*rej1 /= *rej2;
		else
			*rej1 /= liczba;
	}




	else if (rozkaz == "MOV")
	{
		if (liczba == 0)
			*rej1 = *rej2;
		else
			*rej1 = liczba;
	}
	else if (rozkaz == "INC")
	{
		(*rej1)++;
	}
	else if (rozkaz == "DEC")
	{
		(*rej1)--;
	}




	else if (rozkaz == "MW") {}//memory write
	else if (rozkaz == "MR") {}//memory read




	else if (rozkaz == "JMP")
	{
		i = liczba;
	}
	else if (rozkaz == "JZ") //jump if not zero , skok wzarunkowy
	{
		if (rej1 != 0)
			i = liczba;
	}




	else if (rozkaz == "SP") {}//stworz potok
	else if (rozkaz == "UP") {}//usun potok

	else if (rozkaz == "SM") {}//show message
	else if (rozkaz == "RM") {} //read message 




	else if (rozkaz == "CL") { system("cls"); }
	
	
	else if (rozkaz == "WP") {} //wypisz dysk



	//pliki
	else if (rozkaz == "FO")// otwarcie pliku
	{
		//jednak wywolanie metody z file manager
		std::ifstream plik("plik.txt");
		while(!plik.eof())
		{ 
		rozkazy ro;
		plik >> ro.rozkaz >> ro.rej;
		wykonanie_programu(ro);
		}
	}
	else if (rozkaz == "FS")//zapisz do pliku
	{
		//jednak metoda z file manager
		/*std::ofstream plik("plik.txt");
		for (int j = 0;j<program.size();j++)
		{
			plik << program[j].rozkaz<<" "<<program[j].rej<<"\n";
		} dobra, dam to tomkowi do zrobienia
		*/
	}
	else if (rozkaz == "CP") {}//czytaj pamiec


	else if (rozkaz == "PP") {} //pamiec



	//procesy
	else if (rozkaz == "XW") {}//wyswietlanie drzewa procesow
	else if (rozkaz == "XP") {}//tworzenie procesu
	else if (rozkaz == "DP") {}//zabijanie procesu



	else if (rozkaz == "HLT") {}// koniec pliku zapis rejestru do procesu




	else { std::cout << "error\n";  }
}