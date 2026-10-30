#include "DragonEvent.h"
#include "../Core/ResourceManager.h"
#include "../Core/ConfigManager.h"
#include "../Core/AudioManager.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

static constexpr float PI = 3.14159265358979323846f;

// ================================================================
//  DragonFireZone Implementation
// ================================================================
DragonFireZone::DragonFireZone(float cx, float cy)
    : x(cx), y(cy),
      warningTimer(0.8f), activeTimer(4.5f), fadeTimer(0.8f),
      animTimer(0.f), animFrame(0), erupted(false)
{
}

void DragonFireZone::update(float dt) {
    if (warningTimer > 0.f) {
        warningTimer -= dt;
        if (warningTimer <= 0.f) erupted = true;
    } else if (activeTimer > 0.f) {
        activeTimer -= dt;
        animTimer += dt;
        if (animTimer >= 0.10f) {
            animTimer -= 0.10f;
            animFrame = (animFrame + 1) % 4;
        }
    } else if (fadeTimer > 0.f) {
        fadeTimer -= dt;
    }
}

bool DragonFireZone::isInZone(float px, float py) const {
    float dx = px - x;
    float dy = py - y;
    // Bán kính vùng lửa ~55px (bao trọn vệt lửa 110px của sprite 128x128)
    return (dx * dx + dy * dy) <= (55.f * 55.f);
}

void DragonFireZone::draw(sf::RenderWindow& window) {
    sf::Texture& fireTex = ResourceManager::getInstance().getTexture("dragon_fire_sheet.png");

    if (warningTimer > 0.f) {
        // Cảnh báo trước khi nổ: Vòng tròn năng lượng lửa nhấp nháy trên mặt đất
        float pulse = 0.75f + 0.25f * std::sin((0.8f - warningTimer) * 18.f);
        sf::CircleShape warn(52.f * pulse);
        warn.setOrigin(52.f * pulse, 52.f * pulse);
        warn.setPosition(x, y);
        warn.setFillColor(sf::Color(255, 60, 0, (sf::Uint8)(95 * pulse)));
        warn.setOutlineColor(sf::Color(255, 215, 0, (sf::Uint8)(230 * pulse)));
        warn.setOutlineThickness(2.5f);
        window.draw(warn);
    } else if (activeTimer > 0.f) {
        // Biển lửa rực cháy (4 frames cuồn cuộn 128x128)
        sf::Sprite fire(fireTex);
        fire.setTextureRect(sf::IntRect(animFrame * 128, 0, 128, 128));
        fire.setOrigin(64.f, 64.f);
        fire.setPosition(x, y);
        window.draw(fire);
    } else if (fadeTimer > 0.f) {
        // Tro tàn nguội dần
        float alpha = (fadeTimer / 0.8f) * 140.f;
        sf::CircleShape ash(48.f);
        ash.setOrigin(48.f, 48.f);
        ash.setPosition(x, y);
        ash.setFillColor(sf::Color(35, 25, 20, (sf::Uint8)alpha));
        window.draw(ash);
    }
}

// ================================================================
//  DragonEvent Implementation
// ================================================================
DragonEvent::DragonEvent(float mapW, float mapH, float targetX, float targetY)
    : state(DragonState::WARNING),
      animFrame(0), animTimer(0.f),
      warningDuration(1.8f), warningTimer(1.8f),
      speed(440.f),
      spitInterval(1.25f), spitTimer(0.5f), spitsRemaining(5),
      finished(false)
{
    // Xác định điểm xuất phát ngoài map và hướng bay săn lùng người chơi
    float startX = 0.f, startY = 0.f;
    bool spawnLeft = (std::rand() % 2 == 0);

    if (spawnLeft) {
        startX = -180.f;
        startY = (targetY > mapH / 2.f) ? (targetY - 250.f) : (targetY + 250.f);
    } else {
        startX = mapW + 180.f;
        startY = (targetY > mapH / 2.f) ? (targetY - 250.f) : (targetY + 250.f);
    }

    if (startY < 180.f) startY = 180.f;
    if (startY > mapH - 180.f) startY = mapH - 180.f;

    // Vector hướng bay từ điểm spawn cắt qua mục tiêu và bay xuyên suốt qua map
    float dx = targetX - startX;
    float dy = targetY - startY;
    float dist = std::hypot(dx, dy);
    if (dist < 1.f) { dist = 1.f; dx = 1.f; dy = 0.f; }

    vx = (dx / dist) * speed;
    vy = (dy / dist) * speed;
    angle = std::atan2(vy, vx) * 180.f / PI;

    x = startX;
    y = startY;

    // Phát âm thanh gầm rống và giọng lồng tiếng cảnh báo Hỏa Long giáng lâm
    AudioManager::getInstance().playSound("sfx_dragon_roar.ogg");
    AudioManager::getInstance().playVoice("vo_dragon.ogg");
}

