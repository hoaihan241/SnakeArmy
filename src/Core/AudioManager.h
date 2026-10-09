#pragma once
#include <string>
// SINGLETON PATTERN Placeholder
class AudioManager {
private: AudioManager() {}
public:
    static AudioManager& getInstance() { static AudioManager instance; return instance; }
    void playSound(const std::string& name) {}
};