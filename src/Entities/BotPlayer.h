#pragma once
#include "Character.h"
#include <vector>

// ================================================================
//  BotPlayer – Tướng AI Nâng Cao với FSM 8 Trạng Thái
//  + Adaptive Difficulty (càng lâu càng khó)
//  + Threat Assessment (đánh giá mối đe dọa)
//  + Bush Ambush (phục kích trong bụi rậm)
//  + Item Priority System (ưu tiên item chiến lược)
//  Thành viên 3
// ================================================================

// Forward declarations
class Obstacle;

enum class BotState {
    SCOUTING,      // Quét radar tìm lính tự do
    STEALING,      // Rình cướp lính đối thủ
    FLEEING,       // Chạy trốn khi nguy hiểm
    BANKING,       // Về Base cất lính
    EVADING_BOMB,  // Né vùng đỏ bom rơi
    AMBUSH,        // Phục kích trong bụi rậm, chờ đối thủ
    HUNTING,       // Chủ động truy đuổi đối thủ (aggressive)
    ITEM_RUSH      // Lao đến nhặt item chiến lược (Shield/Magnet)
};

// Đánh giá mối đe dọa của đối thủ
struct ThreatAssessment {
    float distance;
    int armyDiff;         // (myArmy - playerArmy), âm = yếu hơn
    bool playerShielded;
    bool playerInBush;
    float threatLevel;    // 0.0 = an toàn, 1.0 = nguy hiểm chết người
};

class BotPlayer : public Character {
private:
    BotState state;
    BotState prevState;         // Trạng thái trước (để detect chuyển state)
    float stateTimer;           // Thời gian ở trạng thái hiện tại
    float dirX, dirY;           // Hướng di chuyển hiện tại
    float decisionTimer;        // Đếm thời gian ra quyết định tiếp theo

    // Vị trí Base của bot
    float baseX, baseY;

    // Tránh vật cản & Chống kẹt nâng cao
    float lastResolvedX, lastResolvedY; // Vị trí sau khi va chạm của frame trước
    float avoidTimer;
    float avoidDirX, avoidDirY;
    float accumulatedMove;  // Tổng quãng đường thực tế di chuyển trong cửa sổ đo
    float stuckCheckTimer;  // Bộ đếm cửa sổ đo stuck
    int   stuckCount;       // Số lần kẹt liên tiếp (để tăng thời gian thoát)
    bool  isStuck;          // Cờ báo hiệu bị kẹt cho think() xử lý bằng raycast

    // === HỆ THỐNG MỚI: Adaptive Difficulty ===
    float gameElapsed;          // Thời gian trôi qua trong trận (nhận từ think)
    float aggressionLevel;     // 0.0 → 1.0, tăng dần theo thời gian
    int bankingThreshold;       // Số lính tối thiểu để về base (thay đổi dynamic)
    float scoutRadius;          // Bán kính quét tìm lính (mở rộng dần)
    float decisionInterval;     // Tần suất ra quyết định (nhanh dần)
    float stealRange;           // Tầm phát hiện đối thủ để steal
    float fleeThreshold;        // Khoảng cách an toàn trước khi flee

    // === Ambush System ===
    float ambushTimer;          // Thời gian nấp trong bụi rậm
    float ambushMaxTime;        // Thời gian tối đa nấp (tránh nấp vô hạn)
    sf::Vector2f ambushTarget;  // Vị trí bụi rậm mục tiêu
    bool seekingBush;           // Đang tìm đường đến bụi rậm

    // === Hunting System ===
    float huntCooldown;         // Cooldown giữa các lần hunt
    float lastHuntTime;         // Thời điểm hunt cuối
    float huntDuration;         // Đã truy đuổi bao lâu (tránh chase vô hạn)

    // === Item Priority ===
    float itemRushTimer;        // Thời gian lao đến nhặt item

    // === Memory: Nhớ vị trí Player gần nhất ===
    sf::Vector2f lastKnownPlayerPos;
    float playerMemoryTimer;    // Nhớ vị trí player trong 3s sau khi mất dấu

    // === Hàm nội bộ ===
    void applyObstacleAvoidance(const std::vector<Obstacle*>& obstacles, float& tDirX, float& tDirY);

    // Raycast để đo độ trống theo hướng (rdx, rdy) – dùng cho chống kẹt
    float castRayOpen(float rdx, float rdy,
                      const std::vector<Obstacle*>& obstacles,
                      float checkDist = 120.f) const;

    // Chọn hướng thoát kẹt bằng raycast 16 hướng
    void bestEscapeDir(const std::vector<Obstacle*>& obstacles,
                       float& outDirX, float& outDirY) const;

    // Đánh giá mối đe dọa
    ThreatAssessment assessThreat(float playerX, float playerY,
                                   int playerArmyCount, bool playerHasShield,
                                   bool playerInBush);

    // Cập nhật Adaptive Difficulty dựa trên thời gian trận đấu
    void updateAdaptiveDifficulty(float dt);

    // Tìm bụi rậm gần nhất (cho Ambush)
    sf::Vector2f findNearestBush(const std::vector<Obstacle*>& obstacles);

    // Kiểm tra có item chiến lược gần không
    bool hasStrategicItemNearby(const std::vector<sf::Vector2f>& itemPositions,
                                 sf::Vector2f& outItemPos);

    // Quyết định nên Banking hay tiếp tục thu thập
    bool shouldBank(int myArmyCount, float distToBase,
                    const ThreatAssessment& threat);

    // === Logic FSM nâng cao ===
    void doScouting(float dt, float playerX, float playerY,
                    int playerArmyCount, bool playerHasShield,
                    const std::vector<sf::Vector2f>& freeTroopPositions,
                    const std::vector<sf::Vector2f>& itemPositions,
                    const std::vector<sf::Vector2f>& bombZones,
                    const std::vector<Obstacle*>& obstacles);

    void doHunting(float dt, float playerX, float playerY,
                   int playerArmyCount, bool playerHasShield,
                   const std::vector<Obstacle*>& obstacles);

    void doAmbush(float dt, float playerX, float playerY,
                  int playerArmyCount,
                  const std::vector<Obstacle*>& obstacles);

    void doItemRush(float dt, const std::vector<sf::Vector2f>& itemPositions,
                    const std::vector<Obstacle*>& obstacles);

public:
    BotPlayer(float startX, float startY, Team t, float bx, float by, HeroClass hClass = HeroClass::VANGUARD);
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    BotState getState() const { return state; }
    float getBaseX() const { return baseX; }
    float getBaseY() const { return baseY; }

    // AI quyết định dựa trên thông tin từ GameEngine
    void think(float dt, float playerX, float playerY,
               int playerArmyCount, bool playerHasShield,
               const std::vector<sf::Vector2f>& freeTroopPositions,
               const std::vector<sf::Vector2f>& itemPositions,
               const std::vector<sf::Vector2f>& bombZones,
               const std::vector<Obstacle*>& obstacles,
               bool playerInBush = false,
               float elapsedTime = 0.f);
};