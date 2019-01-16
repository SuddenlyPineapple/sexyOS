#pragma once
#include <vector>
#include <string>

#include <queue>

class PCB;
class proc_tree;
class Pipe;

class Pipeline {
private:
	proc_tree* tree;
	std::vector<Pipe*> pipes;


public:
	Pipeline(proc_tree* tree_) : tree(tree_){}

	void createPipe(const std::string& p1, const std::string& p2);

	void write(const std::string& p1, const std::string& p2, const std::string& data);
	std::string read(const std::string& p1, const std::string& p2, const size_t& rozmiar);

	void deletePipe(const std::string& p1, const std::string& p2);
	bool existPipe(const std::string& p1, const std::string& p2);
};
class Pipe {
private:
	std::queue<char> buffer;
	PCB* p1;
	PCB* p2;
	Pipeline *p;

public:
	Pipe(PCB* p1, PCB* p2, Pipeline* potok);
	~Pipe();
	std::string read(size_t rozmiar);
	void write(const std::string& wiadomosc);
};