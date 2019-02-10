#include "Interpreter.h"
#include "FileManager.h"
#include "MemoryManager.h"
#include "Processes.h"
#include "Pipe.h"
#include <iostream>

using namespace std;

Interpreter interpreter;

void display_file_error_text(const int &outcome) { //komunikaty o bledach (w sumie tylko do plikow)
	if (outcome == FILE_ERROR_NONE) { return; }
	else if (outcome == FILE_ERROR_EMPTY_NAME) { cout << "Pusta nazwa!\n"; }
	else if (outcome == FILE_ERROR_NAME_USED) { cout << "Nazwa zajeta!\n"; }
	else if (outcome == FILE_ERROR_NO_INODES_LEFT) { cout << "Osiagnieto limit plikow!\n"; }
	else if (outcome == FILE_ERROR_DATA_TOO_BIG) { cout << "Dane za duze!\n"; }
	else if (outcome == FILE_ERROR_NOT_FOUND) { cout << "Nie znaleziono pliku!\n"; }
	else if (outcome == FILE_ERROR_NOT_OPENED) { cout << "Plik nie jest otwarty!\n"; }
	else if (outcome == FILE_ERROR_NOT_R_MODE) { cout << "Plik nie jest do odczytu!\n"; }
	else if (outcome == FILE_ERROR_NOT_W_MODE) { cout << "Plik nie jest do zapisu!\n"; }
	else { cout << "Nie obsluzony blad: " << outcome << "\n"; }
}

Interpreter::Interpreter() = default;

void Interpreter::take_from_proc(const string& procName) {
	shared_ptr<PCB> runningProc = tree.find(procName);

	instructionCounter = runningProc->instructionCounter;
	A = runningProc->registers[0];
	B = runningProc->registers[1];
	C = runningProc->registers[2];
	D = runningProc->registers[3];
}

void Interpreter::update_proc(const string& procName) const {
	shared_ptr<PCB> runningProc = tree.find(procName);
	runningProc->registers[0] = A;
	runningProc->registers[1] = B;
	runningProc->registers[2] = C;
	runningProc->registers[3] = D;
	runningProc->instructionCounter = instructionCounter;
}

array<string, 4> Interpreter::instruction_separate(const string& instructionWhole) {
	string temp;
	array<string, 4> wynik;// mnemonik, rejestr, rejestr
	unsigned int pos = 0;

	for (const char& c : instructionWhole)
	{
		if (c != ' ')
		{
			temp += c;
		}
		else if (!temp.empty())
		{
			wynik[pos] = temp;
			temp.clear();
			pos++;
		}
	}
	return wynik;
}

void Interpreter::display_registers() const {
	cout << " | Licznik Instrukcji (po)    : " << instructionCounter << '\n';
	cout << " | A : " << A << '\n';
	cout << " | B : " << B << '\n';
	cout << " | C : " << C << '\n';
	cout << " | D : " << D << "\n\n";
}


//Wykonywanie
int Interpreter::execute_line(const string& procName) {
	const shared_ptr<PCB> runningProc = tree.find(procName);

	take_from_proc(procName); //Œci¹ga rejestry i inne z procesu

	string instructionWhole;

	//Odczyt instrukcji
	while (true) {
		const char cTemp = mm.get_byte(runningProc, instructionCounter)[0];
		instructionCounter++;
		if (cTemp != ';') { instructionWhole += cTemp; }
		else { instructionWhole += ' '; break; }
	}


	//Wykonanie instrukcji (fa³sz oznacza zakoñczenie - HLT)
	const int result = execute_instruction(instructionWhole, procName);

	if (procName != "system_dummy") {
		if (result != 0) {
			runningProc->executionTimeLeft--;
		}
	}
	cout << "Rozkaz: " << instructionWhole << "\n";
	cout << " | PID Procesu : " << runningProc->PID << '\n';
	cout << " | Pozstalo cykli : " << runningProc->executionTimeLeft << '\n';
	cout << " | Licznik Instrukcji (przed) : " << runningProc->instructionCounter << '\n';
	display_registers();

	update_proc(procName);

	return result;
}

