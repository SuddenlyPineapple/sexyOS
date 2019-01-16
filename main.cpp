#include <iostream>
#include "Procesy.h"
#include <windows.h>
#include "MemoryManager.h"
//#include "Shell.h"
#pragma comment(lib, "Winmm.lib")
//proc_tree Drzewo = proc_tree();
MemoryManager mm = MemoryManager();

int main() {
    //Shell shell;
    //shell.boot();
    PlaySound(TEXT("Startup.wav"), NULL, SND_ALIAS);

    //MemoryManager memoryManager;
    //memoryManager.memoryInit();
    //memoryManager.showFrames();

    //auto test = memoryManager.createPageList(16,1);
    //memoryManager.showPageTable(test);

    //memoryManager.showMem();
    //memoryManager.showFrames();
    //memoryManager.showPageFile();

    /*
    memoryManager.showLRUStack();
    memoryManager.stackUpdate(12);
    memoryManager.stackUpdate(13);
    memoryManager.stackUpdate(1);
    memoryManager.stackUpdate(5);
    memoryManager.stackUpdate(16);
    memoryManager.showStack();
    */

    mm.memoryInit();
    mm.showFrames();
    mm.showPageFile();

    Drzewo.fork(new PCB("kek", 1), "kek", "program1.txt", mm, 100);
    Drzewo.fork(new PCB("kek", 1), "kek", "program2.txt", mm, 100);
    mm.showFrames();
    mm.showPageFile();
    Drzewo.fork(new PCB("dsd", 1), "pensadasdis");
    //Drzewo.display_tree();
    //std::cout << "PID " << Drzewo.proc.GET_kid(2)->PID << " nazwa " << Drzewo.proc.GET_kid(2)->process_name << std::endl;
    Drzewo.fork(new PCB("test dla 2", 2), "test dla 2");
    //Drzewo.display_tree();
    Drzewo.fork(new PCB("test dla 4", 4), "test dla 4");
    Drzewo.fork(new PCB("sadasd",3), "dasdasd");
    Drzewo.display_tree();
    //Drzewo.proc.GET_kid.
    //Drzewo.display_tree(7);


    mm.showFrames();
    mm.showMem();
    mm.showPageFile();
    mm.showStack();
    mm.showPageTable(Drzewo.proc.pageList);

    //shell.exit();
    return 0;
}