#include "Shell.h"
#include <iostream>

Shell::Shell()
{
	running = true;
	command_line.resize(0);
}


Shell::~Shell()
{
}
/*
void ps() {
	int p[2];
	pipe(p);

	if (fork() == 0) {
		close(1);
		dup(p[1]);
		close(p[0]);
		close(p[1]);
		runcmd(left);
	}
	if (fork() == 0) {
		close(0);
		dup(p[0]);
		close(p[0]);
		close(p[1]);
		runcmd(right);
	}
	close(p[0]);
	close(p[1]);
	wait();
	wait();
}

void fg() {

}

void kill() {

}

void cp() {

}

void locate(){

}

void ls() {
	close(fd);
	open(file, mode);
	runcmd(cmd);
}

void rm() {

}

void whoami() {

}

void chmod() {

}
*/

void Shell::help()
{
}

void Shell::boot()
{
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                  `:ossss+-       .+syys+.                                " << std::endl;
	std::cout << "                                                                .yNo.   .hMh`   .dd-  `/M/                                " << std::endl;
	std::cout << "            `:oso/     `:oso.    `/o/     -oo- -oo-      ++/   sMd.      `MMy   dMo     :                                 " << std::endl;
	std::cout << "            +N/..hh   .ym+-yMm   -hsNMs  /ddys`soyMN`    :MM:  yMN`        NMm   oMMh:                                    " << std::endl;
	std::cout << "            mMy`     /Nd` `hMs   .  -NMoho`   `  .MM/   `dMd  :MMo        .MMy    .oNMd/                                  " << std::endl;
	std::cout << "            .hMN+   :MMo+yds-        oMM+         NMy  -mMM:  oMM/        yMN.      `oNMh                                 " << std::endl;
	std::cout << "              :NM+  sMM:.`   `     .sysMN-        yMm`+dmMd   :MMo       sMd.         /MN                                 " << std::endl;
	std::cout << "          +o:.-hd.  :NMy::/os+ `yydd:  yMN+so     :MMNy:MM:    +NN/`  .+md/   .ys:. `:hh.                                 " << std::endl;
	std::cout << "         .+oso/.     .+ss+:`   -so-     /s+.       :+. hMh      `/ossso/`     ./oyyso:`                                   " << std::endl;
	std::cout << "                                                      :Mm`                                                                " << std::endl;
	std::cout << "                                                 `   :my`                                                                 " << std::endl;
	std::cout << "                                               `sNmhy+.                                                                   " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;
	std::cout << "                                                                                        ..----. .. --::.                  " << std::endl;
	std::cout << "                                                                                 .--::::::////:----::/+oso:               " << std::endl;
	std::cout << "                                                                          .-::////////:::::/:/+:-:/::::/+hy:              " << std::endl;
	std::cout << "                                                                      -:////////:-..`    ``-/:.-+o+/:///+hho-             " << std::endl;
	std::cout << "                                                                   -/////////-.            -:`-:/:-:oossshhy:             " << std::endl;
	std::cout << "                                                                  -///+++/-`              ./`.//://o+/oyhdhy/             " << std::endl;
	std::cout << "                                                                   .//++o:`               //`-:://+s+:+ydhhh+             " << std::endl;
	std::cout << "                                                                    `:/++++:.            `/-``-:/+o++oyhdhhho             " << std::endl;
	std::cout << "                                                                      .://+++/-`         `:`---:/osshdhddhyho             " << std::endl;
	std::cout << "                                                                        .:///++/::-.`    --.://+oshdddddhhyhs             " << std::endl;
	std::cout << "                                                                         `.:::/+++o+//:-..--://+osoosyhdhhhhs             " << std::endl;
	std::cout << "                                                                            .-:::/+o+++/--/.-:://---//+syhhhs             " << std::endl;
	std::cout << "                                                                               .-:://:/:..-:-......-://++shhy:            " << std::endl;
	std::cout << "                                                                                 ``--/:...-..--.-/:::///+ooyy/            " << std::endl;
	std::cout << "                                                                                     -.`...-----:+o+////+ooss+-           " << std::endl;
	std::cout << "                                                                                      ``.-:::::::/oyo+/+++osso--          " << std::endl;
	std::cout << "                                                                                  `.-:::://////////+so++++ooss/--         " << std::endl;
	std::cout << "                                                                                 -://////::://///////oo+++++oos/--        " << std::endl;
	std::cout << "                                                                                `-:///////////////////oso++++oos+--       " << std::endl;
	std::cout << "                                                                                ``-://////////////////+syo+++ooos/--      " << std::endl;
	std::cout << "                                                               .::-.`             `..:::////:::/:::////+ohhs+++ooss/-.    " << std::endl;
	std::cout << "                                                             .:///////-.`          ``.---::::::::////////hhhyo++ooss/-.   " << std::endl;
	std::cout << "                                                           .:////+++++///:.`         `:..-::::/::::://////yysooooooss:-.  " << std::endl;
	std::cout << "                                                        `.://:::///+++o+++//:-`       .:.-::::::::::://++oo+:+oooossso--  " << std::endl;
	std::cout << "                                                      `.://:--://///::::://+++/:.`   `-:---:::::::://///+ooooooossssss:-  " << std::endl;
	std::cout << "                                                    `.:/:--://++++++//////::-::::/:-`.:::::///::::://+++oooosssssss+:-    " << std::endl;
	std::cout << "                                                ` -/.-:::///++++++++++++++++++////:::///oo+////////+++ooooosssss+/:-      " << std::endl;
	std::cout << "                                              `-//://////+++++ooo+++++++++++++++:/+/++/+ooooo++/++ooosssyso-.-:           " << std::endl;
	std::cout << "                                           ` -://////+++++++oo+/--://++++++++++//+/oo+++oooo++oosyssssoo++/.              " << std::endl;
	std::cout << "                                        `-://///++++++++++++/:`     `.-:/+++++++++ooossssoosso+++++/::////:               " << std::endl;
	std::cout << "                                  .-://////++++++++++++//-.`           `-/++++++++++++++++++o+:::://///// :               " << std::endl;
	std::cout << "                                `-::///////+++oooooo++//:-                .:/++oooooooo+++++ooo+//////////:`              " << std::endl;
	std::cout << "                             :-////+oo++++++ooo+/--```                      `.-:/+ooooooooooooooss+///////::`             " << std::endl;
	std::cout << "                        `:://+ooossssoo,,``                                     -:+oooosssssssssso//////:::  `            " << std::endl;
	std::cout << "                     `:/+++++o,,,`````                                               -:+oosssssssyyyys+////::``           " << std::endl;
	std::cout << "                    `:/+++`````                                                          :/ossyyyyyyyyyyyo+/.``           " << std::endl;
	std::cout << "                 ,`:/+`                                                                     ```/////////::-               " << std::endl;
	std::cout << "                                                                                                                          " << std::endl;
}

void Shell::exit()
{
}
