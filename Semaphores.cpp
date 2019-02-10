#include "Semaphores.h"
#include <iostream>

using namespace std;

Semaphore::Semaphore(){
	this->value = 99;
}

Semaphore::Semaphore(const int& n) {
	this->value = n;
}

void Semaphore::wait(const shared_ptr<PCB>& pcb) {
	this->value--;

	if (this->value < 0) {
		this->blocked = true;
		block(pcb);
	}
}

void Semaphore::signal() {
	this->value++;
	if (this->value > 0) { this->blocked = false; }
	wakeup();
}

void Semaphore::signal_all() {
	this->value++;
	if (this->value > 0) { this->blocked = false; }
	while (!waitingPCB.empty()) {
		wakeup();
	}
}

void Semaphore::block(const shared_ptr<PCB>& pcb) {
	pcb->change_state(WAITING);
	cout << "Uspiono proces: " << pcb->name << '\n';
	this->waitingPCB.push(pcb);
}

void Semaphore::wakeup() {
	if (!this->waitingPCB.empty()) {
		if (this->waitingPCB.front() != nullptr) {
			cout << "Obudzono proces: " << waitingPCB.front()->name << '\n';
			this->waitingPCB.front()->change_state(RUNNING);
			this->waitingPCB.pop();
		}
	}
}

const bool& Semaphore::is_blocked() const {
	return blocked;
}

const int& Semaphore::get_value() const {
	return this->value;
}

void Semaphore::set_value(const int& val) {
	this->value = val;
}

void Semaphore::show_value() const {
	cout << "Aktualna wartosc zmiennej semaforowej: " << get_value() << endl;
}
