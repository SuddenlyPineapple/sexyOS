#pragma once

#include <list>
#include <memory>

class PCB;

//Zak³amy, ¿e jeden rozkaz wymaga jednego cyklu procesora

class Planist {
private:
	std::list<std::shared_ptr<PCB>> WaitingPCB;

	static bool compare_PCB(const std::shared_ptr<PCB>& first, const std::shared_ptr<PCB>& second);
	void sort_ready_list();

public:
	std::list<std::shared_ptr<PCB>> ReadyPCB;

	Planist() = default;
	~Planist() = default;

	void check();
	void add_process(const std::shared_ptr<PCB>& process);
	void remove_process(const std::shared_ptr<PCB>& process);

	//Praca krokowa
	void display_PCB_lists();
};

extern Planist planist;
