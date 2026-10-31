#pragma once
#include "Item.h"

// ================================================================
//  MagnetItem – Nam Châm Hút Lính
//  applyBuff: Bật từ trường hút lính tự do trong 4 giây
//  Thành viên 5
// ================================================================
class MagnetItem : public Item {
public:
    MagnetItem(float startX, float startY);
    void applyBuff(Character* target) override;
};
