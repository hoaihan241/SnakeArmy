#pragma once
#include <SFML/Graphics.hpp>
struct Troop {
    float x, y;
    int power;
    Troop* next;
    Troop* prev;
    sf::CircleShape shape;
    Troop(float startX, float startY, int p);
};