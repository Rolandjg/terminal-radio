#include "player.h"
#include <iostream>
#include <ncurses.h>
#include <filesystem>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unordered_set>
#include "playerUtil.h"
#include <unistd.h>

int rows, cols;

int amount = 50;
std::string* sortOrder = new std::string("clickcount");;
std::string* country = nullptr;
std::string* tags = nullptr; 
std::string* language = new std::string("english");
bool reverse = true;	

std::string inputBuffer;

/*
 * gets a list of stations, pass in args for filtering.
 */
std::vector<PlayerUtil::Station> getStations(const std::string &args, int shortWordLength){
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
	stations = PlayerUtil::getStreamInfo(response, shortWordLength);	

	if(stations.empty()) {
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

	return message;
}

/*
 * Makes a vector containig a list of only the stations that will be drawn
 */
std::vector<PlayerUtil::Station> constructDisplayedStations(const std::vector<PlayerUtil::Station> &stations, int page) {
    std::vector<PlayerUtil::Station> displayedStations;
    
	if (stations.empty())
		return displayedStations;

    int startIdx = rows * page;

    int endIdx = std::min(startIdx + rows, (int)stations.size());

    for (int i = startIdx; i < endIdx; i++) {
       displayedStations.push_back(stations[i]);
    }
    return displayedStations;
}

std::string urlEncode(const std::string& str) {
    std::ostringstream encoded;
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::uppercase << std::hex << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return encoded.str();
}

/*
 * Reads string input
 */
void typeString(std::string &inputBuffer) {
    int ch;
    inputBuffer.clear();
    move(0, 0);  // Move cursor to top left
    clrtoeol();  // Clear line
    printw("Enter value: ");
    refresh();
    echo();      // Show typed characters
    curs_set(1); // Show cursor
    
    while ((ch = getch()) != '\n') {  // Stop when Enter is pressed
        if (ch == KEY_BACKSPACE || ch == 127) {  // Handle backspace
            if (!inputBuffer.empty()) {
                inputBuffer.pop_back();
                move(0, 13 + inputBuffer.length());
                printw(" ");
                move(0, 13 + inputBuffer.length());
                refresh();
            }
        } else if (isprint(ch)) {  // Accept only printable characters
            inputBuffer += ch;
        }
    }
    
    noecho();    // Turn off echo
    curs_set(0); // Hide cursor again
}

/*
 * Draws the header to the top of the page
 */
void drawHeader(std::string message) {
	
	attron(A_REVERSE); // Highlight
	// Clear
	for (int i = 0; i < cols; i++) 
		mvprintw(0, i, "%s", " ");
	mvprintw(0, 0, "%s", message.c_str());
	attroff(A_REVERSE); // Disable highlight
}

/*
 * Builds a valid argument string for the API request
 */
std::string constructArgs(int limit, int offset, const std::string* order, const std::string* tags, bool reverse, const std::string* country, const std::string* language) {
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
	if (language != nullptr)
		args += "&language=" + *language;  // Fixed: was missing "=" sign
	if (reverse)
		args += "&reverse=true";

	return args;
}

/*
 * Draws the station info the the top right corner of the screen.
*/
void drawInfo(PlayerUtil::Station station, WINDOW *win) {
	box(win, 0, 0);
	
	wattron(win, A_REVERSE); // Highlight

	if((int)station.name.size() > 100)
		mvwprintw(win, 2, 1, "%s", station.name.substr(0,100).c_str());	
	else 
		mvwprintw(win, 2, 1, "%s", station.name.c_str());	

	wattroff(win, A_REVERSE); // Disable highlight

	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, 4, 1, "%s", ("tags - " + station.tags).c_str());	
	wattroff(win, COLOR_PAIR(2));

	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, 8, 1, "%s", ("Country - " + station.country).c_str());	
	wattroff(win, COLOR_PAIR(3));

	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, 9, 1, "%s", ("Language - " + station.language).c_str());	
	wattroff(win, COLOR_PAIR(2));

	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, 10, 1, "%s", ("URL - " + station.url).c_str());	
	wattroff(win, COLOR_PAIR(3));

	wattron(win, COLOR_PAIR(2));
	mvwprintw(win, 12, 1, "%s", ("Website - " + station.homePage).c_str());	
	wattroff(win, COLOR_PAIR(2));

	wattron(win, COLOR_PAIR(3));
	mvwprintw(win, 13, 1, "%s", ("Click Count - " + std::to_string(station.clickCount)).c_str());	
	wattroff(win, COLOR_PAIR(3));

	if ((int)station.geo.size() == 2)
		mvwprintw(win, 14, 1, "%s", ("Location - " + std::to_string(station.geo[0]) + ", " + std::to_string(station.geo[1])).c_str());
}

