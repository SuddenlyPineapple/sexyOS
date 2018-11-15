#pragma once
#include<vector>


class Shell {
private:
	bool running;												// potrzebna do utrzymania pętli shella
	std::vector<std::string> command_line;						// vector funkcji oraz paramterow
	enum spis_funkcji {
		HELP, EXIT,
	};												
public:
	void help();
	void boot();
	void exit();
};
