#include "Obstacle.h"
Obstacle::Obstacle(float startX, float startY, float w, float h) : GameObject(startX, startY) {
    shape.setSize(sf::Vector2f(w, h)); shape.setFillColor(sf::Color(100, 100, 100)); shape.setPosition(x, y);
}
void Obstacle::update(float dt) {}
void Obstacle::draw(sf::RenderWindow& window) { window.draw(shape); }