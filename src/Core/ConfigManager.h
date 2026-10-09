#pragma once
#include <string>
// SINGLETON PATTERN Placeholder
class ConfigManager {
private: ConfigManager() {}
public:
    static ConfigManager& getInstance() { static ConfigManager instance; return instance; }
    std::string getText(const std::string& key) { return "Text"; }
};