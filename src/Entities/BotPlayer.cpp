#include "BotPlayer.h"
#include "Obstacle.h"
#include "../Core/ResourceManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ================================================================
//  Constructor – Khởi tạo toàn bộ hệ thống AI nâng cao
// ================================================================
BotPlayer::BotPlayer(float startX, float startY, Team t, float bx, float by, HeroClass hClass)
    : Character(startX, startY, t, hClass,
                (hClass == HeroClass::VANGUARD) ? "bot_sheet.png" :
                (hClass == HeroClass::TACTICIAN) ? "hero_mage_sheet_red.png" : "hero_titan_sheet_red.png"),
      state(BotState::SCOUTING), prevState(BotState::SCOUTING),
      stateTimer(0.f),
      dirX(0.f), dirY(1.f), decisionTimer(0.f),
      baseX(bx), baseY(by),
      lastResolvedX(startX), lastResolvedY(startY),
      avoidTimer(0.f),
      avoidDirX(0.f), avoidDirY(0.f),
      accumulatedMove(0.f), stuckCheckTimer(0.f), stuckCount(0), isStuck(false),
      // Adaptive Difficulty
      gameElapsed(0.f), aggressionLevel(0.f),
      bankingThreshold(6), scoutRadius(700.f),
      decisionInterval(0.2f), stealRange(350.f), fleeThreshold(200.f),
      // Ambush
      ambushTimer(0.f), ambushMaxTime(5.f),
      ambushTarget(0.f, 0.f), seekingBush(false),
      // Hunting
      huntCooldown(8.f), lastHuntTime(-10.f), huntDuration(0.f),
      // Item Rush
      itemRushTimer(0.f),
      // Memory
      lastKnownPlayerPos(0.f, 0.f), playerMemoryTimer(0.f)
{
}

// ================================================================
//  updateAdaptiveDifficulty – Bot càng lúc càng khó theo thời gian
//  Chia 3 giai đoạn: Early (0-30s), Mid (30-60s), Late (60-90s)
// ================================================================
void BotPlayer::updateAdaptiveDifficulty(float dt) {
    gameElapsed += dt;

    // aggressionLevel tăng từ 0.0 → 1.0 trong 90 giây
    aggressionLevel = std::min(1.f, gameElapsed / 90.f);

    // === Early Game (0-30s): Thận trọng, thu thập ===
    if (gameElapsed < 30.f) {
        bankingThreshold = 6;          // Cất 6 lính mỗi lần
        scoutRadius = 700.f;           // Tầm quét vừa phải
        decisionInterval = 0.2f;       // Ra quyết định mỗi 0.2s
        stealRange = 300.f;            // Chưa hung hãn lắm
        fleeThreshold = 220.f;         // Nhát hơn
        ambushMaxTime = 4.f;           // Nấp ngắn
    }
    // === Mid Game (30-60s): Cân bằng, bắt đầu hunt ===
    else if (gameElapsed < 60.f) {
        bankingThreshold = 5;          // Cất 5 lính
        scoutRadius = 900.f;           // Quét xa hơn
        decisionInterval = 0.15f;      // Nhanh hơn
        stealRange = 400.f;            // Chủ động hơn
        fleeThreshold = 180.f;         // Gan hơn
        ambushMaxTime = 6.f;           // Nấp lâu hơn
    }
    // === Late Game (60-90s): Full aggro, all-in ===
    else {
        bankingThreshold = 4;          // Cất sớm hơn (phòng thủ điểm)
        scoutRadius = 1200.f;          // Quét cả map
        decisionInterval = 0.1f;       // Phản xạ cực nhanh
        stealRange = 500.f;            // Rất hung hãn
        fleeThreshold = 140.f;         // Chỉ chạy khi cực kỳ nguy hiểm
        ambushMaxTime = 8.f;           // Kiên nhẫn phục kích
    }
}

