#pragma once
#include <string>
#include <vector>



struct rozkazy //to 1 rozkaz, ale whatevs
{
	std::string rej;
	std::string rozkaz;
};

//zapodaj cos tu do interfejsu krokowego, jo³.
class Interpreter
{
public:
	Interpreter();
	int A;
	int B;
	int C;
	int D;
	
	int counter = 0;
	
	void pobierz_rejestry();//nope,
	void pobierz_rozkazy();//nic tu nie ma

	void stan_rejestrow();
	void wykonanie_programu(rozkazy);
};

