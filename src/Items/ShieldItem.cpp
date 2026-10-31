#include "ShieldItem.h"
#include "../Entities/Character.h"

ShieldItem::ShieldItem(float startX, float startY)
    : Item(startX, startY, "item_shield_sheet.png")
{
}

// ================================================================
//  applyBuff – Bật khiên bảo vệ: Chống bị cướp lính trong 4 giây
// ================================================================
void ShieldItem::applyBuff(Character* target) {
    target->setInvincible(true, 4.f);
}