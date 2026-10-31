#include "SpeedItem.h"
#include "../Entities/Character.h"

SpeedItem::SpeedItem(float startX, float startY)
    : Item(startX, startY, "item_speed_sheet.png")
{
}

// ================================================================
//  applyBuff – Tăng 30% tốc độ trong 4 giây
//  Đa hình: GameEngine chỉ gọi item->applyBuff(player)
//  Không cần biết đây là SpeedItem hay ShieldItem
// ================================================================
void SpeedItem::applyBuff(Character* target) {
    target->setSpeedBoost(1.3f, 4.f); // 130% speed, 4 giây
}