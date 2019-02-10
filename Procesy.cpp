#include "Procesy.h"
#include "Planist.h"
#include "MemoryManager.h"
#include "Pipe.h"
#include <iostream>
#include <memory>

using namespace std;

ProcTree tree;

//PCB ----------------
void PCB::change_state(const Process_state& x) { this->state = x; planist.check(); }

void PCB::kill_all_childrens() {
	for (auto& i : this->childVector) {
		i->change_state(TERMINATED);
		mm.kill(i->PID);
	}
	this->childVector.clear(); //Tu nast�puje usuni�cie wszystkich dzieci

}

void PCB::set_PID(const unsigned int& i) { this->PID = i; }

shared_ptr<PCB> PCB::get_kid(const unsigned int& PID)
{
	for (shared_ptr<PCB> kid : this->childVector) {
		if (kid->PID == PID) { return kid; }
	}
	for (shared_ptr<PCB> kid : this->childVector) {
		shared_ptr<PCB> foundKid = kid->get_kid(PID); //Użyłem rekurencji
		if (foundKid != nullptr) { return foundKid; }
	}
	return nullptr;
}
shared_ptr<PCB> PCB::get_kid(const string& nazwa)
{
	for (shared_ptr<PCB> kid : this->childVector) {
		if (kid->name == nazwa) { return kid; }
	}
	for (shared_ptr<PCB> kid : this->childVector) {
		shared_ptr<PCB> foundKid = kid->get_kid(nazwa); //U�y�em rekurencji
		if (foundKid != nullptr) { return foundKid; }
	}

	return nullptr;
}

bool PCB::find_kid(const unsigned int& PID) const
{
	for (const shared_ptr<PCB> kid : this->childVector) {
		if (kid->PID == PID) { return true; }
		else if (!kid->childVector.empty()) {
			const bool result = kid->find_kid(PID);
			if (result) { return kid->find_kid(PID); }
		}
	}
	return false;
}

void PCB::display_kids() {
	for (shared_ptr<PCB> kid : this->childVector) {
		cout << '\t';
		if (kid->parentProc->PID == 1) { cout << " Nazwa: " << kid->name << ", PID: " << kid->PID << endl; }
		else { cout << " Nazwa: " << kid->name << ", PID: " << kid->PID << endl; }

		if (!kid->childVector.empty()) {
			kid->display_kids(1);
		}
	}
}
void PCB::display_kids(int a)
{
	a++;

	for (shared_ptr<PCB> kid : this->childVector) {
		for (int i = 1; i < a; i++) {
			cout << "\t";
		}
		cout << " - Nazwa: " << kid->name << ", PID: " << kid->PID << endl;

		if (!kid->childVector.empty()) { kid->display_kids(a); }
	}

	if (a == 2) { cout << endl; }
}

void PCB::delete_pipe()
{
	for (shared_ptr<PCB> kid : this->childVector) {
		if (pipeline.exist_pipe(this->name, kid->name)) {}
		pipeline.delete_pipe(this->name, kid->name);
	}

}

void PCB::display() {
	cout << " | PID : " << PID << '\n';
	cout << " | NAZWA : " << name << '\n';
	if (parentProc != nullptr) {
		cout << " | RODZIC : " << parentProc->name << " (PID : " << parentProc->PID << ")\n";
	}
	cout << " | ROZMIAR : " << procesSize << '\n';
	cout << " | LICZBA DZIECI : " << childVector.size() << '\n';
	cout << " | STAN : ";
	switch (state) {
	case RUNNING: cout << "RUNNING"; break;
	case WAITING: cout << "WAITING"; break;
	case READY: cout << "READY"; break;
	case TERMINATED: cout << "TERMINATED"; break;
	default:;
	}
	cout << '\n';
	cout << " | A : " << registers[0] << '\n';
	cout << " | B : " << registers[1] << '\n';
	cout << " | C : " << registers[2] << '\n';
	cout << " | D : " << registers[3] << '\n';
}



//ProcTree ----------------

void ProcTree::fork(const shared_ptr<PCB>& proc, int size) {
	if (proc->PID == this->dummyProc->PID) {//sprawdza czy id ojca się zgadza i jeśli tak przypisuje go do niego.

		proc->PID = free_PID;
		free_PID++;
		proc->parentProc = this->dummyProc;
		proc->procesSize = size;
		proc->pageList = mm.create_page_list(size, proc->PID);
		planist.add_process(proc);


		this->dummyProc->childVector.push_back(proc);
	}
	else {
		if (this->dummyProc->get_kid(proc->PID)->PID == proc->PID) {
			const int temp = proc->PID;

			proc->parentProc = this->dummyProc->get_kid(temp);
			proc->PID = free_PID;
			this->dummyProc->get_kid(temp)->childVector.push_back(proc);
			//cout << " znaleziono ojca" << endl;
			free_PID++;
			const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
			proc->pageList = mm.create_page_list(size, proc->PID);
			planist.add_process(proc);
			proc->procesSize = pages * 16;

		}
		else {
			cout << "nie znaleziono ojca" << endl;
		}

	}

}

