#include "Obstacle.h"
#include "../Core/ResourceManager.h"
#include <cmath>

Obstacle::Obstacle(float startX, float startY, float w, float h, const std::string& textureName)
    : GameObject(startX, startY), width(w), height(h)
{
    sf::Texture& tex = ResourceManager::getInstance().getTexture(textureName);
    sprite.setTexture(tex);
    // Scale sprite theo kích thước mong muốn
    sprite.setScale(w / tex.getSize().x, h / tex.getSize().y);
    sprite.setPosition(x, y);
}

void Obstacle::update(float dt) {
    // Vật cản tĩnh — không cần update
}

void Obstacle::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

sf::FloatRect Obstacle::getBounds() const {
    return sf::FloatRect(x, y, width, height);
}

sf::FloatRect Obstacle::getCollisionBounds() const {
    // Đá: Khối va chạm cứng chiếm khoảng 80% kích thước trung tâm
    float insetX = width * 0.1f;
    float insetY = height * 0.1f;
    return sf::FloatRect(x + insetX, y + insetY, width - 2.f * insetX, height - 2.f * insetY);
}

// ================================================================
//  Tree – Cây cối kế thừa từ Obstacle
// ================================================================
Tree::Tree(float startX, float startY, int var, float size)
    : Obstacle(startX, startY, size, size, (var == 2) ? "tree2.png" : "tree1.png"),
      variant(var)
{
}

sf::FloatRect Tree::getCollisionBounds() const {
    // Cây: Hitbox va chạm chỉ nằm ở phần GỐC THÂN CÂY (chiếm khoảng 35% rộng và 30% cao ở đáy)
    float boxW = width * 0.35f;
    float boxH = height * 0.30f;
    float boxX = x + (width - boxW) / 2.f;
    float boxY = y + height * 0.65f;
    return sf::FloatRect(boxX, boxY, boxW, boxH);
}

// ================================================================
//  RuinsPillar – Cột đá La Mã gãy đổ (ruins_pillar.png)
// ================================================================
RuinsPillar::RuinsPillar(float startX, float startY, float size)
    : Obstacle(startX, startY, size, size, "ruins_pillar.png")
{
}

sf::FloatRect RuinsPillar::getCollisionBounds() const {
    // Hitbox va chạm ở phần chân đế đá vỡ ngổn ngang dưới đáy cột
    float boxW = width * 0.60f;
    float boxH = height * 0.38f;
    float boxX = x + (width - boxW) / 2.f;
    float boxY = y + height * 0.58f;
    return sf::FloatRect(boxX, boxY, boxW, boxH);
}

// ================================================================
//  RuinsWall – Đoạn tường thành nứt toác có vết móng vuốt (ruins_wall.png)
// ================================================================
RuinsWall::RuinsWall(float startX, float startY, float w, float h)
    : Obstacle(startX, startY, w, h, "ruins_wall.png")
{
}

sf::FloatRect RuinsWall::getCollisionBounds() const {
    // Hitbox va chạm chạy dọc theo móng chân tường đá
    float boxW = width * 0.82f;
    float boxH = height * 0.42f;
    float boxX = x + (width - boxW) / 2.f;
    float boxY = y + height * 0.52f;
    return sf::FloatRect(boxX, boxY, boxW, boxH);
}

// ================================================================
//  Bush – Bụi cỏ ẩn nấp (bush.png)
// ================================================================
Bush::Bush(float startX, float startY, float size)
    : Obstacle(startX, startY, size, size, "bush.png")
{
}

sf::FloatRect Bush::getCollisionBounds() const {
    // Bụi rậm cho phép đi xuyên qua, không cản trở chuyển động
    return sf::FloatRect(0.f, 0.f, 0.f, 0.f);
}

bool Bush::isInside(float px, float py) const {
    // Kiểm tra xem điểm (px, py) có nằm trong vùng bụi rậm không
    float centerX = x + width / 2.f;
    float centerY = y + height / 2.f;
    float dx = px - centerX;
    float dy = py - centerY;
    float r = width * 0.45f;
    return (dx * dx + dy * dy) <= (r * r);
}