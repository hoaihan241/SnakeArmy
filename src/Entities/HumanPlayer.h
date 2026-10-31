#pragma once
#include "Character.h"

// ================================================================
//  HumanPlayer – Tướng người chơi
//  Hỗ trợ 2 bộ phím: P1 (WASD) và P2 (Mũi tên) cho chế độ PvP
//  Thành viên 4, 5
// ================================================================
class HumanPlayer : public Character {
private:
    int playerIndex;  // 1 = Player1 (WASD), 2 = Player2 (Arrows)

public:
    HumanPlayer(float startX, float startY, Team t, int pIndex, HeroClass hClass = HeroClass::VANGUARD);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    int getPlayerIndex() const { return playerIndex; }
};