#include "ShieldItem.h"
ShieldItem::ShieldItem(float startX, float startY) : Item(startX, startY) {
    shape.setRadius(8.0f); shape.setFillColor(sf::Color::Magenta); shape.setPosition(x, y);
}
void ShieldItem::update(float dt) {}
void ShieldItem::draw(sf::RenderWindow& window) { window.draw(shape); }
void ShieldItem::applyBuff(Character* target) { /* Buff khien target */ }