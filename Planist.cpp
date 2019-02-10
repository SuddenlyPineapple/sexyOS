#include "Processes.h"
#include "Planist.h"
#include <list>
#include <sstream>
#include <iostream>

using namespace std;

Planist planist;

void Planist::check() {
	//Lista ReadyPCB
	{
		for (auto rPCB = ReadyPCB.begin(); rPCB != ReadyPCB.end(); ++rPCB) {
			if ((*rPCB)->state == TERMINATED) { rPCB = ReadyPCB.erase(rPCB); }
			else if ((*rPCB)->state != READY && (*rPCB)->state != RUNNING) {
				WaitingPCB.push_back(*rPCB);
				rPCB = ReadyPCB.erase(rPCB);
			}
		}
		sort_ready_list();
	}

	//Lista WaitingPCB
	{
		for (auto wPCB = WaitingPCB.begin(); wPCB != WaitingPCB.end(); ++wPCB) {
			if ((*wPCB)->state == TERMINATED) { wPCB = WaitingPCB.erase(wPCB); }
			else if ((*wPCB)->state == READY || (*wPCB)->state == RUNNING) {
				add_process(*wPCB);
				wPCB = WaitingPCB.erase(wPCB);
			}
			if (wPCB == WaitingPCB.end()) { break; }
		}
	}
}

void Planist::add_process(const shared_ptr<PCB>& process) {
	//Procesy w stanie READY i RUNNING dodajemy do kolejki
	if (process->state == READY || process->state == RUNNING) {
		ReadyPCB.push_back(process);
		sort_ready_list();
	}
	else { WaitingPCB.push_back(process); }
}

void Planist::remove_process(const shared_ptr<PCB>& process) {
	for (auto Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end();) {
		if ((*Rpcb)->PID == process->PID) { Rpcb = ReadyPCB.erase(Rpcb); }
		else { ++Rpcb; }
	}
	for (auto Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end();) {
		if ((*Wpcb)->PID == process->PID) { Wpcb = WaitingPCB.erase(Wpcb); }
		else { ++Wpcb; }
	}
}

bool Planist::compare_PCB(const shared_ptr<PCB>& first, const shared_ptr<PCB>& second) {
	if (first->executionTimeLeft < second->executionTimeLeft) {
		return true; //Brak zamiany
	}
	else if (first->executionTimeLeft == second->executionTimeLeft && first->PID < second->PID) {
		return true; //Brak zamiany
	}
	return false; //Pierwszy należy zamienić z drugim
}

void Planist::sort_ready_list() {
	//Sortowanie listy
	ReadyPCB.sort(compare_PCB);

	//Ustawienie pierwszego procesu na RUNNING a reszty na READY
	for (auto it = ReadyPCB.begin(); it != ReadyPCB.end(); ++it) {
		if (it == ReadyPCB.begin()) { (*it)->state = RUNNING; }
		else if ((*it)->state != READY) { (*it)->state = READY; }
	}
}

void Planist::display_PCB_lists() {
	//Wyświetlanie ReadyPCB
	cout << "Procesy gotowe: " << (ReadyPCB.empty() ? "pusta" : "") << "\n";
	for (const auto& elem : ReadyPCB) {
		cout << " - " << elem->name << ", PID: " << elem->PID;
		if (elem->executionTimeLeft != 9999) {
			cout << "	(cykle: " << elem->executionTimeLeft;
		}
		else {
			cout << "	(cykle: nie dotyczy";
		}
		if (elem->state == RUNNING) { cout << ", RUNNING"; }
		else if (elem->state == READY) { cout << ", READY"; }
		else if (elem->state == WAITING) { cout << ", WAITING"; }
		cout << ")\n";
	}

	//Wyświetlanie WaitingPCB
	cout << "\nProcesy czekajace: " << (WaitingPCB.empty() ? "pusta" : "") << "\n";
	for (const auto& elem : WaitingPCB) {
		cout << " - " << elem->name << ", PID: " << elem->PID;
		cout << "	(cykle: " << elem->executionTimeLeft;
		if (elem->state == RUNNING) { cout << " (RUNNING)"; }
		else if (elem->state == READY) { cout << " (READY)"; }
		else if (elem->state == WAITING) { cout << " (WAITING)"; }
		cout << '\n';
	}
	cout << '\n';
}
