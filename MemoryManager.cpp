// Created by Wojciech Kasperski on 15-Oct-18.
#include "MemoryManager.h"
#include "Processes.h"
#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

MemoryManager mm;

//------------- Konstruktory i destruktory  --------------
MemoryManager::Page::Page(string data) {
	while (data.length() < 16) { data += " "; } // Uzupełnianie argumentu spacjami, jeśli jest za mały
	// Przepisywanie argumentu do stronicy
	for (int i = 0; i < 16; i++) { this->data[i] = data[i]; }
}

MemoryManager::Page::Page() : Page("") {}

PageTableData::PageTableData(bool bit, int frame) : bit(bit), frame(frame) {}

PageTableData::PageTableData() {
	this->bit = false;
	this->frame = -1;
};

MemoryManager::FrameData::FrameData(bool isFree, int PID, int pageID, vector<PageTableData> *pageList) : isFree(isFree), PID(PID), pageID(pageID), pageList(pageList) {}

MemoryManager::MemoryManager() : RAM{} {
	for (int i = 0; i < 16; i++) { Frames.emplace_back(FrameData(true, -1, -1, nullptr)); }
};

MemoryManager::~MemoryManager() = default;

//------------- Funkcje do wyświetlania bieżących stanów pamięci oraz pracy krokowej  --------------
void MemoryManager::Page::print() const {
	for (auto &x : data) {
		if (x == ' ') cout << "_";
		else cout << x;
	}
	cout << '\n';
}

void MemoryManager::show_memory() {
	cout << "RAM - PHYSICAL FRAMES CONTENT: \n";
	cout << "First bit: 0 ->\t0123456789012345 -> 15 :last bit in frame\n";
	for (int i = 0; i < 256; i++) {
		if (i % 16 == 0 && i != 0) { cout << "\nFrame no." << i / 16 << ": \t"; }
		else if (i % 16 == 0) { cout << "Frame no." << i / 16 << ": \t"; }
		if (RAM[i] == ' ' || RAM[i] == '\0') { cout << '_'; }
		else if (RAM[i] == '\n' || RAM[i] == '\r') { cout << '_'; }
		else { cout << RAM[i]; }
	}
	cout << endl;
}

void MemoryManager::show_memory(int begin, int bytes) {
	if (begin + bytes > 256) {
		cout << "Error: Number of bytes to display has excced amount of memory! \n";
	}
	else {
		cout << "Displaying physical memory from cell " << begin << " to " << begin + bytes << ":" << endl;
		for (int i = begin; i < begin + bytes; i++) {
			if (i != 0 && i % 16 == 0) cout << "\n";
			RAM[i] != ' ' ? cout << RAM[i] : cout << '-';
		}
		cout << endl;
	}
}

void MemoryManager::show_page_file() const {
	cout << "-------Page File-------\n";
	for (const auto& process : PageFile) {
		cout << "\n**** PID:" << process.first << " ****\n";
		//cout << "Pages Content:";\n
		for (unsigned int i = 0; i < process.second.size(); i++) {
			cout << setfill('0') << setw(2) << i << ". ";
			process.second[i].print();
		}
	}
}

void MemoryManager::show_page_table(const shared_ptr<vector<PageTableData>>& pageList)
{
	cout << "PAGE\t | \tFRAME \t | \tBIT \n";
	int i = 0;
	for (const auto pageListRecord : *pageList) {
		cout << i++ << "\t\t" << pageListRecord.frame << "\t\t" << pageListRecord.bit << "\n";
	}
}

void MemoryManager::show_stack() {
	cout << "FIFO Stack: ";
	for (auto frame : Stack) {
		cout << frame << " ";
	}
	cout << endl;
}

void MemoryManager::show_frames() {
	cout << "FRAMES INFO: \n";
	cout << "\t\tFREE \tPAGE \tPID " << endl;
	int i = 0;
	for (auto &frame : Frames) {
		cout << "Frame no." << i++ << ":\t" << frame.isFree << "\t" << frame.pageID << "\t" << frame.PID << "\n";
	}
}

