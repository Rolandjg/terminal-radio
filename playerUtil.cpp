#include "playerUtil.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>

const char* home = std::getenv("HOME");
std::string home_string = home;
// Define a set of valid extensions
std::unordered_set<std::string> audioExtensions = {".mp3", ".flac", ".wav", ".aac", ".ogg", ".m4a"}; 

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

std::vector<std::string> PlayerUtil::getMusicFiles(std::string subdir) {
	std::string path = home_string + "/Music" + "/" + subdir;

	return getFilesInDirectory(path, audioExtensions);
}

std::vector<std::string> PlayerUtil::getMusicFiles() {
	std::string path = home_string + "/Music";

	return getFilesInDirectory(path, audioExtensions);
}

std::vector<std::string> PlayerUtil::getMusicDirectories(std::string subdir) {
	std::string path = home_string + "/Music" + "/" + subdir;

	return getDirectoriesInDirectory(path);
}

std::vector<std::string> PlayerUtil::getMusicDirectories() {
	std::string path = home_string + "/Music";

	return getDirectoriesInDirectory(path);
}
