#pragma once
#include <SFML/Graphics.hpp>

// ================================================================
//  BombHazard – Sự kiện Bom Rơi / Không Kích
//  Phase 1: Cảnh báo vùng đỏ nhấp nháy dồn dập (1.7s)
//  Phase 2: Nổ bùng trực diện → Diệt node trúng bom, văng lính, Stun Tướng
//  Phase 3: Vùng Đất Cháy Âm Ỉ (Scorched Earth: 3.5s) → Giảm tốc Tướng, thiêu rụi lính
//  Thành viên 5
// ================================================================
class BombHazard {
private:
    float x, y;           // Tâm vùng nổ
    float radius;         // Bán kính nổ (mặc định 115px)
    float warningTime;    // Thời gian cảnh báo còn lại (1.7s)
    bool exploded;        // Đã nổ chưa
    bool finished;        // Hoàn thành xong (xóa được)

    // Hiệu ứng nổ
    sf::Sprite explosionSprite;
    int explosionFrame;
    float explosionTimer;

    // Cảnh báo
    sf::Sprite warningSprite;
    float warningPulse;   // Timer nhấp nháy

    // Vùng cháy âm ỉ (Scorched Earth)
    bool isScorching;
    float scorchTimer;
    float fireAnimTimer;

public:
    BombHazard(float cx, float cy, float r = 115.f);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool hasExploded() const { return exploded; }
    bool isFinished() const { return finished; }
    bool isWarning() const { return !exploded && !finished && warningTime > 0.f; }
    bool isReadyToExplode() const { return !exploded && !finished && warningTime <= 0.f; }
    bool isScorched() const { return isScorching && scorchTimer > 0.f; }

    float getX() const { return x; }
    float getY() const { return y; }
    float getRadius() const { return radius; }

    // Kiểm tra 1 điểm có nằm trong vùng nổ trực diện không
    bool isInBlastZone(float px, float py) const;

    // Kiểm tra 1 điểm có nằm trong vùng đất cháy âm ỉ không
    bool isInScorchedZone(float px, float py) const;

    // Đánh dấu đã xử lý nổ xong (gọi sau khi GameEngine xử lý damage)
    void markExploded();
};
