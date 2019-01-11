#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>

class proc_tree;
class FileManager;
class MemoryManager;
class PCB;

class Interpreter
{
private:
	FileManager* fileManager;
	MemoryManager* memoryManager;
	proc_tree* tree;

	void registers_state();

	int A;
	int B;
	int C;
	int D;

	int address = -1;
	int number = -1;

	int instruction_counter = 0;
	int RAM_pos = 0; //Aktualny bajt do odczytywania z procesu

	//Mapa pocz¹tków instrukcji
	//Klucz	  - numer rozkazu
	//Wartoœæ - address pierwszego bajtu rozkazu
	std::map<int, int> instrBeginMap;

public:
	Interpreter(FileManager* fileManager_, MemoryManager* memoryManager_, proc_tree* tree_);

	void stan_rejestrow() const;
	void execute_program(const int& PID);
	bool execute_instruction(const std::string& instructionWhole, const int& PID);

private:
	static std::array<std::string, 3> instruction_separate(const std::string& instructionWhole);
	void jump_pos_set(const int& PID);
};

//static Interpreter interpreter;