#pragma once
#include <SFML/Graphics.hpp>

// ================================================================
//  GameObject – LỚP CƠ SỞ TRỪU TƯỢNG TỐI CAO
//  Mọi thực thể trên bản đồ đều kế thừa từ đây.
//  Thành viên 1: Kiến trúc gốc
// ================================================================
class GameObject {
protected:
    float x, y;   // Tọa độ logic
public:
    GameObject(float startX, float startY) : x(startX), y(startY) {}
    virtual ~GameObject() = default;

    // Hàm ảo thuần túy — bắt buộc lớp con phải implement
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;

    // Getter tọa độ
    float getX() const { return x; }
    float getY() const { return y; }
    void setPosition(float nx, float ny) { x = nx; y = ny; }

    // Hitbox cơ bản (lớp con override nếu cần)
    virtual sf::FloatRect getBounds() const {
        return sf::FloatRect(x - 16, y - 16, 32, 32);
    }
};