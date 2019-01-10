#include "Planista.h"
#include <list>
#include <iterator>
#include <cmath>

extern class PCB;
bool WywlaszczeniePCB = 0;

class Planista {
private:
	std::list<PCB> WaitingPCB;
	std::list<PCB>::iterator Rpcb, Wpcb;
	unsigned char trial = 0;
public:
	std::list<PCB> ReadyPCB;
	Planista() {}
	~Planista() {}

	void Check() {
		trial++;
		if (trial == 5) {
			trial = 0;
		}
		for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); Rpcb++) {
			if (Rpcb->state == TERMINATED) {
				Rpcb = ReadyPCB.erase(Rpcb);
			}
			if (Rpcb->state != READY) {
				WaitingPCB.push_back(*Rpcb);
				Rpcb = WaitingPCB.erase(Rpcb);
			}
			else {
				SetPriority(*Rpcb);
			}
		}
		for (Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end(); Wpcb++) {
			if (Wpcb->state == READY) {
				AddProces(*Wpcb);
				Wpcb = WaitingPCB.erase(Wpcb);

			}
		}
		SortReadyPCB();
	}
	void AddProces(PCB Proces) {
		bool x = 0;
		if (Proces.state == READY) {
			if (ReadyPCB.size() == 0) {
				ReadyPCB.push_back(Proces);
			}
			else {
				for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); Rpcb++) {
					if (Proces.priority > Rpcb->priority) {
						ReadyPCB.insert(Rpcb, Proces);
						if (Rpcb == ReadyPCB.begin()) {								// jesli proces będzie na 1 miejscu
							WywlaszczeniePCB = 1;									// flaga i przeładowanie kontekstu
						}
						x = 1;
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
	}
	void RemoveProces(PCB &Proces) {
		for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); Rpcb++) {
			if (Rpcb->PID == Proces.PID) {
				ReadyPCB.erase(Rpcb);
			}
		}
		for (Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end(); Wpcb++) {
			if (Wpcb->PID == Proces.PID) {
				WaitingPCB.erase(Wpcb);
			}
		}
	}
	void SortReadyPCB() {
		bool x;
		for (int i = 0; i < ReadyPCB.size(); i++) {
			x = 0;
			Wpcb = ReadyPCB.begin();
			Wpcb++;
			for (Rpcb = ReadyPCB.begin(); Wpcb != ReadyPCB.end(); Rpcb++, Wpcb++) {
				if (Rpcb->priority > Wpcb->priority) {
					std::swap(*Rpcb, *Wpcb);
					x = 1;
				}
			}
			if (x == 0) {
				break;
			}
		}
	}

	void SetPriority(PCB &Proces) {   // TO jeszcze do poprawy
		float x = 0;
		if (Proces.comand_counter != 0) {
			Proces.last_counter = Proces.comand_counter - Proces.last_counter;
		}
		if (Proces.last_counter > 0) {
			x = (4 * Proces.priority + log(Proces.last_counter)) / 5;
		}
		Proces.priority = (int)x;
		if (Proces.priority >= 10) {
			Proces.priority = 9;
		}
		Proces.last_counter = Proces.comand_counter;
	}
};