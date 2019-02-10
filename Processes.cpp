#include "Processes.h"
#include "Planist.h"
#include "MemoryManager.h"
#include "Pipe.h"
#include "Interpreter.h"
#include "FileManager.h"
#include <iostream>
#include <memory>
#include <fstream>

using namespace std;

ProcTree tree;

//PCB ----------------

void PCB::change_state(const Process_state& x) { this->state = x; planist.check(); }

void PCB::kill() {
	if (!this->childVector.empty()) {
		for (auto& i : this->childVector) {
			i->kill();
		}
	}
	fm.file_close_all(this->name);
	mm.kill(this->PID);
	pipeline.remove(this->name);
	this->childVector.clear(); //Tu następuje usunięcie wszystkich dzieci
	this->change_state(TERMINATED);
}

shared_ptr<PCB> PCB::get_kid(const unsigned int& PID)
{
	for (shared_ptr<PCB> kid : this->childVector) {
		if (kid->PID == PID) { return kid; }
	}
	for (const shared_ptr<PCB>& kid : this->childVector) {
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
	for (const shared_ptr<PCB>& kid : this->childVector) {
		shared_ptr<PCB> foundKid = kid->get_kid(nazwa); //U�y�em rekurencji
		if (foundKid != nullptr) { return foundKid; }
	}

	return nullptr;
}

bool PCB::find_kid(const unsigned int& PID) const
{
	for (const shared_ptr<PCB>& kid : this->childVector) {
		if (kid->PID == PID) { return true; }
		else if (!kid->childVector.empty()) {
			const bool result = kid->find_kid(PID);
			if (result) { return kid->find_kid(PID); }
		}
	}
	return false;
}

void PCB::display_kid_all() {
	for (const shared_ptr<PCB>& kid : this->childVector) {
		cout << '\t';
		if (kid->parent->PID == 1) { cout << " Nazwa: " << kid->name << ", PID: " << kid->PID << endl; }
		else { cout << " Nazwa: " << kid->name << ", PID: " << kid->PID << endl; }

		if (!kid->childVector.empty()) {
			kid->display_kid(1);
		}
	}
}

void PCB::display_kid(int a)
{
	a++;

	for (const shared_ptr<PCB>& kid : this->childVector) {
		for (int i = 1; i < a; i++) {
			cout << "\t";
		}
		cout << " - Nazwa: " << kid->name << ", PID: " << kid->PID << endl;

		if (!kid->childVector.empty()) { kid->display_kid(a); }
	}

	if (a == 2) { cout << endl; }
}

void PCB::display() {
	cout << " | PID : " << PID << '\n';
	cout << " | NAZWA : " << name << '\n';
	if (parent != nullptr) {
		cout << " | RODZIC : " << parent->name << " (PID : " << parent->PID << ")\n";
	}
	cout << " | ROZMIAR : " << size << '\n';
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

void PCB::resize(const unsigned& size) { mm.resize_page_list(size, this); }

//ProcTree ----------------

void ProcTree::fork(const string& procName, const unsigned int& parentPID, const unsigned int& size) {
	//sprawdza czy PID rodzica się zgadza z dummy i jeśli tak przypisuje dziecko do niego.
	if (parentPID == this->dummyProc->PID) {
		const shared_ptr<PCB> proc = make_shared<PCB>(procName, parentPID);
		add_kid(this->dummyProc, proc, size);
	}
	else {
		const shared_ptr<PCB> parent = this->dummyProc->get_kid(parentPID);
		if (parent != nullptr) {
			if (parent->PID == parentPID) {
				const shared_ptr<PCB> proc = make_shared<PCB>(procName, parentPID);
				add_kid(this->dummyProc, proc, size);
			}
			else { cout << "Nie znaleziono rodzica!\n"; }
		}
		else { cout << "Nie znaleziono rodzica!\n"; }
	}
}

void ProcTree::fork(const string& procName, const unsigned int& parentPID, const string& fileName) {
	//sprawdza czy PID rodzica się zgadza z dummy i jeśli tak przypisuje dziecko do niego.
	if (parentPID == this->dummyProc->PID) {
		const shared_ptr<PCB> proc = make_shared<PCB>(procName, parentPID);
		add_kid(this->dummyProc, proc, fileName);
	}
	else {
		const shared_ptr<PCB> parent = this->dummyProc->get_kid(parentPID);
		if (parent != nullptr) {
			if (parent->PID == parentPID) {
				const shared_ptr<PCB> proc = make_shared<PCB>(procName, parentPID);
				add_kid(parent, proc, fileName);
			}
			else { cout << "nie znaleziono rodzica\n"; }
		}
		else { cout << "nie znaleziono rodzica\n"; }

	}
}

void ProcTree::kill(const std::string& procName) const {
	if (procName == this->dummyProc->name) { // kiedy damy id=1 
		cout << "Nie mozna usunac system_dummy!\n";
	}
	else {
		shared_ptr<PCB> temp = this->find(procName);
		if (temp == nullptr) {//jak nie znajdzie dziecka
			cout << "Nie ma takiego procesu!\n";
		}
		else {
			shared_ptr<PCB> parent = temp->parent;
			for (size_t i = 0; i < parent->childVector.size(); i++) {
				if (parent->childVector.at(i)->name == procName) {
					parent->childVector.at(i)->kill();
					parent->childVector.erase(parent->childVector.begin() + i);
					break;
				}
			}
		}
	}
}

void ProcTree::init() {
	this->dummyProc = make_shared<PCB>();
	planist.add_process(dummyProc);
	mm.memory_init();
	dummyProc->pageList = mm.create_page_list(16, 1);
}

void ProcTree::display() const
{
	cout << " Nazwa: " << dummyProc->name << ", PID: " << dummyProc->PID << endl;
	dummyProc->display_kid_all();
}

shared_ptr<PCB> ProcTree::find(const string& nazwa) const
{
	{
		if (nazwa == this->dummyProc->name) {
			return this->dummyProc;// do sprawdzenia czy tak zadzia�a (sprawdzone)
		}
		else return this->dummyProc->get_kid(nazwa);
	}
}



//Prywatne
void ProcTree::add_kid(const std::shared_ptr<PCB>& parent, const std::shared_ptr<PCB>& kid, const unsigned& size) {
	kid->parent = parent;
	kid->PID = freePID;
	parent->childVector.push_back(kid);
	freePID++;
	const auto pageNum = static_cast<unsigned int>(ceil(size / 16.0));
	kid->pageList = mm.create_page_list(size, kid->PID);
	kid->size = pageNum * 16;

	kid->executionTimeLeft = 5;

	planist.add_process(kid);
}

void ProcTree::add_kid(const std::shared_ptr<PCB>& parent, const std::shared_ptr<PCB>& kid, const std::string& fileName) {
	kid->parent = parent;
	kid->PID = freePID;
	parent->childVector.push_back(kid);
	freePID++;

	const int size = mm.load_program(fileName, kid->PID);
	const auto pageNum = static_cast<unsigned int>(ceil(size / 16.0));

	if (size == -1) {
		kill(kid->name);
		return;
	}


	kid->pageList = mm.create_page_list(size, kid->PID);
	kid->size = pageNum * 16;

	//Tymczasowe ściągnięcie programu
	string programWhole;
	ifstream programFile(fileName);
	char c;
	while (programFile >> noskipws >> c) {
		if (c == '\n' || c == '\r') { programWhole += ';'; }
		else { programWhole += c; }
	}
	if (*(programWhole.end() - 1) != ';') { programWhole += ';'; }
	programFile.close();

	kid->executionTimeLeft = interpreter.simulate_program(programWhole);

	planist.add_process(kid);
}
