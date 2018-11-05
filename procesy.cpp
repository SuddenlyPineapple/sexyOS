#include "pch.h"
#include "procesy.h"
 unsigned int free_PID=2;
 
void PCB::change_state(Process_state x)
{
	this->state = x;
}

void PCB::update()
{
}

void PCB::Set_PID(int i)
{
	this->PID = i;
}

PCB* PCB::find_kid(unsigned int PID)
{
	for (PCB* kid : this->child_vector) {
		if (kid->PID == PID) { return kid; }
		else if (!kid->child_vector.empty()) {
		 	 return kid->find_kid(PID);
		};
		


	
	}
	std::cout << "nie znaleziono procesu o takim PID" << std::endl;
	return NULL;
}
void PCB::display_allkids()
{	
	for (PCB* kid : this->child_vector) {
		if(kid->parent_proc->PID==1)std::cout << "\t" << kid->PID << std::endl;
		else
		std::cout <<"\t\t"<< kid->PID << std::endl;
		
		if (!kid->child_vector.empty()) {
			std::cout << "\t"; kid->display_allkids(); 
			
		}
		


	}
	
	std::cout << std::endl;
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
		if (this->proc.find_kid(proc->PID)->PID== proc->PID) {
			int temp = proc->PID;
			
			
			proc->parent_proc = this->proc.find_kid(temp);
			proc->PID = free_PID;
			this->proc.find_kid(temp)->child_vector.push_back(proc);
			std::cout << " znaleziono ojca" << std::endl;
			free_PID++;
			
			;
		}
		else {
			std::cout << "nie znaleziono ojca" << std::endl;
		} 

		





	}
	
}

void proc_tree::fork(PCB * proc, const std::string name, std::string file_name)
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
			if (this->proc.find_kid(proc->PID)->PID == proc->PID) {
				int temp = proc->PID;
				proc->open_files.push_back(file_name);
				proc->parent_proc = this->proc.find_kid(temp);
				proc->PID = free_PID;
				this->proc.find_kid(temp)->child_vector.push_back(proc);
				std::cout << " znaleziono ojca" << std::endl;
				free_PID++;

				;
			}
			else {
				std::cout << "nie znaleziono ojca" << std::endl;
			}
		}
	}
}
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
			if (this->proc.find_kid(proc->PID)->PID == proc->PID) {
				int temp = proc->PID;
				for (std::string file_name : file_names) {
					proc->open_files.push_back(file_name);
				}
				proc->parent_proc = this->proc.find_kid(temp);
				proc->PID = free_PID;
				this->proc.find_kid(temp)->child_vector.push_back(proc);
				std::cout << " znaleziono ojca" << std::endl;
				free_PID++;

				;
			}
			else {
				std::cout << "nie znaleziono ojca" << std::endl;
			}
		}
	}
}

void proc_tree::display_tree()
{
	std::cout << proc.PID << std::endl;
	proc.display_allkids();






}

PCB proc_tree::find_proc(int PID)
{
	if (PID == this->proc.PID){
		return this->proc;
		
	}
	
	
}
