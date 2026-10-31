#include "Character.h"
#include "Obstacle.h"
#include "../Core/ResourceManager.h"
#include "../Core/AudioManager.h"
#include <cmath>
#include <algorithm>

// ================================================================
//  Constructor – Load sprite sheet, khởi tạo buff flags & Kỹ năng
// ================================================================
Character::Character(float startX, float startY, Team t, HeroClass hClass, const std::string& sheetName)
    : GameObject(startX, startY),
      heroClass(hClass),
      speed(210.f), baseSpeed(210.f), team(t),
      animFrame(0), animRow(0), animTimer(0.f), frameSize(64),
      isInvincible(false), shieldTimer(0.f),
      speedTimer(0.f),
      hasMagnet(false), magnetTimer(0.f),
      isStunned(false), stunTimer(0.f),
      inBush(false),
      // Kỹ năng Vanguard: Đoạt Mệnh Trảm
      isDashing(false), dashTimer(0.f), dashDuration(0.20f), dashSpeed(1200.f),
      dashDirX(0.f), dashDirY(1.f), dashStartX(0.f), dashStartY(0.f),
      hasCutInThisDash(false), skillCooldown(7.0f), skillTimer(0.f),
      ghostSpawnTimer(0.f), facingDirX(0.f), facingDirY(1.f),
      // Kỹ năng Tactician: Bát Quái Hấp Tinh
      isVortexActive(false), vortexTimer(0.f), vortexDuration(3.5f), vortexRadius(280.f), vortexAngle(0.f),
      // Kỹ năng Titan: Kim Cang Hống
      isWarcryActive(false), warcryTimer(0.f), warcryDuration(4.0f), warcryRadius(260.f), warcryShockwaveRadius(0.f)
{
    // Thiết lập thuộc tính theo Class Tướng
    if (heroClass == HeroClass::VANGUARD) {
        baseSpeed = 225.f; speed = 225.f;
        skillCooldown = 7.0f;
    } else if (heroClass == HeroClass::TACTICIAN) {
        baseSpeed = 210.f; speed = 210.f;
        skillCooldown = 8.5f;
    } else if (heroClass == HeroClass::TITAN) {
        baseSpeed = 195.f; speed = 195.f;
        skillCooldown = 9.5f;
    }

    sf::Texture& tex = ResourceManager::getInstance().getTexture(sheetName);
    headSprite.setTexture(tex);
    headSprite.setTextureRect(sf::IntRect(0, 0, frameSize, frameSize));
    headSprite.setOrigin(frameSize / 2.f, frameSize / 2.f);
    headSprite.setPosition(x, y);
}

// ================================================================
//  Buff Setters – Được gọi bởi Item::applyBuff() (Đa hình)
// ================================================================
void Character::setInvincible(bool val, float duration) {
    isInvincible = val;
    shieldTimer = duration;
}

void Character::setSpeedBoost(float multiplier, float duration) {
    speed = baseSpeed * multiplier;
    speedTimer = duration;
}

void Character::setMagnet(bool val, float duration) {
    hasMagnet = val;
    magnetTimer = duration;
}

void Character::stun(float duration) {
    isStunned = true;
    stunTimer = duration;
}

// ================================================================
//  updateBuffs – Đếm ngược timer, tắt buff khi hết hạn
//  Gọi mỗi frame trong update()
// ================================================================
void Character::updateBuffs(float dt) {
    // Shield countdown
    if (isInvincible) {
        shieldTimer -= dt;
        if (shieldTimer <= 0.f) {
            isInvincible = false;
            shieldTimer = 0.f;
        }
    }

    // Speed countdown
    if (speedTimer > 0.f) {
        speedTimer -= dt;
        if (speedTimer <= 0.f) {
            speed = baseSpeed; // Reset về tốc độ gốc
            speedTimer = 0.f;
        }
    }

    // Magnet countdown
    if (hasMagnet) {
        magnetTimer -= dt;
        if (magnetTimer <= 0.f) {
            hasMagnet = false;
            magnetTimer = 0.f;
        }
    }

    // Stun countdown
    if (isStunned) {
        stunTimer -= dt;
        if (stunTimer <= 0.f) {
            isStunned = false;
            stunTimer = 0.f;
        }
    }
}

// ================================================================
//  updateHeadAnimation – Chuyển frame sprite sheet cho Tướng
//  dx, dy: delta di chuyển frame này (để xác định hướng)
// ================================================================
void Character::updateHeadAnimation(float dt, float dx, float dy) {
    // Xác định hướng
    if (std::abs(dx) > std::abs(dy)) {
        animRow = (dx < 0) ? 1 : 2;
    } else if (std::abs(dy) > 0.01f) {
        animRow = (dy < 0) ? 3 : 0;
    }

    // Chuyển frame
    animTimer += dt;
    if (animTimer >= 0.15f) {
        animTimer = 0.f;
        animFrame = (animFrame + 1) % 4;
    }
    headSprite.setTextureRect(
        sf::IntRect(animFrame * frameSize, animRow * frameSize, frameSize, frameSize)
    );
}

