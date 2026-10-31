#include "VipDropEvent.h"
#include <cmath>

VipDropEvent::VipDropEvent(float tx, float ty, float triggerAt)
    : triggered(false), spawned(false), triggerTime(triggerAt),
      warningTimer(0.f), isWarning(false), dropX(tx), dropY(ty)
{
    warningCircle.setRadius(50.f);
    warningCircle.setOrigin(50.f, 50.f);
    warningCircle.setPosition(dropX, dropY);
    warningCircle.setFillColor(sf::Color(255, 215, 0, 50));  // Vàng mờ
    warningCircle.setOutlineColor(sf::Color(255, 215, 0, 200));
    warningCircle.setOutlineThickness(3.f);
}

bool VipDropEvent::update(float dt, float elapsedTime) {
    if (spawned) return false;

    // Bắt đầu cảnh báo 3 giây trước khi spawn
    if (!triggered && elapsedTime >= (triggerTime - 3.f)) {
        isWarning = true;
        warningTimer += dt;
    }

    // Spawn khi đến thời điểm
    if (!triggered && elapsedTime >= triggerTime) {
        triggered = true;
        isWarning = false;
        return true; // Báo GameEngine gọi spawnVipTroops()
    }

    return false;
}

void VipDropEvent::draw(sf::RenderWindow& window) {
    if (spawned || !isWarning) return;

    // Vẽ vòng tròn vàng nhấp nháy
    float pulse = std::sin(warningTimer * 5.f) * 0.5f + 0.5f;
    float alpha = 50.f + pulse * 150.f;
    warningCircle.setFillColor(sf::Color(255, 215, 0, (sf::Uint8)alpha));

    // Scale thu nhỏ dần
    float scale = 1.5f - (warningTimer / 3.f) * 0.5f;
    warningCircle.setScale(scale, scale);
    window.draw(warningCircle);
}

std::vector<Troop*> VipDropEvent::spawnVipTroops() {
    spawned = true;
    std::vector<Troop*> vips;

    // Spawn 3 con Kỵ Binh (Power 3) xung quanh điểm drop
    for (int i = 0; i < 3; i++) {
        float angle = (float)i * 2.f * 3.14159f / 3.f;
        float spawnX = dropX + std::cos(angle) * 30.f;
        float spawnY = dropY + std::sin(angle) * 30.f;
        vips.push_back(new Troop(spawnX, spawnY, TroopType::CAVALRY));
    }

    return vips;
}
