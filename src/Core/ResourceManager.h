#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
// SINGLETON PATTERN
class ResourceManager {
private:
    std::map<std::string, sf::Texture> textures;
    ResourceManager() {} // Private constructor
public:
    static ResourceManager& getInstance() { static ResourceManager instance; return instance; }
    sf::Texture& getTexture(const std::string& name);
};