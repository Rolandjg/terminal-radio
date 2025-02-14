#ifndef PLAYERUTIL_H
#define PLAYERUTIL_H

#include <string>
#include <vector>



class PlayerUtil {
private:
public:
	

	struct Station {
		std::string name;
		std::string shortName;
		std::string tags;
		std::string url;
		std::string homePage;
		std::string country;
		std::string countryCode;
		std::string language;
		int clickCount;
		std::vector<double> geo;
	};  
		
	// Returns a vector containing the file paths of the music 
	// files in ~/Music/subdir
	static std::vector<std::string> getMusicFiles(const std::string &subdir);
	
	// Returns a vector containing the directory paths of the 
	// directories in ~/Music/subdir
	static std::vector<std::string> getMusicDirectories(const std::string &subdir);

	// Returns a vector containing the file paths of the music 
	// files in ~/Music
	static std::vector<std::string> getMusicFiles();
	
	// Returns a vector containing the directory paths of the 
	// directories in ~/Music
	static std::vector<std::string> getMusicDirectories();

	// Returns a list of available servers from the hostname
	static std::vector<std::string> getAvailableServers(const std::string &hostname);

	// Gets stations from the server url, specify the arguments in extra
	static std::string fetchDataFromServer(const std::string &serverUrl, const std::string &extra);
	
	// Gets the hostname of the ip addresses
	static std::vector<std::string> getReverseDNS(const std::vector<std::string> &ipAddresses);

	// Return the useful information of all the available streams 
	static std::vector<Station> getStreamInfo(const std::string &json);
};

#endif
