#include "Troop.h"
Troop::Troop(float startX, float startY, int p) : x(startX), y(startY), power(p), next(nullptr), prev(nullptr) {
    shape.setRadius(10.0f);
    shape.setFillColor(sf::Color::Yellow);
    shape.setOrigin(10.0f, 10.0f);
    shape.setPosition(x, y);
}