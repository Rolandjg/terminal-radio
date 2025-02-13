#include "player.h"
#include <iostream>
//#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_set>
#include "playerUtil.h"

int main() {
//	initscr();
//	noecho();
//	curs_set(0);

	// int selected;

	std::vector<std::string> files = PlayerUtil::getMusicFiles();

	Player p;
	p.setStation(files[0]);
	p.play();

	while(true){
		
	}

    return 0;
}

