#pragma once
#include "Item.h"

// ================================================================
//  ShieldItem – Khiên Bảo Vệ
//  applyBuff: Bật isInvincible trong 4 giây
//  Thành viên 5
// ================================================================
class ShieldItem : public Item {
public:
    ShieldItem(float startX, float startY);
    void applyBuff(Character* target) override;
};