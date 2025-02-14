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

int rows, cols;

/*
 * gets a list of stations, pass in args for filtering.
 */
std::vector<PlayerUtil::Station> getStations(const std::string &args){
	std::vector<std::string> ips = PlayerUtil::getAvailableServers("all.api.radio-browser.info");
    std::vector<PlayerUtil::Station> stations;

	std::vector<std::string> servers = PlayerUtil::getReverseDNS(ips);

	if(servers.empty()) {
		std::cout << "No servers found ):" << std::endl;
		return stations;
	}

	// Get a server from the list
    std::string apiUrl = "https://" + servers.front() + "/json/stations/search";
    std::string response = PlayerUtil::fetchDataFromServer(apiUrl, args);

	// Add stations to a list of stations
	stations = PlayerUtil::getStreamInfo(response);	

	if(stations.empty()) {
		std::cout << "No stations found ): " << std::endl;
		return stations;
	}
	return stations;
}

/*
 * Plays the station
 */
std::string playStation(Player &player, const PlayerUtil::Station &station) {
	player.setStation(station.url);
	player.play();

	std::string message = "Playing " + station.shortName;
	for (int i = (int)message.size(); i < cols; i++) {
		message = message + " ";
	}

	return message;
}

/*
 * Makes a vector containig a list of only the stations that will be drawn
 */
std::vector<PlayerUtil::Station> constructDisplayedStations(const std::vector<PlayerUtil::Station> &stations, int page) {
    std::vector<PlayerUtil::Station> displayedStations;
    
    int startIdx = rows * page;
    int endIdx = std::min(startIdx + rows, (int)stations.size());

    for (int i = startIdx; i < endIdx; i++) {
       displayedStations.push_back(stations[i]);
    }
    return displayedStations;
}

/*
 * Draws the header to the top of the page
 */
void drawHeader(std::string message) {
	attron(A_REVERSE); // Highlight
	mvprintw(0, 0, "%s", message.c_str());
	attroff(A_REVERSE); // Disable highlight
}

/*
 * Builds a calid argument string for the API request
 */
std::string constructArgs(int limit, int offset, const std::string* order, const std::string* tags, bool reverse, const std::string* country) {
	std::string args = "?limit=" + 
					   std::to_string(limit) + 
					   "&offset=" + 
				       std::to_string(offset); 

	if (order != nullptr) 
		args += "&order=" + *order;
	if (tags != nullptr)
		args += "&tag=" + *tags;
	if (country != nullptr)
		args += "&country=" + *country;
	if (reverse)
		args += "&reverse=true";

	return args;
}

void drawInfo(PlayerUtil::Station station, WINDOW *win) {
	box(win, 0, 0);
	
	wattron(win, A_REVERSE); // Highlight

	if((int)station.name.size() > 100)
		mvwprintw(win, 2, 1, "%s", station.name.substr(0,100).c_str());	
	else 
		mvwprintw(win, 2, 1, "%s", station.name.c_str());	

	wattroff(win, A_REVERSE); // Disable highlight

	mvwprintw(win, 4, 1, "%s", station.tags.c_str());	
	mvwprintw(win, 8, 1, "%s", station.country.c_str());	
	mvwprintw(win, 9, 1, "%s", station.language.c_str());	
	mvwprintw(win, 10, 1, "%s", station.url.c_str());	
	mvwprintw(win, 11, 1, "%s", station.homePage.c_str());	
	mvwprintw(win, 12, 1, "%s", std::to_string(station.clickCount).c_str());	
}

int main() {
	initscr();
	noecho();
	curs_set(0);
	nodelay(stdscr, TRUE);

	getmaxyx(stdscr, rows, cols);
	WINDOW *infoWindow = newwin(rows, cols / 2, 0, cols - cols / 2);
	nodelay(infoWindow, TRUE);

	int selected = 0;
	int page = 0;
	auto stations = getStations("?limit=100&order=clickcount&reverse=true");
	auto displayedStations = constructDisplayedStations(stations, page);
	std::string header = "";
	
	if (stations.empty()) 
		mvprintw(0, 0, "%s", "No stations found.");

	Player mainPlayer;
	char input;

	std::string* sortOrder = nullptr;
	std::string* country = nullptr;
	std::string* tags = nullptr; 
	bool reverse = false;

	while (true) {
		// Refresh the entire window 
		input = getch();
		drawHeader(header);

		if (selected >= (int)stations.size()-1) {
			std::string args = constructArgs(100, stations.size()+99, sortOrder, tags, reverse, country);
			auto moreStations = getStations(args);	
			
			for (const auto& station : moreStations) {
				stations.push_back(station);
			}
			displayedStations = constructDisplayedStations(stations, page);
		}

		// Input handeling
		if (input == 'q') break;

		if (input == 'j' && selected < (int)stations.size() - 1) {
    		selected++;
    		if (selected >= (page + 1) * rows - 1) {  // Move to next page
				wclear(stdscr);
        		page++;
		        displayedStations = constructDisplayedStations(stations, page);
    		}
		}

		if (input == 'k' && selected > 0) {
		    selected--;
		    if (selected < page * rows - 1) {  // Move to previous page
				wclear(stdscr);
		        page--;
		        displayedStations = constructDisplayedStations(stations, page);
		    }
		}

		if (input == '\n') { // Enter key
			header = playStation(mainPlayer, stations[selected]);
		}
	
		if (input == 'G') {
			wclear(stdscr);
			page = 0;
			selected = 0;
		    displayedStations = constructDisplayedStations(stations, page);
		}

		// Draw stations
		for (int i = 0; i < (int)displayedStations.size(); i++) {
		    int globalIndex = page * rows + i; 
		
		    if (globalIndex == selected)
		        attron(A_REVERSE); // Highlight
		
		    mvprintw(i + 1, 0, "%s", displayedStations[i].shortName.c_str());
		
		    if (globalIndex == selected)
		        attroff(A_REVERSE); // Disable highlight
		}
		
		// Draw station info
		werase(infoWindow);
		drawInfo(stations[selected], infoWindow);
		wrefresh(infoWindow);

		wrefresh(stdscr);
		usleep(10000);
	}

	endwin();
    return 0;
}

