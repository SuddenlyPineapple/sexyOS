#pragma once

//#include "Procesy.h"
#include <list>
#include <vector>

//						Zak�adam, �e proces b�dzie mia� priorytet z zakresu 1 - 10, przy czym pocz�tkowy wynosi 5

class Planista {
private:
	std::list<PCB> WaitingPCB;
	std::list<PCB>::iterator Rpcb, Wpcb;
	unsigned char trial;
public:
	std::list<PCB> ReadyPCB;
	Planista() {}
	~Planista() {}
	void Check(){}
	void AddProces(PCB &Proces){}
	void RemoveProces(PCB &Proces){}
	void SortReadyPCB(){}
	void SetPriority(PCB &Proces){}
};




enum Process_state {
	NEW, READY, RUNNING, WAITING, TERMINATED, ZOMBIE, ORPHAN
};// stany procesu
class PCB {
public:
	std::string process_name; //nazwa procesu
	unsigned int PID;//identyfikator nie bedzie ujemnych 1 JEST "DLA SYSTEMD"
	Process_state state;//stan procesu
	PCB *parent_proc;//wskaznik na ojca procesu
	std::vector<PCB *> child_vector;//vector dzieci
	std::vector<PageTableData> *pageList;//wska?nik wektora stronic
	std::array<int, 2>  FD;//dla krzysia deskryptor
	unsigned int proces_size;//rozmiar procesu w stronicach(chyba).
	int A, B, C, D;//Rejestry dla interpretera
	std::vector<std::string> open_files;//otwarte plik
	int comand_counter;//licznik rozkaz�w
	int priority;//priorytet tu mo?e bya jeszcze wirtualny ale nwm czy nasza grupa musi go miea(wirtualny priorytet)
	int last_counter;
	PCB() {//kontruktor dla systemd
		this->FD[0] = -1;
		this->FD[1] = -1;
		this->state = NEW;
		this->process_name = "systemd";
		this->PID = 1;
		this->A = 0, this->B = 0, this->C = 0, this->D = 0;
		this->parent_proc = NULL;
	};

	PCB(const std::string name, int father_PID) {//kontruktor innych
		this->process_name = name;
		this->PID = father_PID;
		this->state = READY;
		this->proces_size = 16;//nwm ile ma byc
		this->priority = 5;//nwm ile ma byc
	};

	void Set_PID(int i);//funcja pomocnicza
	PCB *GET_kid(unsigned int PID);// funckja pomocnicza do znalezienia procesu po PID
	bool find_kid(unsigned int PID);

	void display_allkids();//funkcja kt�ra pokazuje dzieci procesu i dzieci dzieci
	void display_allkids(int a);//funkcja pomocnicza do tej wy?ej
	void change_state(Process_state x);//zmiana stanu procesu
	void update();//fukncja aktualizujaca dane w procesie (do zrobienia)



};