//------------- Funkcje użytkowe MemoryManagera  --------------
void MemoryManager::memory_init() {
	for (char &cell : RAM) { cell = ' '; }
	PageFile.emplace(pair(1, vector<Page>{ Page("JMP [0];") }));
}

void MemoryManager::stack_update(int frameID) {
	if (frameID > 15) return;

	for (auto it = Stack.begin(); it != Stack.end(); ++it) {
		if (*it == frameID) {
			Stack.erase(it);
			break;
		}
	}

	//Stack.push_front(frameID);
	Stack.push_back(frameID);
}

shared_ptr<vector<PageTableData>> MemoryManager::create_page_list(int mem, int PID) {
	const double pages = ceil(static_cast<double>(mem) / 16);
	shared_ptr<vector<PageTableData>> pageList = make_shared<vector<PageTableData>>();

	for (int i = 0; i < pages; i++) {
		pageList->push_back(PageTableData(false, 0));
	}

	//Panie Kasperczak, zapomniał Pan, że w mapie trzeba najpierw stworzyć pozycję
	//PageFile[PID] gdy nie ma pozycji to ją tworzy

	//Załadowanie pierszej stronicy naszego programu do Pamięci RAM
	load_to_memory(PageFile[PID].at(0), 0, PID, pageList);

	return pageList;
}

void MemoryManager::resize_page_list(int size, PCB* proc) {
	for (int i = int(ceil(proc->size / 16.0)); i < int(ceil((size / 16.0))); i++) {
		proc->pageList->push_back(PageTableData(false, 0));
		PageFile[proc->PID].emplace_back(Page());
	}
	proc->size = static_cast<unsigned int>(ceil(size / 16.0)) * 16;
}

int MemoryManager::seek_free_frame() {
	int seekedFrame = -1;


	for (size_t i = 0; i < Frames.size(); i++) {
		if (Frames[i].isFree) {
			seekedFrame = i;
			break;
		}
	}
	return seekedFrame;
}

void MemoryManager::kill(int PID) {
	for (size_t i = 0; i < Frames.size(); i++) {
		if (Frames[i].PID == PID) {
			for (size_t j = i * 16; j < i * 16 + 16; j++)
				RAM[j] = ' ';
			stack_update(i);
			Frames[i].isFree = true;
			Frames[i].pageID = -1;
			Frames[i].PID = -1;
			PageFile.erase(PID);
		}
	}
}

int MemoryManager::load_program(const string& path, int PID) {
	fstream file(path);			//Plik na dysku
	string scrap;				//Zmienna pomocnicza
	string program;				//Program w jednej linii
	vector<Page> pageVector;	//Wektor stronic do dodania

	if (!file.is_open()) {
		cout << "Error: Nie mozna otworzyc pliku! \n";
		return -1;
	}

	while (getline(file, scrap)) {
		//Dodanie spacji zamiast końca linii
		if (!scrap.empty()) {
			scrap += ";";
			program += scrap;
		}
	}
	const double pagesAmount = ceil(program.length() / 16.0);
	scrap.clear();

	//Dzielenie programu na stronice
	for (char i : program) {
		scrap += i;
		//Tworzenie Stronicy
		if (scrap.size() == 16) {
			pageVector.emplace_back(Page(scrap));
			scrap.clear();
		}
	}

	if (!scrap.empty()) { pageVector.emplace_back(Page(scrap)); }
	scrap.clear();

	if (pagesAmount * 16 < 16 * pageVector.size()) {
		cout << "Error: proces nie ma przypisane wystarczajaco duzo pamieci!\n";
		return -1;
	}


	//Sprawdzanie, czy program nie potrzebuje wiecej stronic w pamięci
	for (int i = pageVector.size(); i < pagesAmount; i++)
		pageVector.emplace_back(scrap);

	//Dodanie stronic do pliku wymiany
	PageFile.insert(make_pair(PID, pageVector));

	return program.length();
}

