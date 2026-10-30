#include "MagnetItem.h"
#include "../Entities/Character.h"

MagnetItem::MagnetItem(float startX, float startY)
    : Item(startX, startY, "item_magnet_sheet.png")
{
}

// ================================================================
//  applyBuff – Bật từ trường hút lính tự do bán kính 120px, 4 giây
// ================================================================
void MagnetItem::applyBuff(Character* target) {
    target->setMagnet(true, 4.f);
}
