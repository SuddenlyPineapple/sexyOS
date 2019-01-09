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
public:
	std::list<PCB> ReadyPCB;
	Planista() {}
	~Planista() {}

	void Check() {
		for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); Rpcb++) {
			if (Rpcb->state == TERMINATED) {
				Rpcb = ReadyPCB.erase(Rpcb);
			}
			if (Rpcb->state != READY) {
				WaitingPCB.push_back(*Rpcb);
				Rpcb = WaitingPCB.erase(Rpcb);
			}
			else {
			//	MakeOlder(Rpcb);
				SetPriority(*Rpcb);
			}
		}
		for (Wpcb = WaitingPCB.begin(); Wpcb != WaitingPCB.end(); Wpcb++) {
			if (Wpcb->state == READY) {
				AddProces(*Wpcb);
				Wpcb = WaitingPCB.erase(Wpcb);

			}
		}
	}
	void MakeOlder(std::list<PCB>::iterator &proces) {

	}
	void AddProces(PCB Proces) {
		bool x=0;
//		SetPriority(Proces);
		if(Proces.state == READY){
			if (ReadyPCB.size() == 0) {
				ReadyPCB.push_back(Proces);
			}
			else {
				for (Rpcb = ReadyPCB.begin(); Rpcb != ReadyPCB.end(); Rpcb++) {
					if (Proces.priority > Rpcb->priority) {							//   <><><> CO KURWA MA BYC
						ReadyPCB.insert(Rpcb, Proces);
						if (Rpcb==ReadyPCB.begin()) {								// jesli proces bêdzie na 1 miejscu
							WywlaszczeniePCB = 1;									// flaga i prze³adowanie kontekstu
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
	void SetPriority(PCB &Proces) {
/*		if (Proces.last_counter == 0) {
			float LogRozmiar = log(Proces.proces_size);
			//int IO = 0;																// jak okreœliæ na wstêpie iloœæ operacji wejscia/wyjscia? 
			Proces.priority = 20 - LogRozmiar / 2; // (IO + LogRozmiar / 2);                // dopisaæ obliczanie od rozmiaru i iloœæ I/O
		}
		else {
*/			float x = 0;
			if (Proces.last_counter == 0) { 
				Proces.last_counter++; 
			}
			if (Proces.comand_counter != 0) { 
				Proces.last_counter = Proces.comand_counter - Proces.last_counter; 
			}
			x = (4 * Proces.priority + 4*log(Proces.last_counter)) / 4;
			Proces.priority = (int)x;
//		}
	}
};