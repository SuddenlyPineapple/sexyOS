#pragma once

#include "Procesy.h"
#include <list>
#include <vector>

//			Zakladam, ze proces bedzie mial priorytet z zakresu 1 - 10, przy czym poczatkowy wynosi 5

class Planista {
private:
	std::list<PCB> WaitingPCB;
	std::list<PCB>::iterator Rpcb, Wpcb;
	unsigned char trial;
	int CounterMax=1;
public:
	std::list<PCB> ReadyPCB;
	Planista() {}
	~Planista() {}
	void Check(){}
	void AddProces(PCB &Proces){}
	void RemoveProces(PCB &Proces){}
	void SortReadyPCB(){}
	void SetPriority(PCB &Proces){}
};
