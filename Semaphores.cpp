#include "Procesy.h"
#include "Semaphores.hpp"
#include "Planista.h"
#include <list>

Semaphore::Semaphore(Planista* plan, PCB* pcb_) : p(plan), pcb(pcb_) {
	this->value = 999;
}

Semaphore::Semaphore(Planista* plan, PCB* pcb_, const int& n) : p(plan), pcb(pcb_) {
	this->value = n;
	if (this->value <= 0) {
		this->blocked = true;
		block();
	}
}
void Semaphore::Wait() {
	this->value--;
	if (this->value <= 0 && !this->blocked) {
		this->blocked = true;
		block();
	}
}
void Semaphore::Signal() {
	this->value++;

	if (this->value > 0 && this->blocked) {
		this->blocked = false;
		wakeup();
	}
}
void Semaphore::block() const {
	if (pcb != nullptr) {
		pcb->change_state(WAITING);
		p->Check();
	}
}
void Semaphore::wakeup() const {
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

