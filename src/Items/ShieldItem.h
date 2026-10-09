#pragma once
#include "Item.h"
class ShieldItem : public Item {
private: sf::CircleShape shape;
public:
    ShieldItem(float startX, float startY);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void applyBuff(Character* target) override;
};