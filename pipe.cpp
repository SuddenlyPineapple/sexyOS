#include "Pipe.h"
#include "Processes.h"
#include <iostream>
#include <utility>

//znaczenie fd(deksryptor)
//0-koniec do czytania
//1-koniec do zapisu
//-1-oznaczenie ze potok nie jest już potrzebny

using namespace std;

Pipeline pipeline;

#define PIPE_READ_END  0
#define PIPE_WRITE_END 1
#define UNUSED	  0
#define USED      1
#define PIPE_MODE_READ string("_R")
#define PIPE_MODE_WRITE string("_W")

// Pipeline ----------------

//Tworzenie potoku (dla procesu parent)
void Pipeline::create(const string& parent, const string& mode) {
	if (pipes.find(parent + mode) == pipes.end()) {
		const auto parentProc = tree.find(parent);
		pipes[parent + mode] = make_shared<Pipe>(parentProc);
		if (mode == "_W") { parentProc->FD[PIPE_WRITE_END] = USED; }
		else if (mode == "_R") { parentProc->FD[PIPE_READ_END] = USED; }
	}
}

//Zapisywanie do potoku w kierunku z p2 do p1 (potok musi istnieć)
int Pipeline::write(const string& p1, const string&p2, string data) {
	if (!p2.empty()) {
		shared_ptr<PCB> parentProc;
		if (tree.find(p1)->parent->name == p2) { parentProc = tree.find(p2); }
		else if (tree.find(p2)->parent->name == p1) { parentProc = tree.find(p1); }
		//Potok nie istnieje
		else { return -1; }

		if (data.length() > Pipe::capacity) { data.resize(Pipe::capacity); }

		//Potok nie istnieje
		if (parentProc->name == p1 && parentProc->FD[PIPE_READ_END] != USED ||
			parentProc->name == p2 && parentProc->FD[PIPE_WRITE_END] != USED) {
			return -1;
		}
		if (parentProc->name == p1) {
			return pipes[p1 + "_R"]->write(tree.find(p1), data);
		}
		else {
			return pipes[p2 + "_W"]->write(tree.find(p1), data);
		}
	}
	else { return pipes[p1 + "_W"]->write(tree.find(p1), data); }
}

//Odczytywanie z potoku w kierunku z p2 do p1 (potok musi istnieć)
string Pipeline::read(const string& p1, const string& p2, size_t size) {
	if (size > Pipe::capacity) { size = Pipe::capacity; }
	else if (size <= 0) { return ""; }

	if (!p2.empty()) {
		shared_ptr<PCB> parentProc;
		if (tree.find(p1)->parent->name == p2) { parentProc = tree.find(p2); }
		else if (tree.find(p2)->parent->name == p1) { parentProc = tree.find(p1); }
		else { return "no_pipe"; }

		if (parentProc->name == p1 && parentProc->FD[PIPE_WRITE_END] != USED ||
			parentProc->name == p2 && parentProc->FD[PIPE_READ_END] != USED) {
			return "no_pipe";
		}

		if (parentProc->name == p1) {
			return pipes[p1 + "_R"]->read(tree.find(p1), size);
		}
		else {
			return pipes[p2 + "_W"]->read(tree.find(p1), size);
		}
	}
	else { return pipes[p1 + "_R"]->read(tree.find(p1), size); }
}

void Pipeline::remove(const string& parent, const string& mode) {
	if (exists(parent, mode)) { pipes.erase(parent + mode); }
}

void Pipeline::remove(const string& parent) {
	remove(parent, PIPE_MODE_READ);
	remove(parent, PIPE_MODE_WRITE);
}

bool Pipeline::exists(const string& parent, const std::string& mode) const {
	if (pipes.find(parent + mode) != pipes.end()) { return true; }
	else { return false; }
}
bool Pipeline::exists(const string& parent) const {
	if (exists(parent, PIPE_MODE_READ) || exists(parent, PIPE_MODE_WRITE)) { return true; }
	else { return false; }
}

void Pipeline::display() {
	for (const auto& elem : this->pipes) {
		if (elem.second != nullptr) {
			cout << "| " << elem.first << ": ";
			auto temp = elem.second->buffer;
			while (!temp.empty()) { cout << temp.front(); temp.pop(); }
			cout << '\n';
		}
	}
}


// Pipe ----------------

Pipeline::Pipe::Pipe(shared_ptr<PCB> parent_) : readSem(0), writeSem(1), parent(move(parent_)) {}

string Pipeline::Pipe::read(const shared_ptr<PCB>& readProc, const size_t& size) {
	//sprawdzanie, czy jest coś w kolejce
	if (buffer.empty() || size > buffer.size()) {
		readSem.wait(readProc);
		return "sem_blocked";
	}
	else {
		string result;
		for (unsigned int i = 0; i < size && !buffer.empty(); i++) {
			result.push_back(buffer.front());
			spaceLeft++;
			buffer.pop();
		}
		if (spaceLeft > 0) {
			writeSem.set_value(0);
			writeSem.signal_all();
		}
		return result;
	}
}

int Pipeline::Pipe::write(const shared_ptr<PCB>& writeProc, const string &message) {
	if (message.length() > spaceLeft) {
		writeSem.wait(writeProc);
		return 0;
	}

	for (auto x : message) { spaceLeft--; buffer.push(x); }
	if (spaceLeft < capacity) {
		readSem.set_value(-1);
		readSem.signal_all();
	}
	return 1;
}
