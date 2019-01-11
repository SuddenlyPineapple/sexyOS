#include "Semaphores.hpp"
#include <list>

Semaphore::Semaphore(int n) {
    this->value = n;
}
void Semaphore::Wait(Semaphore *S) {
    S->value = value--;
    if (S->value < 0) {
        PCB a = p.ReadyPCB.front();
        block(a);
    }
}
void Semaphore::Signal(Semaphore *S){
    S->value = value ++;
    if(S->value <= 0){
		wakeup(p.getWaitingPCB().front());
    }
}
void Semaphore::block(PCB a) {
        a.change_state(WAITING);
        p.Check();
    }
 void Semaphore::wakeup(PCB a){
        a.change_state(RUNNING);
        p.Check();
    }
