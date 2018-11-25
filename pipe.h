#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <queue>
class Pipeline {
public:
	Pipeline();
	std::vector<Pipe*> pipes;
	void createPipe(PCB &p1, PCB &p2);
	void deletePipe(PCB& p1);
	bool existPipe(PCB& p1, PCB& p2);
};
class Pipe{
public:
	Pipe(PCB& p1,PCB& p2,Pipeline& potok);
	~Pipe();
	std::string read(size_t rozmiar);
	void write(const std::string& wiadomosc);
private:
	std::queue<char> buffer;
	PCB *p1;
	PCB *p2;
	Pipeline *p;
};