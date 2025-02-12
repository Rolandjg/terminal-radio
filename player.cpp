#include "player.h"
#include <iostream>

Player::Player() {
    mpv = mpv_create();
    if (!mpv) {
        std::cerr << "Failed to create MPV context." << std::endl;
        return;
    }

    if (mpv_initialize(mpv) < 0) {
        std::cerr << "Failed to initialize MPV." << std::endl;
        return;
    }

    stream = "";
    paused = true;
}

Player::~Player() {
    if (mpv) {
        mpv_terminate_destroy(mpv);
    }
}

Player::Player(const Player& other) {
    mpv = mpv_create();
    if (!mpv) {
        std::cerr << "Failed to create MPV context." << std::endl;
        return;
    }

    if (mpv_initialize(mpv) < 0) {
        std::cerr << "Failed to initialize MPV." << std::endl;
        return;
    }

    stream = other.stream;
    paused = other.paused;
}

Player& Player::operator=(const Player& other) {
    if (this == &other) {
        return *this;
    }

    if (mpv) {
        mpv_terminate_destroy(mpv);
    }

    mpv = mpv_create();
    if (!mpv) {
        std::cerr << "Failed to create MPV context." << std::endl;
    }

    if (mpv_initialize(mpv) < 0) {
        std::cerr << "Failed to initialize MPV." << std::endl;
    }

    stream = other.stream;
    paused = other.paused;

    return *this;
}

void Player::play() {
    if (stream.empty()) {
        std::cerr << "No stream selected." << std::endl;
        return;
    }

    if (!paused) {
        return;  // Already playing
    }

    const char *mpv_cmd[] = {"loadfile", stream.c_str(), nullptr};

    if (mpv_command(mpv, mpv_cmd) < 0) {
        std::cerr << "Failed to play stream." << std::endl;
        return;
    }

    paused = false;
    std::cout << "Playing: " << stream << std::endl;
}

void Player::pause() {
    if (!mpv) {
        std::cerr << "MPV is not initialized." << std::endl;
        return;
    }

    int64_t is_paused;
    if (mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &is_paused) < 0) {
        std::cerr << "Failed to get pause state." << std::endl;
        return;
    }

    is_paused = !is_paused;
    if (mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &is_paused) < 0) {
        std::cerr << "Failed to toggle pause." << std::endl;
    } else {
        paused = is_paused;
    }
}

void Player::setStation(const std::string& newStream) {
    if (stream == newStream) {
        std::cout << "Stream is already set to: " << stream << std::endl;
        return;
    }

    stream = newStream;

    // Stop current playback if necessary
    if (!paused) {
        const char *stopCmd[] = {"stop", nullptr};
        if (mpv_command(mpv, stopCmd) < 0) {
            std::cerr << "Failed to stop current playback." << std::endl;
        }
    }

    play();
}

