#pragma once
#include "GameObject.h"
#include "../DataStructures/TroopLinkedList.h"

// ================================================================
//  HeroClass – 3 Hệ Tướng với Tuyệt Kỹ và Phong Cách Chiến Đấu Riêng
//  1. VANGUARD (Tiên Phong): Đoạt Mệnh Trảm (Dash Slash lướt cướp quân)
//  2. TACTICIAN (Quân Sư): Bát Quái Hấp Tinh (Vortex bão xoáy hút lính & item, làm chậm địch)
//  3. TITAN (Hộ Quốc): Kim Cang Hống (Warcry gầm choáng địch & khiên vàng bất tử toàn quân)
// ================================================================
enum class HeroClass {
    VANGUARD = 0,
    TACTICIAN = 1,
    TITAN = 2
};

// ================================================================
//  Character – LỚP CHA NHÂN VẬT DI CHUYỂN ĐƯỢC
//  Kế thừa từ GameObject. HumanPlayer và BotPlayer kế thừa từ đây.
//  Thành viên 4: Hệ thống điều khiển & buff
// ================================================================
class Character : public GameObject {
protected:
    // === Phân loại Tướng ===
    HeroClass heroClass;

    // === Di chuyển ===
    float speed;           // Tốc độ cơ bản (pixel/giây)
    float baseSpeed;       // Tốc độ gốc (để reset sau buff)
    Team team;             // Phe (BLUE / RED)

    // === Sprite Sheet Animation ===
    sf::Sprite headSprite; // Sprite Tướng (64x64 / frame)
    int animFrame;
    int animRow;           // 0=↓, 1=←, 2=→, 3=↑
    float animTimer;
    int frameSize;         // 64 cho Tướng

    // === Hệ thống Buff (Đa hình Item) ===
    bool isInvincible;     // ShieldItem → true trong 4s
    float shieldTimer;     // Đếm ngược 4s
    float speedTimer;      // SpeedItem → tăng tốc trong 4s
    bool hasMagnet;        // MagnetItem → hút lính tự do trong 4s
    float magnetTimer;

    // === Stun (trúng bom) ===
    bool isStunned;
    float stunTimer;       // Choáng 1.5s

    // === Ẩn nấp / Ngụy trang trong Bụi rậm (Bush Stealth) ===
    bool inBush;

    // === Kỹ năng Tướng "Đoạt Mệnh Trảm" (Fatal Dash) ===
    struct DashGhost {
        float x, y;
        int frame;
        int row;
        float alpha;
        float lifetime;
        float maxLifetime;
    };
    bool isDashing;
    float dashTimer;
    float dashDuration;      // 0.20s lướt chớp nhoáng
    float dashSpeed;         // 1200 px/s
    float dashDirX, dashDirY;
    float dashStartX, dashStartY;
    bool hasCutInThisDash;   // Tránh chém nhiều lần trong 1 cú dash
    float skillCooldown;     // 10.0s hồi chiêu
    float skillTimer;        // Thời gian hồi chiêu còn lại (0 = sẵn sàng)
    float ghostSpawnTimer;
    std::vector<DashGhost> dashGhosts;
    float facingDirX, facingDirY; // Hướng nhìn hiện tại

    // === Kỹ năng Quân Sư "Bát Quái Hấp Tinh" (Vortex Attraction) ===
    bool isVortexActive;
    float vortexTimer;
    float vortexDuration;       // 3.5s
    float vortexRadius;         // 280.f
    float vortexAngle;

    // === Kỹ năng Hộ Quốc "Kim Cang Hống" (Warcry Stun & Golden Aegis) ===
    bool isWarcryActive;
    float warcryTimer;
    float warcryDuration;       // 4.0s
    float warcryRadius;         // 260.f
    float warcryShockwaveRadius;

    // === Đoàn lính ===
    TroopLinkedList army;          // Lính đang dắt theo
    TroopLinkedList bankedArmy;    // Lính đã cất vào Base

public:
    Character(float startX, float startY, Team t, HeroClass hClass, const std::string& sheetName);
    virtual ~Character() = default;

    // === Getter ===
    HeroClass getHeroClass() const { return heroClass; }
    Team getTeam() const { return team; }
    bool getIsInvincible() const { return isInvincible; }
    bool getIsStunned() const { return isStunned; }
    bool getHasMagnet() const { return hasMagnet; }
    bool isInBush() const { return inBush; }
    TroopLinkedList& getArmy() { return army; }
    TroopLinkedList& getBankedArmy() { return bankedArmy; }
    float getSpeed() const { return speed; }

    // Kỹ năng đặc thù
    bool getIsVortexActive() const { return isVortexActive; }
    float getVortexRadius() const { return vortexRadius; }
    bool getIsWarcryActive() const { return isWarcryActive; }
    float getWarcryRadius() const { return warcryRadius; }
    std::string getSkillName() const;

    // === Setter (được gọi bởi Item::applyBuff) ===
    void setInvincible(bool val, float duration = 4.f);
    void setSpeedBoost(float multiplier, float duration = 4.f);
    void setMagnet(bool val, float duration = 4.f);
    void stun(float duration = 1.5f);
    void setInBush(bool val) { inBush = val; }
    void applySlow(float multiplier) {
        if (speedTimer <= 0.f) speed = baseSpeed * multiplier;
    }
    void resetSpeed() {
        if (speedTimer <= 0.f) speed = baseSpeed;
    }

    // === Buff timer update ===
    void updateBuffs(float dt);

    // === Vị trí & Va chạm ===
    void setPosition(float nx, float ny) {
        x = nx;
        y = ny;
        headSprite.setPosition(x, y);
    }
    void resolveObstacles(const std::vector<class Obstacle*>& obstacles);
    void updateArmy(float dt);

    // === Kỹ năng "Đoạt Mệnh Trảm" ===
    bool triggerSkill(float targetDirX = 0.f, float targetDirY = 0.f);
    bool getIsDashing() const { return isDashing; }
    bool getHasCutInThisDash() const { return hasCutInThisDash; }
    void setHasCutInThisDash(bool val) { hasCutInThisDash = val; }
    float getDashStartX() const { return dashStartX; }
    float getDashStartY() const { return dashStartY; }
    float getSkillCooldownProgress() const {
        if (skillCooldown <= 0.f) return 1.f;
        return std::max(0.f, std::min(1.f, 1.f - (skillTimer / skillCooldown)));
    }
    float getSkillTimer() const { return skillTimer; }
    float getSkillCooldown() const { return skillCooldown; }
    bool canUseSkill() const {
        return skillTimer <= 0.f && !isStunned && !isDashing && !isVortexActive && !isWarcryActive;
    }
    void updateSkill(float dt);
    void drawSkillEffects(sf::RenderWindow& window);

    // === Animation & Render riêng ===
    void updateHeadAnimation(float dt, float dx, float dy);
    void drawHeadAndAura(sf::RenderWindow& window);

    // === Hitbox ===
    sf::FloatRect getBounds() const override;
};