#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// ================================================================
//  DragonFireZone – Vùng Lửa Rồng Thiêu Đốt
//  - Phase Cảnh báo (0.8s): Vòng ma trận lửa đỏ cam nhấp nháy trên nền đất
//  - Phase Rực lửa (4.5s): 4 frame lửa cuồn cuộn (dragon_fire_sheet.png, 128x128)
//    + Làm chậm Tướng 50%
//    + Thiêu rụi và ngắt xích lính thành lính tự do
//  - Phase Tro tàn (0.8s): Vệt than nguội mờ dần
// ================================================================
struct DragonFireZone {
    float x, y;
    float warningTimer;
    float activeTimer;
    float fadeTimer;
    float animTimer;
    int animFrame;
    bool erupted;

    DragonFireZone(float cx, float cy);
    void update(float dt);
    bool isActive() const { return erupted && activeTimer > 0.f; }
    bool isFinished() const { return warningTimer <= 0.f && activeTimer <= 0.f && fadeTimer <= 0.f; }
    bool isInZone(float px, float py) const;
    void draw(sf::RenderWindow& window);
};

// ================================================================
//  DragonState – Trạng thái của Rồng
// ================================================================
enum class DragonState {
    WARNING,    // 1.8s: Banner cảnh báo đỏ rực trên đỉnh màn hình
    FLYING,     // Bay qua bản đồ, đập cánh 4 frame, khạc lửa
    DEPARTED    // Rồng đã bay khỏi map, chờ vùng lửa cháy hết
};

// ================================================================
//  DragonEvent – Sự kiện Hỏa Long Sa Trường (Dragon Airstrike)
//  - Bay xé gió qua bản đồ 2800x2000 săn lùng người chơi dẫn đầu
//  - Hoạt ảnh đập cánh 4 frame mượt mà từ dragon_sheet.png (192x192)
//  - Đổ bóng 2.5D đen mờ 80% lướt trên nền cát sa mạc
//  - Khạc 4-5 vùng biển lửa liên tiếp tạo bẫy địa hình hiểm hóc
// ================================================================
class DragonEvent {
private:
    DragonState state;
    float x, y;
    float vx, vy;
    float angle;
    float speed;

    // Sprite Animation
    int animFrame;
    float animTimer;

    // Cảnh báo & Khạc lửa
    float warningDuration;
    float warningTimer;
    float spitInterval;
    float spitTimer;
    int spitsRemaining;

    // Danh sách các vùng lửa trên mặt đất
    std::vector<DragonFireZone> fireZones;
    bool finished;

public:
    DragonEvent(float mapW, float mapH, float targetX, float targetY);

    void update(float dt, float mapW, float mapH);

    // Vẽ lớp mặt đất: Vùng lửa & Đổ bóng rồng 2.5D
    void drawGround(sf::RenderWindow& window);

    // Vẽ lớp bầu trời: Thân Rồng bay trên cao (vẽ sau Y-Sorting)
    void drawAir(sf::RenderWindow& window);

    // Vẽ HUD: Banner cảnh báo Hỏa Long Giáng Lâm
    void drawUI(sf::RenderWindow& window, sf::Font* font);

    bool isFinished() const { return finished; }
    bool isWarning() const { return state == DragonState::WARNING; }
    bool isFlying() const { return state == DragonState::FLYING; }

    std::vector<DragonFireZone>& getFireZones() { return fireZones; }
    float getDragonX() const { return x; }
    float getDragonY() const { return y; }
};
