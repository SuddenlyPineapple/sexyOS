/*
* Tutaj znajdują się zrobione przeze mnie testy dla poszczególnych funkcjonalności
* Trzeba pamiętać, że jeśli plik jest usuwany to w pamięci dysku zostaje, znika
  tylko z wektora bitowego i tablicy z powiązaniami
 */

#include "FileManager.h"
#include <iomanip>
#include <iostream>

using namespace std;

bool messages = true;

//File Create, Delete, Truncate
void test1() {
	FileManager FM;
	FM.Messages(messages);

	FM.FileCreate("plik1.txt", string("TekstTekst Tekst./*+$łążśyłóżźówa"));
	cout << '\n';

	cout << "--- Disk Content (bit) ---\n";
	FM.DisplayDiskContentBinary();
	cout << "--- Disk Content (char) ---\n";
	FM.DisplayDiskContentChar();
	cout << '\n';

	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';

	cout << "--- FAT Table ---\n";
	FM.DisplayFileAllocationTable();
	cout << '\n';

	FM.FileCreate("plik2.txt", string("TekstTekst Tekst./*+$łążśyłóżźówa"));
	cout << '\n';

	FM.DisplayFileInfo("plik1.txt");
	cout << '\n';

	FM.FileTruncate("plik1.txt", 20);
	cout << '\n';

	FM.DisplayFileInfo("plik1.txt");
	cout << '\n';

	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';

	FM.FileDelete("plik1.txt");
	cout << '\n';

	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';

	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();
	cout << '\n';
}

//Directories
void test2() {
	FileManager FM;
	FM.Messages(messages);

	FM.DirectoryCreate("Podkatalog0");
	FM.DirectoryCreate("Podkatalog0");
	FM.DirectoryCreate("Podkatalog1");
	FM.DirectoryCreate("Podkatalog2");
	FM.DirectoryCreate("Podkatalog3");
	cout << '\n';

	FM.DirectoryDown("Katalog");
	cout << '\n';

	FM.DirectoryDown("Podkatalog2");
	FM.FileCreate("Plik1.txt", "Dane Tekstowe");
	FM.FileCreate("Plik2.txt", "Dane Tekstowe");
	FM.FileCreate("Plik3.txt", "Dane Tekstowe");
	cout << '\n';

	FM.DirectoryUp();
	cout << '\n';

	FM.DirectoryUp();
	cout << '\n';

	FM.DirectoryDown("Podkatalog2");
	FM.DirectoryCreate("Dir1");
	cout << '\n';

	FM.DirectoryDown("Dir1");
	FM.DirectoryCreate("Dir2");
	cout << '\n';

	FM.DirectoryDown("Dir2");
	FM.FileCreate("Plikt.txt", "Dane Tekstowe");
	cout << '\n';

	FM.DirectoryRoot();
	FM.FileCreate("Plik1.txt", "Dane Tekstowe");
	FM.FileCreate("Plik2.txt", "Dane Tekstowe Bardzo Długie i Nudne, Żeby Tylko Były");
	cout << '\n';

	FM.DisplayFileInfo("Plik2.txt");
	cout << '\n';

	FM.FileDelete("NoName.txt");
	cout << '\n';

	FM.DisplayDirectoryInfo("Podkatalog2");
	cout << '\n';

	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();

	FM.DirectoryDelete("Podkatalog2");
	cout << '\n';

	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();
}

