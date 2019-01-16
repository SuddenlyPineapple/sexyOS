#include "Interpreter.h"
#include "FileManager.h"
#include "MemoryManager.h"
#include "Procesy.h"
#include "pipe.h"
#include <iostream>


//				Uwagi:
//pamietaj by usunac kiedys te komentarze.																					-- Marcin
//RF,MOV-- wpisanie danych pod adres MOVem zrobic?  wybór lokalizacji zapisu, daj nic, albo adres							-- Marcin
//MF	-- nieskonczona petla																								-- Tomek


void display_file_error_text(const int &outcome) { //komunikaty o bledach (w sumie tylko do plikow)
	if (outcome == FILE_ERROR_NONE) { return; }
	else if (outcome == FILE_ERROR_EMPTY_NAME) { std::cout << "Pusta nazwa!\n"; }
	else if (outcome == FILE_ERROR_NAME_TOO_LONG) { std::cout << "Nazwa za dluga!\n"; }
	else if (outcome == FILE_ERROR_NAME_USED) { std::cout << "Nazwa zajeta!\n"; }
	else if (outcome == FILE_ERROR_NO_INODES_LEFT) { std::cout << "Osiagnieto limit plikow!\n"; }
	else if (outcome == FILE_ERROR_DATA_TOO_BIG) { std::cout << "Dane za duze!\n"; }
	else if (outcome == FILE_ERROR_NOT_FOUND) { std::cout << "Nie znaleziono pliku!\n"; }
	else if (outcome == FILE_ERROR_NOT_OPENED) { std::cout << "Plik nie jest otwarty!\n"; }
	else if (outcome == FILE_ERROR_NOT_R_MODE) { std::cout << "Plik nie jest do odczytu!\n"; }
	else if (outcome == FILE_ERROR_NOT_W_MODE) { std::cout << "Plik nie jest do zapisu!\n"; }
	else { std::cout << "Nie obsluzony blad: " << outcome << "\n"; }
}

Interpreter::Interpreter(FileManager* fileManager_, MemoryManager* memoryManager_, proc_tree* tree_, Pipeline* pipeline_) : fileManager(fileManager_),
memoryManager(memoryManager_), tree(tree_), pipeline(pipeline_), A(0), B(0), C(0), D(0) {}

void Interpreter::take_from_proc(const std::string& procName) {
	PCB* runningProc = tree->find_proc(procName);

	instruction_counter = runningProc->instruction_counter;
	if (instrBeginMap.find(procName) == instrBeginMap.end()) {
		instrBeginMap[procName][0] = 0; //Pierwszy rozkaz zawsze tak samo
	}
	instruction_counter = runningProc->instruction_counter;
	RAM_pos = instrBeginMap[procName][instruction_counter];
	A = runningProc->A;
	B = runningProc->B;
	C = runningProc->C;
	D = runningProc->D;
}

void Interpreter::update_proc(const std::string& procName) {
	PCB* runningProc = tree->find_proc(procName);
	runningProc->A = A;
	runningProc->B = B;
	runningProc->C = C;
	runningProc->D = D;
	runningProc->instruction_counter = instruction_counter;
}

std::array<std::string, 3> Interpreter::instruction_separate(const std::string& instructionWhole)
{
	std::string temp;
	std::array<std::string, 3> wynik;// mnemonik, rejestr, rejestr
	unsigned int pozycjaWyniku = 0;

	for (const char& c : instructionWhole)
	{
		if (c != ' ')
		{
			temp += c;
		}
		else if (!temp.empty())
		{
			wynik[pozycjaWyniku] = temp;
			temp.clear();
			pozycjaWyniku++;
		}
	}
	return wynik;
}

void Interpreter::jump_pos_set(const std::string& procName) {
	if (instrBeginMap[procName].find(address) != instrBeginMap[procName].end()) {
		instruction_counter = address - 1;
		RAM_pos = instrBeginMap[procName][address];//skok do adr jesli adr juz zmapowany
	}
	else
	{
		//Szukanie najdajszej zmapowanej pozycji
		while (true)
		{//idzie do nast rozkazu, i jesli go nie zna to mapuje w 2gim while
			if (instrBeginMap[procName].find(instruction_counter) == instrBeginMap[procName].end()) { break; }
			//jesli nie znaleziono danego rozkazu w mapie to przerywa i idzie do 2giego while'a by go zmapowac
			else
			{//jesli rozkaz zmapowany to idzie do natepnego
				RAM_pos = instrBeginMap[procName][instruction_counter];
				// jesli znaleziono adres w mapie to sprawdza kolejna pozycje
				instruction_counter++;
			}
		}
		while (instruction_counter < address) {//mapowanie nowych rozkazow, bez ich wykonywania az dojdzie do naszego adresu
			std::string temp = memoryManager->get(tree->find_proc(procName), RAM_pos);
			temp += ' ';
			RAM_pos += temp.length();
			instrBeginMap[procName][instruction_counter] = RAM_pos;
			instruction_counter++;
		} // gdy dojdzie to rampos jest na odpowiedniej pozycji i dopiero wtedy bedzie wykonywac rozkaz
		instruction_counter -= 2; //W wykonaj program siê podnosi
	}
}

