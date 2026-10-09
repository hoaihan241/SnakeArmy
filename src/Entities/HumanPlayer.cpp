#include "HumanPlayer.h"
HumanPlayer::HumanPlayer(float startX, float startY) : Character(startX, startY) {
    shape.setRadius(20.0f); shape.setFillColor(sf::Color::Blue); shape.setOrigin(20.0f, 20.0f);
    army.addTroop(startX - 30, startY, 1); // Test troop
}
void HumanPlayer::update(float dt) {
    bool moved = false;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) { y -= speed * dt; moved = true; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) { y += speed * dt; moved = true; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) { x -= speed * dt; moved = true; }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) { x += speed * dt; moved = true; }
    rx = x; ry = y; shape.setPosition(rx, ry);
    if (moved) army.updatePositions(rx, ry);
}
void HumanPlayer::draw(sf::RenderWindow& window) { window.draw(shape); army.draw(window); }