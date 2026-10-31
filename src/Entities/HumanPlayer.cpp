#include "HumanPlayer.h"
#include "../Core/ResourceManager.h"
#include <cmath>

// ================================================================
//  Constructor
//  pIndex=1 → Player1 (WASD, sprite player1_sheet.png)
//  pIndex=2 → Player2 (Arrow keys, sprite player2_sheet.png)
// ================================================================
HumanPlayer::HumanPlayer(float startX, float startY, Team t, int pIndex, HeroClass hClass)
    : Character(startX, startY, t, hClass,
                (pIndex == 1) ?
                    ((hClass == HeroClass::VANGUARD) ? "player1_sheet.png" :
                     (hClass == HeroClass::TACTICIAN) ? "hero_mage_sheet.png" : "hero_titan_sheet.png") :
                    ((hClass == HeroClass::VANGUARD) ? "player2_sheet.png" :
                     (hClass == HeroClass::TACTICIAN) ? "hero_mage_sheet_red.png" : "hero_titan_sheet_red.png")),
      playerIndex(pIndex)
{
}

// ================================================================
//  Update – Đọc input tương ứng với playerIndex
//  Nếu đang bị Stun → không di chuyển
// ================================================================
void HumanPlayer::update(float dt) {
    // Cập nhật buff timers (Shield, Speed, Magnet, Stun)
    updateBuffs(dt);
    // Cập nhật hồi chiêu và trạng thái lướt "Đoạt Mệnh Trảm"
    updateSkill(dt);

    // Nếu đang choáng → đứng yên
    if (isStunned) return;

    // Nếu đang trong cú lướt "Đoạt Mệnh Trảm" → quán tính lướt kiểm soát vị trí
    if (isDashing) return;

    float dx = 0.f, dy = 0.f;

    if (playerIndex == 1) {
        // === Player 1: WASD ===
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dy -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dy += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dx -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dx += 1.f;
    } else {
        // === Player 2: Arrow Keys ===
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    dy -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  dy += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  dx -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) dx += 1.f;
    }

    // Chuẩn hóa vector (để di chuyển chéo không nhanh hơn)
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.f) {
        dx /= len;
        dy /= len;

        x += dx * speed * dt;
        y += dy * speed * dt;

        // Cập nhật hướng mặt
        facingDirX = dx;
        facingDirY = dy;
    }

    // Cập nhật vị trí sprite Tướng
    headSprite.setPosition(x, y);

    // Cập nhật animation hướng di chuyển
    if (len > 0.f) {
        updateHeadAnimation(dt, dx, dy);
    }
}

// ================================================================
//  Draw – Vẽ Tướng + đoàn lính + hiệu ứng Shield Aura nếu có
// ================================================================
void HumanPlayer::draw(sf::RenderWindow& window) {
    drawSkillEffects(window);
    army.draw(window, inBush ? 115 : 255);
    drawHeadAndAura(window);
}