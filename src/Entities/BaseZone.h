#pragma once
#include "GameObject.h"
#include "../DataStructures/TroopLinkedList.h"
#include <vector>

// ================================================================
//  BaseZone – Căn cứ cất lính an toàn
//  Khi Tướng chạm vùng Base → toàn bộ lính trên đuôi được cất vào
//  Thành viên 3
// ================================================================
class BaseZone : public GameObject {
private:
    sf::Sprite sprite;
    Team team;
    float radius;                      // Bán kính vùng kích hoạt
    std::vector<Troop*> storedTroops;  // Lính đã cất (dùng cho Phase 2)
    int totalStoredPower;

public:
    BaseZone(float startX, float startY, Team t);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;

    Team getTeam() const { return team; }
    float getRadius() const { return radius; }
    int getStoredPower() const { return totalStoredPower; }
    int getStoredCount() const { return (int)storedTroops.size(); }

    // Cất lính: Chuyển toàn bộ lính từ army của Character vào Base
    void bankTroops(TroopLinkedList& army);

    // Lấy toàn bộ lính ra cho Phase 2 Clash
    std::vector<Troop*>& getStoredTroops() { return storedTroops; }

    // Kiểm tra Tướng có đang ở trong vùng Base không
    bool isInZone(float px, float py) const;
};