void drawFilters(WINDOW *win, int selected, int chosenFilter, int& selectedFilterSetting) {
	box(win, 0, 0); 
				
	std::vector<const char*> filters = {"AMOUNT", "ORDER", "LANGUAGE", "TAGS", "COUNTRY", "REVERSED"};
	std::vector<const char*> orders = {"name", "url", "homepage", 
									   "favicon", "tag", "country", 
									   "state", "language", "votes",
									   "clickcount", "bitrate", "lastcheckok", 
									   "lastchecktime", "codec", "random" };
	int offset = 2;
	int chosenOffset = -1;
	int col;

	for (int i = 0; i < (int)filters.size(); i++) {
		if(selected == i) {
			col = 7;
			chosenOffset = offset;
		} else {
			col = i%2 + 5;
		}
		
		wattron(win, COLOR_PAIR(col));	
		mvwprintw(win, 1, offset, "%s", filters[i]); 
		wattroff(win, COLOR_PAIR(col));
		offset += strlen(filters[i]) + 1; // Add space between filters
	}

	// Sort order
	if (selected == 1) {
		if (selectedFilterSetting > (int)orders.size() - 1)
			selectedFilterSetting = 0;
		if (selectedFilterSetting < 0)
			selectedFilterSetting = orders.size() - 1;

		for (int i = 0; i < (int)orders.size(); i++) {
			if (i == selectedFilterSetting)
				col = 7;
			else
				col = 1; 
			
			wattron(win, COLOR_PAIR(col));	
			mvwprintw(win, 3+i, chosenOffset+1, "%s", orders[i]);
			wattroff(win, COLOR_PAIR(col));
		}
	}

	// Current values for each filter
	if (selected > -1) {
		int displayRow = 2;
		wattron(win, COLOR_PAIR(2));
		//mvwprintw(win, displayRow, chosenOffset, "Current value: ");
		
		switch(selected) {
			case 0: // AMOUNT
				mvwprintw(win, displayRow, chosenOffset, "%d", amount);
				break;
			case 1: // ORDER
				mvwprintw(win, displayRow, chosenOffset, "%s", sortOrder->c_str());
				break;
			case 2: // LANGUAGE
				mvwprintw(win, displayRow, chosenOffset, "%s", language ? language->c_str() : "none");
				break;
			case 3: // TAGS
				mvwprintw(win, displayRow, chosenOffset, "%s", tags ? tags->c_str() : "none");
				break;
			case 4: // COUNTRY
				mvwprintw(win, displayRow, chosenOffset, "%s", country ? country->c_str() : "none");
				break;
			case 5: // REVERSED
				if (reverse)
					mvwprintw(win, displayRow, chosenOffset, "%s", "true");
				else
					mvwprintw(win, displayRow, chosenOffset, "%s", "false");
				break;
		}
		wattroff(win, COLOR_PAIR(2));
	}
}

void drawStations(WINDOW *win, std::vector<PlayerUtil::Station> displayedStations, int page, int selected) {
	box(win, 0, 0);
	std::string spaces; 

	for(int i = 0; i < cols/2; i++) {
		spaces +=  " ";
	}
	// Draw stations
	for (int i = 0; i < (int)displayedStations.size(); i++) {
	    int globalIndex = page * rows + i; 
	
	    if (globalIndex == selected)
	        wattron(win, A_REVERSE); // Highlight
		else
			wattron(win, COLOR_PAIR(1));
	
		//clear station area
	    mvwprintw(win, i + 1, 0, "%s", spaces.c_str());

	    mvwprintw(win, i + 1, 0, "%s", (" > " + displayedStations[i].shortName).c_str());
	
	    if (globalIndex == selected)
	        wattroff(win, A_REVERSE); // Disable highlight
		else
			wattroff(win, COLOR_PAIR(1)); 
	}
}

bool isInt(std::string str) {
	for (char c : str) {
		if (c < '0' || c > '9')
			return false;
	}
	return !str.empty();	// Added check to avoid empty strings
}

