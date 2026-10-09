#pragma once
#include "Character.h"
class HumanPlayer : public Character {
public:
    HumanPlayer(float startX, float startY);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};