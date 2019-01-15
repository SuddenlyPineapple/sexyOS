#include "Procesy.h"
#include "Planista.h"
#include "MemoryManager.h"


void PCB::change_state(Process_state x)
{
	this->state = x;
}

void PCB::add_file_to_proc(const std::string& open_file)
{
	this->open_files.push_back(open_file);
}

void PCB::kill_all_childrens(MemoryManager & mm)
{
	for (auto& i : this->child_vector)
	{
		i->change_state(TERMINATED);
		mm.kill(i->PID);
	}
	this->child_vector.clear(); //Tu nastêpuje usuniêcie wszystkich dzieci

}



void PCB::Set_PID(int i)
{
	this->PID = i;
}

PCB* PCB::GET_kid(const unsigned int& PID)
{
	for (PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return kid; }
	}
	for (PCB* kid : this->child_vector) {
		PCB* foundKid = kid->GET_kid(PID); //U¿y³em rekurencji
		if (foundKid != nullptr) { return foundKid; }
	}
	return nullptr;

}
PCB* PCB::GET_kid(const std::string& nazwa)
{
	for (PCB* kid : this->child_vector) {
		if (kid->process_name == nazwa) { return kid; }
	}
	for (PCB* kid : this->child_vector) {
		PCB* foundKid = kid->GET_kid(nazwa); //U¿y³em rekurencji
		if (foundKid != nullptr) { return foundKid; }
	}

	return nullptr;
}

bool PCB::find_kid(const unsigned int& PID) const
{
	for (const PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return true; }
		else if (!kid->child_vector.empty()) {
			const bool result = kid->find_kid(PID);
			if (result) { return kid->find_kid(PID); }
		}
	}
	return false;
}
void PCB::display_allkids() {
	for (PCB* kid : this->child_vector) {
		std::cout << '\t';
		if (kid->parent_proc->PID == 1) { std::cout << " Nazwa: " << kid->process_name << ", PID: " << kid->PID << std::endl; }
		else { std::cout << " Nazwa: " << kid->process_name << ", PID: " << kid->PID << std::endl; }

		if (!kid->child_vector.empty()) {
			kid->display_allkids(1);
		}
	}

	std::cout << std::endl;
}
void PCB::display_allkids(int a)
{
	a++;

	for (PCB* kid : this->child_vector) {
		for (int i = 1; i < a; i++) {
			std::cout << "\t";
		}
		std::cout << " - Nazwa: " << kid->process_name << ", PID: " << kid->PID << std::endl;

		if (!kid->child_vector.empty()) {
			kid->display_allkids(a);
		}
	}

	std::cout << std::endl;
}
void proc_tree::fork(PCB* proc, int size) {
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.

		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;
		const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
		proc->pageList = mm->createPageList(size, proc->PID);
		proc->change_state(READY);
		proc->proces_size = pages * 16;


		this->proc.child_vector.push_back(proc);


	}
	else {
		if (this->proc.GET_kid(proc->PID)->PID == proc->PID) {
			int temp = proc->PID;

			proc->parent_proc = this->proc.GET_kid(temp);
			proc->PID = free_PID;
			this->proc.GET_kid(temp)->child_vector.push_back(proc);
			//std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
			proc->pageList = mm->createPageList(size, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;


		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}

	}

}
void proc_tree::fork(PCB* proc, const std::string& file_name, int size) {
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;

		const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
		if (mm->loadProgram(file_name, size, proc->PID) == -1) {
			//exit()
			throw 1;// rzucam cos rzeby funckeje przerwac

		}
		proc->pageList = mm->createPageList(size, proc->PID);
		proc->change_state(READY);
		proc->proces_size = pages * 16;
		this->proc.child_vector.push_back(proc);
	}
	else {
		if (this->proc.GET_kid(proc->PID)->PID == proc->PID) {
			int temp = proc->PID;

			proc->parent_proc = this->proc.GET_kid(temp);
			proc->PID = free_PID;
			this->proc.GET_kid(temp)->child_vector.push_back(proc);
			//std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			const auto pages = static_cast<unsigned int>(ceil(size / 16.0));
			if (mm->loadProgram(file_name, size, proc->PID) != 1) {
				//exit()
				throw 1;// rzucam cos rzeby funckeje przerwac

			}
			proc->pageList = mm->createPageList(size, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;


		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}

	}

}

void proc_tree::fork(PCB* proc)
{
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;
		this->proc.child_vector.push_back(proc);
	}
	else {
		if (this->find_proc(proc->PID) != nullptr) {
			auto parent = this->find_proc(proc->PID);
			proc->parent_proc = parent;
			proc->PID = free_PID;
			parent->child_vector.push_back(proc);
			//std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}
	}

}

void proc_tree::fork(PCB* proc, std::vector<std::string> file_names)
{
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		for (const std::string& file_name : file_names) {
			proc->open_files.push_back(file_name);
		}
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;
		this->proc.child_vector.push_back(proc);
	}
	else {
		if (this->proc.GET_kid(proc->PID)->PID == proc->PID) {
			int temp = proc->PID;
			for (const std::string& file_name : file_names) {
				proc->open_files.push_back(file_name);
			}
			proc->parent_proc = this->proc.GET_kid(temp);
			proc->PID = free_PID;
			this->proc.GET_kid(temp)->child_vector.push_back(proc);
			//std::cout << " znaleziono ojca" << std::endl;
			free_PID++;


		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}
	}
}

void proc_tree::exit(const unsigned& pid)
{
	if (pid == this->proc.PID) { // kiedy damy id=1 
		std::cout << "nie mo¿na usun¹æ inita/systemd" << std::endl;
	}
	else {
		PCB* temp = this->find_proc(pid);
		if (temp == nullptr) {//jak nie znajdzie dziecka
			std::cout << "nie ma takiego procesu" << std::endl;
		}
		else {

			if (!temp->child_vector.empty()) { //kiedy ma dzieci
				//kiedy dziecko ma dzieci
				for (auto& i : temp->child_vector) {
					i->kill_all_childrens(*mm);
					exit(i->PID);
				}
			}
			PCB* parent = temp->parent_proc;
			for (size_t i = 0; i < parent->child_vector.size(); i++) {
				if (parent->child_vector.at(i)->PID == pid) {
					temp->change_state(TERMINATED);
					p->Check();
					mm->kill(pid);
					parent->child_vector.erase(parent->child_vector.begin() + i);
					delete temp;
					break;
				}
			}
		}
	}
}



void proc_tree::display_tree()
{
	std::cout << " Nazwa: " << proc.process_name << ", PID: " << proc.PID << std::endl;
	proc.display_allkids();
}

PCB *proc_tree::find_proc(const unsigned& PID)
{
	if (PID == this->proc.PID) {
		return &this->proc;// do sprawdzenia czy tak zadzia³a (sprawdzone)
	}
	else return this->proc.GET_kid(PID);

}

PCB * proc_tree::find_proc(const std::string& nazwa)
{
	{
		if (nazwa == this->proc.process_name) {
			return &this->proc;// do sprawdzenia czy tak zadzia³a (sprawdzone)
		}
		else return this->proc.GET_kid(nazwa);
	}
}
