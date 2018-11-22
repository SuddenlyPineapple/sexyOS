#include "interpreter.h"
#include <iostream>
#include <string>
#include <fstream>
#include "FileManager.h"




void Interpreter::stan_rejestrow()
{
	std::cout << "\nA: " << A << "\nB: " << B << "\nC: " << C << "\nD: " << D <<"\nilosc rozkazow: "<<counter<< "\n\n";
}
//JMP I JZ nwm,jednak mam zle chyba, moze to kogos innego

// chyba bede musial zmienic niektore nazwy rozkazow w tych ifach, bo asembler to nie asembler najwyrazniej. bucholc mnie takich rozkazow nauczyl jakie mam teraz, a nie jakies dziwne 2literowe,nie wiadomo skad, no bo w sumie to o co chodzi, po co zmieniac nazwy rozkazow? Jest jeden asembler po co tak konfudowac ludzi? No Bambo to pewnie bedzie chcial te dziwne, bo on sam jest dziwny i w ogole co ja mam dac na tej prezentacji, no bo sory, ale on nie dal mi zadnych materialow. Nie wiem co mam jeszcze napisac, by bylo smiesznie, ze jest taki dlugi komentarz. Na co mam jewszcze narzekac? A niewazne, i tak juz jest chyba wystarczajaco dlugi, nie? no dobra, juz koncze. xoxo

void Interpreter::wykonanie_programu(Rozkazy r)
{
	int i=counter++;
	std::string rozkaz = r.rozkaz; // rozkaz bedzie z memory manager potem
	std::string rej = r.rej;

	if (rej.size() < 1)
		rej = "0";

	int *rej1;	rej1 = &A;
	int *rej2;	rej2 = rej1;
	int liczba=0;
	int adres = 0;
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
		else if (rej[0] == '[')
		{
			rej.erase(rej.begin());rej.erase(rej.end());//nie wiem, czy to...
			adres = std::stoi(rej);
			std::cout << adres;
		}
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
	else if (rozkaz == "JNZ") //jump if not zero , skok wzarunkowy
	{
		if (rej1 != 0)
			i = liczba;
	}



	//norberto potoki
	else if (rozkaz == "SP") {}//stworz potok
	else if (rozkaz == "UP") {}//usun potok

	else if (rozkaz == "SM") {}//show message
	else if (rozkaz == "RM") {} //read message 




	else if (rozkaz == "CLS") { system("cls"); }
	
	
	else if (rozkaz == "DSD") //wypisz dysk
	{
		fileManager.DisplayBitVector();
		fileManager.DisplayDiskContentChar();
	} 



	//pliki
	else if (rozkaz == "CF")// stworzenie pliku
	{
		fileManager.FileCreate(rej,rej);//nazwa i dane
	}
	else if (rozkaz == "FO")// otwarcie pliku
	{
		fileManager.FileOpen(rej);
	}
	else if (rozkaz == "FW")//zapisz do pliku
	{
		fileManager.FileWriteData(rej, rej);//nazwa pliku i caly program w stringu
	}
	else if (rozkaz == "FC")//ZAMKNIJ plik
	{
		fileManager.FileClose(rej);//nazwa pliku 
	}
	else if (rozkaz == "FR")//CZYTANIE Z PLIKU
	{
		rej=fileManager.FileReadData(rej);//nazwa pliku 
		std::cout << rej;
	}



	//pamiec
	else if (rozkaz == "CP") {}//czytaj pamiec
	else if (rozkaz == "PP") {} //pamiec



	//procesy
	else if (rozkaz == "XW") {}//wyswietlanie drzewa procesow		
	else if (rozkaz == "XP") {}//tworzenie procesu
	else if (rozkaz == "DP") {}//zabijanie procesu



	else if (rozkaz == "HLT") {}// koniec pliku zapis rejestru do procesu




	else { std::cout << "error\n";  }
}