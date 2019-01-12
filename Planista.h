#pragma once

#include "Procesy.h"
#include <list>

// Zakladam, ze proces bedzie mial priorytet z zakresu 1 - 10, przy czym poczatkowy wynosi 5

class Planista {
private:
	std::list<PCB*>::iterator Rpcb, Wpcb;
	unsigned char trial;
	int CounterMax = 1;
	void SortReadyPCB();
public:
	std::list<PCB*> ReadyPCB, WaitingPCB;
	Planista() {}
	~Planista() {}

	void Check();
	void AddProces(PCB& Proces);
	void RemoveProces(PCB& Proces);
	void SetPriority(PCB& Proces);

	std::list<PCB*>& getWaitingPCB();
};