bool assignSetting(int settingIndex, std::string input, bool& shouldRefreshStations) { 
	if (input.empty()) {
		return false;
	}
	
	shouldRefreshStations = true;
	
	switch (settingIndex) {
		case 0: // AMOUNT
			if (isInt(input)) {
				int newAmount = std::stoi(input);
				if (newAmount > 0) {
					amount = newAmount;
					return true;
				}
			}
			break;
		case 1: // ORDER
			if (sortOrder == nullptr) {
				sortOrder = new std::string(input);
			} else {
				*sortOrder = input;
			}
			return true;
		case 2: // LANGUAGE
			if (language == nullptr) {
				language = new std::string(input);
			} else {
				*language = input;
			}
			return true;
		case 3: // TAGS
			if (tags == nullptr) {
				tags = new std::string(input);
			} else {
				*tags = input;
			}
			return true;
		case 4: // COUNTRY
			if (country == nullptr) {
				country = new std::string(urlEncode(input));
			} else {
				*country = urlEncode(input);
			}
			return true;
		case 5: // REVERSED
			if (input == "true" || input == "1" || input == "yes") {
				reverse = true;
				return true;
			} else if (input == "false" || input == "0" || input == "no") {
				reverse = false;
				return true;
			}
			break;
	}
	
	shouldRefreshStations = false;
	return false;
}
			
