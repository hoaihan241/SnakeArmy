#pragma once
#include <SFML/Graphics.hpp>
class GameObject {
protected:
    float x, y, rx, ry;
public:
    GameObject(float startX, float startY) : x(startX), y(startY), rx(startX), ry(startY) {}
    virtual ~GameObject() = default;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    float getX() const { return x; }
    float getY() const { return y; }
};