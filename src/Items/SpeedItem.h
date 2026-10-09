#pragma once
#include "Item.h"
class SpeedItem : public Item {
private: sf::CircleShape shape;
public:
    SpeedItem(float startX, float startY);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void applyBuff(Character* target) override;
};