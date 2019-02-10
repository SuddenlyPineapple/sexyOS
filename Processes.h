#ifndef SEXYOS_PROCESY_H
#define SEXYOS_PROCESY_H
#include <string>
#include <vector>
#include <array>
#include <memory>

struct PageTableData;
class MemoryManager;
class Planist;
class Pipeline;

enum Process_state {
	READY, RUNNING, WAITING, TERMINATED
}; //stany procesu

class PCB {
public:
	std::string name;		//nazwa procesu
	unsigned int PID;		//identyfikator, 1 jest "dla system_dummy"
	Process_state state;	//stan procesu (planista)

	unsigned int executionTimeLeft = 0; //ilość cykli potrzebna do zakończenia programu (planista)

	std::shared_ptr<PCB> parent = nullptr;		//wskaźnik na rodzica procesu
	std::vector<std::shared_ptr<PCB>> childVector;	//wektor dzieci procesu
	
	//wskaźnik wektora stronic (zarządzanie pamięcią)
	std::shared_ptr<std::vector<PageTableData>>pageList = nullptr;

	//Deskryptor (pipe)
	std::array<int, 2>  FD;

	unsigned int size = 16;	//rozmiar procesu w bajtach (zarządzanie pamięcią)
	//Rejestry (interpreter)
	std::array<int, 4> registers{ 0,0,0,0 };
	unsigned int instructionCounter = 0; //licznik rozkazów (interpreter)

	//konstruktor dla system_dummy
	PCB() {
		this->name = "system_dummy";
		this->executionTimeLeft = 9999; //Nie będzie chyba tak długich programów
		this->PID = 1;
		this->state = READY;
		this->FD.fill(0);
	}

	//konstruktor dla zwykłych procesów
	PCB(const std::string& name, const int& parentPID) {
		this->name = name;
		this->PID = parentPID;
		this->state = READY;
		this->FD.fill(0);
	}

	std::shared_ptr<PCB> get_kid(const unsigned int& PID);  //funckja pomocnicza do znalezienia procesu po PID
	std::shared_ptr<PCB> get_kid(const std::string& nazwa); //funckja pomocnicza do znalezienia procesu po nazwie
	bool find_kid(const unsigned int& PID) const;

	void change_state(const Process_state& x); //zmiana stanu procesu (korzysta z planisty)

	//Zabija proces wraz z wszystkimi dziećmi
	void kill();

	//Zmienia rozmiar pamięci zaalokowanej dla procesu
	void resize(const unsigned int& size);

	void display_kid_all(); //funkcja która pokazuje całe potomstwo procesu
	void display_kid(int a); //funkcja pomocnicza do tej wyżej
	void display();
};

//klasa pomocniczna inicjujemy ją na początku konstruktorem i zawiera system_dummy czyli proces bezczynności
class ProcTree {
private:
	unsigned int freePID = 2;
	std::shared_ptr<PCB> dummyProc;

public:
	//Konstruktory
	ProcTree() = default;

	//fork tworzący proces z podaniem rozmiaru 
	void fork(const std::string& procName, const unsigned int& parentPID, const unsigned int& size);

	//fork tworzący proces z programem z pliku
	void fork(const std::string& procName, const unsigned int& parentPID, const std::string& fileName);

	//usuwanie procesów 
	void kill(const std::string& procName) const;

	//Inicjalizacja pamięci i dodanie procesu dummy
	void init();

	//wyswietla cale drzewa
	void display() const;

	//znajduje proces po nazwie
	std::shared_ptr<PCB> find(const std::string& nazwa) const;

private:
	void add_kid(const std::shared_ptr<PCB>& parent, const std::shared_ptr<PCB>& kid, const unsigned int& size);
	void add_kid(const std::shared_ptr<PCB>& parent, const std::shared_ptr<PCB>& kid, const std::string& fileName);
};

extern ProcTree tree;

#endif  //SEXYOS_PROCESY_H