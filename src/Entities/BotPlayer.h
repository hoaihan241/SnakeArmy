#pragma once
#include "Character.h"
class BotPlayer : public Character {
private:
    int state; // 0: Scout, 1: Flee, 2: Bank
public:
    BotPlayer(float startX, float startY);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};