#pragma once

#include <string>
#include <array>

class PCB;

class Interpreter {
private:
	int A = 0, B = 0, C = 0, D = 0;
	unsigned int instructionCounter = 0;

public:
	Interpreter();

	void display_registers() const; //Wyœwietla stan rejestrów (do pracy krokowej)
	int execute_line(const std::string& procName);
	unsigned int simulate_program(const std::string& programWhole);

private:
	int execute_instruction(const std::string& instructionWhole, const std::string& procName);
	bool simulate_instruction(const std::string& instructionWhole);

	void take_from_proc(const std::string& procName);
	void update_proc(const std::string& procName) const;
	static std::array<std::string, 4> instruction_separate(const std::string& instructionWhole);
};

extern Interpreter interpreter;
