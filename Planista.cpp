#include "Procesy.h"
#include "Planista.h"
#include <list>
#include <cmath>
#include <sstream>

bool WywlaszczeniePCB = false;

void Planista::Check() {
	if (trial == ReadyPCB.size() / 2) {
		trial = 0;
	}
	trial++;
	for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end();) {
		if ((*Rpcb)->state == TERMINATED) {
			Rpcb = ReadyPCB.erase(Rpcb);
		}
		if ((*Rpcb)->state != READY) {
			WaitingPCB.push_back(*Rpcb);
			Rpcb = WaitingPCB.erase(Rpcb);
		}
		else {
			++Rpcb;
			SetPriority(**Rpcb);
		}
	}
	for (Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end();) {
		if ((*Wpcb)->state == READY) {
			AddProces(**Wpcb);
			Wpcb = WaitingPCB.erase(Wpcb);
		}
		else {
			++Wpcb;
		}
	}
	SortReadyPCB();
}
void Planista::AddProces(PCB& Proces) {
	bool x = false;
	if (Proces.state == READY) {
		if (ReadyPCB.size() == 0) {
			ReadyPCB.push_back(&Proces);
		}
		else {
			for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); ++Rpcb) {
				if (Proces.priority > (*Rpcb)->priority) {
					ReadyPCB.insert(Rpcb, &Proces);
					if (Rpcb == ReadyPCB.begin()) {				// jesli proces będzie na 1 miejscu
						WywlaszczeniePCB = true;					// flaga i przeładowanie kontekstu
					}
					x = true;
					break;
				}
			}
			if (x == 0) {
				ReadyPCB.push_back(&Proces);
			}
		}
	}
	else {
		WaitingPCB.push_back(&Proces);
	}
}
void Planista::RemoveProces(PCB &Proces) {
	for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end();) {
		if ((*Rpcb)->PID == Proces.PID) {
			Rpcb = ReadyPCB.erase(Rpcb);
		}
		else {
			++Rpcb;
		}
	}
	for (Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end();) {
		if ((*Wpcb)->PID == Proces.PID) {
			Wpcb = WaitingPCB.erase(Wpcb);
		}
		else {
			++Wpcb;
		}
	}
}
void Planista::SortReadyPCB() {
	bool x;
	for (size_t i = 0; i < ReadyPCB.size(); i++) {
		x = false;
		Wpcb = ReadyPCB.begin();
		++Wpcb;
		for (Rpcb = ReadyPCB.begin(); Wpcb != ReadyPCB.end(); ++Rpcb, ++Wpcb) {
			if ((*Rpcb)->priority > (*Wpcb)->priority) {
				std::swap(*Rpcb, *Wpcb);
				x = true;
			}
		}
		if (x == 0) {
			break;
		}
	}
}

void Planista::SetPriority(PCB &Proces) {
	double x = 0;
	Proces.last_counter = Proces.comand_counter - Proces.last_counter;
	//			USTALENIE MNOZNIKA od najwiekszego skoku
	if (Proces.last_counter > CounterMax) {
		CounterMax = Proces.last_counter;
	}
	if (Proces.last_counter > 0) {
		x = (Proces.priority + Proces.last_counter * 30.0 / CounterMax) / 4.0;
		Proces.priority = static_cast<int>(x);
	}
	if (Proces.priority >= 10) {
		Proces.priority = 9;
	}
	if (Proces.priority < 0) {
		Proces.priority = 0;
	}
	//			POSTARZANIE, jesli proces nie otrzymał przydzialu do procesora w kilku ostatnich sesjach
	if (Proces.priority > 1 && trial == ReadyPCB.size() / 2 && Proces.last_counter == 0) {
		Proces.priority--;
	}
	Proces.last_counter = Proces.comand_counter;
}

std::list<PCB*>& Planista::getWaitingPCB() {
	return WaitingPCB;
}