void ProcTree::fork(const shared_ptr<PCB>& proc, const string& file_name, const int& size) {
	if (proc->PID == this->dummyProc->PID) {//sprawdza czy id ojca si� zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parentProc = this->dummyProc;

		const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
		if (mm.load_program(file_name, size, proc->PID) == -1) {
			exit(proc->PID);
			return;

		}
		proc->pageList = mm.create_page_list(size, proc->PID);
		planist.add_process(proc);
		proc->procesSize = pages * 16;
		this->dummyProc->childVector.push_back(proc);
	}
	else {
		if (this->dummyProc->get_kid(proc->PID)->PID == proc->PID) {
			const int temp = proc->PID;

			proc->parentProc = this->dummyProc->get_kid(temp);
			proc->PID = free_PID;
			this->dummyProc->get_kid(temp)->childVector.push_back(proc);
			//cout << " znaleziono ojca" << endl;
			free_PID++;
			const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
			if (mm.load_program(file_name, size, proc->PID) != 1) {
				exit(proc->PID);
				throw 1;// rzucam cos rzeby funckeje przerwac

			}
			proc->pageList = mm.create_page_list(size, proc->PID);
			planist.add_process(proc);
			proc->procesSize = pages * 16;

		}
		else {
			cout << "nie znaleziono ojca" << endl;
		}

	}

}

void ProcTree::fork(const shared_ptr<PCB>& proc)
{
	if (proc->PID == this->dummyProc->PID) {//sprawdza czy id ojca si� zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parentProc = this->dummyProc;
		this->dummyProc->childVector.push_back(proc);
		planist.add_process(proc);
	}
	else {
		if (this->find_proc(proc->PID) != nullptr) {
			auto parent = this->find_proc(proc->PID);
			proc->parentProc = parent;
			proc->PID = free_PID;
			parent->childVector.push_back(proc);
			//cout << " znaleziono ojca" << endl;
			free_PID++;
			planist.add_process(proc);
		}
		else {
			cout << "nie znaleziono ojca" << endl;
		}
	}

}

void ProcTree::exit(const unsigned& pid)
{
	if (pid == this->dummyProc->PID) { // kiedy damy id=1 
		cout << "Nie mozna usunac inita/system_dummy" << endl;
	}
	else {
		shared_ptr<PCB> temp = this->find_proc(pid);
		if (temp == nullptr) {//jak nie znajdzie dziecka
			cout << "Nie ma takiego procesu!" << endl;
		}
		else {
			if (!temp->childVector.empty()) { //kiedy ma dzieci
				//kiedy dziecko ma dzieci
				for (auto& i : temp->childVector) {
					i->kill_all_childrens();
					//for (auto& elem : i->FD) { //Część dla pipe'ów
					//
					//}
					exit(i->PID);
				}
			}
			shared_ptr<PCB> parent = temp->parentProc;
			for (size_t i = 0; i < parent->childVector.size(); i++) {
				if (parent->childVector.at(i)->PID == pid) {
					for (auto& elem : temp->FD) {
						if (elem.second[0] != -1) {
							pipeline.delete_pipe(elem.second[0]);
						}
					}
					temp->change_state(TERMINATED);
					mm.kill(pid);

					//for()

					parent->childVector.erase(parent->childVector.begin() + i);
					break;
				}
			}
		}
	}
}

void ProcTree::init() {
	planist.add_process(dummyProc);
	mm.memory_init();
	dummyProc->pageList = mm.create_page_list(16, 1);
}

void ProcTree::display_tree()
{
	cout << " Nazwa: " << dummyProc->name << ", PID: " << dummyProc->PID << endl;
	dummyProc->display_kids();
}

shared_ptr<PCB> ProcTree::find_proc(const unsigned& PID)
{
	if (PID == this->dummyProc->PID) {
		return this->dummyProc;// do sprawdzenia czy tak zadzia�a (sprawdzone)
	}
	else return this->dummyProc->get_kid(PID);

}

shared_ptr<PCB> ProcTree::find_proc(const string& nazwa)
{
	{
		if (nazwa == this->dummyProc->name) {
			return this->dummyProc;// do sprawdzenia czy tak zadzia�a (sprawdzone)
		}
		else return this->dummyProc->get_kid(nazwa);
	}
}