void Interpreter::registers_state()
{
	//std::cout << "Rozkaz: " << instructionWhole;
	A = tree->proc.A;
	B = tree->proc.B;
	C = tree->proc.C;
	D = tree->proc.D;
	instruction_counter = tree->proc.instruction_counter;
}

void Interpreter::stan_rejestrow() const
{
	std::cout << "\nA: " << A << "\nB: " << B << "\nC: " << C << "\nD: " << D << "\nilosc rozkazow: " << instruction_counter << "\n\n";
}


//Wykonywanie

void Interpreter::execute_program(const std::string& procName) {
	instruction_counter = 0;
	instrBeginMap.clear();
	instrBeginMap[procName][0] = 0; //Pierwszy rozkaz zawsze tak samo

	while (true)
	{
		std::string instructionWhole;

		//Jeœli jeszcze nie wpisane w mapê
		if (instrBeginMap[procName].find(instruction_counter + 1) == instrBeginMap[procName].end()) {
			instructionWhole = memoryManager->get(tree->find_proc(procName), RAM_pos);
			instructionWhole += ' '; //Spacja na koñcu u³atwia rozdzielanie
			RAM_pos += instructionWhole.length();
			instrBeginMap[procName][instruction_counter + 1] = RAM_pos;
		}
		else {
			RAM_pos = instrBeginMap[procName][instruction_counter];
			instructionWhole = memoryManager->get(tree->find_proc(procName), RAM_pos);//pobieranie linijki kodu
			instructionWhole += ' '; //Spacja na koñcu u³atwia rozdzielanie
			RAM_pos += instructionWhole.length();
		}

		std::cout << "Rozkaz (IC " << instruction_counter << "): " << instructionWhole << '\n';

		if (!execute_instruction(instructionWhole, procName)) { break; }// jesli napotkano na hlt, to konczymy program
		instruction_counter++;
	}
}

bool Interpreter::execute_line(const std::string& procName) {
	PCB* runningProc = tree->find_proc(procName);

	take_from_proc(procName); //Œci¹ga rejestry i inne z procesu

	std::string instructionWhole;

	//Jeœli jeszcze nie wpisane w mapê
	if (instrBeginMap[procName].find(instruction_counter + 1) == instrBeginMap[procName].end()) {
		instructionWhole = memoryManager->get(tree->find_proc(procName), RAM_pos);
		instructionWhole += ' '; //Spacja na koñcu u³atwia rozdzielanie
		RAM_pos += instructionWhole.length();
		instrBeginMap[procName][instruction_counter + 1] = RAM_pos;
	}
	else {
		RAM_pos = instrBeginMap[procName][instruction_counter];
		instructionWhole = memoryManager->get(tree->find_proc(procName), RAM_pos);//pobieranie linijki kodu
		instructionWhole += ' '; //Spacja na koñcu u³atwia rozdzielanie
		RAM_pos += instructionWhole.length();
	}

	std::cout << "Rozkaz (IC " << instruction_counter << "): " << instructionWhole << "\n";

	if (!execute_instruction(instructionWhole, procName)) {
 		instrBeginMap.erase(procName);
		update_proc(procName);

		std::cout << " | PID Procesu : " << runningProc->PID << '\n';
		std::cout << " | Licznik Instrukcji : " << instruction_counter << '\n';
		std::cout << " | A : " << A << '\n';
		std::cout << " | B : " << B << '\n';
		std::cout << " | C : " << C << '\n';
		std::cout << " | D : " << D << "\n\n";

		return false;
	} // Jesli napotkano na hlt, to konczymy program

	instruction_counter++;
	update_proc(procName);

	std::cout << " | PID Procesu : " << runningProc->PID << '\n';
	std::cout << " | Licznik Instrukcji : " << instruction_counter << '\n';
	std::cout << " | A : " << A << '\n';
	std::cout << " | B : " << B << '\n';
	std::cout << " | C : " << C << '\n';
	std::cout << " | D : " << D << "\n\n";

	return true;
}