// ================================================================
//  assessThreat – Đánh giá mối đe dọa từ đối thủ
//  Chỉ báo động khi đối thủ bật Khiên hoặc áp đảo quân số vượt trội
// ================================================================
ThreatAssessment BotPlayer::assessThreat(float playerX, float playerY,
                                          int playerArmyCount, bool playerHasShield,
                                          bool playerInBush) {
    ThreatAssessment t;
    t.distance = std::hypot(x - playerX, y - playerY);
    t.armyDiff = army.getCount() - playerArmyCount;
    t.playerShielded = playerHasShield;
    t.playerInBush = playerInBush;

    t.threatLevel = 0.f;

    // Đối thủ có Khiên bất tử → Đe dọa cao, không được lao vào
    if (t.playerShielded && t.distance < 260.f) {
        t.threatLevel += 0.7f;
    }

    // Đối thủ áp đảo hơn từ 4 lính trở lên
    if (t.armyDiff <= -4 && t.distance < 250.f) {
        t.threatLevel += 0.4f;
    }

    t.threatLevel = std::min(1.f, t.threatLevel);
    return t;
}

// ================================================================
//  findNearestBush – Tìm bụi rậm gần nhất cho chiến thuật Ambush
// ================================================================
sf::Vector2f BotPlayer::findNearestBush(const std::vector<Obstacle*>& obstacles) {
    float bestDist = 99999.f;
    sf::Vector2f bestPos(x, y);
    bool found = false;

    for (const auto& obs : obstacles) {
        if (!obs || !obs->isBush()) continue;
        sf::FloatRect box = obs->getCollisionBounds();
        float cx = box.left + box.width / 2.f;
        float cy = box.top + box.height / 2.f;
        float d = std::hypot(x - cx, y - cy);
        if (d < bestDist) {
            bestDist = d;
            bestPos = sf::Vector2f(cx, cy);
            found = true;
        }
    }
    return bestPos;
}

// ================================================================
//  hasStrategicItemNearby – Kiểm tra item chiến lược gần (Shield/Magnet)
//  Bot ưu tiên item khi item nằm trong tầm scout và
//  gần hơn hoặc bằng lính tự do gần nhất
// ================================================================
bool BotPlayer::hasStrategicItemNearby(const std::vector<sf::Vector2f>& itemPositions,
                                        sf::Vector2f& outItemPos) {
    float bestDist = scoutRadius;
    bool found = false;

    for (const auto& ip : itemPositions) {
        float d = std::hypot(x - ip.x, y - ip.y);
        if (d < bestDist) {
            bestDist = d;
            outItemPos = ip;
            found = true;
        }
    }
    return found;
}

// ================================================================
//  shouldBank – Quyết định nên cất lính hay tiếp tục thu thập
//  Logic phức tạp hơn: xét lính đang mang, khoảng cách base,
//  mức đe dọa, và thời gian còn lại
// ================================================================
bool BotPlayer::shouldBank(int myArmyCount, float distToBase,
                            const ThreatAssessment& threat) {
    // Luôn bank khi mang quá nhiều lính (rủi ro cao)
    if (myArmyCount >= bankingThreshold + 4) return true;

    // Bank khi đủ threshold VÀ đang ở gần căn cứ
    if (myArmyCount >= bankingThreshold && distToBase < 450.f) return true;

    // Sắp hết giờ (sau 78s) thì lo về cất lính chuẩn bị Clash
    if (gameElapsed > 78.f && myArmyCount >= 3) return true;

    return false;
}

// ================================================================
//  castRayOpen – Kiểm tra hướng (rdx, rdy) có bị chặn bởi vật cản không
//  Trả về khoảng trống an toàn tính từ tâm Bot (càng lớn = càng mở)
// ================================================================
float BotPlayer::castRayOpen(float rdx, float rdy,
                             const std::vector<Obstacle*>& obstacles,
                             float checkDist) const {
    float minBlock = checkDist;
    for (const auto& obs : obstacles) {
        if (!obs || obs->isPassable() || obs->isBush()) continue;
        sf::FloatRect box = obs->getCollisionBounds();
        if (box.width <= 0.f || box.height <= 0.f) continue;

        float cx = box.left + box.width / 2.f;
        float cy = box.top + box.height / 2.f;
        // Bán kính an toàn của vật cản + thân bot (20px margin)
        float r  = std::max(box.width, box.height) / 2.f + 20.f;

        float toX = cx - x;
        float toY = cy - y;
        float fwd = toX * rdx + toY * rdy;
        if (fwd <= 0.f || fwd > checkDist + r) continue;
        float lat = std::abs(toX * (-rdy) + toY * rdx);
        if (lat < r) {
            float hitDist = fwd - std::sqrt(std::max(0.f, r * r - lat * lat));
            if (hitDist < minBlock) {
                minBlock = std::max(0.f, hitDist);
            }
        }
    }
    return minBlock;
}

