#include "pipe.h"
#include "Procesy.h"
#include <iostream>

//znaczenie fd(deksryptor)
//0-koniec do czytania
//1-koniec do zapisu
//-1-oznaczenie ze potok nie jest już potrzebny

#define READ_END	0
#define WRITE_END	1
#define UNUSED		-1

void Pipeline::createPipe(const std::string& p1, const std::string& p2) {//tworzenie
	PCB* proc1 = tree->find_proc(p1);
	PCB* proc2 = tree->find_proc(p2);

	for (size_t i = 0; i < pipes.size(); i++) {//zapisywanie do wektora pipów
		if (pipes[i] == nullptr) {
			Pipe* v = new Pipe(proc1, proc2, this);//na wolnym miejscu
			pipes[i] = v;
			proc1->FD[p2 + "_W"][0] = i;
			proc1->FD[p2 + "_W"][1] = WRITE_END;
			proc2->FD[p1 + "_R"][0] = i;
			proc2->FD[p1 + "_R"][1] = READ_END;
			return;
		}
	}

	//Jesli nie bylo wolnych
	Pipe* v = new Pipe(proc1, proc2, this);//na koncu
	pipes.push_back(v);
	proc1->FD[p2 + "_W"][0] = pipes.size() - 1;
	proc1->FD[p2 + "_W"][1] = WRITE_END;
	proc2->FD[p1 + "_R"][0] = pipes.size() - 1;
	proc2->FD[p1 + "_R"][1] = READ_END;
}

void Pipeline::write(const std::string& p1, const std::string& p2, const std::string& data) {
	PCB* proc1 = tree->find_proc(p1);
	PCB* proc2 = tree->find_proc(p2);

	if (proc1->FD[p2 + "_W"][0] != UNUSED && proc1->FD[p2 + "_W"][1] == WRITE_END &&
		proc2->FD[p1 + "_R"][0] != UNUSED && proc2->FD[p1 + "_R"][1] == READ_END) {
		pipes[proc1->FD[p2 + "_W"][0]]->write(data);
	}
	else {
		std::cout << "Nie istnieje taki potok" << std::endl;
	}
}

std::string Pipeline::read(const std::string& p1, const std::string& p2, const size_t& rozmiar) {
	PCB* proc1 = tree->find_proc(p1);
	PCB* proc2 = tree->find_proc(p2);
	std::string t;

	if (proc1->FD[p2 + "_R"][0] != UNUSED && proc1->FD[p2 + "_R"][1] == READ_END &&
		proc2->FD[p1 + "_W"][0] != UNUSED && proc2->FD[p1 + "_W"][1] == WRITE_END) {
		t = pipes[proc1->FD[proc2->name + "_R"][0]]->read(rozmiar);
	}
	else {
		std::cout << "Nie istnieje taki potok" << std::endl;
	}
	return t;
}

void Pipeline::deletePipe(const std::string& p1, const std::string& p2) {//usuwanie pipa
	PCB* proc1 = tree->find_proc(p1);
	PCB* proc2 = tree->find_proc(p2);

	if (proc1->FD[p2 + "_W"][0] != UNUSED) {
		const int v = proc1->FD[p2 + "_W"][0];
		delete pipes[v];
		pipes[v] = nullptr;
		proc1->FD[p2 + "_W"][0] = UNUSED;
		proc1->FD[p2 + "_W"][1] = UNUSED;
		proc2->FD[p1 + "_R"][0] = UNUSED;
		proc2->FD[p1 + "_R"][1] = UNUSED;
	}
	else {
		std::cout << "Nie istnieje taki potok" << std::endl;
	}
}

bool Pipeline::existPipe(const std::string& p1, const std::string& p2) {//sprawdzanie czy istnieje
	PCB* proc1 = tree->find_proc(p1);
	PCB* proc2 = tree->find_proc(p2);

	if (proc1->FD[p2 + "_W"][0] == proc2->FD[p1 + "_R"][0] &&
		proc1->FD[p2 + "_W"][1] == WRITE_END && proc2->FD[p1 + "_R"][1] == READ_END) {
		return true;//istnieje to zwraca true
	}
	else {
		return false;
	}
}

Pipe::Pipe(PCB* p1, PCB* p2, Pipeline* p) {//konstruktor pipe
	this->p1 = p1;
	this->p2 = p2;
	this->p = p;
}

Pipe::~Pipe() { }

std::string Pipe::read(size_t rozmiar) {
	std::string t; //string z wiadomoscia
	if (buffer.empty()) {//sprawdzanie czy jest cos w kolejce
		p2->change_state(WAITING);//zmiana na weiting jeśli pusta kolejka
	}
	else {
		if (rozmiar > buffer.size()) { //wiadomosc dłuższa niż romiar buffera
			while (!buffer.empty()) {
				t.push_back(buffer.front());
				buffer.pop();
			}
		}
		else {
			for (size_t i = 0; i < rozmiar; i++) {
				t.push_back(buffer.front());
				buffer.pop();
			}
		}
	}
	return t;
}

void Pipe::write(const std::string &wiadomosc) {
	for (auto x : wiadomosc) {
		buffer.push(x);
	}
	p2->change_state(READY);//zmiana na ready po odebraniu wiadomosci
}
