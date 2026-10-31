#include "Item.h"
#include "../Core/ResourceManager.h"

// ================================================================
//  Constructor – Load sprite sheet, cắt frame đầu tiên
// ================================================================
Item::Item(float startX, float startY, const std::string& sheetName)
    : GameObject(startX, startY),
      animFrame(0), animTimer(0.f), frameWidth(32), collected(false)
{
    sf::Texture& tex = ResourceManager::getInstance().getTexture(sheetName);
    sprite.setTexture(tex);
    sprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameWidth));
    sprite.setOrigin(frameWidth / 2.f, frameWidth / 2.f);
    sprite.setPosition(x, y);
}

// ================================================================
//  update – Animation lấp lánh (4 frame loop)
// ================================================================
void Item::update(float dt) {
    if (collected) return;

    animTimer += dt;
    if (animTimer >= 0.2f) {
        animTimer = 0.f;
        animFrame = (animFrame + 1) % 4;
        sprite.setTextureRect(sf::IntRect(animFrame * frameWidth, 0, frameWidth, frameWidth));
    }
}

// ================================================================
//  draw
// ================================================================
void Item::draw(sf::RenderWindow& window) {
    if (!collected) {
        window.draw(sprite);
    }
}

// ================================================================
//  getBounds – Hitbox 32x32
// ================================================================
sf::FloatRect Item::getBounds() const {
    return sf::FloatRect(x - frameWidth / 2.f, y - frameWidth / 2.f,
                         (float)frameWidth, (float)frameWidth);
}