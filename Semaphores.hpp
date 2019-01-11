
#ifndef SEMAPHORES_SEMAPHORES_HPP
#define SEMAPHORES_SEMAPHORES_HPP
#pragma once
#include <iostream>
#include <list>
#include <map>
#include "Procesy.hpp"
#include "Planista.h"
class PCB;
class Planista;
class Semaphore{
private:
    Planista p;
    int value;
public:

    Semaphore(int n); //wartość semafora ustawiana na początku
    void Wait(Semaphore *S);
    void Signal(Semaphore *S);
    void block(PCB a); // zmienia stan pierwszego procesu na liście na WAITING i wywołuje funkcję Check() od planisty
    void wakeup(PCB a); // zmienia stan pierwszego procesu na liście WaitingPCB na READY i wywołuje funkcję Check () od planisty
};








#endif //SEMAPHORES_SEMAPHORES_HPP