void DragonEvent::update(float dt, float mapW, float mapH) {
    // 1. Cập nhật các vùng lửa trên mặt đất
    for (auto& fz : fireZones) {
        fz.update(dt);
    }
    fireZones.erase(std::remove_if(fireZones.begin(), fireZones.end(),
        [](const DragonFireZone& fz) { return fz.isFinished(); }), fireZones.end());

    // 2. Logic trạng thái của Rồng
    if (state == DragonState::WARNING) {
        warningTimer -= dt;
        if (warningTimer <= 0.f) {
            state = DragonState::FLYING;
        }
    } else if (state == DragonState::FLYING) {
        warningTimer -= dt; // Dùng để làm mờ dần banner cảnh báo
        x += vx * dt;
        y += vy * dt;

        // Hoạt ảnh đập cánh 4 frame
        animTimer += dt;
        if (animTimer >= 0.12f) {
            animTimer -= 0.12f;
            animFrame = (animFrame + 1) % 4;
        }

        // Khạc biển lửa xuống sa trường
        spitTimer -= dt;
        if (spitTimer <= 0.f && spitsRemaining > 0) {
            // Chỉ thả lửa khi rồng đã bay vào trong địa phận bản đồ
            if (x >= 120.f && x <= mapW - 120.f && y >= 120.f && y <= mapH - 120.f) {
                fireZones.emplace_back(x, y);
                spitsRemaining--;
                spitTimer = spitInterval;
                AudioManager::getInstance().playSound("sfx_dragon_fire.ogg");
            }
        }

        // Kiểm tra xem Rồng đã bay hoàn toàn ra ngoài biên bản đồ chưa
        bool exited = false;
        if (vx > 0.f && x > mapW + 260.f) exited = true;
        if (vx < 0.f && x < -260.f) exited = true;
        if (vy > 0.f && y > mapH + 260.f) exited = true;
        if (vy < 0.f && y < -260.f) exited = true;

        if (exited) {
            state = DragonState::DEPARTED;
        }
    } else if (state == DragonState::DEPARTED) {
        if (fireZones.empty()) {
            finished = true;
        }
    }
}

void DragonEvent::drawGround(sf::RenderWindow& window) {
    // 1. Vẽ tất cả các vùng lửa bốc cháy trên mặt đất
    for (auto& fz : fireZones) {
        fz.draw(window);
    }

    // 2. Vẽ bóng của Rồng lướt trên mặt đất (Đổ bóng 2.5D đen mờ 80%)
    if (state == DragonState::FLYING) {
        sf::Texture& dragonTex = ResourceManager::getInstance().getTexture("dragon_sheet.png");
        sf::Sprite shadow(dragonTex);
        shadow.setTextureRect(sf::IntRect(animFrame * 192, 0, 192, 192));
        shadow.setOrigin(96.f, 96.f);
        // Độ cao bay giả lập: bóng dịch chuyển về phía dưới 75px
        shadow.setPosition(x - 25.f, y + 75.f);
        shadow.setRotation(angle);
        float shadowScaleY = (vx < 0.f) ? -0.85f : 0.85f;
        shadow.setScale(0.85f, shadowScaleY);
        shadow.setColor(sf::Color(0, 0, 0, 80));
        window.draw(shadow);
    }
}

void DragonEvent::drawAir(sf::RenderWindow& window) {
    // Vẽ thân Rồng bay xé gió trên tầng không (Layer cao nhất, che trên cây và người)
    if (state == DragonState::FLYING) {
        sf::Texture& dragonTex = ResourceManager::getInstance().getTexture("dragon_sheet.png");
        sf::Sprite dragon(dragonTex);
        dragon.setTextureRect(sf::IntRect(animFrame * 192, 0, 192, 192));
        dragon.setOrigin(96.f, 96.f);
        dragon.setPosition(x, y);
        dragon.setRotation(angle);
        float dragonScaleY = (vx < 0.f) ? -1.1f : 1.1f;
        dragon.setScale(1.1f, dragonScaleY);
        window.draw(dragon);
    }
}

void DragonEvent::drawUI(sf::RenderWindow& window, sf::Font* font) {
    // Thông báo sự kiện đã được GameEngine vẽ tập trung qua dải lụa hoàng gia banner_ribbon.png
    // Bỏ khung chữ nhật pill cũ để tránh bị lồng/đè 2 thông báo lên nhau
    (void)window;
    (void)font;
}