// ================================================================
//  getBounds – Hitbox cho va chạm
// ================================================================
sf::FloatRect Character::getBounds() const {
    return sf::FloatRect(x - frameSize / 2.f, y - frameSize / 2.f,
                         (float)frameSize, (float)frameSize);
}

// ================================================================
//  resolveObstacles – Ngăn Tướng đi xuyên qua Đá và Cây
//  Sliding collision: trượt mượt mà dọc theo cạnh vật cản
// ================================================================
void Character::resolveObstacles(const std::vector<Obstacle*>& obstacles) {
    // Hitbox chân/thân của Tướng (34x34px)
    float charBoxW = 34.f;
    float charBoxH = 34.f;

    for (int iter = 0; iter < 2; ++iter) {
        bool resolvedAny = false;
        for (const auto& obs : obstacles) {
            if (!obs || obs->isPassable()) continue;
            sf::FloatRect obsBox = obs->getCollisionBounds();
            if (obsBox.width <= 0.f || obsBox.height <= 0.f) continue;
            sf::FloatRect chBox(x - charBoxW / 2.f, y - charBoxH / 2.f, charBoxW, charBoxH);

            sf::FloatRect intersection;
            if (chBox.intersects(obsBox, intersection)) {
                // Đẩy Tướng ra theo phương có độ đè nhỏ nhất
                if (intersection.width < intersection.height) {
                    if (x < obsBox.left + obsBox.width / 2.f) {
                        x -= intersection.width;
                    } else {
                        x += intersection.width;
                    }
                } else {
                    if (y < obsBox.top + obsBox.height / 2.f) {
                        y -= intersection.height;
                    } else {
                        y += intersection.height;
                    }
                }
                headSprite.setPosition(x, y);
                resolvedAny = true;
            }
        }
        if (!resolvedAny) break;
    }
}

void Character::updateArmy(float dt) {
    army.updatePositions(x, y, dt);
}

void Character::drawHeadAndAura(sf::RenderWindow& window) {
    headSprite.setPosition(x, y);
    sf::Uint8 alpha = inBush ? 115 : 255;
    headSprite.setColor(sf::Color(255, 255, 255, alpha));
    window.draw(headSprite);

    if (isInvincible) {
        sf::Texture& auraTex = ResourceManager::getInstance().getTexture("shield_aura.png");
        sf::Sprite aura(auraTex);
        aura.setOrigin(auraTex.getSize().x / 2.f, auraTex.getSize().y / 2.f);
        aura.setPosition(x, y);
        float auraScale = (float)frameSize / auraTex.getSize().x * 1.5f;
        aura.setScale(auraScale, auraScale);
        aura.setColor(sf::Color(255, 255, 255, alpha));
        window.draw(aura);
    }
}

// ================================================================
//  Tuyệt Kỹ Tướng (Hero Skills) – Đa dạng theo từng Hệ Tướng
// ================================================================
std::string Character::getSkillName() const {
    switch (heroClass) {
        case HeroClass::VANGUARD:  return u8"ĐOẠT MỆNH TRẢM";
        case HeroClass::TACTICIAN: return u8"BÁT QUÁI HẤP TINH";
        case HeroClass::TITAN:     return u8"KIM CANG HỐNG";
        default:                   return u8"KỸ NĂNG";
    }
}

bool Character::triggerSkill(float targetDirX, float targetDirY) {
    if (!canUseSkill()) return false;

    if (heroClass == HeroClass::VANGUARD) {
        // === 1. TIÊN PHONG: Đoạt Mệnh Trảm (Dash Slash) ===
        float dX = targetDirX;
        float dY = targetDirY;
        float len = std::hypot(dX, dY);

        if (len < 0.01f) {
            dX = facingDirX;
            dY = facingDirY;
            len = std::hypot(dX, dY);
        }

        if (len < 0.01f) {
            switch (animRow) {
                case 0: dX = 0.f;  dY = 1.f;  break;
                case 1: dX = -1.f; dY = 0.f;  break;
                case 2: dX = 1.f;  dY = 0.f;  break;
                case 3: dX = 0.f;  dY = -1.f; break;
                default: dX = 0.f; dY = 1.f;  break;
            }
        } else {
            dX /= len;
            dY /= len;
        }

        dashDirX = dX;
        dashDirY = dY;
        dashStartX = x;
        dashStartY = y;
        isDashing = true;
        dashTimer = dashDuration;
        skillTimer = skillCooldown;
        hasCutInThisDash = false;
        ghostSpawnTimer = 0.f;

        dashGhosts.push_back({ x, y, animFrame, animRow, 220.f, 0.35f, 0.35f });
        AudioManager::getInstance().playSound("sfx_sword_slash.ogg");
        return true;
    }
    else if (heroClass == HeroClass::TACTICIAN) {
        // === 2. QUÂN SƯ: Bát Quái Hấp Tinh (Vortex Attraction) ===
        isVortexActive = true;
        vortexTimer = vortexDuration;
        skillTimer = skillCooldown;
        vortexAngle = 0.f;

        AudioManager::getInstance().playSound("sfx_item_magnet.ogg");
        return true;
    }
    else if (heroClass == HeroClass::TITAN) {
        // === 3. HỘ QUỐC: Kim Cang Hống & Khiên Bất Diệt (Warcry & Golden Aegis) ===
        isWarcryActive = true;
        warcryTimer = warcryDuration;
        skillTimer = skillCooldown;
        warcryShockwaveRadius = 15.f;

        // Bật khiên hoàng kim bất tử cho bản thân và toàn quân trong 4s
        setInvincible(true, warcryDuration);

        AudioManager::getInstance().playSound("sfx_clash_hit.ogg");
        return true;
    }

    return false;
}

