#include "interpreter.h"
#include "FileManager.h"
#include "MemoryManager.h"
#include "Procesy.h"
#include <iostream>
#include <map>


//				Uwagi:
// pamietaj by usunac kiedys te komentarze.
// chyba bede musial zmienic niektore nazwy rozkazow w tych ifach, by byly 2literowe.
// gdy polacze sie z memorymanager, pewnie strace dane o tym, ktory to bajt i czy ostatni w rozkazie, wiec UWAGA!


int bajtRAM = 0; //miejsce bajtu w procesie

//Mapa pocz¹tków instrukcji
//Klucz	  - numer rozkazu
//Wartoœæ - adres pierwszego bajtu rozkazu
std::map<int, int> poczatkiRozkazow;

Interpreter::Interpreter(FileManager* fileManager_, MemoryManager* memoryManager_, proc_tree* tree_) : fileManager(fileManager_),
memoryManager(memoryManager_), tree(tree_), A(0), B(0), C(0), D(0) {}

std::array<std::string, 3> Interpreter::rozdziel_rozkaz(const std::string& rozkazCaly)
{
	std::string temp;
	std::array<std::string, 3> wynik;
	unsigned int pozycjaWyniku = 0;

	for (const char& c : rozkazCaly)
	{
		if (c != ' ')
		{
			temp += c;
		}
		else
		{
			wynik[pozycjaWyniku] = temp;
			temp.clear();
			pozycjaWyniku++;
		}
	}
	return wynik;
}

void Interpreter::rejestr_rozkaz()
{
	std::cout << "Rozkaz: " << rozkazCaly;

	A = tree->proc.A;
	B = tree->proc.B;
	C = tree->proc.C;
	D = tree->proc.D;


	licznik_rozkazow = tree->proc.comand_counter;
}


void Interpreter::stan_rejestrow() const
{
	std::cout << "\nA: " << A << "\nB: " << B << "\nC: " << C << "\nD: " << D << "\nilosc rozkazow: " << licznik_rozkazow << "\n\n";
}