// ================================================================
//  bestEscapeDir – Chọn hướng thoát kẹt bằng raycast 16 hướng
// ================================================================
void BotPlayer::bestEscapeDir(const std::vector<Obstacle*>& obstacles,
                               float& outDirX, float& outDirY) const {
    constexpr int RAYS = 16;
    float bestOpen = -1.f;
    float bestDx = 1.f, bestDy = 0.f;
    for (int i = 0; i < RAYS; ++i) {
        float angle = i * (2.f * 3.14159265f / RAYS);
        float rdx = std::cos(angle);
        float rdy = std::sin(angle);
        float open = castRayOpen(rdx, rdy, obstacles, 140.f);
        if (open > bestOpen) {
            bestOpen = open;
            bestDx = rdx;
            bestDy = rdy;
        }
    }
    outDirX = bestDx;
    outDirY = bestDy;
}

// ================================================================
//  applyObstacleAvoidance – Thuật toán né vật cản & khe hẹp thông minh
//  - Khắc phục triệt để việc kẹt giữa 2 hòn đá / thân cây
//  - Context Steering & Whisker Clearance: tìm khe hở rộng nhất
//  - Đẩy khẩn cấp nếu bot bị ép vào hitbox của vật cản
// ================================================================
void BotPlayer::applyObstacleAvoidance(const std::vector<Obstacle*>& obstacles, float& tDirX, float& tDirY) {
    if (obstacles.empty()) return;

    // ── 1. Kiểm tra khẩn cấp: Bot bị đè sát hoặc nằm trong hitbox vật cản ──
    for (const auto& obs : obstacles) {
        if (!obs || obs->isPassable() || obs->isBush()) continue;
        sf::FloatRect box = obs->getCollisionBounds();
        if (box.width <= 0.f || box.height <= 0.f) continue;

        // Vùng đệm 18px quanh hộp va chạm
        sf::FloatRect expBox(box.left - 18.f, box.top - 18.f, box.width + 36.f, box.height + 36.f);
        if (expBox.contains(x, y)) {
            float cx = box.left + box.width / 2.f;
            float cy = box.top + box.height / 2.f;
            float px = x - cx;
            float py = y - cy;
            float plen = std::hypot(px, py);
            if (plen > 0.001f) {
                tDirX = px / plen;
                tDirY = py / plen;
                return;
            }
        }
    }

    // ── 2. Kiểm tra hướng hiện tại có bị cản trở phía trước không ──
    const float CHECK_DIST = 90.f;
    const float SAFE_CLEARANCE = 60.f;

    float currentClear = castRayOpen(tDirX, tDirY, obstacles, CHECK_DIST);
    if (currentClear >= SAFE_CLEARANCE) {
        // Hướng đi hoàn toàn thông thoáng, không bị vướng
        return;
    }

    // ── 3. Hướng hiện tại bị vướng/bị ép vào khe hẹp!
    // Quét các góc lệch đối xứng để tìm hướng thông thoáng gần nhất
    float desiredAngle = std::atan2(tDirY, tDirX);

    const float angleOffsets[] = {
        0.436f,  -0.436f,   // ±25°
        0.873f,  -0.873f,   // ±50°
        1.309f,  -1.309f,   // ±75°
        1.745f,  -1.745f,   // ±100°
        2.182f,  -2.182f,   // ±125°
        2.618f,  -2.618f,   // ±150°
        3.14159f            // 180° (quay đầu ngược lại)
    };

    float bestScore = -99999.f;
    float chosenDx = tDirX;
    float chosenDy = tDirY;

    for (float delta : angleOffsets) {
        float testAngle = desiredAngle + delta;
        float cdx = std::cos(testAngle);
        float cdy = std::sin(testAngle);

        float clear = castRayOpen(cdx, cdy, obstacles, CHECK_DIST);
        // Điểm đánh giá: Ưu tiên khoảng trống lớn, phạt nhẹ nếu lệch góc quá xa
        float penalty = std::abs(delta) * 12.f;
        float score = clear - penalty;

        if (clear >= SAFE_CLEARANCE) {
            // Hướng này thông thoáng và có góc lệch nhỏ nhất -> chọn ngay!
            chosenDx = cdx;
            chosenDy = cdy;
            bestScore = score;
            break;
        }

        if (score > bestScore) {
            bestScore = score;
            chosenDx = cdx;
            chosenDy = cdy;
        }
    }

    tDirX = chosenDx;
    tDirY = chosenDy;
}

