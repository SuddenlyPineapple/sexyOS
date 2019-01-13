#include "Procesy.h"
#include "Semaphores.hpp"
#include "Planista.h"
#include <list>

Semaphore::Semaphore(Planista* plan) : p(plan) {
	this->value = 997;
}

Semaphore::Semaphore(Planista* plan, const int& n) : p(plan) {
	if (n <= 0) {
		this->blocked = true;
		block(*(p->ReadyPCB.front()));
	}
	this->value = n;
}
void Semaphore::Wait() {
	this->value--;
	if (this->value <= 0 && !blocked) {
		blocked = true;

		block(*(p->ReadyPCB.front()));
	}
	p->Check();
}
void Semaphore::Signal() {
	this->value++;

	if (this->value > 0 && blocked) {
		blocked = false;
		wakeup(*(p->WaitingPCB.front()));
	}
	p->Check();
}
void Semaphore::block(PCB& a) {
	a.change_state(WAITING);
}
void Semaphore::wakeup(PCB& a) {
	a.change_state(RUNNING);
}

const bool& Semaphore::is_blocked() const {
	return blocked;
}

const int& Semaphore::get_value() const {
	return this->value;
}

