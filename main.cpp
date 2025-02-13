#include "player.h"
#include <iostream>
#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdlib>
#include <unordered_set>
#include "playerUtil.h"
#include <unistd.h>

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

	if(servers.empty()) {
		std::cout << "No servers found ):" << std::endl;
		return 0;
	}

	// Get a server from the list
    std::string apiUrl = "https://" + servers.front() + "/json/stations/search";
    std::cout << "\nFetching data from: " << apiUrl << std::endl;
    std::string response = PlayerUtil::fetchDataFromServer(apiUrl, "?tag=rock&limit=5&order=clickcount&reverse=true");

	// Add stations to a list of stations
    std::vector<PlayerUtil::Station> stations = PlayerUtil::getStreamInfo(response);

	if(stations.empty()) {
		std::cout << "No stations found ): " << std::endl;
		return 0;
	}

    for (const auto& station : stations){
    	std::cout << station.name << " - " << station.clickCount << " - " << station.tags << std::endl;
    }

	// Play the first station
	Player Player1;
	Player1.setStation(stations[0].url);
	std::cout << "playing station 0" << std::endl;
	Player1.play();

	usleep(5000000);

	std::cout << "swtich to station 1" << std::endl;	
	Player1.setStation(stations[1].url);
	Player1.play();

	while(true){
		
	}

    return 0;
}

