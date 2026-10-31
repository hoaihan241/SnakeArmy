#pragma once
#include "Item.h"

// ================================================================
//  SpeedItem – Giày Tốc Độ
//  applyBuff: Tăng 30% speed trong 4 giây
//  Thành viên 5
// ================================================================
class SpeedItem : public Item {
public:
    SpeedItem(float startX, float startY);
    void applyBuff(Character* target) override;
};