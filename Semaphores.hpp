
#ifndef SEMAPHORES_SEMAPHORES_HPP
#define SEMAPHORES_SEMAPHORES_HPP


class PCB;
class Planista;

class Semaphore {
private:
	Planista* p;
	PCB* pcb;
	bool blocked = false;
	int value;
public:

	Semaphore(Planista* plan, PCB* pcb_); //Podstawowy konstruktor, ustawia wartość na -1
	Semaphore(Planista* plan, PCB* pcb_, const int& n); //wartość semafora ustawiana na początku
	void Wait();
	void Signal();
	void block() const; // zmienia stan pierwszego procesu na liście na WAITING i wywołuje funkcję Check() od planisty
	void wakeup() const; // zmienia stan pierwszego procesu na liście WaitingPCB na READY i wywołuje funkcję Check () od planisty

	const bool& is_blocked() const;
	const int& get_value() const;
};


#endif //SEMAPHORES_SEMAPHORES_HPP
