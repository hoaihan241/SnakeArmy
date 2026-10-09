#pragma once
#include "GameObject.h"
class Obstacle : public GameObject {
private: sf::RectangleShape shape;
public:
    Obstacle(float startX, float startY, float w, float h);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
};