#pragma once
#include "GameObject.h"
#include "../DataStructures/TroopLinkedList.h"
class Character : public GameObject {
protected:
    float speed;
    sf::CircleShape shape;
    TroopLinkedList army;
public:
    Character(float startX, float startY);
    virtual ~Character() = default;
};