#pragma once
#include <SFML/Graphics.hpp>
#include "../DataStructures/Troop.h"
#include <vector>

// ================================================================
//  VipDropEvent – Sự kiện rơi lính VIP giữa map
//  Spawn cụm 3 lính Kỵ Binh (Power 3) ở giây thứ 45
//  Thành viên 5
// ================================================================
class VipDropEvent {
private:
    bool triggered;
    bool spawned;
    float triggerTime;     // Giây thứ mấy trong Phase 1 (45s)

    // Hiệu ứng cảnh báo trước khi rơi
    float warningTimer;
    bool isWarning;
    sf::CircleShape warningCircle;
    float dropX, dropY;

public:
    VipDropEvent(float tx, float ty, float triggerAt = 45.f);

    // Gọi mỗi frame, trả về true nếu vừa spawn lính
    bool update(float dt, float elapsedTime);
    void draw(sf::RenderWindow& window);

    bool hasTriggered() const { return triggered; }
    bool hasSpawned() const { return spawned; }

    // Lấy danh sách lính VIP vừa spawn (gọi 1 lần duy nhất)
    std::vector<Troop*> spawnVipTroops();

    float getDropX() const { return dropX; }
    float getDropY() const { return dropY; }
};
