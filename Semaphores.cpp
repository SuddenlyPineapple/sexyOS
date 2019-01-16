#include "Procesy.h"
#include "Semaphores.hpp"
#include "Planista.h"

Semaphore::Semaphore(Planista* plan) : p(plan) {
	this->value = 999;
}

Semaphore::Semaphore(Planista* plan, const int& n) : p(plan) {
	this->value = n;
}
void Semaphore::Wait(PCB* pcb) {
	this->value--;
	if (this->value <= 0 && !this->blocked) {
		this->blocked = true;
		block(pcb);
	}
}
void Semaphore::Signal(PCB* pcb) {
	this->value++;

	if (this->value > 0 && this->blocked) {
		this->blocked = false;
		wakeup(pcb);
	}
}
void Semaphore::block(PCB* pcb) const {
	if (pcb != nullptr) {
		pcb->change_state(WAITING);
		p->Check();
	}
}
void Semaphore::wakeup(PCB* pcb) const {
	if (pcb != nullptr) {
		pcb->change_state(RUNNING);
		p->Check();
	}
}

const bool& Semaphore::is_blocked() const {
	return blocked;
}

const int& Semaphore::get_value() const {
	return this->value;
}
void Semaphore::show_value() const{
	std::cout<<"Aktualna wartosc zmiennej semaforowej: << get_value() << std::endl;
}
