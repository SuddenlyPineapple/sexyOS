#include "Procesy.h"
#include "MemoryManager.h"

 unsigned int free_PID=2;

void PCB::change_state(Process_state x)
{
	this->state = x;
}

void PCB::add_file_to_proc(std::string open_file)
{
	this->open_files.push_back(open_file);
}

void PCB::kill_all_childrens(MemoryManager & mm)
{
	
	for (int i=0;i< this->child_vector.size(); ++i)
	{
		mm.kill(this->child_vector.at(i)->PID);
		delete this->child_vector.at(i);
		
	}
	this->child_vector.clear();
	
}



void PCB::Set_PID(int i)
{
	this->PID = i;
}

PCB* PCB::GET_kid(unsigned int PID)
{
	for (PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return kid; }
		for (PCB* grandkid : kid->child_vector) {
			if (grandkid->PID == PID) { return grandkid; }
			for (PCB* ggrandkid : grandkid->child_vector) {
				if (ggrandkid->PID == PID) { return ggrandkid; }


			}

		}
	}


		for (PCB* kid : this->child_vector) {
			if (kid->PID == PID) { return kid; }
			else if (!kid->child_vector.empty()) {
				return kid->GET_kid(PID);
			}
		}
		return NULL;

	}
PCB* PCB::GET_kid(std::string nazwa)
{
	for (PCB* kid : this->child_vector) {
		if (kid->process_name == nazwa) { return kid; }
		for (PCB* grandkid : kid->child_vector) {
			if (grandkid->process_name == nazwa) { return grandkid; }
			for (PCB* ggrandkid : grandkid->child_vector) {
				if (ggrandkid->process_name == nazwa) { return ggrandkid; }


			}

		}
	}


	for (PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return kid; }
		else if (!kid->child_vector.empty()) {
			return kid->GET_kid(PID);
		}
	}
	return NULL;

}

bool PCB::find_kid(unsigned int PID)
{
	for (PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return true; }
		else if (!kid->child_vector.empty()) {
			if( kid->find_kid(PID))
				return kid->find_kid(PID);
		}
	}
	return false;
}
void PCB::display_allkids()
{
	int i = 0;
	for (PCB* kid : this->child_vector) {
		if(kid->parent_proc->PID==1)std::cout << "\t nazwa procesu " << kid->process_name << " PID procesu "<< kid->PID << std::endl;
		else
		std::cout <<"\t\t nazwa procesu " << kid->process_name << " PID procesu " << kid->PID << std::endl;

		if (!kid->child_vector.empty()) {
			std::cout << "\t"; kid->display_allkids(i);

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
			std::cout << "\t\t nazwa procesu " << kid->process_name << " PID procesu " << kid->PID << std::endl;

		if (!kid->child_vector.empty()) {

			std::cout << "\t"; kid->display_allkids(a);

		}



	}

	std::cout << std::endl;
}
void proc_tree::fork(PCB *proc,const std::string name, MemoryManager &mm, int rozmiar) {
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
	
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;
		double pages = ceil((double)rozmiar / 16);
		proc->pageList = mm.createPageList(rozmiar, proc->PID);
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
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			double pages = ceil((double)rozmiar / 16);
			proc->pageList = mm.createPageList(rozmiar, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;

			
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}

	}

}
void proc_tree::fork(PCB *proc, const std::string name, std::string file_name, MemoryManager &mm, int rozmiar) {
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;

		double pages = ceil((double)rozmiar / 16);
		if ( mm.loadProgram( file_name,rozmiar,proc->PID ) == -1 ) {
			//exit()
			throw 1;// rzucam cos rzeby funckeje przerwac

		}
		proc->pageList = mm.createPageList(rozmiar, proc->PID);
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
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			double pages = ceil((double)rozmiar / 16);
			if (mm.loadProgram(file_name, rozmiar, proc->PID) != 1) {
				//exit()
				throw 1;// rzucam cos rzeby funckeje przerwac

			}
			proc->pageList = mm.createPageList(rozmiar, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;

			
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}

	}

}

