#ifndef PLAYERUTIL_H
#define PLAYERUTIL_H

#include <string>
#include <vector>

class PlayerUtil {
private:
public:
	// Returns a vector containing the file paths of the music 
	// files in ~/Music/subdir
	static std::vector<std::string> getMusicFiles(std::string subdir);
	
	// Returns a vector containing the directory paths of the 
	// directories in ~/Music/subdir
	static std::vector<std::string> getMusicDirectories(std::string subdir);

	// Returns a vector containing the file paths of the music 
	// files in ~/Music
	static std::vector<std::string> getMusicFiles();
	
	// Returns a vector containing the directory paths of the 
	// directories in ~/Music
	static std::vector<std::string> getMusicDirectories();
};

#endif
