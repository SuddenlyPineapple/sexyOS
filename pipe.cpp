#include "pipe.h"
#include "Procesy.h"
#include <iostream>

//znaczenie fd(deksryptor)
//0-koniec do czytania
//1-koniec do zapisu
//-1-oznaczenie ze potok nie jest już potrzebny



Pipe::Pipe(PCB& p1, PCB &p2, Pipeline& p) {//konstruktor pipe
	this->p1 = &p1;
	this->p2 = &p2;
	this->p = &p;
}

Pipe::~Pipe() {
	p1->FD[0] = -1;
	p2->FD[0] = -1;
}

std::string Pipe::read(size_t rozmiar) {
	std::string t;//string z wiadomoscia
	if (p2->FD[1] == 0) {//sprawdzanie czy to odpowiedni koniec rury
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
	}
	else if (p2->FD[1] != 0) {//sytuacja jesli jest to zły koniec pipa,
		std::cout << "Nie ta strona rury xD" << std::endl;
	}
	return t;
}
void Pipe::write(const std::string &wiadomosc) {
	if (p1->FD[1] == 1) {//sprawdzanie czy to dobry koniec
		for (auto x : wiadomosc) {
			buffer.push(x);
		}
		p2->change_state(READY);//zmiana na ready po odebraniu wiadomosci
	}
	else if (p1->FD[1] != 1) {
		std::cout << "Nie ten koniec rury xD" << std::endl;
	}
}
void Pipeline::createPipe(PCB &p1, PCB& p2) {//tworzenie
	bool created = false;

	for (size_t i = 0; i < pipes.size(); i++) {//zapisywanie do wektora pipów
		if (pipes[i] == nullptr) {
			Pipe* v = new Pipe(p1, p2, (*this));//na wolnym miejscu
			pipes[i] = v;
			p1.FD[0] = i;
			p1.FD[1] = 1;
			p2.FD[0] = i;
			p2.FD[1] = 0;
			created = true;
		}
	}
	if (!created) {
		Pipe* v = new Pipe(p1, p2, (*this));//na koncu
		pipes.push_back(v);
		p1.FD[0] = pipes.size() - 1;
		p1.FD[1] = 1;
		p2.FD[0] = pipes.size() - 1;
		p2.FD[1] = 0;
	}
}
void Pipeline::deletePipe(PCB &p1) {//usuwanie pipa
	if (p1.FD[0] != -1) {
		const int v = p1.FD[0];
		delete(pipes[v]);
		pipes[v] = nullptr;
	}
	else {
		std::cout << "Nie istnieje taki potok" << std::endl;
	}
}
bool Pipeline::existPipe(PCB& p1, PCB& p2) {//sprawdzanie czy istnieje
	if (p1.FD[0] == p2.FD[0] && p1.FD[1] == 1 && p1.FD[0] != -1) {
		return true;//istnieje to zwraca true
	}
	else {
		createPipe(p1, p2);//nie istnieje wiec jest taki pipe tworzony
		return true;
	}
}