void Character::updateSkill(float dt) {
    // 1. Giảm thời gian hồi chiêu chung
    if (skillTimer > 0.f) {
        skillTimer -= dt;
        if (skillTimer < 0.f) skillTimer = 0.f;
    }

    // 2. Cập nhật Kỹ năng Tiên Phong (Dash)
    if (isDashing) {
        dashTimer -= dt;

        x += dashDirX * dashSpeed * dt;
        y += dashDirY * dashSpeed * dt;
        headSprite.setPosition(x, y);

        ghostSpawnTimer += dt;
        if (ghostSpawnTimer >= 0.035f) {
            ghostSpawnTimer = 0.f;
            dashGhosts.push_back({ x, y, animFrame, animRow, 200.f, 0.30f, 0.30f });
        }

        if (dashTimer <= 0.f) {
            isDashing = false;
        }
    }

    for (auto it = dashGhosts.begin(); it != dashGhosts.end(); ) {
        it->lifetime -= dt;
        it->alpha = (it->lifetime / it->maxLifetime) * 200.f;
        if (it->lifetime <= 0.f) {
            it = dashGhosts.erase(it);
        } else {
            ++it;
        }
    }

    // 3. Cập nhật Kỹ năng Quân Sư (Vortex)
    if (isVortexActive) {
        vortexTimer -= dt;
        vortexAngle += 260.f * dt;
        if (vortexAngle >= 360.f) vortexAngle -= 360.f;

        if (vortexTimer <= 0.f) {
            isVortexActive = false;
        }
    }

    // 4. Cập nhật Kỹ năng Hộ Quốc (Titan Warcry)
    if (isWarcryActive) {
        warcryTimer -= dt;
        if (warcryShockwaveRadius < warcryRadius) {
            warcryShockwaveRadius += 650.f * dt;
            if (warcryShockwaveRadius > warcryRadius) {
                warcryShockwaveRadius = warcryRadius;
            }
        }
        if (warcryTimer <= 0.f) {
            isWarcryActive = false;
        }
    }
}

