#ifndef SEXYOS_PROCESY_H
#define SEXYOS_PROCESY_H
#include <string>
#include <vector>
#include <iostream>

struct PageTableData;

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
    unsigned int proces_size;//rozmiar procesu w stronicach(chyba).
    int A, B, C, D;//Rejestry dla interpretera
    std::vector<std::string> open_files;//otwarte plik
    int comand_counter;//licznik rozkazów
    int priority;//priorytet tu mo¿e byæ jeszcze wirtualny ale nwm czy nasza grupa musi go mieæ(wirtualny priorytet)

    PCB() {//kontruktor dla systemd
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
        this->priority = 15;//nwm ile ma byc
    };

    void Set_PID(int i);//funcja pomocnicza
    PCB *GET_kid(unsigned int PID);// funckja pomocnicza do znalezienia procesu po PID
    bool find_kid(unsigned int PID);

    void display_allkids();//funkcja która pokazuje dzieci procesu i dzieci dzieci
    void display_allkids(int a);//funkcja pomocnicza do tej wy¿ej
    void change_state(Process_state x);//zmiana stanu procesu
    void update();//fukncja aktualizujaca dane w procesie (do zrobienia)



};

class proc_tree {//klasa pomocniczna inicjujemy j¹ na poczatku konstruktorem domyslny i zawiera systemd czyli g³owny proces (proc_tree Drzewo = proc_tree();)
public:
    PCB proc;

    void fork(PCB *proc,
              const std::string name/*,tutaj memory mangment z przydzielaniem pamieci */ );//dodaje kopieprocesu(dzieciaka) procesu do drzewa
    void fork(PCB *proc, const std::string name/*,tutaj memory mangment z przydzielaniem pamieci */,
              std::string file_name);//to samo co wyzej tylko z nazwa pliku
    void fork(PCB *proc, const std::string name/*,tutaj memory mangment z przydzielaniem pamieci */,
              std::vector<std::string> file_names);//to samo co wyzej tylko z vectorem plików
    /*void exit() usuwanie procesów  do zrobieina*/
    void display_tree();//wyswietla cale drzewa
    PCB find_proc(int PID);//znajduje proces
    proc_tree() { this->proc = PCB(); }

    proc_tree(PCB *proc) { this->proc = *proc; };
};

#endif  //SEXYOS_PROCESY_H