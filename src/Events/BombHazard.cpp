#include "BombHazard.h"
#include "../Core/ResourceManager.h"
#include <cmath>

// ================================================================
//  Constructor – Tạo vùng cảnh báo bom (mặc định 115px)
// ================================================================
BombHazard::BombHazard(float cx, float cy, float r)
    : x(cx), y(cy), radius(r),
      warningTime(1.7f), exploded(false), finished(false),
      explosionFrame(0), explosionTimer(0.f), warningPulse(0.f),
      isScorching(false), scorchTimer(0.f), fireAnimTimer(0.f)
{
    // Sprite cảnh báo (vòng đỏ)
    sf::Texture& warnTex = ResourceManager::getInstance().getTexture("warning_zone.png");
    warningSprite.setTexture(warnTex);
    float warnScale = (radius * 2.f) / warnTex.getSize().x;
    warningSprite.setScale(warnScale, warnScale);
    warningSprite.setOrigin(warnTex.getSize().x / 2.f, warnTex.getSize().y / 2.f);
    warningSprite.setPosition(x, y);

    // Sprite nổ (6 frame)
    sf::Texture& expTex = ResourceManager::getInstance().getTexture("explosion_sheet.png");
    explosionSprite.setTexture(expTex);
    explosionSprite.setTextureRect(sf::IntRect(0, 0, 64, 64));
    float expScale = (radius * 2.1f) / 64.f;
    explosionSprite.setScale(expScale, expScale);
    explosionSprite.setOrigin(32.f, 32.f);
    explosionSprite.setPosition(x, y);
}

// ================================================================
//  update
// ================================================================
void BombHazard::update(float dt) {
    if (finished) return;

    if (!exploded) {
        // === Phase cảnh báo (1.7s) ===
        warningTime -= dt;
        // Tần suất nhấp nháy tăng tốc nhanh dần theo độ khẩn cấp (dồn dập)
        float urgency = 1.f - (warningTime / 1.7f);
        if (urgency < 0.f) urgency = 0.f;
        warningPulse += dt * (5.f + urgency * 16.f);

        if (warningTime <= 0.f) {
            // Chuyển sang phase nổ — GameEngine sẽ xử lý damage
        }
    } else if (!isScorching) {
        // === Phase animation nổ ===
        explosionTimer += dt;
        if (explosionTimer >= 0.07f) {
            explosionTimer = 0.f;
            explosionFrame++;
            if (explosionFrame >= 6) {
                // Hết animation nổ -> Bắt đầu Vùng Đất Cháy Âm Ỉ (3.5s)
                isScorching = true;
                scorchTimer = 3.5f;
            } else {
                explosionSprite.setTextureRect(
                    sf::IntRect(explosionFrame * 64, 0, 64, 64)
                );
            }
        }
    } else {
        // === Phase Vùng Đất Cháy Âm Ỉ (3.5s) ===
        scorchTimer -= dt;
        fireAnimTimer += dt * 8.f;
        if (scorchTimer <= 0.f) {
            finished = true;
        }
    }
}

