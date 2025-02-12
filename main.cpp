#include "player.h"
#include <iostream>
//#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_set>

std::vector<std::string> getMusicFiles() {
	const char* home = std::getenv("HOME");
	std::string home_string = home;

	std::vector<std::string> files;
	
	std::string path = home_string + "/Music";

	std::unordered_set<std::string> audioExtensions = {".mp3", ".flac", ".wav", ".aac", ".ogg", ".m4a"};

    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string ext = entry.path().extension().string(); // Get file extension as string

        if (audioExtensions.find(ext) != audioExtensions.end()) {  // Check if it's in the set
            files.push_back(entry.path().string());  // Add file to list
        }
    }
	return files;
}


int main() {
//	initscr();
//	noecho();
//	curs_set(0);

	// int selected;

	Player p;
	p.setStation("http://ice1.somafm.com/groovesalad-128-mp3");
	p.play();

	while(true){
		
	}

	std::vector<std::string> files = getMusicFiles();

	for(std::string file : files){
		std::cout << file << std::endl;
	}
    return 0;
}

