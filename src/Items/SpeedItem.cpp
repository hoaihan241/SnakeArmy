#include "SpeedItem.h"
SpeedItem::SpeedItem(float startX, float startY) : Item(startX, startY) {
    shape.setRadius(8.0f); shape.setFillColor(sf::Color::Cyan); shape.setPosition(x, y);
}
void SpeedItem::update(float dt) {}
void SpeedItem::draw(sf::RenderWindow& window) { window.draw(shape); }
void SpeedItem::applyBuff(Character* target) { /* Tang toc target */ }