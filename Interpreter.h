#pragma once

#include <string>
#include <vector>
#include <array>

class proc_tree;
class FileManager;
class MemoryManager;
class PCB;

class Interpreter
{
public:
	FileManager* fileManager;
	MemoryManager* memoryManager;
	proc_tree* tree;

	void rejestr_rozkaz();

	std::string rozkazCaly;

	int A;
	int B;
	int C;
	int D;

	int adres = -1;
	int liczba = -1;

	int licznik_rozkazow = 0;

	Interpreter(FileManager* fileManager_, MemoryManager* memoryManager_, proc_tree* tree_);

	//Funkcja testowa
	static std::array<std::string, 3> rozdziel_rozkaz(const std::string& rozkazCaly);
	//Koniec funkcja

	void stan_rejestrow() const;
	bool wykonanie_rozkazu();
};

//static Interpreter interpreter;