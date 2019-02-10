#ifndef SEXYOS_PROCESY_H
#define SEXYOS_PROCESY_H
#include <string>
#include <vector>
#include <array>
#include <map>
#include <memory>

struct PageTableData;
class MemoryManager;
class Planist;
class Pipeline;

enum Process_state {
	READY, RUNNING, WAITING, TERMINATED
};// stany procesu

class PCB {
public:
	int priority;					// priorytet
	int lastCounter = 0;			//dla procesora
	std::string name;				//nazwa procesu
	unsigned int PID;				//identyfikator nie będzie ujemnych, 1 jest "dla system_dummy"
	Process_state state;			//stan procesu
	std::shared_ptr<PCB> parentProc = nullptr;		//wskaznik na ojca procesu
	std::vector<std::shared_ptr<PCB>> childVector;	//vector dzieci
	std::vector<PageTableData> *pageList = nullptr; //wskaźnik wektora stronic

	//Mapa deskryptorów dla pipe
	//Klucz to nazwa procesu (z dodatkiem)
	std::map<std::string, std::array<int, 2>>  FD;

	unsigned int procesSize = 16;	//rozmiar procesu w bajtach
	//Rejestry dla interpretera
	std::array<int, 4> registers{ 0,0,0,0 };
	int instructionCounter = 0;		//licznik rozkaz�w

	//kontruktor dla system_dummy
	PCB() {
		this->priority = 20;
		this->name = "system_dummy";
		this->PID = 1;
		this->state = READY;
	}

	//kontruktor innych
	PCB(const std::string& name, const int& father_PID) {
		this->priority = 5;
		this->name = name;
		this->PID = father_PID;
		this->state = READY;
	}

	void set_PID(const unsigned int& i);	// setter
	std::shared_ptr<PCB> get_kid(const unsigned int& PID);  // funckja pomocnicza do znalezienia procesu po PID
	std::shared_ptr<PCB> get_kid(const std::string& nazwa); // funckja pomocnicza do znalezienia procesu po nazwie
	bool find_kid(const unsigned int& PID) const;

	void display_kids();//funkcja która pokazuje całe potomstwo procesu
	void display_kids(int a); //funkcja pomocnicza do tej wyżej
	void change_state(const Process_state& x);//zmiana stanu procesu
	void kill_all_childrens();
	void delete_pipe();

	void display();

};

//klasa pomocniczna inicjujemy ją na początku konstruktorem i zawiera system_dummy czyli proces bezczynności
class ProcTree {
private:
	unsigned int free_PID = 2;
	std::shared_ptr<PCB> dummyProc;

public:

	//tu fork tworz�cy z jakiegos pliku
	void fork(const std::shared_ptr<PCB>& proc, const std::string& file_name, const int& size);

	// tu jest funckja tworząca proces z memory managera takze marcin uzywaj go
	void fork(const std::shared_ptr<PCB>& proc, int size);

	//dodaje kopię procesu(dzieciaka) procesu do drzewa
	void fork(const std::shared_ptr<PCB>& proc);

	//usuwanie procesów 
	void exit(const unsigned& pid);

	//Inicjalizacja pamięci
	void init();

	//wyswietla cale drzewa
	void display_tree();

	//znajduje proces przez PID 
	std::shared_ptr<PCB> find_proc(const unsigned& PID);

	//znajduje proces po nazwie
	std::shared_ptr<PCB> find_proc(const std::string& nazwa);

	//Konstruktory
	ProcTree() = default;
};

extern ProcTree tree;

#endif  //SEXYOS_PROCESY_H