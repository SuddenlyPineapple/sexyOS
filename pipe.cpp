#include "pipe.h"
//znaczenie fd(deksryptor)
//0-koniec do czytania
//1-koniec do zapisu
//-1-oznaczenie ze potok nie jest ju¿ potrzebny
//FD dodaæ w zarzadzaniu procesami
//ktos ma pomysl jak oznaczy pipy by interpreter sie nie pomyli³?

Pipe::Pipe(PCB& p1, PCB &p2, Pipeline& p) {//konstruktor pipe
	this->p1 = &p1;
	this->p2 = &p2;
	this->p = &p;
}
Pipe::Pipe(PCB& p1, PCB& p2, Pipeline& p) {
	p1->FD[0] = -1;
	p2->FD[0] = -1;
}
std::string Pipe::read(size_t rozmiar) {
	std::string t;//string z wiadomoscia
	if (p2->FD[1] == 0) {//sprawdzanie czy to odpowiedni koniec rury
		if (buffer.empty()) {//sprawdzanie czy jest cos w kolejce
			p2;//zmiana statusu procesu na czekanie bo buffor jest pusty(nie wiem jaka funkcja to zmienia jeszcze)
		}
		else {
			if (rozmiar > buffer.size()) {//wiadomosc d³u¿sza ni¿ romiar buffera
				while (buffer.size() > 0) {
					t.push_back(buffer.front());
					buffer.pop();
				}
			}
			else {
				for (int i = 0; i < rozmiar; i++) {
					t.push_back(buffer.front());
					buffer.pop();
				}
			}
		}
	}else if (p2->FD[1] != 0) {//sytuacja jesli jest to z³y koniec pipa,
		std::cout << "Nie ta strona rury xD" << std::endl;
	}
	return t;
}
void Pipe::write(const std::string &wiadomosc) {
	if (p1->FD[1] == 1) {//sprawdzanie czy to dobry koniec
		for (auto x : wiadomosc) {
			buffer.push(x);
		}
		p2;//funkcja zmiany procesu na ready ma tu byæ
	}
	else if (p1->FD[1] != 1) {
		std::cout << "Nie ten koniec rury xD" << std::endl;
	}
}
void Pipeline::createPipe(PCB &p1, PCB& p2) {//tworzenie
	for (int i = 0; i < pipes.size(); i++) {//zapisywanie do wektora pipów
		if(pipes[i] == NULL){
			Pipe* v = new Pipe(p1, p2, (*this));//na wolnym miejscu
			pipes[i] = v;
			p1.FD[0] = i;
			p1.FD[1] = 1;
			p2.FD[0] = i;
			p2.FD[1] = 0;
		}
		else {
			Pipe* v = new Pipe(p1, p2, (*this));//na koncu
			pipes.push_back(v);
			p1.FD[0] = pipes.size() - 1;
			p1.FD[1] = 1;
			p2.FD[0] = pipes.size() - 1;
			p2.FD[1] = 0;
		}
	}
}
void Pipeline::deletePipe(PCB &p1) {//usuwanie pipa
	if (p1.FD[0] != -1) {
		int v = p1.FD[0];
		delete(pipes[v]);
		pipes[v] = NULL;
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