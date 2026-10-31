#pragma once
#include "../Entities/GameObject.h"

// Forward declaration để tránh circular include
class Character;

// ================================================================
//  Item – LỚP TRỪU TƯỢNG VẬT PHẨM (Polymorphism)
//  Hàm ảo thuần túy applyBuff() → loại bỏ hoàn toàn if-else
//  Thành viên 5
// ================================================================
class Item : public GameObject {
protected:
    sf::Sprite sprite;
    int animFrame;
    float animTimer;
    int frameWidth;   // 32 px / frame
    bool collected;   // Đã bị nhặt chưa

public:
    Item(float startX, float startY, const std::string& sheetName);
    virtual ~Item() = default;

    // Hàm ảo thuần túy — mỗi item implement riêng
    virtual void applyBuff(Character* target) = 0;

    bool isCollected() const { return collected; }
    void markCollected() { collected = true; }

    // Update animation lấp lánh
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
};