#include "player.h"
#include <iostream>
#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_set>
#include "playerUtil.h"

int main() {
	//initscr();
	//noecho();
	//curs_set(0);

	// int selected;

	std::vector<std::string> ips = PlayerUtil::getAvailableServers("all.api.radio-browser.info");

	for(const auto& ip: ips) {
		std::cout << ip << std::endl;
	}

	std::vector<std::string> servers = PlayerUtil::getReverseDNS(ips);

	if (!servers.empty()) {
        std::string apiUrl = "https://" + servers.front() + "/json/stations";
        std::cout << "\nFetching data from: " << apiUrl << std::endl;
        std::string response = PlayerUtil::fetchDataFromServer(apiUrl, 0);
	
		std::cout << "print response " << std::endl;
		std::cout << response << std::endl;
	
		std::cout << "formatting" << std::endl;
		std::vector<PlayerUtil::Station> stations = PlayerUtil::getStreamInfo(response);
	
	
		for (const auto& station : stations){
			std::cout << station.name << std::endl;
		}
    }

	while(true){
		
	}

    return 0;
}