void Character::drawSkillEffects(sf::RenderWindow& window) {
    // ================================================================
    //  1. HIỆU ỨNG TIÊN PHONG: Tàn ảnh bóng ma & Vệt chém kiếm rực lửa
    // ================================================================
    if (heroClass == HeroClass::VANGUARD) {
        const sf::Texture* tex = headSprite.getTexture();
        if (tex) {
            for (const auto& g : dashGhosts) {
                sf::Sprite ghost(*tex);
                ghost.setTextureRect(sf::IntRect(g.frame * frameSize, g.row * frameSize, frameSize, frameSize));
                ghost.setOrigin(frameSize / 2.f, frameSize / 2.f);
                ghost.setPosition(g.x, g.y);

                sf::Color tint = (team == Team::BLUE) ? sf::Color(100, 200, 255) : sf::Color(255, 120, 90);
                tint.a = (sf::Uint8)std::max(0.f, std::min(255.f, g.alpha));
                ghost.setColor(tint);
                window.draw(ghost);
            }
        }

        if (isDashing) {
            float dx = x - dashStartX;
            float dy = y - dashStartY;
            float dist = std::hypot(dx, dy);
            if (dist > 8.f) {
                float angle = std::atan2(dy, dx) * 180.f / 3.14159265f;

                sf::RectangleShape beam(sf::Vector2f(dist, 14.f));
                beam.setOrigin(0.f, 7.f);
                beam.setPosition(dashStartX, dashStartY);
                beam.setRotation(angle);
                beam.setFillColor((team == Team::BLUE) ? sf::Color(80, 190, 255, 180) : sf::Color(255, 150, 40, 180));
                window.draw(beam);

                sf::RectangleShape core(sf::Vector2f(dist, 5.f));
                core.setOrigin(0.f, 2.5f);
                core.setPosition(dashStartX, dashStartY);
                core.setRotation(angle);
                core.setFillColor(sf::Color(255, 255, 255, 240));
                window.draw(core);
            }
        }
    }

    // ================================================================
    //  2. HIỆU ỨNG QUÂN SƯ: Bão Xoáy Bát Quái Hấp Tinh (Swirling Vortex)
    // ================================================================
    if (heroClass == HeroClass::TACTICIAN && isVortexActive) {
        float alphaFactor = std::min(1.f, vortexTimer / 0.5f);
        sf::Uint8 aOuter = (sf::Uint8)(95 * alphaFactor);
        sf::Uint8 aRune = (sf::Uint8)(180 * alphaFactor);

        // Vòng tròn ma trận Bát Quái ngoài cùng
        sf::CircleShape vortexCircle(vortexRadius);
        vortexCircle.setOrigin(vortexRadius, vortexRadius);
        vortexCircle.setPosition(x, y);
        sf::Color bgCol = (team == Team::BLUE) ? sf::Color(40, 120, 255, aOuter / 2) : sf::Color(220, 40, 120, aOuter / 2);
        vortexCircle.setFillColor(bgCol);
        sf::Color borderCol = (team == Team::BLUE) ? sf::Color(100, 220, 255, aRune) : sf::Color(255, 120, 180, aRune);
        vortexCircle.setOutlineColor(borderCol);
        vortexCircle.setOutlineThickness(2.5f);
        window.draw(vortexCircle);

        // 4 luồng gió xoáy cuốn năng lượng vào tâm
        for (int i = 0; i < 4; ++i) {
            float curAngle = vortexAngle + i * 90.f;
            float rad = curAngle * 3.14159f / 180.f;
            float spiralDist = vortexRadius * 0.65f;
            float px = x + std::cos(rad) * spiralDist;
            float py = y + std::sin(rad) * spiralDist;

            sf::CircleShape spiralNode(8.f);
            spiralNode.setOrigin(8.f, 8.f);
            spiralNode.setPosition(px, py);
            spiralNode.setFillColor(borderCol);
            window.draw(spiralNode);
        }
    }

    // ================================================================
    //  3. HIỆU ỨNG HỘ QUỐC: Sóng Chấn Động Kim Cang Hống & Khiên Vàng
    // ================================================================
    if (heroClass == HeroClass::TITAN && isWarcryActive) {
        // Sóng xung kích lan tỏa ra ngoài
        if (warcryShockwaveRadius < warcryRadius) {
            float progress = warcryShockwaveRadius / warcryRadius;
            sf::Uint8 shockAlpha = (sf::Uint8)(220 * (1.f - progress));

            sf::CircleShape shockwave(warcryShockwaveRadius);
            shockwave.setOrigin(warcryShockwaveRadius, warcryShockwaveRadius);
            shockwave.setPosition(x, y);
            shockwave.setFillColor(sf::Color::Transparent);
            shockwave.setOutlineThickness(4.f);
            shockwave.setOutlineColor(sf::Color(255, 220, 60, shockAlpha));
            window.draw(shockwave);
        }

        // Hào quang Kim Thân Bất Diệt bao bọc Tướng
        sf::CircleShape titanAura(46.f);
        titanAura.setOrigin(46.f, 46.f);
        titanAura.setPosition(x, y);
        titanAura.setFillColor(sf::Color(255, 215, 0, 75));
        titanAura.setOutlineColor(sf::Color(255, 235, 120, 200));
        titanAura.setOutlineThickness(2.5f);
        window.draw(titanAura);
    }

    // ================================================================
    //  4. VÒNG CHỈ THỊ KỸ NĂNG SẴN SÀNG DƯỚI CHÂN TƯỚNG (Skill Ready Ring)
    // ================================================================
    if (canUseSkill()) {
        sf::CircleShape ring(26.f);
        ring.setOrigin(26.f, 26.f);
        ring.setPosition(x, y + 14.f);
        ring.setScale(1.f, 0.45f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.2f);

        sf::Color ringCol = sf::Color(255, 255, 255, 170);
        if (heroClass == HeroClass::VANGUARD) {
            ringCol = (team == Team::BLUE) ? sf::Color(100, 210, 255, 180) : sf::Color(255, 130, 80, 180);
        } else if (heroClass == HeroClass::TACTICIAN) {
            ringCol = (team == Team::BLUE) ? sf::Color(150, 120, 255, 180) : sf::Color(255, 100, 200, 180);
        } else if (heroClass == HeroClass::TITAN) {
            ringCol = sf::Color(255, 225, 70, 200);
        }

        ring.setOutlineColor(ringCol);
        window.draw(ring);
    }
}