int main() {
	initscr();
	noecho();
	cbreak();               // Line buffering disabled
	keypad(stdscr, TRUE);   // We get F1, F2 etc..
	curs_set(0);
	nodelay(stdscr, TRUE);
	start_color();
	use_default_colors();

	getmaxyx(stdscr, rows, cols);

	WINDOW *infoWindow = newwin(rows/2, cols / 2, 1, cols / 2);
	nodelay(infoWindow, TRUE);

	WINDOW *stationsWindow = newwin(rows, cols/2, 1, 0);

	WINDOW *filtersWindow = newwin(rows / 2, cols / 2, rows / 2+1, cols / 2);

	// Define colors
	init_pair(1, COLOR_BLUE, -1);
	init_pair(2, COLOR_GREEN, -1);
	init_pair(3, COLOR_RED, -1);

	init_pair(4, COLOR_WHITE, COLOR_BLUE);
	init_pair(5, COLOR_BLACK, COLOR_GREEN);
	init_pair(6, COLOR_WHITE, COLOR_RED);
	init_pair(7, COLOR_BLACK, COLOR_WHITE);

	// Print at beginning
	mvwprintw(stationsWindow, 0, 0, "%s", "Loading top stations...");
	wrefresh(stationsWindow);

	int selected = 0;
	int page = 0;
	auto stations = getStations(constructArgs(amount, 0, sortOrder, tags, reverse, country, language), cols/2 - 3);
	auto displayedStations = constructDisplayedStations(stations, page);
	std::vector<PlayerUtil::Station> history;	
	std::string header = "Loaded stations. Press 'f' for filters, Enter to play, 'p' to pause, 'q' to quit.";
	
	bool inFiltersWindow = false;
	int selectedFilter = -1;
	int chosenFilter = -1;
	int selectedFilterSetting = -1;
	
	if (stations.empty()) 
		mvprintw(1, 0, "%s", "No stations found.");

	Player mainPlayer;
	char input;
	int volume = 100;
	std::string oldHeader = "";
	bool shouldRefreshStations = false;
	std::vector<const char*> orders = {"name", "url", "homepage", 
									   "favicon", "tag", "country", 
									   "state", "language", "votes",
									   "clickcount", "bitrate", "lastcheckok", 
									   "lastchecktime", "codec", "random" };

	while (true) {
		input = getch();

		if (selected >= (int)stations.size() - 2) {
			std::string args = constructArgs(amount, stations.size(), sortOrder, tags, reverse, country, language);
			auto moreStations = getStations(args, cols/2-3);	
			
			for (int i = 0; i < (int)moreStations.size(); i++) {
				bool duplicate = false;

				for (int j = 0; j < (int)stations.size(); j++) {
					if (stations[j].name == moreStations[i].name) {
						duplicate = true;
					}
				}
				if (!duplicate) 
					stations.push_back(moreStations[i]);
			}
			displayedStations = constructDisplayedStations(stations, page);
		}

		// Input handeling
		if (input == 'q') break; // quit

		if (input == '=') {
			volume += 5;
			if (volume > 100)
				volume = 100;
			mainPlayer.setVolume(volume);
			header = "Set volume to: " + std::to_string(volume);
		}

		if (input == '-') {
			volume -= 5;
			if (volume < 0)
				volume = 0;
			mainPlayer.setVolume(volume);
			header = "Set volume to: " + std::to_string(volume);
		}

		if (input == 'f') { 
			if (!inFiltersWindow) {
				selectedFilter = 0;
				inFiltersWindow = !inFiltersWindow;
				oldHeader = header;
				header = "Editing Filters. Use h/l to switch filters, Enter to edit, j/k to scroll options.";
			} else {
				inFiltersWindow = !inFiltersWindow;
				selectedFilter = -1;
				chosenFilter = -1;
				
				if (oldHeader != "")
					header = oldHeader;
			}
		}
		
		if (input == 'p') {
			if (mainPlayer.isPlaying()){
				mainPlayer.pause();
				oldHeader = header;
				header = " Paused";
			} else {
				mainPlayer.play();
				header = oldHeader;
			}
		} 
	
		// Up and down
		if (input == 'j') {
			if (!inFiltersWindow) {
				if (selected < (int)stations.size() - 1) {
    				selected++;
    				if (selected >= (page + 1) * rows - 1) {  // Move to next page
        				page++;
			    	    displayedStations = constructDisplayedStations(stations, page);
    				}
				}
			} else {
				selectedFilterSetting++;	
			}
		}

		if (input == 'k') {
			if (!inFiltersWindow) {	
				if (selected > 0) {
			    	selected--;
			    	if (selected < page * rows) {  // Fixed: was using page * rows - 1
			    	    page--;
			    	    displayedStations = constructDisplayedStations(stations, page);
			    	}
				}
			} else { 
				selectedFilterSetting--;
			}
		}
		
		// Left and right
		if(input == 'h' && inFiltersWindow) {
			selectedFilter--;
			if (selectedFilter < 0)
				selectedFilter = 5;
		}

		if(input == 'l' && inFiltersWindow) {
			selectedFilter++;
			if (selectedFilter > 5)
				selectedFilter = 0;
		}
			
		
		if (input == '\n') { // Enter key
			if (!inFiltersWindow) {
				if (!stations.empty()) { // Protect against empty station list
					header = playStation(mainPlayer, stations[selected]);
				}
			} else {
				// Special handling for ORDER selection with j/k navigation
				if (selectedFilter == 1 && selectedFilterSetting >= 0 && selectedFilterSetting < (int)orders.size()) {
					// Select from the list of orders
					if (sortOrder == nullptr) {
						sortOrder = new std::string(orders[selectedFilterSetting]);
					} else {
						*sortOrder = orders[selectedFilterSetting];
					}
					shouldRefreshStations = true;
				} else if (selectedFilter == 5) { // REVERSED
					// Toggle the reverse value
					reverse = !reverse;
					shouldRefreshStations = true;
					header = "Reversed: " + std::string(reverse ? "true" : "false");
				} else {
					// For other filters, get input
					nodelay(stdscr, FALSE); // Temporarily disable non-blocking
					typeString(inputBuffer);
					nodelay(stdscr, TRUE);  // Re-enable non-blocking
					
					bool success = assignSetting(selectedFilter, inputBuffer, shouldRefreshStations);
					
					if (success) {
						header = "Filter updated: " + inputBuffer;
					} else {
						header = "Invalid input: " + inputBuffer;
					}
				}
				
				// Refresh stations list if filter was changed
				if (shouldRefreshStations) {
					// Reset page and selected to beginning of list
					page = 0;
					selected = 0;
					stations.clear();
					stations = getStations(constructArgs(amount, 0, sortOrder, tags, reverse, country, language), cols/2 - 3);
					displayedStations = constructDisplayedStations(stations, page);
					shouldRefreshStations = false;
				}
			}
		}
	
		if (input == 'G') {
			page = 0;
			selected = 0;
		    displayedStations = constructDisplayedStations(stations, page);
		}

		werase(stationsWindow);
		drawStations(stationsWindow, displayedStations, page, selected);
		
		// Draw station info
		werase(infoWindow);
		if (!stations.empty()) { // Protect against empty station list
			drawInfo(stations[selected], infoWindow);
		}

		// Draw filters window
		werase(filtersWindow);
		drawFilters(filtersWindow, selectedFilter, chosenFilter, selectedFilterSetting);
		
		wrefresh(stationsWindow);
		wrefresh(infoWindow);
		wrefresh(filtersWindow);
		doupdate();

		drawHeader(header);

		usleep(10000);
	}
	
	delete sortOrder;
	delete country;
	delete tags; 
	delete language;

	endwin();
    return 0;
}
