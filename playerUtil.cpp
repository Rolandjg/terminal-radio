#include "playerUtil.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <curl/curl.h>
#include <iostream>
#include <cstring>
#include "json.hpp"

const char* home = std::getenv("HOME");
std::string home_string = home;

// Define a set of valid extensions
std::unordered_set<std::string> audioExtensions = {".mp3", ".flac", ".wav", ".aac", ".ogg", ".m4a"}; 

/*
 * Call back function for curl to capture http response
 */
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
    size_t totalSize = size * nmemb;
    userp->append((char *)contents, totalSize);
    return totalSize;
}

/*
 * Helper method for getting the directories in a directory.
 */
std::vector<std::string> getDirectoriesInDirectory(std::string path){
	std::vector<std::string> dirs; 

	for(const auto& entry : std::filesystem::directory_iterator(path)) {
		std::string ext = entry.path().extension().string(); // Get file extension as string

		if(ext == ""){ // Check if there is no extension
			dirs.push_back(entry.path().string()); // Add file to the list
		}
	}
	return dirs;
}

/*
 * Helper method for getting the files in a directory
 */
std::vector<std::string> getFilesInDirectory(std::string path, std::unordered_set<std::string> allowedExtensions){
	std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        std::string ext = entry.path().extension().string(); // Get file extension as string

        if (allowedExtensions.find(ext) != allowedExtensions.end()) {  // Check if it's in the set
            files.push_back(entry.path().string());  // Add file to list
        }
    }
	return files;
}

std::vector<std::string> PlayerUtil::getMusicFiles(const std::string &subdir) {
	std::string path = home_string + "/Music" + "/" + subdir;

	return getFilesInDirectory(path, audioExtensions);
}

std::vector<std::string> PlayerUtil::getMusicFiles() {
	std::string path = home_string + "/Music";

	return getFilesInDirectory(path, audioExtensions);
}

std::vector<std::string> PlayerUtil::getMusicDirectories(const std::string &subdir) {
	std::string path = home_string + "/Music" + "/" + subdir;

	return getDirectoriesInDirectory(path);
}

std::vector<std::string> PlayerUtil::getMusicDirectories() {
	std::string path = home_string + "/Music";

	return getDirectoriesInDirectory(path);
}

std::vector<std::string> PlayerUtil::getAvailableServers(const std::string &hostname) {

	std::vector<std::string> servers;
    struct addrinfo hints{}, *res, *p;
    char ipStr[INET6_ADDRSTRLEN];

    hints.ai_family = AF_UNSPEC; // Allow both IPv4 and IPv6
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(hostname.c_str(), nullptr, &hints, &res) != 0) {
        perror("getaddrinfo");
        return servers;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) { // IPv4
            addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
        } else { // IPv6
            addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
        }
        inet_ntop(p->ai_family, addr, ipStr, sizeof(ipStr));
        servers.push_back(ipStr);
    }

    freeaddrinfo(res);
    return servers;
}	

std::string PlayerUtil::fetchDataFromServer(const std::string &serverUrl, int amount) {
    CURL *curl;
    CURLcode res;
    std::string responseData;

    curl = curl_easy_init();
    if (curl) {
		std::string limittedRequest;
		
		// If the search amount is zero, search all
		if(amount != 0) { 
			limittedRequest = serverUrl + "/json/stations?limit=" + std::to_string(amount);
		} else { 
			limittedRequest = serverUrl;
		}

        curl_easy_setopt(curl, CURLOPT_URL, limittedRequest.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    
    return responseData;
}

std::string PlayerUtil::fetchDataFromServer(const std::string &serverUrl, int amount, const std::string& sortType) {
    CURL *curl;
    CURLcode res;
    std::string responseData;

    curl = curl_easy_init();
    if (curl) {
		std::string limittedRequest;
		
		// If the search amount is zero, search all
		if(amount != 0) { 
			limittedRequest = serverUrl + "/json/stations?limit=" + std::to_string(amount);
			limittedRequest = serverUrl + "&order=" + sortType;
		} else { 
			limittedRequest = serverUrl + "/json/stations?order=" + sortType;
		}

        curl_easy_setopt(curl, CURLOPT_URL, limittedRequest.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }
    
    return responseData;
}

std::vector<std::string> PlayerUtil::getReverseDNS(const std::vector<std::string> &ipAddresses) {
	std::vector<std::string> hostnames;
    struct sockaddr_in sa;
    struct sockaddr_in6 sa6;
    char host[NI_MAXHOST];

    for (const auto &ip : ipAddresses) {
        memset(&sa, 0, sizeof(sa));
        memset(&sa6, 0, sizeof(sa6));

        if (inet_pton(AF_INET, ip.c_str(), &sa.sin_addr)) {
            sa.sin_family = AF_INET;
            if (getnameinfo((struct sockaddr *)&sa, sizeof(sa), host, sizeof(host), nullptr, 0, NI_NAMEREQD) == 0) {
                hostnames.push_back(host);
            } else {
                hostnames.push_back(ip);
            }
        } else if (inet_pton(AF_INET6, ip.c_str(), &sa6.sin6_addr)) {
            sa6.sin6_family = AF_INET6;
            if (getnameinfo((struct sockaddr *)&sa6, sizeof(sa6), host, sizeof(host), nullptr, 0, NI_NAMEREQD) == 0) {
                hostnames.push_back(host);
            } else {
                hostnames.push_back(ip);
            }
        }
    }

    return hostnames;
}

std::vector<PlayerUtil::Station> PlayerUtil::getStreamInfo(const std::string &json) {
	std::vector<PlayerUtil::Station> stationList;

	nlohmann::json jsonData = nlohmann::json::parse(json);
	
	std::cout << "showing all info" << std::endl;
	for(auto& [key, value] : jsonData.items()){
		std::cout << "Key: " << key << ", Value: " << value << std::endl;
	}

	std::cout << "formatting struct" << std::endl;
	for(const auto& obj : jsonData) { 
		PlayerUtil::Station station;

		station.name = obj["name"];
		station.tags = obj["tags"];
		station.url = obj["url"];
		station.homePage = obj["homepage"];
		station.country = obj["country"];
		station.countryCode = obj["countryCode"];
		station.language = obj["langauge"];
		station.clickCount = obj["clickcount"];

		std::vector<double> geo;
		geo.push_back(obj["geo_lat"]);
		geo.push_back(obj["geo_long"]);
		stationList.push_back(station);
	}
	return stationList;
}
	