int Interpreter::execute_instruction(const string& instructionWhole, const string& procName) {
	const shared_ptr<PCB> runningProc = tree.find(procName);
	array<string, 4> instructionParts = instruction_separate(instructionWhole);
	const string instruction = instructionParts[0];

	int number = -1;
	unsigned int address = -1;

	string strData1;
	string strData2;

	int reg1_noPtr = 0;
	int reg2_noPtr = 0;

	int* reg1 = &reg1_noPtr;
	int* reg2 = &reg2_noPtr;

	//Wpisywanie wartoœci do reg1 (pierwszy wyraz rozkazu)
	if (!instructionParts[1].empty()) {
		if (instructionParts[1] == "A") reg1 = &A;
		else if (instructionParts[1] == "B") reg1 = &B;
		else if (instructionParts[1] == "C") reg1 = &C;
		else if (instructionParts[1] == "D") reg1 = &D;
		else if (instructionParts[1] == "R") strData1 = "_R";
		else if (instructionParts[1] == "W") strData1 = "_W";
		else if (instructionParts[1][0] == '[')
		{
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			address = stoi(instructionParts[1]);
		}
		else if (instructionParts[1][0] == '"') {
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			strData1 = instructionParts[1];
		}
		else { number = stoi(instructionParts[1]); reg1 = &number; }
	}

	//Wpisywanie wartoœci do reg2 (drugi wyraz rozkazu)
	if (!instructionParts[2].empty()) {
		if (instructionParts[2] == "A") reg2 = &A;
		else if (instructionParts[2] == "B") reg2 = &B;
		else if (instructionParts[2] == "C") reg2 = &C;
		else if (instructionParts[2] == "D") reg2 = &D;
		else if (instructionParts[2] == "R") *reg2 = FILE_OPEN_R_MODE;
		else if (instructionParts[2] == "W") *reg2 = FILE_OPEN_W_MODE;
		else if (instructionParts[2][0] == '[') {
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			address = stoi(instructionParts[2]);
		}
		else if (instructionParts[2][0] == '"') {
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			strData2 = instructionParts[2];
		}
		else { number = stoi(instructionParts[2]); reg2 = &number; }
	}

	//Wpisywanie wartoœci do reg2 (trzeci wyraz rozkazu)
	if (!instructionParts[3].empty()) {
		if (instructionParts[3][0] == '[') {
			instructionParts[3].erase(instructionParts[3].begin());
			instructionParts[3].pop_back();
			address = stoi(instructionParts[3]);
		}
	}


	//Rozkazy interpretacja
	{
		if (instruction == "ADD") { *reg1 += *reg2; }
		else if (instruction == "SUB") { *reg1 -= *reg2; }
		else if (instruction == "MUL") { *reg1 *= *reg2; }
		else if (instruction == "DIV") {
			if (*reg2 == 0) {
				cout << "Dzielenie przez 0! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else { *reg1 /= *reg2; }
		}
		else if (instruction == "MOD") {
			if (*reg2 == 0) {
				cout << "Dzielenie przez 0! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else { *reg1 %= *reg2; }
		}
		else if (instruction == "MOV") { *reg1 = *reg2; }
		else if (instruction == "INC") { (*reg1)++; }
		else if (instruction == "DEC") { (*reg1)--; }
		else if (instruction == "WRITE") {
			if (address > runningProc->size + strData1.length()) {
				if (address + strData1.length() > 256) { return -1; }
				runningProc->resize(address + strData1.length() - 1);
			}

			if (strData2.empty()) {
				string temp;
				temp += static_cast<char>(*reg2);
				mm.write(runningProc, address, temp);
			}
			else {
				mm.write(runningProc, address, strData2);
			}
		}
		else if (instruction == "GET") {
			const int tempi = address;
			*reg2 = mm.get_byte(runningProc, tempi)[0];
		}


		//Rozkazy skoki
		else if (instruction == "JMP") {
			instructionCounter = address;
		}
		//Jump if zero
		else if (instruction == "JZ") {
			if (*reg1 == 0) { instructionCounter = address; }
		}
		//Jump if not zero
		else if (instruction == "JMZ") {
			if (*reg1 != 0) { instructionCounter = address; }
		}


		//Rozkazy pliki

		//Stworzenie pliku
		else if (instruction == "MF") {
			if (fm.file_create(strData1, runningProc->name) == FILE_ERROR_SYNC) {
				instructionCounter -= instructionWhole.length(); //Doliczony œrednik/spacja
				return 0;
			}
		}
		//Otwarcie pliku, ma flage ze jest otwarty
		else if (instruction == "OF") {
			const int result = fm.file_open(strData1, runningProc->name, *reg2);
			if (result == FILE_ERROR_SYNC) {
				instructionCounter -= instructionWhole.length(); //Doliczony œrednik/spacja
				return 0;
			}
			else if (result != FILE_ERROR_NONE) {
				cout << "Blad!\n";
			}
		}
		//Nadpisz do pliku
		else if (instruction == "WF") {
			if (!strData2.empty()) {
				display_file_error_text(fm.file_write(strData1, runningProc->name, strData2));
			}
			else {
				string temp;
				temp += static_cast<char>(*reg2);
				display_file_error_text(fm.file_write(strData1, runningProc->name, temp));
			}
		}
		//Dopisz do pliku
		else if (instruction == "AF") {
			if (!strData2.empty()) {
				display_file_error_text(fm.file_append(strData1, runningProc->name, strData2));
			}
			else {
				string temp;
				temp += static_cast<char>(*reg2);
				display_file_error_text(fm.file_append(strData1, runningProc->name, temp));
			}
		}
		//Czytanie z pliku (i zapisanie do RAM'u)
		else if (instruction == "RF") {
			string temp;
			if (!instructionParts[3].empty()) {
				if (address + *reg2 > 256) {
					const int tooMuch = address + *reg2 - 256;
					*reg2 -= tooMuch;
				}
				if (address + *reg2 > runningProc->size) {
					runningProc->resize(address + *reg2 - 1);
				}
				display_file_error_text(fm.file_read(strData1, runningProc->name, *reg2, temp));
				mm.write(runningProc, address, temp);
				cout << "Odczytano z pliku dane \"" << temp << "\" i zapisano pod adresem " << address << "\n";
			}
			else {
				display_file_error_text(fm.file_read(strData1, runningProc->name, 1, temp));
				if (!temp.empty()) { *reg2 = temp[0]; }
				else { *reg2 = 0; }
				cout << "Odczytano z pliku liczbe \"" << static_cast<int>(temp[0]) << "\" i zapisano do rejestru ";
				if (reg2 == &A) { cout << "A"; }
				else if (reg2 == &B) { cout << "B"; }
				else if (reg2 == &C) { cout << "C"; }
				else if (reg2 == &D) { cout << "D"; }
				cout << "\n";
			}

		}
		//Zamknij plik
		else if (instruction == "CF") {
			display_file_error_text(fm.file_close(strData1, runningProc->name));
		}


		//Rozkazy procesy

		//Tworzenie procesu
		else if (instruction == "CP") { tree.fork(strData1, runningProc->PID, 16); }
		//Zabijanie procesu
		else if (instruction == "DP") { tree.kill(strData1); }


		//Rozkazy potoki

		//Stwórz potok
		else if (instruction == "SP") { pipeline.create(runningProc->name, strData1); }
		//Usuñ potok
		else if (instruction == "UP") { pipeline.remove(runningProc->name); }


		//Odczytaj wiadomoœæ (od rodzica)
		else if (instruction == "RMP") {
			string result;
			if (address != -1) {
				result = pipeline.read(runningProc->name, runningProc->parent->name, *reg1);
			}
			else {
				result = pipeline.read(runningProc->name, runningProc->parent->name, 1);
			}

			if (result == "no_pipe") {
				cout << "Potok nie istnieje! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else if (result == "sem_blocked") {
				//Doliczony œrednik/spacja
				instructionCounter -= instructionWhole.length();
				return 0;
			}

			if (address != -1) {
				if (address + result.length() > 256) {
					const int tooMuch = address + result.length() - 256;
					result.resize(result.length() - tooMuch);
				}
				if (address + result.length() > runningProc->size) {
					runningProc->resize(address + result.length() - 1);
				}
				mm.write(runningProc, address, result);
			}
			else {
				*reg1 = static_cast<char>(result[0]);
				cout << "Odczytano z potoku liczbe \"" << static_cast<int>(result[0]) << "\" i zapisano do rejestru ";
				if (reg2 == &A) { cout << "A"; }
				else if (reg2 == &B) { cout << "B"; }
				else if (reg2 == &C) { cout << "C"; }
				else if (reg2 == &D) { cout << "D"; }
				cout << "\n";
			}
		}
		//Odczytaj wiadomoœæ (od dzieci)
		else if (instruction == "RMK") {
			string firstKidName;
			if (!runningProc->childVector.empty()) { firstKidName = runningProc->childVector[0]->name; }

			string result;
			if (address != -1) {
				result = pipeline.read(runningProc->name, firstKidName, *reg1);
				cout << "Odczytano z potoku liczbe \"" << static_cast<int>(result[0]) << "\" i zapisano do rejestru ";
				if (reg2 == &A) { cout << "A"; }
				else if (reg2 == &B) { cout << "B"; }
				else if (reg2 == &C) { cout << "C"; }
				else if (reg2 == &D) { cout << "D"; }
				cout << "\n";
			}
			else {
				result = pipeline.read(runningProc->name, firstKidName, 1);
			}

			if (result == "no_pipe") {
				cout << "Potok nie istnieje! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else if (result == "sem_blocked") {
				//Doliczony œrednik/spacja
				instructionCounter -= instructionWhole.length();
				return 0;
			}

			if (address != -1) {
				if (address + result.length() > 256) {
					const int tooMuch = address + result.length() - 256;
					result.resize(result.length() - tooMuch);
				}
				if (address + result.length() > runningProc->size) {
					runningProc->resize(address + result.length() - 1);
				}
				mm.write(runningProc, address, result);
			}
			else {
				*reg1 = static_cast<char>(result[0]);
			}
		}

		//Wyœlij wiadomoœæ (do rodzica)
		else if (instruction == "SMP") {
			int result;
			if (!strData1.empty()) { result = pipeline.write(runningProc->parent->name, runningProc->name, strData1); }
			else {
				string temp; temp += static_cast<char>(*reg2);
				result = pipeline.write(runningProc->parent->name, runningProc->name, temp);
			}

			if (result == -1) {
				cout << "Potok nie istnieje! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else if (result == 0) {
				//Doliczony œrednik/spacja
				instructionCounter -= instructionWhole.length();
				return 0;
			}
		}
		//Wyœlij wiadomoœæ (do dzieci)
		else if (instruction == "SMK") {
			int result;
			if (!strData1.empty()) { result = pipeline.write(runningProc->childVector[0]->name, runningProc->name, strData1); }
			else {
				string temp; temp += static_cast<char>(*reg2);
				result = pipeline.write(runningProc->childVector[0]->name, runningProc->name, temp);
			}

			if (result == -1) {
				cout << "Potok nie istnieje! Proces " << runningProc->name << " zostaje zabity!\n";
				return -1;
			}
			else if (result == 0) {
				//Doliczony œrednik/spacja
				instructionCounter -= instructionWhole.length();
				return 0;
			}
		}

		//Rozkaz koniec procesu
		else if (instruction == "HLT") { return -1; }
		//Rozkaz beczynnoœci
		else if (instruction == "NOP") {}

		//B³¹d
		else { cout << "error\n"; }
	}

	return 1;
}

bool Interpreter::simulate_instruction(const string& instructionWhole) {
	array<string, 4> instructionParts = instruction_separate(instructionWhole);
	const string instruction = instructionParts[0];

	int number = -1;
	unsigned int address = -1;

	string strData1;

	int reg1_noPtr = 0;
	int reg2_noPtr = 0;

	int* reg1 = &reg1_noPtr;
	int* reg2 = &reg2_noPtr;

	//Wpisywanie wartoœci do reg1 (pierwszy wyraz rozkazu)
	if (!instructionParts[1].empty()) {
		if (instructionParts[1] == "A") reg1 = &A;
		else if (instructionParts[1] == "B") reg1 = &B;
		else if (instructionParts[1] == "C") reg1 = &C;
		else if (instructionParts[1] == "D") reg1 = &D;
		else if (instructionParts[1] == "R") strData1 = "_R";
		else if (instructionParts[1] == "W") strData1 = "_W";
		else if (instructionParts[1][0] == '[')
		{
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			address = stoi(instructionParts[1]);
		}
		else if (instructionParts[1][0] == '"') {
			instructionParts[1].erase(instructionParts[1].begin());
			instructionParts[1].pop_back();
			strData1 = instructionParts[1];
		}
		else { number = stoi(instructionParts[1]); reg1 = &number; }
	}

	//Wpisywanie wartoœci do reg2 (drugi wyraz rozkazu)
	if (!instructionParts[2].empty()) {
		if (instructionParts[2] == "A") reg2 = &A;
		else if (instructionParts[2] == "B") reg2 = &B;
		else if (instructionParts[2] == "C") reg2 = &C;
		else if (instructionParts[2] == "D") reg2 = &D;
		else if (instructionParts[2] == "R") *reg2 = FILE_OPEN_R_MODE;
		else if (instructionParts[2] == "W") *reg2 = FILE_OPEN_W_MODE;
		else if (instructionParts[2][0] == '[') {
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			address = stoi(instructionParts[2]);
		}
		else if (instructionParts[2][0] == '"') {
			instructionParts[2].erase(instructionParts[2].begin());
			instructionParts[2].pop_back();
			string strData2 = instructionParts[2];
		}
		else { number = stoi(instructionParts[2]); reg2 = &number; }
	}

	//Wpisywanie wartoœci do reg2 (trzeci wyraz rozkazu)
	if (!instructionParts[3].empty()) {
		if (instructionParts[3][0] == '[') {
			instructionParts[3].erase(instructionParts[3].begin());
			instructionParts[3].pop_back();
			address = stoi(instructionParts[3]);
		}
	}


	//Rozkazy interpretacja
	{
		if (instruction == "ADD") { *reg1 += *reg2; }
		else if (instruction == "SUB") { *reg1 -= *reg2; }
		else if (instruction == "MUL") { *reg1 *= *reg2; }
		else if (instruction == "DIV") {
			if (*reg2 == 0) {
				return false;
			}
			else { *reg1 /= *reg2; }
		}
		else if (instruction == "MOD") {
			if (*reg2 == 0) {
				return false;
			}
			else { *reg1 %= *reg2; }
		}
		else if (instruction == "MOV") { *reg1 = *reg2; }
		else if (instruction == "INC") { (*reg1)++; }
		else if (instruction == "DEC") { (*reg1)--; }
		else if (instruction == "WRITE") {}


		//Rozkazy skoki
		else if (instruction == "JMP") {
			instructionCounter = address;
		}
		//Jump if zero
		else if (instruction == "JZ") {
			if (*reg1 == 0) { instructionCounter = address; }
		}
		//Jump if not zero
		else if (instruction == "JMZ") {
			if (*reg1 != 0) { instructionCounter = address; }
		}


		//Rozkazy pliki

		//Stworzenie pliku
		else if (instruction == "MF") {}

		//Otwarcie pliku, ma flage ze jest otwarty
		else if (instruction == "OF") {}

		//Nadpisz do pliku
		else if (instruction == "WF") {}

		//Dopisz do pliku
		else if (instruction == "AF") {}

		//Zamknij plik
		else if (instruction == "CF") {}

		//Czytanie z pliku (i zapisanie do RAM'u)
		else if (instruction == "RF") {}


		//Rozkazy procesy

		//Tworzenie procesu
		else if (instruction == "CP") {}
		//Zabijanie procesu
		else if (instruction == "DP") {}


		//Rozkazy potoki

		//Stwórz potok
		else if (instruction == "SP") {}
		//Usuñ potok
		else if (instruction == "UP") {}


		//Odczytaj wiadomoœæ (od rodzica)
		else if (instruction == "RMP") {}

		//Odczytaj wiadomoœæ (od dzieci)
		else if (instruction == "RMK") {}

		//Wyœlij wiadomoœæ (do rodzica)
		else if (instruction == "SMP") {}

		//Wyœlij wiadomoœæ (do dzieci)
		else if (instruction == "SMK") {}

		//Rozkaz koniec procesu
		else if (instruction == "HLT") { return false; }
		//Rozkaz beczynnoœci
		else if (instruction == "NOP") {}

		//B³¹d
		else { return false; }
	}

	return true;
}

unsigned int Interpreter::simulate_program(const string& programWhole) {
	//Zapisanie rejestrów
	const int Aprev = A, Bprev = B, Cprev = C, Dprev = D;
	const int instructionCounterPrev = instructionCounter;

	//Zerowanie rejestrów
	A = 0; B = 0; C = 0; D = 0;
	instructionCounter = 0;

	unsigned int executionTime = 0;

	while (true) {
		string instructionWhole;
		while (true) {
			if (instructionCounter >= programWhole.length()) { break; }
			if (programWhole[instructionCounter] == ';') {
				instructionWhole += ' ';
				instructionCounter++;
				break;
			}
			else {
				instructionWhole += programWhole[instructionCounter];
			}

			instructionCounter++;
		}
		executionTime++;
		if (!simulate_instruction(instructionWhole)) { break; }
	}

	//Przywrócenie stanów rejestrów
	A = Aprev; B = Bprev; C = Cprev; D = Dprev;
	instructionCounter = instructionCounterPrev;

	return executionTime;
}