int MemoryManager::load_to_memory(Page page, int pageID, int PID, const shared_ptr<vector<PageTableData>>& pageList) {
	int frame = seek_free_frame();

	if (frame == -1) { frame = insert_page(pageID, PID); }

	//Przepisywanie stronicy do pamięci RAM
	int it = 0;
	const int end = frame * 16 + 16;
	for (int i = frame * 16; i < end; i++)
		RAM[i] = page.data[it++];

	//Zmienianie bit'u w indeksie wymiany stronic
	pageList->at(pageID).bit = true;
	pageList->at(pageID).frame = frame;

	//Aktualizacja stosu używalności
	stack_update(frame);

	//Aktualizacja informacji o ramce
	Frames[frame].isFree = false;
	Frames[frame].pageID = pageID;
	Frames[frame].PID = PID;
	Frames[frame].pageList = pageList;

	return frame;
}

string MemoryManager::get_byte(const shared_ptr<PCB>& process, int address) {
	string response;
	unsigned int PageID = address / 16;

	//przekroczenie zakres dla tego procesu
	if (process->pageList->size() <= PageID) {
		cout << "Error: Exceeded memory range!";
		return "ERROR";
	}

	PageID = address / 16; //Numer stronicy w pamięci

	//Sprawdza, czy stronica znajduje się w pamięci operacyjnej
	if (!process->pageList->at(PageID).bit)
		load_to_memory(PageFile[process->PID][PageID], PageID, process->PID, process->pageList);

	if (process->pageList->at(PageID).bit) {
		const int Frame = process->pageList->at(PageID).frame;//Bieżąco używana ramka

		//stack_update(Frame);//Ramka została użyta, więc trzeba zaktualizować stos

		response += RAM[Frame * 16 + address - 16 * PageID];
	}

	return response;
}

int MemoryManager::write(const shared_ptr<PCB>& process, int address, string data) {
	if (data.empty()) { return 1; }

	if (address + data.length() - 1 > process->pageList->size() * 16 || address < 0) {
		cout << "Error: Exceeded memory amount for this process! \n";
		return -1;
	}

	//Aktualizacja pliku wymiany
	{
		int pageId = int(floor((address + 1) / 16.0));

		int addressTemp = address;
		for (unsigned int i = 0;; i++) {
			PageFile[process->PID][pageId].data[addressTemp % 16] = data[i];

			if (i == data.length() - 1) { break; }
			addressTemp++;
			if (addressTemp % 16 == 0) { pageId++; }

		}
	}

	for (size_t i = 0; i < data.length(); i++) {
		const int pageID = int(floor((address + i) / 16.0));
		if (process->pageList->at(pageID).bit == 0) {
			load_to_memory(PageFile[process->PID][pageID], pageID, process->PID, process->pageList);
		}
		RAM[process->pageList->at(pageID).frame * 16 + address + i - (16 * pageID)] = data[i];
	}
	return 1;
}

int MemoryManager::write_direct(int address, std::string data) {
	for (size_t i = address; i < address + data.length(); i++) {
		RAM[i] = data[i-address];
	}
	return 1;
}

int MemoryManager::insert_page(int pageID, int PID) {
	//Numer ramki ktora jest ofiarą
	const int Frame = *Stack.begin();
	// Przepisuje zawartosc z ramki ofiary do pliku wymiany
	for (int i = Frame * 16; i < Frame * 16 + 16; i++) {
		PageFile[Frames[Frame].PID][Frames[Frame].pageID].data[i - (Frame * 16)] = RAM[i];
	}

	//Zmieniam wartosci w tablicy stronic ofiary
	Frames[Frame].pageList->at(Frames[Frame].pageID).bit = false;
	Frames[Frame].pageList->at(Frames[Frame].pageID).frame = -1;

	return Frame;
}