// ================================================================
//  doScouting – Quét tìm lính tự do VÀ item (có ưu tiên)
//  So với bản cũ: rộng hơn, tìm item song song, ưu tiên cụm lính
// ================================================================
void BotPlayer::doScouting(float dt, float playerX, float playerY,
                            int playerArmyCount, bool playerHasShield,
                            const std::vector<sf::Vector2f>& freeTroopPositions,
                            const std::vector<sf::Vector2f>& itemPositions,
                            const std::vector<sf::Vector2f>& bombZones,
                            const std::vector<Obstacle*>& obstacles) {
    float bestDist = scoutRadius;
    sf::Vector2f bestTarget(x + dirX * 100.f, y + dirY * 100.f);
    bool foundTarget = false;
    float bestScore = -99999.f;

    // === Thuật toán Scoring: tìm mục tiêu tốt nhất ===
    // Score = -(khoảng cách) + (bonus nếu gần cụm lính) - (penalty nếu gần đối thủ)
    for (const auto& tp : freeTroopPositions) {
        float d = std::hypot(x - tp.x, y - tp.y);
        if (d >= scoutRadius) continue;

        float score = -d;

        // Bonus: Lính gần base bot → ưu tiên (không để player nhặt)
        float distToBotBase = std::hypot(tp.x - baseX, tp.y - baseY);
        if (distToBotBase < 400.f) score += 150.f;

        // Bonus: Lính nằm gần cụm (nhiều lính gần nhau → nhặt hiệu quả)
        int clusterCount = 0;
        for (const auto& tp2 : freeTroopPositions) {
            if (std::hypot(tp.x - tp2.x, tp.y - tp2.y) < 120.f)
                clusterCount++;
        }
        score += clusterCount * 40.f;

        // Chỉ né lính nếu đối thủ đang bật Khiên bất tử
        float distToEnemy = std::hypot(tp.x - playerX, tp.y - playerY);
        if (distToEnemy < 200.f && playerHasShield) {
            score -= 150.f;
        }

        if (score > bestScore) {
            bestScore = score;
            bestDist = d;
            bestTarget = tp;
            foundTarget = true;
        }
    }

    // === Tìm item (cạnh tranh với lính tự do) ===
    for (const auto& ip : itemPositions) {
        float d = std::hypot(x - ip.x, y - ip.y);
        if (d >= scoutRadius) continue;

        // Item có bonus score cố định (đặc biệt quan trọng mid-late game)
        float score = -d + 120.f + aggressionLevel * 100.f;

        if (score > bestScore) {
            bestScore = score;
            bestDist = d;
            bestTarget = ip;
            foundTarget = true;
        }
    }

    if (!foundTarget) {
        // Đi lang thang thông minh: hướng về phía có nhiều lính tự do hơn
        // Thay vì random hoàn toàn, di chuyển về phía giữa map rồi quét xung quanh
        float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
        dirX = std::cos(angle);
        dirY = std::sin(angle);

        // Bias hướng về vùng trung tâm bản đồ (tránh kẹt ở góc)
        float mapCenterX = 1400.f;
        float mapCenterY = 1000.f;
        float toMidX = mapCenterX - x;
        float toMidY = mapCenterY - y;
        float toMidLen = std::hypot(toMidX, toMidY) + 0.01f;
        dirX = dirX * 0.5f + (toMidX / toMidLen) * 0.5f;
        dirY = dirY * 0.5f + (toMidY / toMidLen) * 0.5f;
        float len = std::hypot(dirX, dirY) + 0.01f;
        dirX /= len;
        dirY /= len;
    } else {
        float dx = bestTarget.x - x;
        float dy = bestTarget.y - y;
        float len = std::hypot(dx, dy) + 0.01f;
        dirX = dx / len;
        dirY = dy / len;
    }

    applyObstacleAvoidance(obstacles, dirX, dirY);
}

