#ifndef SEXYOS_PROCESY_H
#define SEXYOS_PROCESY_H
#include <string>
#include <vector>
#include <array>
#include <iostream>

struct PageTableData;
class MemoryManager;
class Planista;

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
	std::array<int, 2>  FD {-1,-1}; //dla krzysia deskryptor
	unsigned int proces_size;		//rozmiar procesu w stronicach(chyba).
	int A = 0, B = 0, C = 0, D = 0;	//Rejestry dla interpretera
	std::vector<std::string> open_files; //otwarte plik
	int comand_counter = 0;			//licznik rozkazów


	PCB() {//kontruktor dla systemd
		this->priority = 12;
		this->last_counter = 0;
		this->state = NEW;
		this->process_name = "systemd";
		this->PID = 1;
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
	PCB *GET_kid(const unsigned int& PID);// funckja pomocnicza do znalezienia procesu po PID
	PCB *GET_kid(const std::string& nazwa);// funckja pomocnicza do znalezienia procesu po nazwie
	bool find_kid(const unsigned int& PID) const;

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
	void fork(PCB proc, const std::string& file_name, int rozmiar);

	// tu jest funckja tworzaca proces z memory managera takze marcin uzywaj go
	void fork(PCB proc, int rozmiar); 
	
	//dodaje kopieprocesu(dzieciaka) procesu do drzewa
	void fork(PCB proc);

	//to samo co wyzej tylko z vectorem plików ktore moze otworzyc
	//tutaj memory management z przydzielaniem pamieci
	void fork(PCB proc, std::vector<std::string> file_names);

	//usuwanie procesów 
	void exit(const int& pid);

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