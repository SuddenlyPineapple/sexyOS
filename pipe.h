#pragma once
#include <string>
#include <memory>
#include <queue>
#include <map>
#include "Semaphores.h"

class PCB;

class Pipeline {
private:
	class Pipe {
	private:
		//Z tych semaforów korzystam jak z binarnych
		Semaphore readSem;
		Semaphore writeSem;

	public:
		static const unsigned int capacity = 10;
		unsigned int spaceLeft = capacity;

		std::queue<char> buffer;
		std::shared_ptr<PCB> parent;

		explicit Pipe(std::shared_ptr<PCB> parent_);

		std::string read(const std::shared_ptr<PCB>& readProc, const size_t& size);
		int write(const std::shared_ptr<PCB>& writeProc, const std::string& message);
	};

	std::map<std::string, std::shared_ptr<Pipe>> pipes;

public:
	Pipeline() = default;

	void create(const std::string& parent, const std::string& mode);

	int write(const std::string& p1, const std::string& p2, std::string data);
	std::string read(const std::string& p1, const std::string& p2, size_t size);

	void remove(const std::string& parent, const std::string& mode);
	void remove(const std::string& parent);
	bool exists(const std::string& parent, const std::string& mode) const;
	bool exists(const std::string& parent) const;

	void display();
};


extern Pipeline pipeline;
