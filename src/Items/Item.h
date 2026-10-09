#pragma once
#include "../Entities/GameObject.h"
#include "../Entities/Character.h"
class Item : public GameObject {
public:
    Item(float startX, float startY) : GameObject(startX, startY) {}
    virtual void applyBuff(Character* target) = 0;
};