bool Interpreter::execute_instruction(const std::string& instructionWhole, const std::string& procName)
{
	PCB* runningProc = tree->find_proc(procName);
	std::array<std::string, 3> instructionParts = instruction_separate(instructionWhole);
	const std::string instruction = instructionParts[0];

	//registers_state();
	number = -1; address = -1;
	std::string nazwa1 = ""; //string w ""
	std::string nazwa2 = ""; //string w ""

	int *reg1 = &A;
	int *reg2 = reg1;

	//Wpisywanie wartoœci do rej1 (pierwszy wyraz rozkazu)
	if (!instructionParts[1].empty()) {
		if (instructionParts[1] == "A") reg1 = &A;
		else if (instructionParts[1] == "B") reg1 = &B;
		else if (instructionParts[1] == "C") reg1 = &C;
		else if (instructionParts[1] == "D") reg1 = &D;
		else if (instructionParts[1][0] == '[')
		{
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			address = std::stoi(instructionParts[1]);
		}
		else if (instructionParts[1][0] == '"')
		{
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			nazwa1 = instructionParts[1];
		}
		else { number = std::stoi(instructionParts[2]); reg1 = &number; }
	}

	//Wpisywanie wartoœci do rej2 (drugi wyraz rozkazu)
	if (!instructionParts[2].empty()) {
		if (instructionParts[2] == "A") reg2 = &A;
		else if (instructionParts[2] == "B") reg2 = &B;
		else if (instructionParts[2] == "C") reg2 = &C;
		else if (instructionParts[2] == "D") reg2 = &D;
		else if (instructionParts[2] == "R") *reg2 = FILE_OPEN_R_MODE;
		else if (instructionParts[2] == "W") *reg2 = FILE_OPEN_W_MODE;
		else if (instructionParts[2][0] == '[')
		{
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			address = std::stoi(instructionParts[2]);
		}
		else if (instructionParts[2][0] == '"')
		{
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			nazwa2 = instructionParts[2];
		}
		else { number = std::stoi(instructionParts[2]); reg2 = &number; }
	}

	//Rozkazy interpretacja
	{
		if (instruction == "ADD")
		{
			*reg1 += *reg2;
		}
		else if (instruction == "SUB")
		{
			*reg1 -= *reg2;
		}
		else if (instruction == "MUL")
		{
			*reg1 *= *reg2;
		}
		else if (instruction == "DIV")
		{
			if (*reg2 == 0)
				std::cout << "nie wykonano rozkazu, dzielenie przez 0\n";
			else *reg1 /= *reg2;
		}
		else if (instruction == "MOV")
		{
			*reg1 = *reg2;
		}
		else if (instruction == "INC")
		{
			(*reg1)++;
		}
		else if (instruction == "DEC")
		{
			(*reg1)--;
		}


		//Rozkazy skoki
		else if (instruction == "JMP")
		{
			jump_pos_set(procName);
		}
		else if (instruction == "JZ") //jump if zero , skok warunkowy
		{
			if (*reg1 == 0)
			{
				jump_pos_set(procName);
			}
		}


		//Rozkazy pliki
		else if (instruction == "MF")// stworzenie pliku
		{
			display_file_error_text(fileManager->file_create(nazwa1, runningProc->name));
		}
		else if (instruction == "OF") // otwarcie pliku, ma flage ze jest otwarty
		{
			if (fileManager->file_open(nazwa1, runningProc->name, *reg2) != FILE_ERROR_NONE) {
				std::cout << "Blad!\n";
			}
		}
		else if (instruction == "WF")//nadpisz do pliku
		{
			display_file_error_text(fileManager->file_write(nazwa1, runningProc->name, std::to_string(*reg2)));
		}
		else if (instruction == "AF")//dopisz do pliku
		{
			display_file_error_text(fileManager->file_append(nazwa1, runningProc->name, std::to_string(*reg2)));
		}
		else if (instruction == "CF")//ZAMKNIJ plik
		{
			display_file_error_text(fileManager->file_close(nazwa1, runningProc->name));
		}
		else if (instruction == "RF")//CZYTANIE Z PLIKU
		{
			std::string temp;
			display_file_error_text(fileManager->file_read_all(nazwa1, runningProc->name, temp));
			std::cout << "\na prog to " << temp;

		}


		//Rozkazy procesy
		else if (instruction == "CP") //tworzenie procesu
		{
			tree->fork(new PCB(nazwa1, runningProc->PID), runningProc->proces_size);
		}
		else if (instruction == "DP") //zabijanie procesu
		{
			tree->exit(tree->find_proc(nazwa1)->PID);
		}


		//Rozkazy potoki
		else if (instruction == "SP") //stworz potok
		{
			pipeline->createPipe(runningProc->name, nazwa1);//rodzic,dziecko  
		}
		else if (instruction == "UP") //usun potok
		{
			pipeline->deletePipe(runningProc->name, nazwa1);
		}
		else if (instruction == "SM") //send message 
		{

			if (nazwa2.size() > 0) pipeline->write(runningProc->name, nazwa1, nazwa2);
			else
				pipeline->write(runningProc->name, nazwa1, std::to_string(*reg2));

		}
		else if (instruction == "RM") //read message 
		{
			if (pipeline->existPipe(runningProc->name, nazwa1))
				std::cout << "odczytana wiadomosc: " << pipeline->read(nazwa1, runningProc->name, *reg2) << '\n';//wysylajacy, odbierajacy, dlugosc plku
		}


		//Rozkaz koniec procesu
		else if (instruction == "HLT") { return false; }

		//B³¹d
		else { std::cout << "error\n"; }
	}


	//teraz zapisz rejestry



	return true;
}
