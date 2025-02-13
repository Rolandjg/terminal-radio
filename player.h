#ifndef PLAYER_H
#define PLAYER_H

#include <mpv/client.h>
#include <string>

class Player{
private:
    std::string stream;
    mpv_handle *mpv;
    bool paused;
public:
    Player();
    ~Player();
    
    // Copy constructor
    Player(const Player& other);
    
    Player& operator=(const Player& other);
    
    // Play the station 
    void play();
    
    // Pause the station
    void pause();

    // Sets the station
    void setStation(const std::string& newStream);

	// True if the player is playing, false otherwise
	bool isPlaying();
};    

#endif

