#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

#include "../Entities/HumanPlayer.h"
#include "../Entities/BotPlayer.h"
#include "../Entities/Obstacle.h"
#include "../Entities/BaseZone.h"
#include "../Items/Item.h"
#include "../Events/BombHazard.h"
#include "../Events/VipDropEvent.h"
#include "../Events/DragonEvent.h"

#include "ConfigManager.h"

// ================================================================
//  GameState – Chu trình chuyển cảnh
//  MENU → HERO_SELECT → GATHERING (90s) → CLASH (Auto-battler) → RESULT
// ================================================================
enum class GameState { MENU, HERO_SELECT_PVE, HERO_SELECT_PVP, GATHERING, CLASH, RESULT };
enum class GameMode  { PVP, PVE };
enum class MenuModal { NONE, STORY, SETTINGS };
enum class StoryTab  { STORY, RULES };

// ================================================================
//  GameEngine – Lõi hệ thống
//  Quản lý Game Loop, State Machine, Collision, Spawn, Phase 2
//  Thành viên 1
// ================================================================
class GameEngine {
private:
    // === Cửa sổ & Trạng thái ===
    sf::RenderWindow window;
    GameState currentState;
    GameMode gameMode;

    // === Bản đồ ===
    static constexpr float WORLD_W = 2800.f;
    static constexpr float WORLD_H = 2000.f;
    sf::View camera;
    sf::View cameraP1;          // Camera nửa trái cho Player 1
    sf::View cameraP2;          // Camera nửa phải cho Player 2
    bool isSplitScreen;         // true = chia đôi màn hình, false = gộp chung
    sf::VertexArray bgVertices; // Tiled background

    // === Thực thể ===
    HumanPlayer* player1;
    HumanPlayer* player2;  // nullptr nếu PvE
    BotPlayer* bot;         // nullptr nếu PvP
    std::vector<Obstacle*> obstacles;
    BaseZone* baseBlue;
    BaseZone* baseRed;

    // === Lính tự do trên map ===
    std::vector<Troop*> freeTroops;

    // === Vật phẩm ===
    std::vector<Item*> items;
    float itemSpawnTimer;

    // === Sự kiện ===
    std::vector<BombHazard*> bombs;
    float bombSpawnTimer;
    VipDropEvent* vipDrop;
    std::vector<DragonEvent*> dragonEvents;
    float dragonSpawnTimer;

    // === Timer Phase 1 ===
    float gatheringTimer;       // Đếm ngược 90s
    float elapsedTime;          // Thời gian đã trôi qua

    // === Phase 2: Clash Animations & VFX ===
    struct SlashEffect {
        float x, y;
        float angle;
        float lifetime;
        float maxLifetime;
        sf::Color color;
    };
    struct SparkParticle {
        float x, y;
        float vx, vy;
        float lifetime;
        float maxLifetime;
        sf::Color color;
    };
    struct FloatingText {
        float x, y;
        std::string text;
        float lifetime;
        float maxLifetime;
        sf::Color color;
    };
    struct DeadTroopAnim {
        sf::Sprite sprite;
        float x, y;
        float vx, vy;
        float alpha;
        float lifetime;
    };

    std::vector<Troop*> blueClashArmy;
    std::vector<Troop*> redClashArmy;
    std::vector<SlashEffect> clashSlashes;
    std::vector<SparkParticle> clashSparks;
    std::vector<FloatingText> clashFloatTexts;
    std::vector<DeadTroopAnim> clashDeadTroops;

    float clashTimer;
    int blueTotalPower;
    int redTotalPower;
    float tugOfWarRatio;     // 0.0 = Blue thua hoàn toàn, 1.0 = Blue thắng hoàn toàn
    float clashMarchTimer;   // Đếm ngược 2.4s khi 2 bên từ từ bước ra dàn trận
    float clashChargeTimer;  // Đếm ngược 1.0s khi 2 bên bứt tốc xung phong lao vào nhau
    float clashFrontlineX;   // Tọa độ X chiến tuyến động (640.f)
    float screenShakeTimer;  // Rung màn hình
    float screenShakeIntensity;
    float clashFightTimer;   // Nhịp đọ kiếm
    float clashRoutTimer;    // Đếm ngược ăn mừng khi 1 phe bị quét sạch
    bool clashIsEnding;      // true khi 1 phe hết lính

    // === Kết quả & Thống kê trận đấu ===
    std::string resultText;
    int matchBlueBanked;
    int matchRedBanked;
    int matchBlueClash;
    int matchRedClash;
    int matchWinner; // 1 = Blue (P1), 2 = Red (P2/Bot), 0 = Hòa

    // Thống kê chi tiết lính từng loại cho màn hình kết quả
    int matchBlueSwordsman;
    int matchBlueGuardian;
    int matchBlueCavalry;
    int matchBlueTotalTroops;
    int matchBlueTotalPower;

    int matchRedSwordsman;
    int matchRedGuardian;
    int matchRedCavalry;
    int matchRedTotalTroops;
    int matchRedTotalPower;

    // === Banner Thông Báo Sự Kiện Nổi ===
    std::string bannerText;
    float bannerTimer;
    float bannerMaxDuration;
    sf::Color bannerColor;
    bool bannerAlert10sTriggered;
    void triggerBanner(const std::string& text, float duration = 3.0f, sf::Color col = sf::Color(255, 235, 140));

    // === UI & Menu State ===
    sf::Font* gameFont;
    MenuModal menuModal;
    StoryTab storyTab;
    bool isPaused;
    int hoveredButtonIndex; // 0..4 cho menu chinh, -1 neu khong hover
    sf::Vector2f mousePos;

    // === Chọn Tướng (Hero Selection) ===
    HeroClass p1SelectedHero;
    HeroClass p2SelectedHero;
    int heroSelectP1Hover; // 0 = Vanguard, 1 = Mage, 2 = Titan
    int heroSelectP2Hover; // 0 = Vanguard, 1 = Mage, 2 = Titan
    bool p1HeroConfirmed;
    bool p2HeroConfirmed;
    float heroAnimTimer;

    // === Hàm nội bộ ===
    void processInput();
    void updateMenu(float dt);
    void updateHeroSelect(float dt);
    void updateGathering(float dt);
    void updateClash(float dt);
    void render();

    // Game lifecycle
    void startMatch(GameMode mode, HeroClass p1Hero = HeroClass::VANGUARD, HeroClass p2Hero = HeroClass::VANGUARD);
    void resetGame();

    // Game logic
    void initObstaclesAndTrees();
    void spawnFreeTroops(int count);
    void spawnRandomItem();
    void spawnBomb();
    void checkCollisions();
    void checkBaseZones();
    void checkMagnet(Character* character);
    void handleBombExplosions();
    void handleDragonFire();
    void clampToWorld(float& px, float& py);

    // Phase 2
    void startClash();

    // Render helpers
    void buildTiledBackground();
    void clampCamera(sf::View& cam, float targetX, float targetY);
    void renderWorld();
    void renderMenu();
    void renderHeroSelectPve();
    void renderHeroSelectPvp();
    void renderStoryModal();
    void renderSettingsModal();
    void renderPauseMenu();
    void renderGathering();
    void renderClash();
    void renderResult();
    void renderHUD();

    // === Lưu trữ dữ liệu bền vững (File CSV Persistence) ===
    void saveMatchHistoryToCSV(const std::string& filename = "match_history.csv");

public:
    GameEngine();
    ~GameEngine();
    void run();
};