// ================================================================
//  draw
// ================================================================
void BombHazard::draw(sf::RenderWindow& window) {
    if (finished) return;

    if (!exploded) {
        // 1. Vùng cảnh báo nhấp nháy đỏ
        float alpha = 130.f + 110.f * std::sin(warningPulse);
        if (alpha < 30.f) alpha = 30.f;
        if (alpha > 255.f) alpha = 255.f;
        warningSprite.setColor(sf::Color(255, 60, 60, (sf::Uint8)alpha));
        window.draw(warningSprite);

        // Vòng tròn viền đỏ neon bao ngoài
        sf::CircleShape outerRing(radius);
        outerRing.setOrigin(radius, radius);
        outerRing.setPosition(x, y);
        outerRing.setFillColor(sf::Color::Transparent);
        outerRing.setOutlineColor(sf::Color(255, 40, 40, (sf::Uint8)alpha));
        outerRing.setOutlineThickness(2.5f);
        window.draw(outerRing);

        // Tâm ngắm pháo kích (Target Crosshair Ring)
        float innerR = radius * 0.45f;
        sf::CircleShape innerRing(innerR);
        innerRing.setOrigin(innerR, innerR);
        innerRing.setPosition(x, y);
        innerRing.setFillColor(sf::Color::Transparent);
        innerRing.setOutlineColor(sf::Color(255, 220, 0, (sf::Uint8)alpha));
        innerRing.setOutlineThickness(1.8f);
        window.draw(innerRing);

        // 2. Quả bom rơi xuống từ trên cao (thu nhỏ dần về tâm)
        sf::Texture& bombTex = ResourceManager::getInstance().getTexture("bomb.png");
        sf::Sprite bomb(bombTex);
        bomb.setOrigin(bombTex.getSize().x / 2.f, bombTex.getSize().y / 2.f);
        bomb.setPosition(x, y);
        float bombScale = 0.35f + (warningTime / 1.7f) * 0.45f;
        bomb.setScale(bombScale, bombScale);
        window.draw(bomb);
    } else if (!isScorching) {
        // Vẽ animation nổ tung
        window.draw(explosionSprite);
    } else {
        // Vẽ VÙNG ĐẤT CHÁY ÂM Ỉ (Scorched Earth)
        float alphaRatio = (scorchTimer < 0.8f) ? (scorchTimer / 0.8f) : 1.0f;
        float r = radius * 0.85f;

        // 1. Hố tro than cháy sẫm ở mặt đất
        sf::CircleShape crater(r);
        crater.setOrigin(r, r);
        crater.setPosition(x, y);
        crater.setFillColor(sf::Color(28, 16, 12, (sf::Uint8)(210 * alphaRatio)));
        crater.setOutlineColor(sf::Color(180, 50, 20, (sf::Uint8)(160 * alphaRatio)));
        crater.setOutlineThickness(2.5f);
        window.draw(crater);

        // 2. Các ngọn lửa và tàn than rực cháy bốc lên
        for (int i = 0; i < 7; ++i) {
            float angle = (i * 51.4f) * 3.14159f / 180.f + std::sin(fireAnimTimer * 0.5f + i) * 0.3f;
            float dist = (r * 0.55f) * (0.35f + 0.55f * std::sin(fireAnimTimer * 0.8f + i * 1.5f));
            float fx = x + std::cos(angle) * dist;
            float fy = y + std::sin(angle) * dist - (3.f + 5.f * std::sin(fireAnimTimer + i));

            float flameSize = 7.f + 4.f * std::sin(fireAnimTimer * 1.2f + i);
            sf::CircleShape flame(flameSize);
            flame.setOrigin(flameSize, flameSize);
            flame.setPosition(fx, fy);
            flame.setFillColor((i % 2 == 0) ? sf::Color(255, 120, 20, (sf::Uint8)(220 * alphaRatio))
                                            : sf::Color(255, 210, 40, (sf::Uint8)(240 * alphaRatio)));
            window.draw(flame);
        }
    }
}

// ================================================================
//  isInBlastZone – Kiểm tra điểm có nằm trong vùng nổ trực diện
// ================================================================
bool BombHazard::isInBlastZone(float px, float py) const {
    float dx = px - x;
    float dy = py - y;
    return (dx * dx + dy * dy) <= (radius * radius);
}

// ================================================================
//  isInScorchedZone – Kiểm tra điểm có nằm trong vùng đất cháy âm ỉ
// ================================================================
bool BombHazard::isInScorchedZone(float px, float py) const {
    if (!isScorching || scorchTimer <= 0.f) return false;
    float dx = px - x;
    float dy = py - y;
    float r = radius * 0.85f;
    return (dx * dx + dy * dy) <= (r * r);
}

// ================================================================
//  markExploded – GameEngine gọi sau khi xử lý damage
// ================================================================
void BombHazard::markExploded() {
    exploded = true;
    explosionFrame = 0;
    explosionTimer = 0.f;
}