//Space fill and file delete
void test3() {
	FileManager FM;
	FM.Messages(messages);

	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';
	FM.FileCreate("plik1.txt", string("TekstTekst Tekst./*+$łążśyłóżźówa"));
	FM.FileCreate("plik2.txt", string("Blablablablablablablablabla"));
	FM.FileCreate("plik3.txt", string("Sratatatatatata"));
	FM.FileDelete("plik2.txt");
	FM.FileCreate("plik2.txt", string("TekstTekst Tekst./*+$łążśyłóżźówaaaaaaaaaaaaa"));
	FM.FileCreate("plik4.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik5.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik6.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik7.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik8.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik9.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik10.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik11.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik12.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik13.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik14.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik15.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik16.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));
	FM.FileCreate("plik17.txt", string("WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"));

	cout << '\n';
	cout << "--- Disk Content (char) ---\n";
	FM.DisplayDiskContentChar();
	cout << '\n';
	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';
	cout << "--- FAT Table ---\n";
	FM.DisplayFileAllocationTable();
	cout << '\n';
	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();
}

//Allocation test
void test4() {
	FileManager FM;
	FM.Messages(messages);

	FM.FileCreate("plik1.txt", string("Plik1Plik1Plik1Plik1Plik1Plik1"));
	FM.FileCreate("plik2.txt", string("Plik2Plik2Plik2Plik2Plik2Plik2"));
	FM.FileCreate("plik3.txt", string("Plik3Plik3"));
	FM.FileCreate("plik4.txt", string("Plik4Plik4"));
	FM.FileCreate("plik5.txt", string("Plik5Plik5Plik5Plik5"));

	cout << '\n';
	cout << "--- Disk Content (char) ---\n";
	FM.DisplayDiskContentChar();
	cout << '\n';
	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';

	FM.FileDelete("plik2.txt");
	FM.FileDelete("plik4.txt");

	FM.FileCreate("plik6.txt", string("Plik6Plik6"));
	cout << '\n';
	cout << "--- Disk Content (char) ---\n";
	FM.DisplayDiskContentChar();
	cout << '\n';
	cout << "--- Bit Vector ---\n";
	FM.DisplayBitVector();
	cout << '\n';
	cout << "--- FAT Table ---\n";
	FM.DisplayFileAllocationTable();
}

//Rename test
void test5() {
	FileManager FM;
	FM.Messages(messages);

	FM.FileCreate("plik1.txt", string("Tekst"));
	FM.FileCreate("plik2.txt", string("Tekst"));
	FM.FileCreate("plik3.txt", string("Tekst"));
	cout << '\n';
	FM.DisplayFileInfo("plik1.txt");
	cout << '\n';
	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();
	cout << '\n';

	FM.FileRename("plik1.txt", "Awesome.txt");
	FM.FileRename("plik2.txt", "Super.txt");
	FM.FileRename("plik3.txt", "Fantastic.txt");
	cout << '\n';
	FM.DisplayFileInfo("plik1.txt");
	cout << '\n';
	FM.DisplayFileInfo("Awesome.txt");
	cout << '\n';
	cout << "--- Struktura Katalogów ---\n";
	FM.DisplayDirectoryStructure();
	cout << '\n';
}

//Static class object test
void test6() {
	//Za drugim wywołaniem testu w pamięci powinny być już zapisane dane oraz nazwa pliku będzie już zajęta.
	fileManager.Messages(true);

	cout << "--- Disk Content (char) ---\n";
	fileManager.DisplayDiskContentChar();
	cout << '\n';

	fileManager.FileCreate("Plik1", "DataSrata");

	fileManager.Messages(false);
}

int main() {
	const vector<string>desc{ "File Create, Delete, Truncate", "Directories", "Space fill and file delete", "Allocation test", "Rename test", "Static class object test" };

	const auto run = [](void fun()) {
		system("cls");
		fun();
		cout << '\n';
		system("pause");
		system("cls");
	};

	system("chcp 1250");
	system("cls");
	const vector<void(*)()>functions{ &test1,&test2,&test3,&test4,&test5, &test6 };

	while (true) {
		//Elementy stałe
		cout << "Messages: " << (messages ? "Enabled" : "Disabled") << "\n\n";
		cout << setfill('0') << setw(2) << -1 << ". " << "Enable/Disable Messages" << '\n';
		cout << setfill('0') << setw(2) << 0 << ". " << "Exit" << "\n\n";

		//Wyświetla numery i opisy poszczególnych testów
		for (unsigned int i = 0; i < desc.size(); i++) {
			cout << setfill('0') << setw(2) << i + 1 << ". " << desc[i] << '\n';
		}
		cout << "\nPodaj numer(-1 do " << functions.size() << "): ";

		int testNum;
		cin >> testNum;

		//Włączanie/wyłączanie wiadomości
		if (testNum == -1) {
			if (messages) {
				std::cout << "\nMessages disabled\n\n";
				system("pause");
				messages = false;
			}
			else {
				std::cout << "\nMessages enabled\n\n";
				system("pause");
				messages = true;
			}
		}

		//Działa jeśli wybrano poprawny numer testu
		else if (static_cast<size_t>(testNum) >= 1 && static_cast<size_t>(testNum) <= functions.size()) {
			run(functions[testNum - 1]);
		}

		//Wyjście
		else if (testNum == 0) { break; }

		system("cls");
	}
}
