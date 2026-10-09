#include "BotPlayer.h"
BotPlayer::BotPlayer(float startX, float startY) : Character(startX, startY), state(0) {
    shape.setRadius(20.0f); shape.setFillColor(sf::Color::Red); shape.setOrigin(20.0f, 20.0f);
}
void BotPlayer::update(float dt) {
    // AI State Machine Basic Logic
    y += (speed * 0.5f) * dt; // Bot di chuyen cham test
    rx = x; ry = y; shape.setPosition(rx, ry);
    army.updatePositions(rx, ry);
}
void BotPlayer::draw(sf::RenderWindow& window) { window.draw(shape); army.draw(window); }