// ================================================================
//  doHunting – Chủ động truy đuổi đối thủ để cướp lính
//  Chỉ kích hoạt khi mình mạnh hơn đáng kể
// ================================================================
void BotPlayer::doHunting(float dt, float playerX, float playerY,
                           int playerArmyCount, bool playerHasShield,
                           const std::vector<Obstacle*>& obstacles) {
    huntDuration += dt;

    // Timeout: Không truy đuổi quá 6 giây (tránh mất thời gian)
    if (huntDuration > 6.f) {
        state = BotState::SCOUTING;
        huntDuration = 0.f;
        lastHuntTime = gameElapsed;
        return;
    }

    // Nếu đối thủ bật shield → bỏ cuộc
    if (playerHasShield) {
        state = BotState::FLEEING;
        huntDuration = 0.f;
        return;
    }

    // Đuổi theo player, nhưng chạy hơi lệch (intercept angle)
    // Dự đoán vị trí player dựa trên hướng di chuyển gần đây
    float targetX = playerX;
    float targetY = playerY;

    // Nếu nhớ vị trí cũ → tính hướng di chuyển để dự đoán
    if (playerMemoryTimer > 0.f) {
        float pdx = playerX - lastKnownPlayerPos.x;
        float pdy = playerY - lastKnownPlayerPos.y;
        // Dự đoán vị trí tương lai (0.5s ahead)
        targetX += pdx * 2.f;
        targetY += pdy * 2.f;
    }

    float dx = targetX - x;
    float dy = targetY - y;
    float len = std::hypot(dx, dy) + 0.01f;
    dirX = dx / len;
    dirY = dy / len;

    applyObstacleAvoidance(obstacles, dirX, dirY);
}

