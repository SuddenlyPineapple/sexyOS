
#ifndef SEMAPHORES_SEMAPHORES_HPP
#define SEMAPHORES_SEMAPHORES_HPP

#include <Processes.h>
#include <queue>

class Semaphore {
private:
	std::queue<std::shared_ptr<PCB>> waitingPCB;
	bool blocked = false;
	int value;
public:

	Semaphore(); //Podstawowy konstruktor, ustawia wartość na 99
	Semaphore(const int& n); //wartość semafora ustawiana na początku
	void wait(const std::shared_ptr<PCB>& pcb);
	void signal();
	void signal_all();

	const bool& is_blocked() const;
	const int& get_value() const;
	void set_value(const int& val);
	void show_value() const;

private:
	void block(const std::shared_ptr<PCB>& pcb); //Zmienia stan procesu na WAITING i wywołuje funkcję check() od planisty
	void wakeup(); //Zmienia stan pierwszego procesu na liście WaitingPCB na READY i wywołuje funkcję check () od planisty
};

#endif //SEMAPHORES_SEMAPHORES_HPP