bool Interpreter::wykonanie_rozkazu()
{

	std::cout << "BajtRAM: " << bajtRAM << '\n';
	std::cout << "Licznik rozkazow: " << licznik_rozkazow << '\n';
	rozkazCaly = memoryManager->get((tree->proc.GET_kid(2)), bajtRAM);
	poczatkiRozkazow[licznik_rozkazow] = bajtRAM;

	std::cout << "Caly rozkaz: " << rozkazCaly << '\n';
	rozkazCaly += ' ';
	bajtRAM += rozkazCaly.length(); //+1 bo œrednik jeszcze
	std::array<std::string, 3> rozkazCzesci = rozdziel_rozkaz(rozkazCaly);
	const std::string rozkaz = rozkazCzesci[0];

	//rejestr_rozkaz();
	liczba = -1; adres = -1;
	licznik_rozkazow++;
	std::string nazwa; //nazwa pliku

	int *rej1 = &A;
	int *rej2 = rej1;

	nazwa = "dzialaj";


	//Wpisywanie wartoœci do rej1 (pierwszy wyraz rozkazu)
	if (!rozkazCzesci[1].empty()) {
		if (rozkazCzesci[1] == "A")
			rej1 = &A;
		else if (rozkazCzesci[1] == "B")
			rej1 = &B;
		else if (rozkazCzesci[1] == "C")
			rej1 = &C;
		else if (rozkazCzesci[1] == "D")
			rej1 = &D;
		else if (rozkazCzesci[1][0] == '[')
		{
			rozkazCzesci[1].erase(0);
			rozkazCzesci[1].pop_back();
			adres = std::stoi(rozkazCzesci[1]);
		}
		else if (rozkazCzesci[1][0] == '"')
		{
			rozkazCzesci[1].erase(0);
			rozkazCzesci[1].pop_back();
			nazwa = rozkazCzesci[1];
		}
		else liczba = std::stoi(rozkazCzesci[1]);
	}

	//Wpisywanie wartoœci do rej2 (drugi wyraz rozkazu)
	if (!rozkazCzesci[2].empty()) {
		if (rozkazCzesci[2] == "A") rej2 = &A;
		else if (rozkazCzesci[2] == "B") rej2 = &B;
		else if (rozkazCzesci[2] == "C") rej2 = &C;
		else if (rozkazCzesci[2] == "D") rej2 = &D;
		else if (rozkazCzesci[2] == "R") *rej2 = OPEN_R_MODE;
		else if (rozkazCzesci[2] == "W") *rej2 = OPEN_W_MODE;
		else if (rozkazCzesci[2] == "RW") *rej2 = OPEN_RW_MODE;
		else if (rozkazCzesci[2][0] == '[')
		{
			rozkazCzesci[2].erase(0);
			rozkazCzesci[2].pop_back();
			adres = std::stoi(rozkazCzesci[2]);
		}
		else if (rozkazCzesci[2][0] == '"')
		{
			rozkazCzesci[2].erase(0);
			rozkazCzesci[2].pop_back();
			nazwa = rozkazCzesci[2];
		}
		else liczba = std::stoi(rozkazCzesci[2]);
	}

	//Rozkazy arytmetyczne
	if (rozkaz == "ADD")
	{
		if (liczba == -1)
			*rej1 += *rej2;
		else
			*rej1 += liczba;
	}
	else if (rozkaz == "SUB")
	{
		if (liczba == -1)
			*rej1 -= *rej2;
		else
			*rej1 -= liczba;
	}
	else if (rozkaz == "MUL")
	{
		if (liczba == -1)
			*rej1 *= *rej2;
		else
			*rej1 *= liczba;
	}
	else if (rozkaz == "DIV")
	{
		if (liczba == -1)
			*rej1 /= *rej2;
		else if (liczba == 0)
			std::cout << "nie wykonano rozkazu, dzielenie przez 0\n";
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

	//Rozkazy pamiêæ RAM
	else if (rozkaz == "MW") {}//memory write
	else if (rozkaz == "MR")
	{
		memoryManager->showMem();
	}//memory read

	//Rozkazy skoki
	else if (rozkaz == "JMP")
	{
		licznik_rozkazow = adres;
		bajtRAM = poczatkiRozkazow[licznik_rozkazow];
	}
	else if (rozkaz == "JZ") //jump if zero , skok warunkowy
	{
		if (*rej1 == 0)
		{
			licznik_rozkazow = adres;
			bajtRAM = poczatkiRozkazow[licznik_rozkazow];
		}
	}


	//Rozkazy potoki
	else if (rozkaz == "SP") //stworz potok
	{
		//createpipe(tree->find_proc(rej1),tree->find_proc(rej2));//rodzic,dziecko  
	}
	else if (rozkaz == "UP") //usun potok
	{
		//int id = std::stoi(memoryManager->GET(tree->proc,licznik_rozkazow));
		//deletePipe(tree->Get_process(id));
	}
	else if (rozkaz == "SM")//send message 
	{
		/*PCB p1;
		p1 = tree->Get_process_1(rej1);
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
		p1 = tree->Get_process_1(rej2);
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

	//Rozkazy wyœwietlanie
	else if (rozkaz == "DSD") //wypisz dysk
	{
		fileManager->display_bit_vector();
		fileManager->display_disk_content_char();
	}

	//Rozkazy pliki
	else if (rozkaz == "MF")// stworzenie pliku
	{
		fileManager->file_create(nazwa);//nazwa
	}
	else if (rozkaz == "OF") // otwarcie pliku, ma flage ze jest otwarty
	{
		if(nazwa == "R") {  }
		if (fileManager->file_open(nazwa, *rej2) != FILE_ERROR_NONE) {
			std::cout << "Blad!\n";
		}
	}
	else if (rozkaz == "WF")//nadpisz do pliku
	{
		fileManager->file_write(nazwa, std::to_string(*rej2));//nazwa pliku i caly program w stringu
	}
	else if (rozkaz == "AF")//dopisz do pliku
	{
		fileManager->file_append(nazwa, std::to_string(*rej2));//nazwa pliku i caly program w stringu
	}
	else if (rozkaz == "CF")//ZAMKNIJ plik
	{
		fileManager->file_close(nazwa);//nazwa pliku 
	}
	else if (rozkaz == "RF")//CZYTANIE Z PLIKU
	{
		std::string temp;
		std::cout << "\nkod bledu: " << fileManager->file_read_all(nazwa, temp);//nazwa pliku, string do zapisu
		std::cout << "\na prog to " << temp;
		//*rej2 = std::stoi(temp);
	}

	//Rozkazy pamiêæ
	else if (rozkaz == "PP") {} //pokaz pamiec

	//Rozkazy procesy
	else if (rozkaz == "TP") //wyswietlanie drzewa procesow		
	{
		tree->display_tree();
	}
	else if (rozkaz == "CP") //tworzenie procesu
	{
		tree->fork(&tree->proc, nazwa, *memoryManager, tree->proc.proces_size);//Pid rodzica,nazwa dziecka,rozmiar programu rodzica
	}
	else if (rozkaz == "DP") //zabijanie procesu
	{
		tree->fork(new PCB("proces1", 1), "proces1"); // nope chyba
	}

	//Rozkaz koniec procesu
	else if (rozkaz == "HLT") { return false; }

	//B³¹d
	else { std::cout << "error\n"; }


	//teraz zapisz rejestry
	PCB* runningProc = tree->proc.GET_kid(2);
	runningProc->A = A;
	runningProc->B = B;
	runningProc->C = C;
	runningProc->D = D;
	runningProc->comand_counter = licznik_rozkazow;
	//runningProc->CPU += licznik_rozkazow; //Spytaæ Julka

	std::string krok;
	std::cout << "\n";
	std::cin >> krok;
	if (krok == "r") stan_rejestrow();

	return true;
}