// ================================================================
//  doAmbush – Phục kích trong bụi rậm
//  Bot nấp và chờ player đi ngang qua → bất ngờ lao ra cướp
// ================================================================
void BotPlayer::doAmbush(float dt, float playerX, float playerY,
                          int playerArmyCount,
                          const std::vector<Obstacle*>& obstacles) {
    if (seekingBush) {
        // Đang tìm đường đến bụi rậm
        float dx = ambushTarget.x - x;
        float dy = ambushTarget.y - y;
        float dist = std::hypot(dx, dy);

        if (dist < 40.f) {
            // Đã vào bụi → bắt đầu nấp
            seekingBush = false;
            ambushTimer = 0.f;
            dirX = 0.f;
            dirY = 0.f;
            return;
        }

        float len = dist + 0.01f;
        dirX = dx / len;
        dirY = dy / len;
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    // Đang nấp trong bụi
    ambushTimer += dt;
    dirX = 0.f;
    dirY = 0.f;

    // Kiểm tra player có đi ngang qua không
    float distToPlayer = std::hypot(x - playerX, y - playerY);

    // Player đi vào tầm cướp → lao ra!
    if (distToPlayer < 200.f && army.getCount() >= playerArmyCount) {
        state = BotState::HUNTING;
        huntDuration = 0.f;
        return;
    }

    // Hết thời gian nấp → quay lại scouting
    if (ambushTimer > ambushMaxTime) {
        state = BotState::SCOUTING;
        ambushTimer = 0.f;
        return;
    }
}

// ================================================================
//  doItemRush – Lao đến nhặt item chiến lược
// ================================================================
void BotPlayer::doItemRush(float dt, const std::vector<sf::Vector2f>& itemPositions,
                            const std::vector<Obstacle*>& obstacles) {
    itemRushTimer += dt;

    // Timeout sau 4s → quay lại scouting
    if (itemRushTimer > 4.f || itemPositions.empty()) {
        state = BotState::SCOUTING;
        itemRushTimer = 0.f;
        return;
    }

    // Tìm item gần nhất
    float bestDist = 99999.f;
    sf::Vector2f bestItem;
    bool found = false;
    for (const auto& ip : itemPositions) {
        float d = std::hypot(x - ip.x, y - ip.y);
        if (d < bestDist) {
            bestDist = d;
            bestItem = ip;
            found = true;
        }
    }

    if (!found || bestDist > scoutRadius) {
        state = BotState::SCOUTING;
        itemRushTimer = 0.f;
        return;
    }

    float dx = bestItem.x - x;
    float dy = bestItem.y - y;
    float len = std::hypot(dx, dy) + 0.01f;
    dirX = dx / len;
    dirY = dy / len;

    applyObstacleAvoidance(obstacles, dirX, dirY);
}

// ================================================================
//  think() – Bộ não FSM Nâng Cao
//  8 States, Adaptive Difficulty, Threat Assessment, Memory
// ================================================================
void BotPlayer::think(float dt, float playerX, float playerY,
                      int playerArmyCount, bool playerHasShield,
                      const std::vector<sf::Vector2f>& freeTroopPositions,
                      const std::vector<sf::Vector2f>& itemPositions,
                      const std::vector<sf::Vector2f>& bombZones,
                      const std::vector<Obstacle*>& obstacles,
                      bool playerInBush,
                      float elapsedTime)
{
    // === Cập nhật Adaptive Difficulty ===
    updateAdaptiveDifficulty(dt);

    // 1. Kiểm tra kẹt cứng từ frame trước → tìm ngay hướng thoát mở nhất bằng raycast 16 hướng
    if (isStuck) {
        bestEscapeDir(obstacles, avoidDirX, avoidDirY);
        dirX = avoidDirX;
        dirY = avoidDirY;
        avoidTimer = (stuckCount >= 2) ? 1.0f : 0.6f;
        isStuck = false;
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    // 2. Nếu đang trong chu kỳ thoát kẹt → tiếp tục di chuyển và né vật cản theo hướng thoát
    if (avoidTimer > 0.f) {
        dirX = avoidDirX;
        dirY = avoidDirY;
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    decisionTimer -= dt;
    if (decisionTimer > 0.f) {
        // Vẫn duy trì né chướng ngại vật từng frame theo hướng hiện tại (tránh đâm vào đá khi chờ decision)
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }
    decisionTimer = decisionInterval; // Dynamic interval

    // === Đánh giá mối đe dọa ===
    ThreatAssessment threat = assessThreat(playerX, playerY,
                                            playerArmyCount, playerHasShield, playerInBush);

    float distToPlayer = threat.distance;
    int myArmyCount = army.getCount();

    // === Hệ thống Memory: Nhớ vị trí player ===
    bool playerVisible = true;
    if (playerInBush && distToPlayer > 220.f) {
        playerVisible = false;
        // Nhưng vẫn nhớ vị trí cuối cùng trong 3 giây
        if (playerMemoryTimer > 0.f) {
            playerMemoryTimer -= dt;
        }
    } else {
        lastKnownPlayerPos = sf::Vector2f(playerX, playerY);
        playerMemoryTimer = 3.f;
    }

    // Lưu state trước
    prevState = state;

    // ================================================================
    //  ƯU TIÊN 1: Né vùng bom / lửa rồng (EVADING_BOMB)
    //  Phản xạ nhanh hơn bản cũ: mở rộng tầm phát hiện
    // ================================================================
    float nearestBombDist = 99999.f;
    sf::Vector2f nearestBomb;
    for (const auto& bz : bombZones) {
        float distToBomb = std::hypot(x - bz.x, y - bz.y);
        if (distToBomb < nearestBombDist) {
            nearestBombDist = distToBomb;
            nearestBomb = bz;
        }
    }
    // Tầm phát hiện mở rộng theo aggression (bot gan hơn sẽ né muộn hơn)
    float bombEvadeRadius = 170.f - aggressionLevel * 30.f;
    if (nearestBombDist < bombEvadeRadius) {
        state = BotState::EVADING_BOMB;
        float len = nearestBombDist + 0.01f;
        dirX = (x - nearestBomb.x) / len;
        dirY = (y - nearestBomb.y) / len;
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    // ================================================================
    //  ƯU TIÊN 2: Banking — Logic nâng cao shouldBank()
    // ================================================================
    float distToBase = std::hypot(baseX - x, baseY - y);
    if (shouldBank(myArmyCount, distToBase, threat)) {
        state = BotState::BANKING;
        float dbx = baseX - x;
        float dby = baseY - y;
        float len = std::hypot(dbx, dby) + 0.01f;
        dirX = dbx / len;
        dirY = dby / len;
        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    // ================================================================
    //  ƯU TIÊN 3: KÍCH HOẠT TUYỆT KỸ TẤN CÔNG (COMBAT SKILLS)
    //  Bot chủ động tung chiêu khi giáp mặt người chơi thay vì bỏ chạy!
    // ================================================================
    if (playerVisible && !playerHasShield && distToPlayer <= 290.f) {
        // Tiên Phong: Lướt Đoạt Mệnh Trảm chém thẳng vào người chơi để cướp lính!
        if (canUseSkill() && heroClass == HeroClass::VANGUARD && distToPlayer >= 100.f) {
            triggerSkill(playerX - x, playerY - y);
        }
        // Hộ Quốc: Gầm Kim Cang Hống gây choáng người chơi khi áp sát!
        else if (canUseSkill() && heroClass == HeroClass::TITAN && distToPlayer <= 250.f) {
            triggerSkill();
        }
    }

    // ================================================================
    //  ƯU TIÊN 4: FLEEING — Chỉ chạy khi gặp nguy hiểm chết người
    //  (Người chơi có Khiên bất tử HOẶC áp đảo hơn 5 lính và bot hết chiêu)
    // ================================================================
    bool mustFlee = false;
    if (playerVisible && distToPlayer <= 220.f) {
        if (playerHasShield) {
            mustFlee = true; // Người chơi bất tử -> phải tránh xa
        } else if (playerArmyCount >= myArmyCount + 5 && !canUseSkill()) {
            mustFlee = true; // Quân địch quá đông và bot chưa hồi chiêu
        }
    }

    if (mustFlee) {
        state = BotState::FLEEING;
        float len = distToPlayer + 0.01f;

        float fleeX = (x - playerX) / len;
        float fleeY = (y - playerY) / len;

        // Chạy tản ra hướng về phía căn cứ
        float toBaseX = (baseX - x);
        float toBaseY = (baseY - y);
        float toBaseLen = std::hypot(toBaseX, toBaseY) + 0.01f;
        toBaseX /= toBaseLen;
        toBaseY /= toBaseLen;

        dirX = fleeX * 0.6f + toBaseX * 0.4f;
        dirY = fleeY * 0.6f + toBaseY * 0.4f;
        float dirLen = std::hypot(dirX, dirY) + 0.01f;
        dirX /= dirLen;
        dirY /= dirLen;

        applyObstacleAvoidance(obstacles, dirX, dirY);
        return;
    }

    // ================================================================
    //  ƯU TIÊN 5: HUNTING & STEALING — Chủ động tấn công cướp lính!
    //  Không chạy trốn hèn nhát; nếu người chơi có lính -> lao vào cướp!
    // ================================================================
    if (playerVisible && playerArmyCount > 0 && !playerHasShield && distToPlayer <= stealRange) {
        // Nếu bot đông quân hơn -> săn lùng quyết liệt (HUNTING)
        // Nếu bot ít quân hơn -> rình rập cướp đuôi (STEALING)
        state = (myArmyCount >= playerArmyCount) ? BotState::HUNTING : BotState::STEALING;

        float len = distToPlayer + 0.01f;
        dirX = (playerX - x) / len;
        dirY = (playerY - y) / len;
        applyObstacleAvoidance(obstacles, dirX, dirY);

        // Tung chiêu Đoạt Mệnh Trảm nếu áp sát
        if (canUseSkill() && heroClass == HeroClass::VANGUARD && distToPlayer <= 280.f) {
            triggerSkill(playerX - x, playerY - y);
        }
        return;
    }

    // ================================================================
    //  ƯU TIÊN 6: ITEM_RUSH — Ưu tiên lao đến nhặt item
    //  Kích hoạt khi có item trong tầm VÀ không quá đông lính
    // ================================================================
    if (state == BotState::ITEM_RUSH) {
        doItemRush(dt, itemPositions, obstacles);
        return;
    }
    if (myArmyCount < bankingThreshold) {
        sf::Vector2f itemTarget;
        if (hasStrategicItemNearby(itemPositions, itemTarget)) {
            float itemDist = std::hypot(x - itemTarget.x, y - itemTarget.y);
            // Chỉ rush item nếu nó gần hơn hoặc mid-late game
            if (itemDist < 300.f || aggressionLevel > 0.3f) {
                state = BotState::ITEM_RUSH;
                itemRushTimer = 0.f;
                doItemRush(dt, itemPositions, obstacles);
                return;
            }
        }
    }

    // ================================================================
    //  ƯU TIÊN 7: AMBUSH — Phục kích trong bụi rậm (mid-late game)
    //  Điều kiện: mid-late game, mang lính ít, đối thủ không thấy
    // ================================================================
    if (state == BotState::AMBUSH) {
        doAmbush(dt, playerX, playerY, playerArmyCount, obstacles);
        return;
    }
    if (aggressionLevel > 0.35f && myArmyCount >= 2 && myArmyCount <= 4 &&
        !playerVisible &&
        (std::rand() % 100) < (int)(aggressionLevel * 15.f)) {
        sf::Vector2f bushPos = findNearestBush(obstacles);
        float bushDist = std::hypot(x - bushPos.x, y - bushPos.y);
        if (bushDist < 500.f && bushDist > 30.f) {
            state = BotState::AMBUSH;
            ambushTarget = bushPos;
            seekingBush = true;
            ambushTimer = 0.f;
            doAmbush(dt, playerX, playerY, playerArmyCount, obstacles);
            return;
        }
    }

    // ================================================================
    //  ƯU TIÊN 8: Scouting — Quét tìm mục tiêu (fallback)
    //  Thuật toán Scoring nâng cao: cụm lính, gần base, xa đối thủ
    // ================================================================
    state = BotState::SCOUTING;

    // Quân Sư kích hoạt Bát Quái Hấp Tinh khi có từ 2 lính xung quanh
    if (canUseSkill() && heroClass == HeroClass::TACTICIAN) {
        int nearbyCount = 0;
        for (const auto& tp : freeTroopPositions) {
            if (std::hypot(tp.x - x, tp.y - y) <= 260.f) nearbyCount++;
        }
        if (nearbyCount >= 2) triggerSkill();
    }

    doScouting(dt, playerX, playerY, playerArmyCount, playerHasShield,
               freeTroopPositions, itemPositions, bombZones, obstacles);
}

// ================================================================
//  update() – Di chuyển và phát hiện chống kẹt
// ================================================================
void BotPlayer::update(float dt) {
    updateBuffs(dt);
    // Cập nhật hồi chiêu và lướt Đoạt Mệnh Trảm
    updateSkill(dt);
    stateTimer += dt;

    if (isStunned) return;

    // Nếu đang trong cú lướt Đoạt Mệnh Trảm → quán tính lướt kiểm soát vị trí
    if (isDashing) return;

    // ================================================================
    //  Cơ chế chống kẹt nâng cao
    //  - Đo độ dịch chuyển THỰC TẾ giữa các frame (sau khi giải quyết va chạm)
    //  - Nếu trong 0.35s di chuyển < 8px → kích hoạt cờ isStuck để think() xử lý
    // ================================================================
    float actualMoved = std::hypot(x - lastResolvedX, y - lastResolvedY);
    accumulatedMove += actualMoved;
    lastResolvedX = x;
    lastResolvedY = y;

    stuckCheckTimer += dt;
    const float STUCK_WINDOW   = 0.35f;  // Cửa sổ đo
    const float STUCK_DIST     = 8.f;    // Phải đi được ≥ 8px trong 0.35s

    if (stuckCheckTimer >= STUCK_WINDOW) {
        if (avoidTimer <= 0.f && accumulatedMove < STUCK_DIST) {
            // Bị kẹt vật lý giữa đá/cây! Bật cờ để think() kích hoạt raycast thoát
            isStuck = true;
            stuckCount++;
        } else if (accumulatedMove >= STUCK_DIST) {
            stuckCount = 0; // Đã thoát kẹt thành công
            isStuck = false;
        }
        accumulatedMove  = 0.f;
        stuckCheckTimer  = 0.f;
    }

    facingDirX = dirX;
    facingDirY = dirY;

    if (avoidTimer > 0.f) {
        avoidTimer -= dt;
        dirX = avoidDirX;
        dirY = avoidDirY;
    }

    // Di chuyển
    float dx = dirX * speed * dt;
    float dy = dirY * speed * dt;
    x += dx;
    y += dy;

    headSprite.setPosition(x, y);

    // Animation
    if (std::abs(dx) > 0.01f || std::abs(dy) > 0.01f) {
        updateHeadAnimation(dt, dx, dy);
    }
}

// ================================================================
//  draw() – Vẽ Bot + đoàn lính + hiệu ứng kỹ năng
// ================================================================
void BotPlayer::draw(sf::RenderWindow& window) {
    drawSkillEffects(window);
    army.draw(window, inBush ? 115 : 255);
    drawHeadAndAura(window);
}