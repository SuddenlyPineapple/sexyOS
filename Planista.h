#pragma once

#include <list>

class PCB;

// Zakladam, ze proces bedzie mial priorytet z zakresu 1 - 10, przy czym poczatkowy wynosi 5

class Planista {
private:
	unsigned char trial;
	int CounterMax = 1;
	bool WywlaszczeniePCB = false;

	void SortReadyPCB();
public:
	std::list<PCB*> ReadyPCB, WaitingPCB;
	Planista() = default;
	~Planista() = default;

	void Check();
	void AddProces(PCB* Proces);
	void RemoveProces(PCB* Proces);
	void SetPriority(PCB& Proces);

	void displayPCBLists();
};
