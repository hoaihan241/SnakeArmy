#include "ResourceManager.h"
sf::Texture& ResourceManager::getTexture(const std::string& name) {
    if (textures.find(name) == textures.end()) {
        sf::Texture tex;
        if(tex.loadFromFile("assets/sprites/" + name)) textures[name] = tex;
    }
    return textures[name];
}