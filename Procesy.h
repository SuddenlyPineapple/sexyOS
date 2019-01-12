#ifndef SEXYOS_PROCESY_H
#define SEXYOS_PROCESY_H
#include <string>
#include <vector>
#include <array>
#include <iostream>

struct PageTableData;
class MemoryManager;

enum Process_state {
	NEW, READY, RUNNING, WAITING, TERMINATED, ZOMBIE, ORPHAN
};// stany procesu

class PCB {
public:
	int priority;					// priorytet
	int last_counter;				//dla procesora
	std::string process_name;		//nazwa procesu
	unsigned int PID;				//identyfikator nie bedzie ujemnych 1 JEST "DLA SYSTEMD"
	Process_state state;			//stan procesu
	PCB *parent_proc;				//wskaznik na ojca procesu
	std::vector<PCB> child_vector;	//vector dzieci
	std::vector<PageTableData> *pageList;//wska?nik wektora stronic
	std::array<int, 2>  FD;			//dla krzysia deskryptor
	unsigned int proces_size;		//rozmiar procesu w stronicach(chyba).
	int A, B, C, D;					//Rejestry dla interpretera
	std::vector<std::string> open_files; //otwarte plik
	int comand_counter;				//licznik rozkazów


	PCB() {//kontruktor dla systemd
		this->priority = 12;
		this->last_counter = 0;
		this->state = NEW;
		this->process_name = "systemd";
		this->PID = 1;
		this->FD[0] = -1;
		this->FD[1] = -1;
		this->A = 0, this->B = 0, this->C = 0, this->D = 0;
		this->parent_proc = nullptr;
		this->proces_size = 16;
	};

	PCB(const std::string& name, const int& father_PID) {//kontruktor innych
		this->priority = 12;
		this->last_counter = 0;
		this->process_name = name;
		this->PID = father_PID;
		this->state = READY;
		// this->proces_size = 16;//nwm ile ma byc

	};

	void Set_PID(int i);//funcja pomocnicza
	PCB *GET_kid(unsigned int PID);// funckja pomocnicza do znalezienia procesu po PID
	PCB *GET_kid(const std::string& nazwa);// funckja pomocnicza do znalezienia procesu po nazwie
	bool find_kid(unsigned int PID);

	void display_allkids();//funkcja która pokazuje dzieci procesu i dzieci dzieci
	void display_allkids(int a);//funkcja pomocnicza do tej wy¿ej
	void change_state(Process_state x);//zmiana stanu procesu
	void add_file_to_proc(const std::string& open_file);
	void kill_all_childrens(MemoryManager &mm);



};

//klasa pomocniczna inicjujemy j¹ na poczatku konstruktorem domyslny i zawiera systemd czyli g³owny proces
class proc_tree {
private:
	MemoryManager* mm;

public:
	PCB proc;

	//tu fork tworz¹cy z jakiegos pliku
	void fork(PCB proc, const std::string& name, const std::string& file_name, int rozmiar);

	// tu jest funckja tworzaca proces z memory managera takze marcin uzywaj go
	void fork(PCB proc, const std::string& name, int rozmiar); 
	
	//dodaje kopieprocesu(dzieciaka) procesu do drzewa
	void fork(PCB proc, const std::string& name);

	//to samo co wyzej tylko z nazwa pliku ktory moze otworzyc
	//void fork(PCB *proc, const std::string name, MemoryManager mm, int rozmiar, std::string open_file);

	//to samo co wyzej tylko z vectorem plików ktore moze otworzyc
	//tutaj memory mangment z przydzielaniem pamieci
	void fork(PCB proc, const std::string& name, std::vector<std::string> file_names);

	//usuwanie procesów 
	void exit(int pid);

	//wyswietla cale drzewa
	void display_tree();

	//znajduje proces przez PID 
	PCB* find_proc(int PID);

	//znajduje proces po nazwie
	PCB* find_proc(const std::string& nazwa);

	//Konstruktory
	proc_tree(MemoryManager* mm_) : mm(mm_) { this->proc = PCB(); }
	proc_tree(MemoryManager* mm_, PCB proc) : mm(mm_) { this->proc = proc; };
};

#endif  //SEXYOS_PROCESY_H