#pragma once
#include <string>
#include <vector>

 



//zapodaj cos tu do interfejsu krokowego, jol.
class Interpreter
{
public:
	void rejestr_rozkaz();

	std::string rozkaz;
	std::string rej;
		
	int A;
	int B;
	int C;
	int D;

	int adres;
	int liczba;

	int licznik_rozkazow = 0;

	Interpreter() : A(0), B(0), C(0), D(0) {}

	void stan_rejestrow();
	void wykonanie_programu();
};

static Interpreter interpreter;