void proc_tree::fork(PCB * proc, const std::string name)
{
	if(proc->PID==this->proc.PID){//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc =&this->proc;
		this->proc.child_vector.push_back(proc);
	}
	else {
		if (this->proc.GET_kid(proc->PID)->PID== proc->PID) {
			int temp = proc->PID;


			proc->parent_proc = this->proc.GET_kid(temp);
			proc->PID = free_PID;
			this->proc.GET_kid(temp)->child_vector.push_back(proc);
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;

			
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}
	}

}
/*void proc_tree::fork(PCB * proc, const std::string name, MemoryManager mm, int rozmiar, std::string open_file)
{
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
		
		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;
		proc->add_file_to_proc(open_file);
		double pages = ceil((double)rozmiar / 16);
		proc->pageList = mm.createPageList(rozmiar, proc->PID);
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
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			double pages = ceil((double)rozmiar / 16);
			proc->pageList = mm.createPageList(rozmiar, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;
			proc->add_file_to_proc(open_file);
			
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}

	}
}*/
/*void proc_tree::fork(PCB *proc, const std::string name, std::string file_name, MemoryManager mm, int rozmiar) {
	if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.

		proc->PID = free_PID;
		free_PID++;
		proc->parent_proc = &this->proc;

		double pages = ceil((double)rozmiar / 16);// tu zaczyna sie ustawianie pamiêci
		proc->pageList = mm.createPageList(rozmiar, proc->PID);
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
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			double pages = ceil((double)rozmiar / 16);
			proc->pageList = mm.createPageList(rozmiar, proc->PID);
			proc->change_state(READY);
			proc->proces_size = pages * 16;

			
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		}
	}

}

/*void proc_tree::fork(PCB * proc, const std::string name, std::string file_name) tu jest bez mm
{
	{
		if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
			proc->open_files.push_back(file_name);
			proc->PID = free_PID;
			free_PID++;
			proc->parent_proc = &this->proc;
			this->proc.child_vector.push_back(proc);

		}
		else {
			if (this->proc.GET_kid(proc->PID)->PID == proc->PID) {
				int temp = proc->PID;
				proc->open_files.push_back(file_name);
				proc->parent_proc = this->proc.GET_kid(temp);
				proc->PID = free_PID;
				this->proc.GET_kid(temp)->child_vector.push_back(proc);
				std::cout << " znaleziono ojca" << std::endl;
				free_PID++;

				;
			}
			else {
				std::cout << "nie znaleziono ojca" << std::endl;
			}
		}
	}
}*/
void proc_tree::fork(PCB * proc, const std::string name, std::vector<std::string> file_names)
{
	{
		if (proc->PID == this->proc.PID) {//sprawdza czy id ojca siê zgadza i jestli tak przypisuje go do niego.
			for (std::string file_name : file_names) {
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
				for (std::string file_name : file_names) {
					proc->open_files.push_back(file_name);
				}
				proc->parent_proc = this->proc.GET_kid(temp);
				proc->PID = free_PID;
				this->proc.GET_kid(temp)->child_vector.push_back(proc);
				std::cout << " znaleziono ojca" << std::endl;
				free_PID++;

				
			}
			else {
				std::cout << "nie znaleziono ojca" << std::endl;
			}
		}
	}
}
void proc_tree::exit(MemoryManager & mm, int pid)
{
	if (pid == this->proc.PID) {// kiedy damy id=1 
		std::cout << "nie mo¿na usun¹æ inita/systemd" << std::endl;
		}
	else {	
		if (this->proc.GET_kid(pid) == NULL) {//jak nie znajdzie dziecka
			std::cout << "nie ma takiego procesu" << std::endl;
		}
		else {
			PCB* temp = this->proc.GET_kid(pid);
			if (temp->child_vector.size() == 0) {//proces nie ma dzieci
				for(int i=0;i<temp->parent_proc->child_vector.size();i++){
					if (temp->parent_proc->child_vector.at(i)->PID == pid) {
						delete temp->parent_proc->child_vector.at(i);
						temp->parent_proc->child_vector.erase(temp->parent_proc->child_vector.begin() + i);
						mm.kill(pid);
					}
					}
				}
			else { //kiedy ma dzieci
				//kiedy dziecko ma dzieci
					for (int i = 0; i < temp->child_vector.size(); i++) {
						if (temp->child_vector.at(i)->child_vector.size() != 0)//kiedy jest patologia
						{
							for (int j = 0; j < temp->child_vector.at(i)->child_vector.size(); j++)
								temp->child_vector.at(i)->child_vector.at(j)->kill_all_childrens(mm);
						}
						temp->child_vector.at(i)->kill_all_childrens(mm);

					
					}
				

			}
			
			}
		
		
		
		}
		



	}





void proc_tree::display_tree()
{
	std::cout <<"nazwa procesu "<<proc.process_name<<" PID procesu "<< proc.PID << std::endl;
	proc.display_allkids();






}

PCB *proc_tree::find_proc(int PID)
{
	if (PID == this->proc.PID){
		return &this->proc;// do sprawdzenia czy tak zadziaa

	}
	else return this->proc.GET_kid(PID);

}

PCB * proc_tree::find_proc(std::string nazwa)
{
	{
		if (nazwa == this->proc.process_name) {
			return &this->proc;// do sprawdzenia czy tak zadziaa

		}
		else return this->proc.GET_kid(nazwa);

	}
}
