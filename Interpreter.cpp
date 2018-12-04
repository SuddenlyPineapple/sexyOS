#include "interpreter.h"
#include <iostream>
#include <string>
#include "FileManager.h"
#include "Procesy.h"
#include "MemoryManager.h"


//				Uwagi:
//JMP I JZ nwm,jednak mam zle chyba, moze to kogos innego
//pamietaj by usunac kiedys te komentarze.
//W plikach dales wszedzie rej zamiast wlasciwego stringa nazwy. Nieladnie.


// chyba bede musial zmienic niektore nazwy rozkazow w tych ifach, bo asembler to nie asembler najwyrazniej. bucholc mnie takich rozkazow nauczyl jakie mam teraz, a nie jakies dziwne 2literowe,nie wiadomo skad, no bo w sumie to o co chodzi, po co zmieniac nazwy rozkazow? Jest jeden asembler po co tak konfudowac ludzi? No Bambo to pewnie bedzie chcial te dziwne, bo on sam jest dziwny i w ogole co ja mam dac na tej prezentacji, no bo sory, ale on nie dal mi zadnych materialow. Nie wiem co mam jeszcze napisac, by bylo smiesznie, ze jest taki dlugi komentarz. Na co mam jewszcze narzekac? A niewazne, i tak juz jest chyba wystarczajaco dlugi, nie? no dobra, juz koncze. xoxo
void Interpreter::rejestr_rozkaz()
{
	rozkaz = "XW";//memoryManager.GET(&tree.proc, interpreter.licznik_rozkazow);
std::cout <<"rozkaz = "<< rozkaz;

A = tree.proc.A;
B = tree.proc.B;
C = tree.proc.C;
D = tree.proc.D;

licznik_rozkazow = tree.proc.comand_counter;

}


void Interpreter::stan_rejestrow()
{
	std::cout << "\nA: " << A << "\nB: " << B << "\nC: " << C << "\nD: " << D <<"\nilosc rozkazow: "<<licznik_rozkazow<< "\n\n";
}


void Interpreter::wykonanie_programu()
{
	rejestr_rozkaz();
	licznik_rozkazow++;

	std::string rej = memoryManager.RAM;
	std::cout <<"\nRAM "<< rej;
	if (rej.size() < 1)		rej = "0";

	std::string nazwa; //nazwa pliku
	int *rej1;	rej1 = &A;
	int *rej2;	rej2 = rej1;

	
	

	//zamiana na rozkaz i rejestr chcialem adres zrobic, ale chyba mi nie wyszlo :(																							tak jak wszystko w zyciu D:
	/*if (rej[1] == ' ')
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
	else liczba = std::stoi(rej);*/


	





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
	else if (rozkaz == "MR") 
	{
		memoryManager.showMem();
	}//memory read




	else if (rozkaz == "JMP")
	{
		//i = liczba;
	}
	else if (rozkaz == "JNZ") //jump if not zero , skok wzarunkowy
	{
		//if (rej1 != 0)
			//i = liczba;
	}



	//norberto potoki
	else if (rozkaz == "SP") //stworz potok
	{
		//createpipe(tree.find_proc(rej1),tree.find_proc(rej2));//dziecko rodzic, rodzic dziecko
	}
	else if (rozkaz == "UP") //usun potok
	{
		//int id = std::stoi(memoryManager.GET(tree.proc,licznik_rozkazow));
		//deletePipe(tree.Get_process(id));
	}
	
	else if (rozkaz == "SM")//send message 
	{
		/*PCB p1;
		p1 = tree.Get_process_1(rej1);
		if (p1.Descriptor[0] >= 0)
		{
			pipeline.pipes[p1.Descriptor[0]]->write(rej2);
		}
		else
		{
			std::cout << "Proces nie przypisany do potoku" << std::endl;
		}*/
	}

	else if (rozkaz == "RM") //read message 
	{
		/*int dlugosc;
		rej2 = pobierzRozkaz(mm, pcb);
		std::string adres;
		adres = pobierzRozkaz(mm, pcb);
		int adr;
		adr = stoi(adres);
		dlugosc = stoi(pobierzRozkaz(mm, pcb));
		PCB p1;
		p1 = tree.Get_process_1(rej2);
		if (p1.Descriptor[0] >= 0)
		{
			rej1 = pipeline.pipes[p1.Descriptor[0]]->read(dlugosc);
			if (mm.Write(&p1, adr, rej1) == -1)
			{
				planista.make_zombie(p1, tree, mm);
				std::cout << "Pamiec pelna!" << std::endl;
			}
			else {
				std::cout << "Odczytana wiadomosc: " << rej1;
			}

		}
		else
		{
			std::cout << "Proces nie przypisany do potoku" << std::endl;
		}
		StanRej();
		zapiszRejestry(pcb);*/
	} 




	else if (rozkaz == "CLS") { system("cls"); }
	
	
	else if (rozkaz == "DSD") //wypisz dysk
	{
		fileManager.DisplayBitVector();
		fileManager.DisplayDiskContentChar();
	} 



	//pliki
	else if (rozkaz == "CF")// stworzenie pliku
	{
		fileManager.FileCreate(nazwa);//nazwa i dane
	}
	else if (rozkaz == "FO")// otwarcie pliku, ma flage ze jest otwarty
	{
	if (!fileManager.FileOpen(nazwa))
		std::cout << "teorzenie pliku o nazwie";
		
	}
	else if (rozkaz == "FW")//zapisz do pliku
	{
		fileManager.FileWriteData(nazwa,std::to_string(*rej1));//nazwa pliku i caly program w stringu
	}
	else if (rozkaz == "FC")//ZAMKNIJ plik
	{
		fileManager.FileClose(nazwa);//nazwa pliku 
	}
	else if (rozkaz == "FR")//CZYTANIE Z PLIKU
	{
		*rej1 = std::stoi(fileManager.FileReadData(nazwa));//nazwa pliku 
	}



	//pamiec
	else if (rozkaz == "CP") {} //czytaj pamiec/potok?
	else if (rozkaz == "PP") {} //pokaz pamiec



	//procesy
	else if (rozkaz == "TP") //wyswietlanie drzewa procesow		
	{
		tree.display_tree();
	}
	else if (rozkaz == "CP") //tworzenie procesu
	{
		tree.fork(new PCB("proces1", 1), "proces1");
	}
	else if (rozkaz == "DP") //zabijanie procesu
	{
		tree.fork(new PCB("proces1", 1), "proces1"); // nope chyba
	}



	else if (rozkaz == "HLT") {}// koniec procesu




	else { std::cout << "error\n";  }


	//teraz zapisz rejestry
	/*pcb.PID = PID;
		pcb.Reg1 = rejA;
		pcb.Reg2 = rejB;
		pcb.Reg3 = rejC;
		pcb.Reg4 = rejD;
		pcb.Command_counter = liczRoz;
		pcb.CPU += liczRoz;  */

}