#include "BaseZone.h"
#include "../Core/ResourceManager.h"
#include <cmath>

// ================================================================
//  Constructor – Load sprite căn cứ theo phe
// ================================================================
BaseZone::BaseZone(float startX, float startY, Team t)
    : GameObject(startX, startY), team(t), radius(60.f), totalStoredPower(0)
{
    std::string texName = (t == Team::BLUE) ? "base_blue.png" : "base_red.png";
    sf::Texture& tex = ResourceManager::getInstance().getTexture(texName);
    sprite.setTexture(tex);

    // Scale Base về ~120x120 px hiển thị
    float scale = 120.f / tex.getSize().x;
    sprite.setScale(scale, scale);
    sprite.setOrigin(tex.getSize().x / 2.f, tex.getSize().y / 2.f);
    sprite.setPosition(x, y);
}

void BaseZone::update(float dt) {
    // Base tĩnh — không cần update logic
}

void BaseZone::draw(sf::RenderWindow& window) {
    window.draw(sprite);

    // Vẽ vòng tròn phạm vi kích hoạt (mờ)
    sf::CircleShape zone(radius);
    zone.setOrigin(radius, radius);
    zone.setPosition(x, y);
    zone.setFillColor(sf::Color(255, 255, 255, 30));
    zone.setOutlineColor(
        (team == Team::BLUE) ? sf::Color(100, 150, 255, 100) : sf::Color(255, 120, 100, 100)
    );
    zone.setOutlineThickness(2.f);
    window.draw(zone);
}

sf::FloatRect BaseZone::getBounds() const {
    return sf::FloatRect(x - radius, y - radius, radius * 2.f, radius * 2.f);
}

// ================================================================
//  isInZone – Kiểm tra khoảng cách đến tâm Base
// ================================================================
bool BaseZone::isInZone(float px, float py) const {
    float dx = px - x;
    float dy = py - y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

// ================================================================
//  bankTroops – "Gửi ngân hàng" lính vào Base
//  Chuyển toàn bộ lính từ DLL army sang storedTroops vector
// ================================================================
void BaseZone::bankTroops(TroopLinkedList& army) {
    std::vector<Troop*> troops = army.releaseAll();
    for (Troop* t : troops) {
        totalStoredPower += t->power;
        storedTroops.push_back(t);
    }
}
