#include "Procesy.h"
#include "Planista.h"
#include <list>
#include <sstream>
#include <iostream>


void Planista::Check() {
	if (trial >= ReadyPCB.size() / 2) {
		trial = 0;
	}
	trial++;
	for (auto Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end();) {
		if ((*Rpcb)->state == TERMINATED) {
			Rpcb = ReadyPCB.erase(Rpcb);
		}
		else if ((*Rpcb)->state != READY && (*Rpcb)->state != RUNNING) {
			WaitingPCB.push_back(*Rpcb);
			Rpcb = ReadyPCB.erase(Rpcb);
		}
		else {
			++Rpcb;
			if (Rpcb == ReadyPCB.end()) { break; }
			SetPriority(**Rpcb);
		}
	}
	SortReadyPCB();
	for (auto Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end();) {
		if ((*Wpcb)->state == TERMINATED ) {
			Wpcb = WaitingPCB.erase(Wpcb);
		}
		else if ((*Wpcb)->state == READY || (*Wpcb)->state == RUNNING) {
			AddProces(*Wpcb);
			Wpcb = WaitingPCB.erase(Wpcb);
		}
		else {
			++Wpcb;
		}
	}
}
void Planista::AddProces(PCB* Proces) {
	bool x = false;
	if (Proces->state == READY || Proces->state == RUNNING) {
		if (ReadyPCB.empty()) {
			ReadyPCB.push_back(Proces);
		}
		else {
			for (auto Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); ++Rpcb) {
				if (Proces->priority > (*Rpcb)->priority) {
					ReadyPCB.insert(Rpcb, Proces);
					if (Rpcb == ReadyPCB.begin()) {			// jesli proces będzie na 1 miejscu
						WywlaszczeniePCB = true;			// flaga i przeładowanie kontekstu
					}
					x = true;
					break;
				}
			}
			if (x == 0) {
				ReadyPCB.push_back(Proces);
			}
		}
	}
	else {
		WaitingPCB.push_back(Proces);
	}
	SortReadyPCB();
}
void Planista::RemoveProces(PCB* Proces) {
	for (auto Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end();) {
		if ((*Rpcb)->PID == Proces->PID) {
			Rpcb = ReadyPCB.erase(Rpcb);
		}
		else {
			++Rpcb;
		}
	}
	for (auto Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end();) {
		if ((*Wpcb)->PID == Proces->PID) {
			Wpcb = WaitingPCB.erase(Wpcb);
		}
		else {
			++Wpcb;
		}
	}
}
void Planista::SortReadyPCB() {
	for (size_t i = 0; i < ReadyPCB.size(); i++) {
		bool x = false;
		auto Wpcb = ReadyPCB.begin();
		++Wpcb;
		for (auto Rpcb = ReadyPCB.begin(); Wpcb != ReadyPCB.end(); ++Rpcb, ++Wpcb) {
			if ((*Rpcb)->priority > (*Wpcb)->priority) {
				std::swap(*Rpcb, *Wpcb);
				x = true;
			}
		}
		if (x == 0) {
			break;
		}
	}

	//Ustawienie pierwszego elementu na RUNNING a reszty na READY
	for (auto it = ReadyPCB.begin(); it != ReadyPCB.end(); ++it) {
		if (it == ReadyPCB.begin()) { (*it)->state = RUNNING; }
		else if ((*it)->state != READY) { (*it)->state = READY; }
	}
}

void Planista::SetPriority(PCB& Proces) {
	if (Proces.PID == 1 || Proces.PID == 2) { return; }

	Proces.last_counter = Proces.instruction_counter - Proces.last_counter;
	//	USTALENIE MNOZNIKA od najwiekszego skoku
	if (Proces.last_counter > CounterMax) {
		CounterMax = Proces.last_counter;
	}
	if (Proces.last_counter > 0) {
		Proces.priority = (Proces.priority + Proces.last_counter * 30.0 / CounterMax) / 4.0;
	}
	if (Proces.priority >= 10) {
		Proces.priority = 9;
	}
	else if (Proces.priority < 0) {
		Proces.priority = 0;
	}
	//	POSTARZANIE, jesli proces nie otrzymał przydzialu do procesora w kilku ostatnich sesjach
	if (Proces.priority > 1 && trial == ReadyPCB.size() / 2 && Proces.last_counter <= 0) {
		Proces.priority--;
	}
	Proces.last_counter = Proces.instruction_counter;
}

void Planista::displayPCBLists() {
	std::cout << "\nProcesy gotowe: " << (ReadyPCB.empty()? "EMPTY" : "") << "\n";
	for(const auto& elem : ReadyPCB) {
		std::cout << " - " << elem->name << ", PID: " << elem->PID;
		std::cout << "	(prio: " << elem->priority;
		if (elem->state == RUNNING) { std::cout << ", RUNNING"; }
		else if (elem->state == READY) { std::cout << ", READY"; }
		else if (elem->state == WAITING) { std::cout << ", WAITING"; }
		std::cout << ")\n";
	}
	std::cout << "\nProcesy czekajace: " << (WaitingPCB.empty() ? "EMPTY" : "") << "\n";
	for(const auto& elem : WaitingPCB) {
		std::cout << " - " << elem->name << ", PID: " << elem->PID;
		std::cout << "	(prio: " << elem->priority;
		if (elem->state == RUNNING) { std::cout << " (RUNNING)"; }
		else if (elem->state == READY) { std::cout << " (READY)"; }
		else if (elem->state == WAITING) { std::cout << " (WAITING)"; }
		std::cout << '\n';
	}
	std::cout << '\n';
}
