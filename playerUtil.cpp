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
#include "include/json.hpp"

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

std::string PlayerUtil::fetchDataFromServer(const std::string &serverUrl, const std::string &extra) {
    CURL *curl;
    CURLcode res;
    std::string responseData;

    curl = curl_easy_init();
    if (curl) {
		std::string limittedRequest;
		
		limittedRequest = serverUrl + extra;

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

    try {
        nlohmann::json jsonData = nlohmann::json::parse(json);

        for (const auto& obj : jsonData) { 
            PlayerUtil::Station station;

            station.name = obj.value("name", "Unknown");
            station.tags = obj.value("tags", "");
            station.url = obj.value("url", "");
            station.homePage = obj.value("homepage", "");
            station.country = obj.value("country", "");
            station.countryCode = obj.value("countrycode", ""); // Fixed key name
            station.language = obj.value("language", ""); // Fixed spelling
            station.clickCount = obj.value("clickcount", 0);

            std::vector<double> geo;
            if (!obj["geo_lat"].is_null() && obj["geo_lat"].is_number()) {
                geo.push_back(obj["geo_lat"].get<double>());
            } else {
                geo.push_back(0.0); // Default value
            }

            if (!obj["geo_long"].is_null() && obj["geo_long"].is_number()) {
                geo.push_back(obj["geo_long"].get<double>());
            } else {
                geo.push_back(0.0); // Default value
            }

            stationList.push_back(station);
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    }

    return stationList;
}

	
