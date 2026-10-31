#include "GameEngine.h"
#include "ResourceManager.h"
#include "AudioManager.h"
#include "../Items/SpeedItem.h"
#include "../Items/ShieldItem.h"
#include "../Items/MagnetItem.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include <random>
#include <functional>
#include <iostream>
#include <sstream>
#include <fstream>
#include <chrono>

// ================================================================
//  Constructor
// ================================================================
GameEngine::GameEngine()
    : window(sf::VideoMode(1280, 720), "Snake Army 2.0 - Full OOP"),
      currentState(GameState::MENU), gameMode(GameMode::PVE),
      menuModal(MenuModal::NONE), storyTab(StoryTab::STORY),
      hoveredButtonIndex(-1), mousePos(0.f, 0.f),
      player1(nullptr), player2(nullptr), bot(nullptr),
      baseBlue(nullptr), baseRed(nullptr),
      itemSpawnTimer(0.f), bombSpawnTimer(0.f),
      vipDrop(nullptr),
      gatheringTimer(90.f), elapsedTime(0.f),
      clashTimer(0.f), blueTotalPower(0), redTotalPower(0),
      tugOfWarRatio(0.5f), clashMarchTimer(0.f), clashChargeTimer(0.f), clashFrontlineX(640.f),
      screenShakeTimer(0.f), screenShakeIntensity(0.f), clashFightTimer(0.f),
      clashRoutTimer(0.f), clashIsEnding(false), gameFont(nullptr),
      matchBlueBanked(0), matchRedBanked(0), matchBlueClash(0), matchRedClash(0), matchWinner(0),
      matchBlueSwordsman(0), matchBlueGuardian(0), matchBlueCavalry(0), matchBlueTotalTroops(0), matchBlueTotalPower(0),
      matchRedSwordsman(0), matchRedGuardian(0), matchRedCavalry(0), matchRedTotalTroops(0), matchRedTotalPower(0),
      bannerText(""), bannerTimer(0.f), bannerMaxDuration(3.0f), bannerColor(sf::Color(255, 235, 140)),
      bannerAlert10sTriggered(false),
      p1SelectedHero(HeroClass::VANGUARD), p2SelectedHero(HeroClass::VANGUARD),
      heroSelectP1Hover(0), heroSelectP2Hover(0),
      p1HeroConfirmed(false), p2HeroConfirmed(false),
      heroAnimTimer(0.f)
{
    window.setFramerateLimit(60);
    std::srand((unsigned)std::time(nullptr));

    // Preload tất cả asset
    ResourceManager::getInstance().preloadAll();

    // Khởi tạo AudioManager và bắt đầu nhạc nền Menu
    AudioManager::getInstance().init();
    AudioManager::getInstance().playBGM("bgm_menu.ogg");

    // Camera
    camera.setSize(1280.f, 720.f);
    camera.setCenter(640.f, 360.f);
    camera.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

    cameraP1.setSize(640.f, 720.f);
    cameraP1.setCenter(320.f, 360.f);
    cameraP1.setViewport(sf::FloatRect(0.f, 0.f, 0.5f, 1.f));

    cameraP2.setSize(640.f, 720.f);
    cameraP2.setCenter(320.f, 360.f);
    cameraP2.setViewport(sf::FloatRect(0.5f, 0.f, 0.5f, 1.f));

    isSplitScreen = false;
    isPaused = false;

    // Build tiled background
    buildTiledBackground();

    // Load font
    try {
        gameFont = &ResourceManager::getInstance().getFont("arial.ttf");
    } catch (...) {
        gameFont = nullptr;
    }
}

// ================================================================
//  resetGame – Dọn dẹp tài nguyên trận đấu để sẵn sàng trận mới
// ================================================================
void GameEngine::resetGame() {
    delete player1; player1 = nullptr;
    delete player2; player2 = nullptr;
    delete bot; bot = nullptr;
    delete baseBlue; baseBlue = nullptr;
    delete baseRed; baseRed = nullptr;
    delete vipDrop; vipDrop = nullptr;
    for (auto o : obstacles) delete o; obstacles.clear();
    for (auto i : items) delete i; items.clear();
    for (auto b : bombs) delete b; bombs.clear();
    for (auto de : dragonEvents) delete de; dragonEvents.clear();
    dragonSpawnTimer = 0.f;
    for (auto t : freeTroops) delete t; freeTroops.clear();
    for (auto t : blueClashArmy) delete t; blueClashArmy.clear();
    for (auto t : redClashArmy) delete t; redClashArmy.clear();
    clashSlashes.clear();
    clashSparks.clear();
    clashFloatTexts.clear();
    clashDeadTroops.clear();
    clashTimer = 0.f;
    blueTotalPower = 0;
    redTotalPower = 0;
    tugOfWarRatio = 0.5f;
    clashMarchTimer = 0.f;
    clashChargeTimer = 0.f;
    clashFrontlineX = 640.f;
    screenShakeTimer = 0.f;
    screenShakeIntensity = 0.f;
    clashFightTimer = 0.f;
    clashRoutTimer = 0.f;
    clashIsEnding = false;
    isPaused = false;
    resultText = "";
    matchBlueBanked = 0;
    matchRedBanked = 0;
    matchBlueClash = 0;
    matchRedClash = 0;
    matchWinner = 0;
    matchBlueSwordsman = 0;
    matchBlueGuardian = 0;
    matchBlueCavalry = 0;
    matchBlueTotalTroops = 0;
    matchBlueTotalPower = 0;
    matchRedSwordsman = 0;
    matchRedGuardian = 0;
    matchRedCavalry = 0;
    matchRedTotalTroops = 0;
    matchRedTotalPower = 0;
    bannerText = "";
    bannerTimer = 0.f;
    bannerAlert10sTriggered = false;

    AudioManager::getInstance().stopAllSounds();
    AudioManager::getInstance().stopVoice();
    AudioManager::getInstance().playBGM("bgm_menu.ogg");
}

void GameEngine::triggerBanner(const std::string& text, float duration, sf::Color col) {
    bannerText = text;
    bannerTimer = duration;
    bannerMaxDuration = duration;
    bannerColor = col;
}

// ================================================================
//  Destructor
// ================================================================
GameEngine::~GameEngine() {
    resetGame();
}

// ================================================================
//  startMatch – Khởi tạo ván đấu mới (PvE hoặc PvP)
// ================================================================
void GameEngine::startMatch(GameMode mode, HeroClass p1Hero, HeroClass p2Hero) {
    resetGame();
    gameMode = mode;
    p1SelectedHero = p1Hero;
    p2SelectedHero = p2Hero;
    currentState = GameState::GATHERING;
    menuModal = MenuModal::NONE;

    if (gameMode == GameMode::PVE) {
        isSplitScreen = false;
        player1 = new HumanPlayer(300.f, WORLD_H / 2.f, Team::BLUE, 1, p1Hero);
        bot = new BotPlayer(WORLD_W - 300.f, WORLD_H / 2.f, Team::RED,
                            WORLD_W - 150.f, WORLD_H / 2.f, p2Hero);
        baseBlue = new BaseZone(150.f, WORLD_H / 2.f, Team::BLUE);
        baseRed = new BaseZone(WORLD_W - 150.f, WORLD_H / 2.f, Team::RED);

        initObstaclesAndTrees();
        spawnFreeTroops(35);
        vipDrop = new VipDropEvent(WORLD_W / 2.f, WORLD_H / 2.f, 45.f);

        gatheringTimer = 90.f;
        elapsedTime = 0.f;

        clampCamera(camera, 300.f, WORLD_H / 2.f);
        AudioManager::getInstance().playVoice("pve_intro.ogg");
        triggerBanner(u8"GIAI ĐOẠN 1: THU THẬP BINH LỰC (90S)!", 3.5f, sf::Color(255, 235, 140));
    } else {
        isSplitScreen = true;
        player1 = new HumanPlayer(300.f, WORLD_H / 2.f, Team::BLUE, 1, p1Hero);
        player2 = new HumanPlayer(WORLD_W - 300.f, WORLD_H / 2.f, Team::RED, 2, p2Hero);
        baseBlue = new BaseZone(150.f, WORLD_H / 2.f, Team::BLUE);
        baseRed = new BaseZone(WORLD_W - 150.f, WORLD_H / 2.f, Team::RED);

        initObstaclesAndTrees();
        spawnFreeTroops(35);
        vipDrop = new VipDropEvent(WORLD_W / 2.f, WORLD_H / 2.f, 45.f);

        gatheringTimer = 90.f;
        elapsedTime = 0.f;

        clampCamera(cameraP1, 300.f, WORLD_H / 2.f);
        clampCamera(cameraP2, WORLD_W - 300.f, WORLD_H / 2.f);
        AudioManager::getInstance().playVoice("pvp_intro.ogg");
        triggerBanner(u8"ĐẠI HỘI TRANH TÀI: MAU CHIÊU MỘ QUÂN ĐOÀN!", 3.5f, sf::Color(255, 235, 140));
    }

    AudioManager::getInstance().playBGM("bgm_gathering.ogg");
}

// ================================================================
//  run – Game Loop chính
// ================================================================
void GameEngine::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f; // Cap delta time

        // Cập nhật Audio Mixer (Audio Ducking thời gian thực)
        AudioManager::getInstance().update(dt);

        processInput();

        switch (currentState) {
            case GameState::MENU:            updateMenu(dt); break;
            case GameState::HERO_SELECT_PVE:
            case GameState::HERO_SELECT_PVP: updateHeroSelect(dt); break;
            case GameState::GATHERING:       if (!isPaused) updateGathering(dt); break;
            case GameState::CLASH:           if (!isPaused) updateClash(dt); break;
            case GameState::RESULT:          break; // Chờ input
        }

        render();
    }
}

// ================================================================
//  processInput
// ================================================================
void GameEngine::processInput() {
    // Cập nhật tọa độ chuột theo màn hình UI (view cố định 1280x720)
    mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window), window.getDefaultView());

    // Cập nhật trạng thái Hover cho 5 nút Menu chính
    if (currentState == GameState::MENU && menuModal == MenuModal::NONE) {
        int oldHovered = hoveredButtonIndex;
        hoveredButtonIndex = -1;
        for (int i = 0; i < 5; ++i) {
            float btnY = 330.f + i * 70.f;
            sf::FloatRect btnRect(490.f, btnY - 30.f, 300.f, 60.f);
            if (btnRect.contains(mousePos)) {
                hoveredButtonIndex = i;
                break;
            }
        }
        if (hoveredButtonIndex != -1 && hoveredButtonIndex != oldHovered) {
            AudioManager::getInstance().playSound("sfx_hover.ogg");
        }
    }
    // Hover cho Chọn Tướng PvE
    else if (currentState == GameState::HERO_SELECT_PVE) {
        int oldHover = heroSelectP1Hover;
        heroSelectP1Hover = -1;
        sf::FloatRect c0(110.f, 112.f, 320.f, 465.f);
        sf::FloatRect c1(480.f, 112.f, 320.f, 465.f);
        sf::FloatRect c2(850.f, 112.f, 320.f, 465.f);
        if (c0.contains(mousePos)) heroSelectP1Hover = 0;
        else if (c1.contains(mousePos)) heroSelectP1Hover = 1;
        else if (c2.contains(mousePos)) heroSelectP1Hover = 2;

        if (heroSelectP1Hover != -1 && heroSelectP1Hover != oldHover) {
            AudioManager::getInstance().playSound("sfx_hover.ogg");
        }
    }
    // Hover cho Chọn Tướng PvP
    else if (currentState == GameState::HERO_SELECT_PVP) {
        int oldP1 = heroSelectP1Hover;
        int oldP2 = heroSelectP2Hover;
        heroSelectP1Hover = -1;
        heroSelectP2Hover = -1;
        sf::FloatRect p1c0(35.f, 116.f, 182.f, 458.f);
        sf::FloatRect p1c1(227.f, 116.f, 182.f, 458.f);
        sf::FloatRect p1c2(419.f, 116.f, 182.f, 458.f);
        if (p1c0.contains(mousePos)) heroSelectP1Hover = 0;
        else if (p1c1.contains(mousePos)) heroSelectP1Hover = 1;
        else if (p1c2.contains(mousePos)) heroSelectP1Hover = 2;

        sf::FloatRect p2c0(679.f, 116.f, 182.f, 458.f);
        sf::FloatRect p2c1(871.f, 116.f, 182.f, 458.f);
        sf::FloatRect p2c2(1063.f, 116.f, 182.f, 458.f);
        if (p2c0.contains(mousePos)) heroSelectP2Hover = 0;
        else if (p2c1.contains(mousePos)) heroSelectP2Hover = 1;
        else if (p2c2.contains(mousePos)) heroSelectP2Hover = 2;

        if ((heroSelectP1Hover != -1 && heroSelectP1Hover != oldP1) ||
            (heroSelectP2Hover != -1 && heroSelectP2Hover != oldP2)) {
            AudioManager::getInstance().playSound("sfx_hover.ogg");
        }
    }

    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();

        // -------------------------------------------------------------
        //  XỬ LÝ CLICK CHUỘT
        // -------------------------------------------------------------
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (currentState == GameState::MENU) {
                if (menuModal == MenuModal::NONE) {
                    for (int i = 0; i < 5; ++i) {
                        float btnY = 330.f + i * 70.f;
                        sf::FloatRect btnRect(490.f, btnY - 30.f, 300.f, 60.f);
                        if (btnRect.contains(mousePos)) {
                            AudioManager::getInstance().playSound("sfx_click.ogg");
                            if (i == 0) {
                                currentState = GameState::HERO_SELECT_PVE;
                                p1SelectedHero = HeroClass::VANGUARD;
                                heroSelectP1Hover = 0;
                                p1HeroConfirmed = false;
                            }
                            else if (i == 1) {
                                currentState = GameState::HERO_SELECT_PVP;
                                p1SelectedHero = HeroClass::VANGUARD;
                                p2SelectedHero = HeroClass::VANGUARD;
                                heroSelectP1Hover = 0;
                                heroSelectP2Hover = 0;
                                p1HeroConfirmed = false;
                                p2HeroConfirmed = false;
                            }
                            else if (i == 2) { menuModal = MenuModal::STORY; storyTab = StoryTab::STORY; }
                            else if (i == 3) { menuModal = MenuModal::SETTINGS; }
                            else if (i == 4) window.close();
                            break;
                        }
                    }
                }
                else if (menuModal == MenuModal::STORY) {
                    sf::FloatRect panelRect(210.f, 90.f, 860.f, 540.f);
                    sf::FloatRect tab1Rect(355.f, 162.f, 230.f, 42.f);
                    sf::FloatRect tab2Rect(695.f, 162.f, 230.f, 42.f);
                    sf::FloatRect closeBtnRect(510.f, 505.f, 260.f, 48.f);

                    if (tab1Rect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        storyTab = StoryTab::STORY;
                    } else if (tab2Rect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        storyTab = StoryTab::RULES;
                    } else if (closeBtnRect.contains(mousePos) || !panelRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        menuModal = MenuModal::NONE;
                    }
                }
                else if (menuModal == MenuModal::SETTINGS) {
                    sf::FloatRect panelRect(210.f, 90.f, 860.f, 540.f);
                    sf::FloatRect musicToggleRect(550.f, 215.f, 90.f, 40.f);
                    sf::FloatRect musicMinusRect(670.f, 215.f, 40.f, 40.f);
                    sf::FloatRect musicPlusRect(800.f, 215.f, 40.f, 40.f);

                    sf::FloatRect sfxToggleRect(550.f, 280.f, 90.f, 40.f);
                    sf::FloatRect sfxMinusRect(670.f, 280.f, 40.f, 40.f);
                    sf::FloatRect sfxPlusRect(800.f, 280.f, 40.f, 40.f);

                    sf::FloatRect langToggleRect(550.f, 350.f, 290.f, 42.f);
                    sf::FloatRect closeBtnRect(510.f, 505.f, 260.f, 48.f);

                    if (musicToggleRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleMusic();
                    } else if (musicMinusRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustMusicVolume(-10);
                    } else if (musicPlusRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustMusicVolume(+10);
                    } else if (sfxToggleRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleSfx();
                    } else if (sfxMinusRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustSfxVolume(-10);
                    } else if (sfxPlusRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustSfxVolume(+10);
                    } else if (langToggleRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleLanguage();
                    } else if (closeBtnRect.contains(mousePos) || !panelRect.contains(mousePos)) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        menuModal = MenuModal::NONE;
                    }
                }
            }
            else if (currentState == GameState::HERO_SELECT_PVE) {
                sf::FloatRect c0(110.f, 112.f, 320.f, 465.f);
                sf::FloatRect c1(480.f, 112.f, 320.f, 465.f);
                sf::FloatRect c2(850.f, 112.f, 320.f, 465.f);
                sf::FloatRect backBtn(110.f, 592.f, 180.f, 50.f);
                sf::FloatRect startBtn(490.f, 588.f, 300.f, 58.f);

                if (c0.contains(mousePos)) {
                    p1SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (c1.contains(mousePos)) {
                    p1SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (c2.contains(mousePos)) {
                    p1SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (backBtn.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    currentState = GameState::MENU;
                } else if (startBtn.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    HeroClass botHero = static_cast<HeroClass>(std::rand() % 3);
                    startMatch(GameMode::PVE, p1SelectedHero, botHero);
                }
            }
            else if (currentState == GameState::HERO_SELECT_PVP) {
                sf::FloatRect p1c0(35.f, 116.f, 182.f, 458.f);
                sf::FloatRect p1c1(227.f, 116.f, 182.f, 458.f);
                sf::FloatRect p1c2(419.f, 116.f, 182.f, 458.f);
                if (p1c0.contains(mousePos)) {
                    p1SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (p1c1.contains(mousePos)) {
                    p1SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (p1c2.contains(mousePos)) {
                    p1SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                }

                sf::FloatRect p2c0(679.f, 116.f, 182.f, 458.f);
                sf::FloatRect p2c1(871.f, 116.f, 182.f, 458.f);
                sf::FloatRect p2c2(1063.f, 116.f, 182.f, 458.f);
                if (p2c0.contains(mousePos)) {
                    p2SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (p2c1.contains(mousePos)) {
                    p2SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (p2c2.contains(mousePos)) {
                    p2SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                }

                sf::FloatRect backBtn(60.f, 592.f, 160.f, 48.f);
                sf::FloatRect startBtn(490.f, 588.f, 300.f, 56.f);
                if (backBtn.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    currentState = GameState::MENU;
                } else if (startBtn.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    startMatch(GameMode::PVP, p1SelectedHero, p2SelectedHero);
                }
            }
            else if (currentState == GameState::RESULT) {
                sf::FloatRect replayRect(360.f, 516.f, 240.f, 46.f);
                sf::FloatRect menuRect(680.f, 516.f, 240.f, 46.f);
                if (replayRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    startMatch(gameMode, p1SelectedHero, p2SelectedHero);
                } else if (menuRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    resetGame();
                    currentState = GameState::MENU;
                    menuModal = MenuModal::NONE;
                }
            }
            else if ((currentState == GameState::GATHERING || currentState == GameState::CLASH) && isPaused) {
                sf::FloatRect resumeBtnRect(495.f, 206.f, 290.f, 48.f);
                sf::FloatRect musicToggleRect(540.f, 280.f, 90.f, 36.f);
                sf::FloatRect musicMinusRect(660.f, 280.f, 36.f, 36.f);
                sf::FloatRect musicPlusRect(790.f, 280.f, 36.f, 36.f);

                sf::FloatRect sfxToggleRect(540.f, 332.f, 90.f, 36.f);
                sf::FloatRect sfxMinusRect(660.f, 332.f, 36.f, 36.f);
                sf::FloatRect sfxPlusRect(790.f, 332.f, 36.f, 36.f);

                sf::FloatRect langToggleRect(540.f, 384.f, 286.f, 36.f);
                sf::FloatRect quitBtnRect(495.f, 424.f, 290.f, 48.f);

                if (resumeBtnRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    isPaused = false;
                    AudioManager::getInstance().resumeBGM();
                } else if (musicToggleRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().toggleMusic();
                } else if (musicMinusRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().adjustMusicVolume(-10);
                } else if (musicPlusRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().adjustMusicVolume(+10);
                } else if (sfxToggleRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().toggleSfx();
                } else if (sfxMinusRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().adjustSfxVolume(-10);
                } else if (sfxPlusRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().adjustSfxVolume(+10);
                } else if (langToggleRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    ConfigManager::getInstance().toggleLanguage();
                } else if (quitBtnRect.contains(mousePos)) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    resetGame();
                    currentState = GameState::MENU;
                    menuModal = MenuModal::NONE;
                    isPaused = false;
                }
            }
        }

        // -------------------------------------------------------------
        //  XỬ LÝ BÀN PHÍM
        // -------------------------------------------------------------
        if (event.type == sf::Event::KeyPressed) {
            if (currentState == GameState::MENU) {
                if (menuModal == MenuModal::NONE) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        currentState = GameState::HERO_SELECT_PVE;
                        p1SelectedHero = HeroClass::VANGUARD;
                        heroSelectP1Hover = 0;
                        p1HeroConfirmed = false;
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                    } else if (event.key.code == sf::Keyboard::Num2) {
                        currentState = GameState::HERO_SELECT_PVP;
                        p1SelectedHero = HeroClass::VANGUARD;
                        p2SelectedHero = HeroClass::VANGUARD;
                        heroSelectP1Hover = 0;
                        heroSelectP2Hover = 0;
                        p1HeroConfirmed = false;
                        p2HeroConfirmed = false;
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                    } else if (event.key.code == sf::Keyboard::Num3) {
                        menuModal = MenuModal::STORY;
                        storyTab = StoryTab::STORY;
                    } else if (event.key.code == sf::Keyboard::Num4) {
                        menuModal = MenuModal::SETTINGS;
                    } else if (event.key.code == sf::Keyboard::Num5 || event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }
                }
                else if (menuModal == MenuModal::STORY) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        storyTab = StoryTab::STORY;
                    } else if (event.key.code == sf::Keyboard::Num2) {
                        storyTab = StoryTab::RULES;
                    } else if (event.key.code == sf::Keyboard::Tab) {
                        storyTab = (storyTab == StoryTab::STORY) ? StoryTab::RULES : StoryTab::STORY;
                    } else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        menuModal = MenuModal::NONE;
                    }
                }
                else if (menuModal == MenuModal::SETTINGS) {
                    if (event.key.code == sf::Keyboard::Num1) {
                        ConfigManager::getInstance().toggleMusic();
                    } else if (event.key.code == sf::Keyboard::Num2) {
                        ConfigManager::getInstance().toggleSfx();
                    } else if (event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Left) {
                        ConfigManager::getInstance().adjustMusicVolume(-10);
                    } else if (event.key.code == sf::Keyboard::Num4 || event.key.code == sf::Keyboard::Right) {
                        ConfigManager::getInstance().adjustMusicVolume(+10);
                    } else if (event.key.code == sf::Keyboard::Num5) {
                        ConfigManager::getInstance().toggleLanguage();
                    } else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        menuModal = MenuModal::NONE;
                    }
                }
            }
            else if (currentState == GameState::HERO_SELECT_PVE) {
                if (event.key.code == sf::Keyboard::Num1) {
                    p1SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Num2) {
                    p1SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Num3) {
                    p1SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::Left) {
                    int cur = static_cast<int>(p1SelectedHero);
                    cur = (cur + 2) % 3;
                    p1SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                } else if (event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::Right) {
                    int cur = static_cast<int>(p1SelectedHero);
                    cur = (cur + 1) % 3;
                    p1SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                } else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    HeroClass botHero = static_cast<HeroClass>(std::rand() % 3);
                    startMatch(GameMode::PVE, p1SelectedHero, botHero);
                } else if (event.key.code == sf::Keyboard::Escape) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    currentState = GameState::MENU;
                }
            }
            else if (currentState == GameState::HERO_SELECT_PVP) {
                // P1 phím: 1, 2, 3 hoặc A / D
                if (event.key.code == sf::Keyboard::Num1) {
                    p1SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Num2) {
                    p1SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Num3) {
                    p1SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::A) {
                    int cur = static_cast<int>(p1SelectedHero);
                    cur = (cur + 2) % 3;
                    p1SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                } else if (event.key.code == sf::Keyboard::D) {
                    int cur = static_cast<int>(p1SelectedHero);
                    cur = (cur + 1) % 3;
                    p1SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                }

                // P2 phím: Numpad 1, 2, 3 hoặc Left / Right / J / L
                if (event.key.code == sf::Keyboard::Numpad1) {
                    p2SelectedHero = HeroClass::VANGUARD;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Numpad2) {
                    p2SelectedHero = HeroClass::TACTICIAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Numpad3) {
                    p2SelectedHero = HeroClass::TITAN;
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                } else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::J) {
                    int cur = static_cast<int>(p2SelectedHero);
                    cur = (cur + 2) % 3;
                    p2SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                } else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::L) {
                    int cur = static_cast<int>(p2SelectedHero);
                    cur = (cur + 1) % 3;
                    p2SelectedHero = static_cast<HeroClass>(cur);
                    AudioManager::getInstance().playSound("sfx_hover.ogg");
                }

                // Khai chiến hoặc Thoát
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    startMatch(GameMode::PVP, p1SelectedHero, p2SelectedHero);
                } else if (event.key.code == sf::Keyboard::Escape) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    currentState = GameState::MENU;
                }
            }
            else if (currentState == GameState::GATHERING || currentState == GameState::CLASH) {
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P) {
                    isPaused = !isPaused;
                    AudioManager::getInstance().playSound("sfx_pause.ogg");
                    if (isPaused) {
                        AudioManager::getInstance().pauseBGM();
                    } else {
                        AudioManager::getInstance().resumeBGM();
                    }
                }
                else if (!isPaused && currentState == GameState::GATHERING) {
                    // === Kỹ năng Tướng "Đoạt Mệnh Trảm" ===
                    // Player 1: Phím Space
                    if (event.key.code == sf::Keyboard::Space) {
                        if (player1) player1->triggerSkill();
                    }
                    // Player 2: Phím Right Shift hoặc Numpad 0 / Enter
                    if (event.key.code == sf::Keyboard::RShift || event.key.code == sf::Keyboard::RControl || event.key.code == sf::Keyboard::Numpad0) {
                        if (player2 && gameMode == GameMode::PVP) player2->triggerSkill();
                    }
                }
                else if (isPaused) {
                    if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        isPaused = false;
                        AudioManager::getInstance().resumeBGM();
                    } else if (event.key.code == sf::Keyboard::M || event.key.code == sf::Keyboard::Q) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        resetGame();
                        currentState = GameState::MENU;
                        menuModal = MenuModal::NONE;
                        isPaused = false;
                    } else if (event.key.code == sf::Keyboard::Num1) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleMusic();
                    } else if (event.key.code == sf::Keyboard::Num2) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleSfx();
                    } else if (event.key.code == sf::Keyboard::Num3 || event.key.code == sf::Keyboard::Left) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustMusicVolume(-10);
                    } else if (event.key.code == sf::Keyboard::Num4 || event.key.code == sf::Keyboard::Right) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().adjustMusicVolume(+10);
                    } else if (event.key.code == sf::Keyboard::Num5) {
                        AudioManager::getInstance().playSound("sfx_click.ogg");
                        ConfigManager::getInstance().toggleLanguage();
                    }
                }
            }
            else if (currentState == GameState::RESULT) {
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    startMatch(gameMode, p1SelectedHero, p2SelectedHero);
                } else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::M) {
                    AudioManager::getInstance().playSound("sfx_click.ogg");
                    resetGame();
                    currentState = GameState::MENU;
                    menuModal = MenuModal::NONE;
                }
            }
        }
    }
}

// ================================================================
//  updateMenu
// ================================================================
void GameEngine::updateMenu(float dt) {
    // Menu tương tác linh hoạt với chuột và phím
}

// ================================================================
//  updateHeroSelect – Hiệu ứng chuyển động nhân vật tại màn hình chọn tướng
// ================================================================
void GameEngine::updateHeroSelect(float dt) {
    heroAnimTimer += dt;
}

// ================================================================
//  updateGathering – Phase 1: Thu thập lính 90 giây
// ================================================================
void GameEngine::updateGathering(float dt) {
    elapsedTime += dt;
    gatheringTimer -= dt;

    // Giảm timer banner thông báo sự kiện
    if (bannerTimer > 0.f) {
        bannerTimer -= dt;
    }

    // Cảnh báo 10 giây cuối cùng
    if (gatheringTimer <= 10.f && !bannerAlert10sTriggered) {
        bannerAlert10sTriggered = true;
        triggerBanner(u8"10 GIÂY CUỐI CÙNG - CHUẨN BỊ ĐẠI CHIẾN!", 3.2f, sf::Color(255, 230, 80));
    }

    // Hết giờ → chuyển Phase 2
    if (gatheringTimer <= 0.f) {
        startClash();
        return;
    }

    // Cập nhật trạng thái ẩn nấp trong bụi rậm (Bush Stealth)
    auto checkBushStealth = [&](Character* ch) {
        if (!ch) return;
        bool wasInBush = ch->isInBush();
        bool inBush = false;
        for (auto obs : obstacles) {
            if (obs && obs->isBush()) {
                Bush* b = static_cast<Bush*>(obs);
                if (b->isInside(ch->getX(), ch->getY())) {
                    inBush = true;
                    break;
                }
            }
        }
        if (!wasInBush && inBush) {
            AudioManager::getInstance().playSound("sfx_bush.ogg");
        }
        ch->setInBush(inBush);
    };
    checkBushStealth(player1);
    if (player2) checkBushStealth(player2);
    if (bot) checkBushStealth(bot);

    // Update players & Xử lý va chạm vật cản (Đá, Cây, Tàn tích)
    player1->update(dt);
    player1->resolveObstacles(obstacles);
    float p1x = player1->getX(), p1y = player1->getY();
    clampToWorld(p1x, p1y);
    player1->setPosition(p1x, p1y);
    player1->updateArmy(dt);

    if (gameMode == GameMode::PVP && player2) {
        player2->update(dt);
        player2->resolveObstacles(obstacles);
        float p2x = player2->getX(), p2y = player2->getY();
        clampToWorld(p2x, p2y);
        player2->setPosition(p2x, p2y);
        player2->updateArmy(dt);
    }
    if (gameMode == GameMode::PVE && bot) {
        // Cung cấp context cho AI (bao gồm vùng bom, vùng đất cháy, và các vùng lửa rồng)
        std::vector<sf::Vector2f> ftPos, itPos, bzPos;
        for (auto t : freeTroops) ftPos.push_back(sf::Vector2f(t->x, t->y));
        for (auto i : items) if (!i->isCollected()) itPos.push_back(sf::Vector2f(i->getX(), i->getY()));
        for (auto b : bombs) if (b->isWarning() || b->isScorched()) bzPos.push_back(sf::Vector2f(b->getX(), b->getY()));
        for (auto de : dragonEvents) {
            for (const auto& fz : de->getFireZones()) {
                if (fz.isActive()) bzPos.push_back(sf::Vector2f(fz.x, fz.y));
            }
        }

        bot->think(dt, player1->getX(), player1->getY(),
                   player1->getArmy().getCount(), player1->getIsInvincible(),
                   ftPos, itPos, bzPos, obstacles, player1->isInBush(),
                   elapsedTime);
        bot->update(dt);
        bot->resolveObstacles(obstacles);
        float bx = bot->getX(), by = bot->getY();
        clampToWorld(bx, by);
        bot->setPosition(bx, by);
        bot->updateArmy(dt);
    }

    // Reset tốc độ Tướng về bình thường (nếu không bị vùng đất cháy hoặc lửa rồng làm chậm)
    if (player1) player1->resetSpeed();
    if (player2) player2->resetSpeed();
    if (bot) bot->resetSpeed();

    // ================================================================
    //  TƯƠNG TÁC KỸ NĂNG ĐẶC THÙ CỦA TƯỚNG (HERO ABILITY INTERACTIONS)
    // ================================================================
    auto processHeroSkills = [&](Character* hero, Character* enemy) {
        if (!hero) return;

        // 1. Quân Sư: Bão Xoáy Bát Quái Hấp Tinh (Swirling Vortex Pull & Slow)
        if (hero->getIsVortexActive()) {
            float cx = hero->getX();
            float cy = hero->getY();
            float vr = hero->getVortexRadius(); // 280.f

            // Hút lính tự do trong tầm 280px về phía Tướng
            for (auto t : freeTroops) {
                float dx = cx - t->x;
                float dy = cy - t->y;
                float dist = std::hypot(dx, dy);
                if (dist > 5.f && dist <= vr) {
                    float pullSpeed = 195.f * dt;
                    t->x += (dx / dist) * pullSpeed;
                    t->y += (dy / dist) * pullSpeed;
                    t->sprite.setPosition(t->x, t->y);
                }
            }

            // Hút vật phẩm trong tầm 280px về phía Tướng
            for (auto i : items) {
                if (!i->isCollected()) {
                    float dx = cx - i->getX();
                    float dy = cy - i->getY();
                    float dist = std::hypot(dx, dy);
                    if (dist > 5.f && dist <= vr) {
                        float pullSpeed = 165.f * dt;
                        i->setPosition(i->getX() + (dx / dist) * pullSpeed, i->getY() + (dy / dist) * pullSpeed);
                    }
                }
            }

            // Làm chậm đối thủ 60% khi lọt vào trận pháp
            if (enemy) {
                float dx = cx - enemy->getX();
                float dy = cy - enemy->getY();
                if (std::hypot(dx, dy) <= vr) {
                    enemy->applySlow(0.40f); // Tốc độ giảm 60% (còn 40%)
                }
            }
        }

        // 2. Hộ Quốc: Kim Cang Hống (Warcry Enemy Stun)
        if (hero->getIsWarcryActive() && enemy) {
            float dx = hero->getX() - enemy->getX();
            float dy = hero->getY() - enemy->getY();
            float dist = std::hypot(dx, dy);
            if (dist <= hero->getWarcryRadius() && !enemy->getIsStunned() && !enemy->getIsInvincible()) {
                enemy->stun(1.5f);
                clashFloatTexts.push_back({ enemy->getX(), enemy->getY() - 35.f, u8"CHOÁNG VÁNG! (1.5s)", 1.2f, 1.2f, sf::Color(255, 220, 80) });
                AudioManager::getInstance().playSound("sfx_clash_hit.ogg");
            }
        }
    };

    Character* enemyP1 = (gameMode == GameMode::PVP) ? static_cast<Character*>(player2) : static_cast<Character*>(bot);
    processHeroSkills(player1, enemyP1);
    if (gameMode == GameMode::PVP && player2) {
        processHeroSkills(player2, player1);
    } else if (bot) {
        processHeroSkills(bot, player1);
    }

    // Giảm timer rung chấn màn hình
    if (screenShakeTimer > 0.f) {
        screenShakeTimer -= dt;
    }

    // Cập nhật hiệu ứng chém, tia lửa và chữ nổi bay lên trong thế giới
    for (auto it = clashSlashes.begin(); it != clashSlashes.end(); ) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.f) it = clashSlashes.erase(it);
        else ++it;
    }
    for (auto it = clashSparks.begin(); it != clashSparks.end(); ) {
        it->lifetime -= dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        if (it->lifetime <= 0.f) it = clashSparks.erase(it);
        else ++it;
    }
    for (auto it = clashFloatTexts.begin(); it != clashFloatTexts.end(); ) {
        it->lifetime -= dt;
        it->y -= 25.f * dt;
        if (it->lifetime <= 0.f) it = clashFloatTexts.erase(it);
        else ++it;
    }

    // Update items
    for (auto i : items) i->update(dt);

    // Spawn thêm lính tự do theo thời gian (giữ map 2800x2000 luôn sôi động)
    itemSpawnTimer += dt;
    if (itemSpawnTimer >= 4.5f && freeTroops.size() < 45) {
        spawnFreeTroops(4);
        itemSpawnTimer = 0.f;
    }

    // Spawn item ngẫu nhiên
    bombSpawnTimer += dt;
    if (bombSpawnTimer >= 7.5f) {
        if (items.size() < 7) spawnRandomItem();
        bombSpawnTimer = 0.f;
    }

    // Pháo kích nhắm mục tiêu (Targeted Airstrike: kích hoạt từ giây thứ 15)
    if (elapsedTime > 15.f) {
        static float nextBombTime = 15.f;
        if (elapsedTime >= nextBombTime && bombs.size() < 4) {
            spawnBomb();
            nextBombTime = elapsedTime + 11.f + (float)(std::rand() % 5);
        }
    }

    // VIP Drop event
    if (vipDrop && vipDrop->update(dt, elapsedTime)) {
        auto vips = vipDrop->spawnVipTroops();
        for (auto v : vips) freeTroops.push_back(v);
        triggerBanner(u8"TIẾP TẾ CHIẾN LƯỢC ĐÃ RƠI - TRANH ĐOẠT NGAY!", 3.2f, sf::Color(100, 255, 180));
        if (gameMode == GameMode::PVE) {
            AudioManager::getInstance().playVoice("pve_drop.ogg");
        } else {
            AudioManager::getInstance().playVoice("pvp_drop.ogg");
        }
    }

    // Update bombs
    for (auto b : bombs) b->update(dt);
    handleBombExplosions();

    // === SỰ KIỆN HỎA LONG SA TRƯỜNG (Dragon Airstrike) ===
    // Xuất hiện 2 lần trong 90s: ở giây ~24s và ~62s
    static bool dragonSpawned1 = false;
    static bool dragonSpawned2 = false;
    if (elapsedTime < 1.0f) {
        dragonSpawned1 = false;
        dragonSpawned2 = false;
    }
    if (!dragonSpawned1 && elapsedTime >= 24.f) {
        // Rồng săn lùng người chơi đang dẫn đầu (nhiều lính nhất)
        float targetX = player1->getX();
        float targetY = player1->getY();
        int p1Score = player1->getArmy().getCount() + (baseBlue ? baseBlue->getStoredCount() : 0);
        int p2Score = 0;
        if (player2) {
            p2Score = player2->getArmy().getCount() + (baseRed ? baseRed->getStoredCount() : 0);
            if (p2Score > p1Score) {
                targetX = player2->getX();
                targetY = player2->getY();
            }
        } else if (bot) {
            p2Score = bot->getArmy().getCount() + (baseRed ? baseRed->getStoredCount() : 0);
            if (p2Score > p1Score) {
                targetX = bot->getX();
                targetY = bot->getY();
            }
        }
        dragonEvents.push_back(new DragonEvent(WORLD_W, WORLD_H, targetX, targetY));
        dragonSpawned1 = true;
        triggerBanner(u8"HỎA LONG GIÁNG LÂM - MAU TÌM NƠI ẨN NẤP!", 3.2f, sf::Color(255, 120, 80));
    }
    if (!dragonSpawned2 && elapsedTime >= 62.f) {
        float targetX = player1->getX();
        float targetY = player1->getY();
        int p1Score = player1->getArmy().getCount() + (baseBlue ? baseBlue->getStoredCount() : 0);
        int p2Score = 0;
        if (player2) {
            p2Score = player2->getArmy().getCount() + (baseRed ? baseRed->getStoredCount() : 0);
            if (p2Score > p1Score) {
                targetX = player2->getX();
                targetY = player2->getY();
            }
        } else if (bot) {
            p2Score = bot->getArmy().getCount() + (baseRed ? baseRed->getStoredCount() : 0);
            if (p2Score > p1Score) {
                targetX = bot->getX();
                targetY = bot->getY();
            }
        }
        dragonEvents.push_back(new DragonEvent(WORLD_W, WORLD_H, targetX, targetY));
        dragonSpawned2 = true;
        triggerBanner(u8"CỔ LONG TÁI XUẤT - BÃO LỬA RỰC TRỜI!", 3.2f, sf::Color(255, 120, 80));
    }

    // Cập nhật sự kiện rồng
    for (auto de : dragonEvents) {
        de->update(dt, WORLD_W, WORLD_H);
    }
    handleDragonFire();

    // Collisions
    checkCollisions();
    checkBaseZones();

    // Magnet effect
    checkMagnet(player1);
    if (player2) checkMagnet(player2);
    if (bot) checkMagnet(bot);

    // ================================================================
    //  Cập nhật Camera (PvE vs PvP Dynamic Split-Screen)
    // ================================================================
    if (gameMode == GameMode::PVP && player2) {
        float p1x = player1->getX(), p1y = player1->getY();
        float p2x = player2->getX(), p2y = player2->getY();
        float dx = std::abs(p1x - p2x);
        float dy = std::abs(p1y - p2y);
        float dist = std::sqrt(dx * dx + dy * dy);

        // Cơ chế Hysteresis:
        // Đang chia màn hình -> Gộp lại khi 2 tướng đến gần nhau (nhìn thấy nhau)
        // Màn hình 1280x720: dx < 780 và dy < 460 thì cả 2 tướng đều nằm gọn trong tầm nhìn màn hình chung
        if (isSplitScreen) {
            if (dx < 780.f && dy < 460.f && dist < 750.f) {
                isSplitScreen = false;
            }
        } else {
            // Đang gộp chung -> Tách đôi màn hình khi 2 tướng đi xa nhau
            if (dx > 950.f || dy > 520.f || dist > 900.f) {
                isSplitScreen = true;
            }
        }

        if (isSplitScreen) {
            // Chia đôi: Camera P1 nửa trái, Camera P2 nửa phải
            clampCamera(cameraP1, p1x, p1y);
            clampCamera(cameraP2, p2x, p2y);
        } else {
            // Gộp chung: Camera nằm ở trung điểm 2 tướng, hiển thị cả 2 tướng cùng lúc
            float midX = (p1x + p2x) / 2.f;
            float midY = (p1y + p2y) / 2.f;
            clampCamera(camera, midX, midY);
        }
    } else {
        // PvE: Camera theo dõi Player 1
        clampCamera(camera, player1->getX(), player1->getY());
    }

    // Cleanup bombs finished
    bombs.erase(
        std::remove_if(bombs.begin(), bombs.end(),
            [](BombHazard* b) { if (b->isFinished()) { delete b; return true; } return false; }),
        bombs.end()
    );

    // Cleanup dragon events finished
    dragonEvents.erase(
        std::remove_if(dragonEvents.begin(), dragonEvents.end(),
            [](DragonEvent* de) { if (de->isFinished()) { delete de; return true; } return false; }),
        dragonEvents.end()
    );

    // Cleanup collected items
    items.erase(
        std::remove_if(items.begin(), items.end(),
            [](Item* i) { if (i->isCollected()) { delete i; return true; } return false; }),
        items.end()
    );
}

// ================================================================
//  updateClash – Phase 2: Đại chiến Giáp Lá Cà (Dynamic Charge & Combat)
// ================================================================
void GameEngine::updateClash(float dt) {
    clashTimer += dt;

    // Giảm timer banner thông báo sự kiện
    if (bannerTimer > 0.f) {
        bannerTimer -= dt;
    }

    // Cập nhật rung chấn màn hình
    if (screenShakeTimer > 0.f) {
        screenShakeTimer -= dt;
    }

    // Cập nhật các hiệu ứng chém (Slashes)
    for (auto it = clashSlashes.begin(); it != clashSlashes.end(); ) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.f) it = clashSlashes.erase(it);
        else ++it;
    }

    // Cập nhật tia lửa (Sparks)
    for (auto it = clashSparks.begin(); it != clashSparks.end(); ) {
        it->lifetime -= dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->vy += 220.f * dt; // Trọng lực nhẹ
        if (it->lifetime <= 0.f) it = clashSparks.erase(it);
        else ++it;
    }

    // Cập nhật chữ bay (Floating Texts)
    for (auto it = clashFloatTexts.begin(); it != clashFloatTexts.end(); ) {
        it->lifetime -= dt;
        it->y -= 35.f * dt; // Bay lên từ từ
        if (it->lifetime <= 0.f) it = clashFloatTexts.erase(it);
        else ++it;
    }

    // Cập nhật lính bị hạ gục văng ra sau (Dead Troops knockback & fade)
    for (auto it = clashDeadTroops.begin(); it != clashDeadTroops.end(); ) {
        it->lifetime -= dt;
        it->x += it->vx * dt;
        it->y += it->vy * dt;
        it->alpha -= 520.f * dt;
        if (it->alpha < 0.f) it->alpha = 0.f;
        if (it->lifetime <= 0.f || it->alpha <= 0.f) {
            it = clashDeadTroops.erase(it);
        } else {
            ++it;
        }
    }

    // Cập nhật animation bước chân cho Tướng và Lính 2 bên (luôn nhìn thẳng về phía đối phương)
    float animSpeed = (clashMarchTimer > 0.f) ? 0.95f : 1.8f;
    if (player1) player1->updateHeadAnimation(dt * animSpeed, 1.f, 0.f); // Tướng Xanh luôn nhìn sang Phải

    Character* redLeader = (gameMode == GameMode::PVP && player2) ? (Character*)player2 : (Character*)bot;
    if (redLeader) redLeader->updateHeadAnimation(dt * animSpeed, -1.f, 0.f); // Tướng Đỏ luôn nhìn sang Trái

    for (auto t : blueClashArmy) {
        t->animRow = 2; // Quân Xanh luôn nhìn sang Phải
        t->updateAnimation(dt * animSpeed);
    }
    for (auto t : redClashArmy) {
        t->animRow = 1; // Quân Đỏ luôn nhìn sang Trái
        t->updateAnimation(dt * animSpeed);
    }

    // Tính toán mục tiêu chiến tuyến dựa trên tỉ lệ Tug of War
    int totalPow = blueTotalPower + redTotalPower;
    if (totalPow > 0) {
        tugOfWarRatio = (float)blueTotalPower / totalPow;
    }
    float targetFrontlineX = 640.f + (tugOfWarRatio - 0.5f) * 550.f;
    // Làm mượt chuyển động chiến tuyến (Lerp)
    clashFrontlineX += (targetFrontlineX - clashFrontlineX) * std::min(1.f, dt * 4.f);

    float centerY = 460.f; // Trục Y trung tâm đấu trường

    // ================================================================
    //  Giai đoạn 1A: Hai bên từ từ bước ra dàn trận (March Out: 2.4s)
    //  Hai Tướng luôn dẫn đầu đi ở phía trước, lính theo sau thành hàng lối
    // ================================================================
    if (clashMarchTimer > 0.f) {
        clashMarchTimer -= dt;

        float marchSpeed = 200.f; // Nhịp bước hành quân oai vệ

        // Tướng Xanh dẫn đầu tiến ra điểm dàn trận x = 420.f
        if (player1) {
            float p1x = player1->getX() + marchSpeed * dt;
            if (p1x > 420.f) p1x = 420.f;
            player1->setPosition(p1x, centerY);

            // Toàn bộ lính Xanh di chuyển theo sau lưng Tướng Xanh
            for (size_t i = 0; i < blueClashArmy.size(); ++i) {
                int col = (int)(i / 5);
                int row = (int)(i % 5);
                float targetX = p1x - 65.f - col * 44.f;
                float targetY = centerY + (row - 2) * 44.f;
                blueClashArmy[i]->x += (targetX - blueClashArmy[i]->x) * std::min(1.f, dt * 10.f);
                blueClashArmy[i]->y += (targetY - blueClashArmy[i]->y) * std::min(1.f, dt * 10.f);
                blueClashArmy[i]->sprite.setPosition(blueClashArmy[i]->x, blueClashArmy[i]->y);
            }
        }

        // Tướng Đỏ dẫn đầu tiến ra điểm dàn trận x = 860.f
        if (redLeader) {
            float redX = redLeader->getX() - marchSpeed * dt;
            if (redX < 860.f) redX = 860.f;
            redLeader->setPosition(redX, centerY);

            // Toàn bộ lính Đỏ di chuyển theo sau lưng Tướng Đỏ
            for (size_t i = 0; i < redClashArmy.size(); ++i) {
                int col = (int)(i / 5);
                int row = (int)(i % 5);
                float targetX = redX + 65.f + col * 44.f;
                float targetY = centerY + (row - 2) * 44.f;
                redClashArmy[i]->x += (targetX - redClashArmy[i]->x) * std::min(1.f, dt * 10.f);
                redClashArmy[i]->y += (targetY - redClashArmy[i]->y) * std::min(1.f, dt * 10.f);
                redClashArmy[i]->sprite.setPosition(redClashArmy[i]->x, redClashArmy[i]->y);
            }
        }

        // Hết thời gian dàn trận -> chuyển sang bứt tốc xung phong!
        if (clashMarchTimer <= 0.f) {
            clashChargeTimer = 1.0f;
        }
        return;
    }

    // ================================================================
    //  Giai đoạn 1B: Bứt tốc xung phong lao vào nhau (Charge: 1.0s)
    //  Hai Tướng dũng mãnh xung phong đi đầu, lính bứt tốc theo sát
    // ================================================================
    if (clashChargeTimer > 0.f) {
        clashChargeTimer -= dt;

        float chargeSpeed = 520.f; // Bứt tốc phi nước đại

        if (player1) {
            float p1x = player1->getX() + chargeSpeed * dt;
            if (p1x > clashFrontlineX - 45.f) p1x = clashFrontlineX - 45.f;
            player1->setPosition(p1x, centerY);

            for (size_t i = 0; i < blueClashArmy.size(); ++i) {
                int col = (int)(i / 5);
                int row = (int)(i % 5);
                float targetX = p1x - 65.f - col * 44.f;
                float targetY = centerY + (row - 2) * 44.f;
                blueClashArmy[i]->x += (targetX - blueClashArmy[i]->x) * std::min(1.f, dt * 14.f);
                blueClashArmy[i]->y += (targetY - blueClashArmy[i]->y) * std::min(1.f, dt * 14.f);
                blueClashArmy[i]->sprite.setPosition(blueClashArmy[i]->x, blueClashArmy[i]->y);
            }
        }

        if (redLeader) {
            float redX = redLeader->getX() - chargeSpeed * dt;
            if (redX < clashFrontlineX + 45.f) redX = clashFrontlineX + 45.f;
            redLeader->setPosition(redX, centerY);

            for (size_t i = 0; i < redClashArmy.size(); ++i) {
                int col = (int)(i / 5);
                int row = (int)(i % 5);
                float targetX = redX + 65.f + col * 44.f;
                float targetY = centerY + (row - 2) * 44.f;
                redClashArmy[i]->x += (targetX - redClashArmy[i]->x) * std::min(1.f, dt * 14.f);
                redClashArmy[i]->y += (targetY - redClashArmy[i]->y) * std::min(1.f, dt * 14.f);
                redClashArmy[i]->sprite.setPosition(redClashArmy[i]->x, redClashArmy[i]->y);
            }
        }

        // Cú va chạm đầu tiên (First Impact!)
        if (clashChargeTimer <= 0.f) {
            AudioManager::getInstance().playSound("sfx_clash_hit.ogg");
            screenShakeTimer = 0.35f;
            screenShakeIntensity = 11.f;
            // Bắn chùm tia lửa va chạm ngay giữa 2 Tướng
            for (int k = 0; k < 20; ++k) {
                float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
                float spd = 70.f + (float)(std::rand() % 160);
                clashSparks.push_back({
                    clashFrontlineX, centerY,
                    std::cos(angle) * spd, std::sin(angle) * spd,
                    0.4f, 0.4f,
                    sf::Color(255, 220, 100)
                });
            }
            clashFloatTexts.push_back({
                clashFrontlineX, 310.f,
                ConfigManager::toUtf8(u8"GIÁP LÁ CÀ!"), 0.9f, 0.9f,
                sf::Color(255, 215, 0)
            });

            // Nếu 1 trong 2 bên không có lính nào từ đầu -> Kích hoạt Quét Sạch (Rout) ngay sau cú va chạm
            if (blueClashArmy.empty() || redClashArmy.empty()) {
                clashIsEnding = true;
                clashRoutTimer = 1.4f;
            }
        }
        return;
    }

    // ================================================================
    //  Giai đoạn Kết thúc: Quét sạch quân địch (Rout & Victory Rush)
    //  Tướng thắng trận dẫn đầu toàn quân tràn sang căn cứ địch
    // ================================================================
    if (clashIsEnding) {
        clashRoutTimer -= dt;
        float rushSpeed = 360.f;

        if (redClashArmy.empty() && !blueClashArmy.empty()) {
            // Xanh thắng -> Tướng Xanh dẫn đầu toàn quân tràn sang phải
            if (player1) {
                float p1x = player1->getX() + rushSpeed * dt;
                player1->setPosition(p1x, centerY);
                for (size_t i = 0; i < blueClashArmy.size(); ++i) {
                    int col = (int)(i / 5);
                    int row = (int)(i % 5);
                    float targetX = p1x - 65.f - col * 44.f;
                    float targetY = centerY + (row - 2) * 44.f;
                    blueClashArmy[i]->x = targetX;
                    blueClashArmy[i]->y = targetY;
                    blueClashArmy[i]->sprite.setPosition(targetX, targetY);
                }
            }
        } else if (blueClashArmy.empty() && !redClashArmy.empty()) {
            // Đỏ thắng -> Tướng Đỏ dẫn đầu toàn quân tràn sang trái
            if (redLeader) {
                float redX = redLeader->getX() - rushSpeed * dt;
                redLeader->setPosition(redX, centerY);
                for (size_t i = 0; i < redClashArmy.size(); ++i) {
                    int col = (int)(i / 5);
                    int row = (int)(i % 5);
                    float targetX = redX + 65.f + col * 44.f;
                    float targetY = centerY + (row - 2) * 44.f;
                    redClashArmy[i]->x = targetX;
                    redClashArmy[i]->y = targetY;
                    redClashArmy[i]->sprite.setPosition(targetX, targetY);
                }
            }
        }

        if (clashRoutTimer <= 0.f) {
            currentState = GameState::RESULT;
            if (blueClashArmy.empty() && redClashArmy.empty()) {
                matchWinner = 0; // Hòa
                resultText = u8"BẤT PHÂN THẮNG BẠI!\nHai bên đồng quy vu tận trên sa trường!";
                if (gameMode == GameMode::PVE) {
                    AudioManager::getInstance().playVoice("pve_draw.ogg");
                } else {
                    AudioManager::getInstance().playVoice("pvp_draw.ogg");
                }
            } else if (redClashArmy.empty()) {
                matchWinner = 1; // P1 Thắng
                if (gameMode == GameMode::PVE) {
                    resultText = u8"ĐẠI THẮNG QUÂN GIẶC!\nĐức Vua gả Nàng Trần Thị Dung cho Dũng Tướng!";
                    AudioManager::getInstance().playBGM("bgm_victory.ogg", false);
                    AudioManager::getInstance().playVoice("pve_win.ogg");
                } else {
                    resultText = u8"HIỆP SĨ LAM Y ĐẠI THẮNG!\nRước Nàng Trần Thị Dung về lâu đài!";
                    AudioManager::getInstance().playBGM("bgm_victory.ogg", false);
                    AudioManager::getInstance().playVoice("pvp_win_p1.ogg");
                }
            } else {
                matchWinner = 2; // P2 / Bot Thắng
                if (gameMode == GameMode::PVE) {
                    resultText = u8"VƯƠNG QUỐC THẤT THỦ!\nBạo Chúa cướp lấy Nàng Trần Thị Dung...";
                    AudioManager::getInstance().playBGM("bgm_defeat.ogg", false);
                    AudioManager::getInstance().playVoice("pve_lose.ogg");
                } else {
                    resultText = u8"HIỆP SĨ HỒNG Y ĐOẠT MỸ NHÂN!\nNàng Dung thuộc về Phe Đỏ!";
                    AudioManager::getInstance().playBGM("bgm_victory.ogg", false);
                    AudioManager::getInstance().playVoice("pvp_win_p2.ogg");
                }
            }
            saveMatchHistoryToCSV("match_history.csv");
        }
        return;
    }

    // ================================================================
    //  Giai đoạn 2: Giáp lá cà từng đợt chiến đấu (Continuous Frontline Combat)
    //  Hai Tướng mặt đối mặt ngay tại tiền tuyến, lính xếp hàng ngay ngắn phía sau
    // ================================================================
    // Kiểm tra an toàn: Nếu 1 trong 2 bên không có lính nào -> Lập tức kích hoạt quét sạch
    if (blueClashArmy.empty() || redClashArmy.empty()) {
        clashIsEnding = true;
        clashRoutTimer = 1.4f;
        screenShakeTimer = 0.3f;
        screenShakeIntensity = 8.f;
        return;
    }
    if (player1) player1->setPosition(clashFrontlineX - 45.f, centerY);
    if (redLeader) redLeader->setPosition(clashFrontlineX + 45.f, centerY);

    for (size_t i = 0; i < blueClashArmy.size(); ++i) {
        int col = (int)(i / 5);
        int row = (int)(i % 5);
        float tx = (clashFrontlineX - 45.f) - 65.f - col * 44.f;
        float ty = centerY + (row - 2) * 44.f;
        blueClashArmy[i]->x += (tx - blueClashArmy[i]->x) * std::min(1.f, dt * 8.f);
        blueClashArmy[i]->y += (ty - blueClashArmy[i]->y) * std::min(1.f, dt * 8.f);
        blueClashArmy[i]->sprite.setPosition(blueClashArmy[i]->x, blueClashArmy[i]->y);
    }
    for (size_t i = 0; i < redClashArmy.size(); ++i) {
        int col = (int)(i / 5);
        int row = (int)(i % 5);
        float tx = (clashFrontlineX + 45.f) + 65.f + col * 44.f;
        float ty = centerY + (row - 2) * 44.f;
        redClashArmy[i]->x += (tx - redClashArmy[i]->x) * std::min(1.f, dt * 8.f);
        redClashArmy[i]->y += (ty - redClashArmy[i]->y) * std::min(1.f, dt * 8.f);
        redClashArmy[i]->sprite.setPosition(redClashArmy[i]->x, redClashArmy[i]->y);
    }

    // Nhịp đấu: Mỗi 0.38 giây đọ kiếm 1 lần
    clashFightTimer += dt;
    if (clashFightTimer >= 0.38f && !blueClashArmy.empty() && !redClashArmy.empty()) {
        clashFightTimer = 0.f;

        AudioManager::getInstance().playSound("sfx_sword_slash.ogg");
        AudioManager::getInstance().playSound("sfx_clash_hit.ogg");

        Troop* blueT = blueClashArmy.front();
        Troop* redT = redClashArmy.front();

        // Khắc chế binh chủng: Kỵ binh > Kiếm sĩ, Kiếm sĩ > Hộ vệ, Hộ vệ > Kỵ binh
        int blueBonus = 0;
        int redBonus = 0;
        std::string bonusText = "";
        sf::Color bonusColor = sf::Color::Yellow;

        if (blueT->type == TroopType::CAVALRY && redT->type == TroopType::SWORDSMAN) {
            blueBonus = 1;
            bonusText = u8"KỴ BINH XUYÊN PHÁ!";
            bonusColor = sf::Color(100, 200, 255);
        } else if (blueT->type == TroopType::SWORDSMAN && redT->type == TroopType::GUARDIAN) {
            blueBonus = 1;
            bonusText = u8"CHÉM GIÁP HỘ VỆ!";
            bonusColor = sf::Color(255, 215, 0);
        } else if (blueT->type == TroopType::GUARDIAN && redT->type == TroopType::CAVALRY) {
            blueBonus = 1;
            bonusText = u8"CHẶN ĐỨNG KỴ BINH!";
            bonusColor = sf::Color(120, 255, 120);
        } else if (redT->type == TroopType::CAVALRY && blueT->type == TroopType::SWORDSMAN) {
            redBonus = 1;
            bonusText = u8"ĐỎ XUYÊN PHÁ!";
            bonusColor = sf::Color(255, 120, 100);
        } else if (redT->type == TroopType::SWORDSMAN && blueT->type == TroopType::GUARDIAN) {
            redBonus = 1;
            bonusText = u8"ĐỎ CHÉM GIÁP!";
            bonusColor = sf::Color(255, 120, 100);
        } else if (redT->type == TroopType::GUARDIAN && blueT->type == TroopType::CAVALRY) {
            redBonus = 1;
            bonusText = u8"ĐỎ CHẶN ĐỨNG!";
            bonusColor = sf::Color(255, 120, 100);
        }

        int blueEffectivePow = blueT->power + blueBonus;
        int redEffectivePow = redT->power + redBonus;

        float hitY = (blueT->y + redT->y) / 2.f;

        // Tạo vệt chém Slash Effect
        float slashAngle = -30.f + (float)(std::rand() % 60);
        clashSlashes.push_back({
            clashFrontlineX, hitY,
            slashAngle, 0.16f, 0.16f,
            (blueEffectivePow >= redEffectivePow) ? sf::Color(120, 180, 255) : sf::Color(255, 120, 100)
        });

        // Bắn tia lửa va chạm
        for (int k = 0; k < 6; ++k) {
            float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
            float spd = 40.f + (float)(std::rand() % 90);
            clashSparks.push_back({
                clashFrontlineX, hitY,
                std::cos(angle) * spd, std::sin(angle) * spd,
                0.28f, 0.28f,
                sf::Color(255, 200, 80)
            });
        }

        // Rung nhẹ màn hình
        screenShakeTimer = 0.12f;
        screenShakeIntensity = 4.f;

        // Nếu có khắc chế, hiện chữ bay
        if (!bonusText.empty()) {
            clashFloatTexts.push_back({
                clashFrontlineX, hitY - 25.f,
                bonusText, 0.6f, 0.6f,
                bonusColor
            });
        }

        // Xử lý sát thương
        if (blueEffectivePow >= redEffectivePow) {
            blueT->power -= redT->power;
            redTotalPower -= redT->power;

            // Red troop bị đánh bật ngửa ra sau
            DeadTroopAnim dead;
            dead.sprite = redT->sprite;
            dead.x = redT->x;
            dead.y = redT->y;
            dead.vx = 140.f + (float)(std::rand() % 40);
            dead.vy = -50.f + (float)(std::rand() % 100);
            dead.alpha = 255.f;
            dead.lifetime = 0.45f;
            clashDeadTroops.push_back(dead);
            AudioManager::getInstance().playSound("sfx_troop_death.ogg");

            delete redT;
            redClashArmy.erase(redClashArmy.begin());

            if (blueT->power <= 0) {
                DeadTroopAnim deadB;
                deadB.sprite = blueT->sprite;
                deadB.x = blueT->x;
                deadB.y = blueT->y;
                deadB.vx = -140.f - (float)(std::rand() % 40);
                deadB.vy = -50.f + (float)(std::rand() % 100);
                deadB.alpha = 255.f;
                deadB.lifetime = 0.45f;
                clashDeadTroops.push_back(deadB);
                AudioManager::getInstance().playSound("sfx_troop_death.ogg");

                delete blueT;
                blueClashArmy.erase(blueClashArmy.begin());
            }
        } else {
            redT->power -= blueT->power;
            blueTotalPower -= blueT->power;

            DeadTroopAnim deadB;
            deadB.sprite = blueT->sprite;
            deadB.x = blueT->x;
            deadB.y = blueT->y;
            deadB.vx = -140.f - (float)(std::rand() % 40);
            deadB.vy = -50.f + (float)(std::rand() % 100);
            deadB.alpha = 255.f;
            deadB.lifetime = 0.45f;
            clashDeadTroops.push_back(deadB);
            AudioManager::getInstance().playSound("sfx_troop_death.ogg");

            delete blueT;
            blueClashArmy.erase(blueClashArmy.begin());

            if (redT->power <= 0) {
                DeadTroopAnim deadR;
                deadR.sprite = redT->sprite;
                deadR.x = redT->x;
                deadR.y = redT->y;
                deadR.vx = 140.f + (float)(std::rand() % 40);
                deadR.vy = -50.f + (float)(std::rand() % 100);
                deadR.alpha = 255.f;
                deadR.lifetime = 0.45f;
                clashDeadTroops.push_back(deadR);
                AudioManager::getInstance().playSound("sfx_troop_death.ogg");

                delete redT;
                redClashArmy.erase(redClashArmy.begin());
            }
        }

        // Kiểm tra xem có phe nào hết lính không
        if (blueClashArmy.empty() || redClashArmy.empty()) {
            clashIsEnding = true;
            clashRoutTimer = 1.4f;
            screenShakeTimer = 0.3f;
            screenShakeIntensity = 8.f;
        }
    }
}

// ================================================================
// ================================================================
//  initObstaclesAndTrees – Rải đá và rừng cây khắp toàn bộ bản đồ 2800x2000
//  Phân bổ đều khắp các khu vực (trung tâm, giữa map, các lối đi)
// ================================================================
void GameEngine::initObstaclesAndTrees() {
    for (auto o : obstacles) delete o;
    obstacles.clear();

    // Dùng seed cố định để map cân bằng, đẹp mắt và ổn định
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> jitter(-40.f, 40.f);
    std::uniform_int_distribution<int> typeDist(0, 24);
    std::uniform_int_distribution<int> varDist(1, 2);
    std::uniform_real_distribution<float> sizeDist(92.f, 110.f);
    std::uniform_real_distribution<float> rockSizeDist(62.f, 76.f);

    auto isExcluded = [](float ox, float oy) {
        float midY = WORLD_H / 2.f;

        // Base Xanh
        float dBlue = std::hypot(ox - 150.f, oy - midY);
        if (dBlue < 230.f) return true;

        // Base Đỏ
        float dRed = std::hypot(ox - (WORLD_W - 150.f), oy - midY);
        if (dRed < 230.f) return true;

        // Điểm spawn Player 1
        float dP1 = std::hypot(ox - 300.f, oy - midY);
        if (dP1 < 140.f) return true;

        // Điểm spawn Bot / Player 2
        float dP2 = std::hypot(ox - (WORLD_W - 300.f), oy - midY);
        if (dP2 < 140.f) return true;

        // Điểm rơi VIP giây 45
        float dVip = std::hypot(ox - (WORLD_W / 2.f), oy - midY);
        if (dVip < 140.f) return true;

        return false;
    };

    // Phân bổ trên lưới động phủ kín toàn bộ bản đồ WORLD_W x WORLD_H (2800x2000)
    std::vector<float> cols;
    for (float c = 200.f; c <= WORLD_W - 200.f; c += 185.f) cols.push_back(c);
    std::vector<float> rows;
    for (float r = 180.f; r <= WORLD_H - 180.f; r += 185.f) rows.push_back(r);

    for (float c : cols) {
        for (float r : rows) {
            float ox = c + jitter(rng);
            float oy = r + jitter(rng);

            // Giữ trong biên an toàn
            if (ox < 90.f) ox = 90.f;
            if (ox > WORLD_W - 170.f) ox = WORLD_W - 170.f;
            if (oy < 90.f) oy = 90.f;
            if (oy > WORLD_H - 170.f) oy = WORLD_H - 170.f;

            if (isExcluded(ox, oy)) continue;

            // Đảm bảo không dẫm quá sát một vật cản đã có
            bool tooClose = false;
            for (auto existing : obstacles) {
                float d = std::hypot(ox - existing->getX(), oy - existing->getY());
                if (d < 115.f) { tooClose = true; break; }
            }
            if (tooClose) continue;

            int roll = typeDist(rng);
            if (roll < 12) {
                // ~48% Cây sa mạc (xen kẽ tree1 và tree2, size tự nhiên 92-110px)
                int var = varDist(rng);
                float sz = sizeDist(rng);
                obstacles.push_back(new Tree(ox, oy, var, sz));
            } else if (roll < 17) {
                // ~20% Bụi rậm ngụy trang (bush.png, 128x128) - Ẩn nấp Stealth, nhân vật mờ 55%, AI mất dấu
                obstacles.push_back(new Bush(ox, oy, 115.f));
            } else if (roll < 21) {
                // ~16% Tảng đá sa mạc (size 62-76px)
                float sz = rockSizeDist(rng);
                obstacles.push_back(new Obstacle(ox, oy, sz, sz));
            } else if (roll < 24) {
                // ~12% Cột đá La Mã gãy đổ cháy xém (ruins_pillar.png, 128x128)
                obstacles.push_back(new RuinsPillar(ox, oy, 110.f));
            } else {
                // ~4% (Chỉ 1/25) Đoạn tường thành nứt toác có vết vuốt rồng (ruins_wall.png, 160x128)
                // Rất hiếm, xuất hiện làm điểm nhấn cổ kính mà không gây nghẽn đường di chuyển!
                obstacles.push_back(new RuinsWall(ox, oy, 150.f, 115.f));
            }
        }
    }

    // Sắp xếp các vật cản theo trục Y tăng dần để vẽ đúng thứ tự chiều sâu
    std::sort(obstacles.begin(), obstacles.end(), [](Obstacle* a, Obstacle* b) {
        return a->getY() < b->getY();
    });
}

// ================================================================
//  Spawn functions
// ================================================================
void GameEngine::spawnFreeTroops(int count) {
    for (int i = 0; i < count; i++) {
        float sx = 0.f, sy = 0.f;
        bool found = false;

        // Thử tìm vị trí thoáng không vướng vật cản
        for (int attempt = 0; attempt < 20; ++attempt) {
            sx = 100.f + (float)(std::rand() % (int)(WORLD_W - 200.f));
            sy = 100.f + (float)(std::rand() % (int)(WORLD_H - 200.f));

            bool blocked = false;
            for (auto o : obstacles) {
                if (o->getBounds().contains(sx, sy)) { blocked = true; break; }
            }
            if (baseBlue && baseBlue->isInZone(sx, sy)) blocked = true;
            if (baseRed && baseRed->isInZone(sx, sy)) blocked = true;

            if (!blocked) {
                found = true;
                break;
            }
        }

        if (!found) continue;

        // Random loại lính theo tỉ lệ: 60% Kiếm Sĩ, 30% Hộ Vệ, 10% Kỵ Binh
        int r = std::rand() % 100;
        TroopType type;
        if (r < 60)      type = TroopType::SWORDSMAN;
        else if (r < 90)  type = TroopType::GUARDIAN;
        else              type = TroopType::CAVALRY;

        freeTroops.push_back(new Troop(sx, sy, type));
    }
}

void GameEngine::spawnRandomItem() {
    float sx = 0.f, sy = 0.f;
    bool found = false;

    for (int attempt = 0; attempt < 15; ++attempt) {
        sx = 120.f + (float)(std::rand() % (int)(WORLD_W - 240.f));
        sy = 120.f + (float)(std::rand() % (int)(WORLD_H - 240.f));

        bool blocked = false;
        for (auto o : obstacles) {
            if (o->getBounds().contains(sx, sy)) { blocked = true; break; }
        }
        if (!blocked) { found = true; break; }
    }
    if (!found) return;

    int r = std::rand() % 3;
    switch (r) {
        case 0: items.push_back(new SpeedItem(sx, sy)); break;
        case 1: items.push_back(new ShieldItem(sx, sy)); break;
        case 2: items.push_back(new MagnetItem(sx, sy)); break;
    }
}

void GameEngine::spawnBomb() {
    // 1. Lựa chọn mục tiêu thông minh (Target Selection)
    std::vector<Character*> targets;
    if (player1) targets.push_back(player1);
    if (gameMode == GameMode::PVP && player2) targets.push_back(player2);
    else if (bot) targets.push_back(bot);

    if (targets.empty()) return;

    // Ưu tiên nhắm vào phe đang có nhiều lính hơn để tăng kịch tính và cơ hội lật kèo
    Character* target = targets[0];
    if (targets.size() > 1) {
        int count0 = targets[0]->getArmy().getCount();
        int count1 = targets[1]->getArmy().getCount();
        if (count0 > count1 + 2) {
            target = targets[0];
        } else if (count1 > count0 + 2) {
            target = targets[1];
        } else {
            target = targets[std::rand() % targets.size()];
        }
    }

    float tx = target->getX();
    float ty = target->getY();

    // 2. Quyết định chiến thuật pháo kích:
    // 40% Tỉ lệ: Nhắm vào giữa hoặc đuôi đàn lính (Cắt đuôi)
    // 60% Tỉ lệ: Nhắm đón đầu đường chạy phía trước Tướng
    bool targetTail = (std::rand() % 100 < 40) && (target->getArmy().getCount() > 2);

    if (targetTail) {
        int count = target->getArmy().getCount();
        int pickIdx = count / 2 + (std::rand() % (count - count / 2));
        Troop* curr = target->getArmy().getHead();
        for (int i = 0; i < pickIdx && curr && curr->next; ++i) {
            curr = curr->next;
        }
        if (curr) {
            tx = curr->x;
            ty = curr->y;
        }
    } else {
        // Suy ra hướng di chuyển từ lính theo sau tới Tướng
        float dirX = 0.f, dirY = 0.f;
        if (target->getArmy().getHead()) {
            float dx = target->getX() - target->getArmy().getHead()->x;
            float dy = target->getY() - target->getArmy().getHead()->y;
            float len = std::hypot(dx, dy);
            if (len > 5.f) {
                dirX = dx / len;
                dirY = dy / len;
            }
        }
        // Nếu không có lính hoặc đang đứng yên, lấy góc ngẫu nhiên
        if (std::abs(dirX) < 0.1f && std::abs(dirY) < 0.1f) {
            float angle = (float)(std::rand() % 360) * 3.14159f / 180.f;
            dirX = std::cos(angle);
            dirY = std::sin(angle);
        }

        // Đón đầu 85 - 125px phía trước mặt Tướng
        float leadDist = 85.f + (float)(std::rand() % 40);
        tx += dirX * leadDist;
        ty += dirY * leadDist;
    }

    // Jitter ngẫu nhiên ±25px để tạo cơ hội phản xạ né
    tx += (-25.f + (float)(std::rand() % 50));
    ty += (-25.f + (float)(std::rand() % 50));

    // Tuyệt đối không rơi vào vùng Base của 2 phe (Base là khu an toàn)
    if (baseBlue && baseBlue->isInZone(tx, ty)) {
        tx += 250.f;
    }
    if (baseRed && baseRed->isInZone(tx, ty)) {
        tx -= 250.f;
    }

    // Giữ trong biên an toàn của map 2800x2000
    if (tx < 150.f) tx = 150.f;
    if (tx > WORLD_W - 150.f) tx = WORLD_W - 150.f;
    if (ty < 150.f) ty = 150.f;
    if (ty > WORLD_H - 150.f) ty = WORLD_H - 150.f;

    // Bán kính nổ 115px (nguy hiểm và rộng hơn bản cũ 80px)
    bombs.push_back(new BombHazard(tx, ty, 115.f));
    AudioManager::getInstance().playSound("sfx_bomb_siren.ogg");
}

// ================================================================
//  checkCollisions – Va chạm Tướng vs Lính tự do, Items, Lính đối thủ
// ================================================================
void GameEngine::checkCollisions() {
    auto checkCharacter = [&](Character* character) {
        if (!character) return;
        sf::FloatRect charBounds = character->getBounds();

        // 1. Nhặt lính tự do
        for (auto it = freeTroops.begin(); it != freeTroops.end(); ) {
            Troop* t = *it;
            float dx = character->getX() - t->x;
            float dy = character->getY() - t->y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < 36.f) {
                t->team = character->getTeam();
                t->sprite.setColor(t->getTeamColor());
                character->getArmy().addTroopNode(t);
                it = freeTroops.erase(it);
                AudioManager::getInstance().playSound("sfx_recruit.ogg");
            } else {
                ++it;
            }
        }

        // 2. Nhặt Item (Đa hình!)
        for (auto i : items) {
            if (i->isCollected()) continue;
            sf::FloatRect itemBounds = i->getBounds();
            if (charBounds.intersects(itemBounds)) {
                if (dynamic_cast<ShieldItem*>(i)) {
                    AudioManager::getInstance().playSound("sfx_item_shield.ogg");
                } else if (dynamic_cast<SpeedItem*>(i)) {
                    AudioManager::getInstance().playSound("sfx_item_speed.ogg");
                } else if (dynamic_cast<MagnetItem*>(i)) {
                    AudioManager::getInstance().playSound("sfx_item_magnet.ogg");
                }
                i->applyBuff(character); // Polymorphism!
                i->markCollected();
            }
        }
    };

    checkCharacter(player1);
    if (player2) checkCharacter(player2);
    if (bot) checkCharacter(bot);

    // 3.1. Kỹ năng Tướng "Đoạt Mệnh Trảm" (Fatal Dash Slash - Cắt đứt đuôi xuyên đội hình)
    auto checkDashSlash = [&](Character* attacker, Character* defender) {
        if (!attacker || !defender) return;
        if (!attacker->getIsDashing() || attacker->getHasCutInThisDash()) return;

        float ax = attacker->getDashStartX();
        float ay = attacker->getDashStartY();
        float bx = attacker->getX();
        float by = attacker->getY();

        auto distPointToSegment = [](float px, float py, float x1, float y1, float x2, float y2) -> float {
            float dx = x2 - x1;
            float dy = y2 - y1;
            float lenSq = dx * dx + dy * dy;
            if (lenSq < 0.001f) return std::hypot(px - x1, py - y1);
            float t = ((px - x1) * dx + (py - y1) * dy) / lenSq;
            t = std::max(0.f, std::min(1.f, t));
            float projX = x1 + t * dx;
            float projY = y1 + t * dy;
            return std::hypot(px - projX, py - projY);
        };

        // 1. Quét qua đoàn lính đối thủ
        Troop* curr = defender->getArmy().getHead();
        Troop* targetCut = nullptr;
        float cutX = 0.f, cutY = 0.f;

        while (curr) {
            float d = distPointToSegment(curr->x, curr->y, ax, ay, bx, by);
            if (d < 52.f) { // Bề rộng nhát chém quét 52px
                targetCut = curr;
                cutX = curr->x;
                cutY = curr->y;
                break; // Cắt từ đốt gần đầu nhất
            }
            curr = curr->next;
        }

        if (targetCut) {
            attacker->setHasCutInThisDash(true);

            if (defender->getIsInvincible()) {
                // Khiên bảo vệ
                AudioManager::getInstance().playSound("sfx_clash_hit.ogg");
                clashSparks.push_back({ cutX, cutY, 0.f, -50.f, 0.4f, 0.4f, sf::Color(255, 230, 100) });
                clashFloatTexts.push_back({ cutX, cutY - 20.f, u8"KHIÊN ĐỠ ĐÒN!", 1.0f, 1.0f, sf::Color(100, 200, 255) });
            } else {
                // Chém đứt đuôi và cướp lính!
                Troop* stolen = defender->getArmy().stealFrom(targetCut);
                if (stolen) {
                    int chainCount = 0;
                    Troop* chainTail = stolen;
                    Troop* temp = stolen;
                    while (temp) {
                        temp->team = attacker->getTeam();
                        temp->sprite.setColor(temp->getTeamColor());
                        chainCount++;
                        chainTail = temp;
                        temp = temp->next;
                    }
                    attacker->getArmy().appendChain(stolen, chainTail, chainCount);

                    // Âm thanh uy lực
                    AudioManager::getInstance().playSound("sfx_sword_slash.ogg");
                    AudioManager::getInstance().playSound("sfx_steal.ogg");

                    // Rung màn hình
                    screenShakeTimer = 0.35f;
                    screenShakeIntensity = 12.f;

                    // Slash VFX chữ X
                    clashSlashes.push_back({ cutX, cutY, 45.f, 0.45f, 0.45f, sf::Color(255, 215, 0) });
                    clashSlashes.push_back({ cutX, cutY, -45.f, 0.45f, 0.45f, sf::Color(255, 255, 255) });

                    // Bầy tia lửa
                    for (int sp = 0; sp < 12; ++sp) {
                        float ang = (float)(std::rand() % 360) * 3.14159f / 180.f;
                        float spd = 120.f + (std::rand() % 150);
                        clashSparks.push_back({ cutX, cutY, std::cos(ang) * spd, std::sin(ang) * spd, 0.5f, 0.5f, sf::Color(255, 220, 80) });
                    }

                    // Chữ nổi thông báo
                    std::string title = (attacker->getTeam() == Team::BLUE) ? u8"P1: ĐOẠT MỆNH TRẢM! (+" + std::to_string(chainCount) + u8" LÍNH)"
                                                                            : u8"BOT: ĐOẠT MỆNH TRẢM! (+" + std::to_string(chainCount) + u8" LÍNH)";
                    sf::Color txtCol = (attacker->getTeam() == Team::BLUE) ? sf::Color(255, 230, 80) : sf::Color(255, 90, 80);
                    clashFloatTexts.push_back({ cutX, cutY - 35.f, title, 1.4f, 1.4f, txtCol });
                }
            }
        }

        // 2. Đồng thời cuốn lính tự do trên đường lướt
        for (auto it = freeTroops.begin(); it != freeTroops.end(); ) {
            Troop* ft = *it;
            float d = distPointToSegment(ft->x, ft->y, ax, ay, bx, by);
            if (d < 48.f) {
                ft->team = attacker->getTeam();
                ft->sprite.setColor(ft->getTeamColor());
                attacker->getArmy().addTroopNode(ft);
                it = freeTroops.erase(it);
                AudioManager::getInstance().playSound("sfx_recruit.ogg");
            } else {
                ++it;
            }
        }
    };

    // 3.2. Cướp lính thông thường khi tiếp xúc cự ly gần (O(1) algorithm)
    auto trySteal = [&](Character* attacker, Character* defender) {
        if (!attacker || !defender) return;
        if (defender->getIsInvincible()) return; // Có khiên → không cướp được

        Troop* curr = defender->getArmy().getHead();
        while (curr) {
            Troop* next = curr->next; // Lưu trước vì có thể bị cắt
            float dx = attacker->getX() - curr->x;
            float dy = attacker->getY() - curr->y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < 32.f) {
                // Cướp! Cắt từ curr đến tail
                Troop* stolen = defender->getArmy().stealFrom(curr);
                if (stolen) {
                    // Đếm + tìm tail của đoạn bị cắt
                    int chainCount = 0;
                    Troop* chainTail = stolen;
                    Troop* temp = stolen;
                    while (temp) {
                        temp->team = attacker->getTeam();
                        temp->sprite.setColor(temp->getTeamColor());
                        chainCount++;
                        chainTail = temp;
                        temp = temp->next;
                    }
                    attacker->getArmy().appendChain(stolen, chainTail, chainCount);
                    AudioManager::getInstance().playSound("sfx_steal.ogg");
                }
                return; // Chỉ cướp 1 lần mỗi frame
            }
            curr = next;
        }
    };

    if (gameMode == GameMode::PVE) {
        checkDashSlash(player1, bot);
        checkDashSlash(bot, player1);
        trySteal(player1, bot);
        trySteal(bot, player1);
    } else {
        checkDashSlash(player1, player2);
        checkDashSlash(player2, player1);
        trySteal(player1, player2);
        trySteal(player2, player1);
    }
}

// ================================================================
//  checkBaseZones – Kiểm tra Tướng có ở trong Base để cất lính
// ================================================================
void GameEngine::checkBaseZones() {
    // Player 1 → Base Xanh
    if (baseBlue && baseBlue->isInZone(player1->getX(), player1->getY())) {
        if (player1->getArmy().getCount() > 0) {
            baseBlue->bankTroops(player1->getArmy());
            AudioManager::getInstance().playSound("sfx_bank.ogg");
        }
    }

    if (gameMode == GameMode::PVP && player2) {
        // Player 2 → Base Đỏ
        if (baseRed && baseRed->isInZone(player2->getX(), player2->getY())) {
            if (player2->getArmy().getCount() > 0) {
                baseRed->bankTroops(player2->getArmy());
                AudioManager::getInstance().playSound("sfx_bank.ogg");
            }
        }
    }

    if (gameMode == GameMode::PVE && bot) {
        // Bot → Base Đỏ
        if (baseRed && baseRed->isInZone(bot->getX(), bot->getY())) {
            if (bot->getArmy().getCount() > 0) {
                baseRed->bankTroops(bot->getArmy());
                AudioManager::getInstance().playSound("sfx_bank.ogg");
            }
        }
    }
}

// ================================================================
//  checkMagnet – Hút lính tự do về phía Tướng có buff Magnet
// ================================================================
void GameEngine::checkMagnet(Character* character) {
    if (!character || !character->getHasMagnet()) return;

    for (auto t : freeTroops) {
        float dx = character->getX() - t->x;
        float dy = character->getY() - t->y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 120.f && dist > 1.f) {
            // Kéo lính lại gần Tướng
            float pull = 100.f / dist; // Lực hút mạnh hơn khi gần
            if (pull > 1.f) pull = 1.f;
            t->x += dx * pull * 0.05f;
            t->y += dy * pull * 0.05f;
            t->sprite.setPosition(t->x, t->y);
        }
    }
}

// ================================================================
//  handleBombExplosions – Xử lý bom nổ trực diện + Vùng đất cháy âm ỉ
// ================================================================
void GameEngine::handleBombExplosions() {
    for (auto b : bombs) {
        if (b->isFinished()) continue;

        // 1. SỰ KIỆN BOM NỔ TRỰC DIỆN (Direct Blast Impact)
        if (!b->isWarning() && !b->hasExploded()) {
            // Rung màn hình uy lực
            screenShakeTimer = 0.28f;
            screenShakeIntensity = 8.f;
            AudioManager::getInstance().playSound("sfx_explosion.ogg");

            auto processBlast = [&](Character* ch) {
                if (!ch) return;

                // Tướng dính bom -> Choáng (Stun 1.5s)
                if (b->isInBlastZone(ch->getX(), ch->getY())) {
                    ch->stun(1.5f);
                }

                // Lính dính bom trực diện -> Ngắt xích & Văng thành lính tự do
                Troop* curr = ch->getArmy().getHead();
                while (curr) {
                    Troop* next = curr->next;
                    if (b->isInBlastZone(curr->x, curr->y)) {
                        auto freed = ch->getArmy().bombExplode(curr);
                        for (auto f : freed) {
                            // Đẩy lính văng nhẹ ra xa tâm nổ
                            float fdx = f->x - b->getX();
                            float fdy = f->y - b->getY();
                            float flen = std::hypot(fdx, fdy) + 0.01f;
                            f->x += (fdx / flen) * 45.f;
                            f->y += (fdy / flen) * 45.f;
                            f->sprite.setPosition(f->x, f->y);
                            freeTroops.push_back(f);
                        }
                        break; // 1 vụ nổ chỉ ngắt từ node đầu tiên dính bom
                    }
                    curr = next;
                }
            };

            processBlast(player1);
            if (player2) processBlast(player2);
            if (bot) processBlast(bot);

            b->markExploded();
        }

        // 2. VÙNG ĐẤT CHÁY ÂM Ỉ (Scorched Earth / Lingering Fire: 3.5s)
        if (b->isScorched()) {
            auto processScorched = [&](Character* ch) {
                if (!ch) return;

                // Tướng đi qua vùng cháy -> Giảm 45% tốc độ (lún vào tro than rực lửa)
                if (b->isInScorchedZone(ch->getX(), ch->getY())) {
                    ch->applySlow(0.55f);
                }

                // Lính đi qua vùng cháy -> Bị thiêu rụi đứt xích văng ra đất
                Troop* curr = ch->getArmy().getHead();
                while (curr) {
                    Troop* next = curr->next;
                    if (b->isInScorchedZone(curr->x, curr->y)) {
                        auto freed = ch->getArmy().bombExplode(curr);
                        for (auto f : freed) {
                            freeTroops.push_back(f);
                        }
                        break;
                    }
                    curr = next;
                }
            };

            processScorched(player1);
            if (player2) processScorched(player2);
            if (bot) processScorched(bot);
        }
    }
}

// ================================================================
//  handleDragonFire – Xử lý vùng biển lửa rồng thiêu đốt
//  Tướng: Làm chậm 50%
//  Lính: Thiêu rụi ngắt xích văng thành lính tự do
// ================================================================
void GameEngine::handleDragonFire() {
    for (auto de : dragonEvents) {
        for (auto& fz : de->getFireZones()) {
            if (!fz.isActive()) continue;

            auto processFire = [&](Character* ch) {
                if (!ch) return;

                // Tướng dính lửa rồng -> Giảm 50% tốc độ
                if (fz.isInZone(ch->getX(), ch->getY())) {
                    ch->applySlow(0.50f);
                }

                // Lính dính lửa rồng -> Bị thiêu rụi ngắt xích văng ra đất
                Troop* curr = ch->getArmy().getHead();
                while (curr) {
                    Troop* next = curr->next;
                    if (fz.isInZone(curr->x, curr->y)) {
                        auto freed = ch->getArmy().bombExplode(curr);
                        for (auto f : freed) {
                            freeTroops.push_back(f);
                        }
                        break;
                    }
                    curr = next;
                }
            };

            processFire(player1);
            if (player2) processFire(player2);
            if (bot) processFire(bot);
        }
    }
}

// ================================================================
//  clampToWorld – Giới hạn vị trí trong bản đồ
// ================================================================
void GameEngine::clampToWorld(float& px, float& py) {
    if (px < 32.f) px = 32.f;
    if (py < 32.f) py = 32.f;
    if (px > WORLD_W - 32.f) px = WORLD_W - 32.f;
    if (py > WORLD_H - 32.f) py = WORLD_H - 32.f;
}

// ================================================================
//  startClash – Chuyển sang Phase 2: Gom lính và dàn trận xung phong
// ================================================================
void GameEngine::startClash() {
    currentState = GameState::CLASH;
    clashTimer = 0.f;
    clashMarchTimer = 2.4f;
    clashChargeTimer = 0.f;
    clashFightTimer = 0.f;
    clashRoutTimer = 0.f;
    clashIsEnding = false;
    screenShakeTimer = 0.f;
    screenShakeIntensity = 0.f;

    if (gameMode == GameMode::PVE) {
        AudioManager::getInstance().playVoice("pve_clash.ogg");
    } else {
        AudioManager::getInstance().playVoice("pvp_clash.ogg");
    }
    AudioManager::getInstance().playBGM("bgm_clash.ogg");

    clashSlashes.clear();
    clashSparks.clear();
    clashFloatTexts.clear();
    clashDeadTroops.clear();

    // Gom lính: Base + lính đang dắt theo
    matchBlueBanked = baseBlue ? (int)baseBlue->getStoredTroops().size() : 0;
    matchRedBanked = baseRed ? (int)baseRed->getStoredTroops().size() : 0;

    // Phe Xanh
    if (baseBlue) {
        for (auto t : baseBlue->getStoredTroops()) blueClashArmy.push_back(t);
        baseBlue->getStoredTroops().clear();
    }
    auto p1troops = player1->getArmy().releaseAll();
    for (auto t : p1troops) blueClashArmy.push_back(t);

    // Phe Đỏ
    if (baseRed) {
        for (auto t : baseRed->getStoredTroops()) redClashArmy.push_back(t);
        baseRed->getStoredTroops().clear();
    }
    if (player2) {
        auto p2troops = player2->getArmy().releaseAll();
        for (auto t : p2troops) redClashArmy.push_back(t);
    }
    if (bot) {
        auto botTroops = bot->getArmy().releaseAll();
        for (auto t : botTroops) redClashArmy.push_back(t);
    }

    matchBlueClash = (int)blueClashArmy.size();
    matchRedClash = (int)redClashArmy.size();

    // Tính tổng power và thống kê chi tiết từng loại lính ban đầu
    blueTotalPower = 0;
    matchBlueSwordsman = 0;
    matchBlueGuardian = 0;
    matchBlueCavalry = 0;
    for (auto t : blueClashArmy) {
        blueTotalPower += t->power;
        if (t->type == TroopType::SWORDSMAN) matchBlueSwordsman++;
        else if (t->type == TroopType::GUARDIAN) matchBlueGuardian++;
        else if (t->type == TroopType::CAVALRY) matchBlueCavalry++;
    }
    matchBlueTotalTroops = (int)blueClashArmy.size();
    matchBlueTotalPower = blueTotalPower;

    redTotalPower = 0;
    matchRedSwordsman = 0;
    matchRedGuardian = 0;
    matchRedCavalry = 0;
    for (auto t : redClashArmy) {
        redTotalPower += t->power;
        if (t->type == TroopType::SWORDSMAN) matchRedSwordsman++;
        else if (t->type == TroopType::GUARDIAN) matchRedGuardian++;
        else if (t->type == TroopType::CAVALRY) matchRedCavalry++;
    }
    matchRedTotalTroops = (int)redClashArmy.size();
    matchRedTotalPower = redTotalPower;

    int totalPow = blueTotalPower + redTotalPower;
    tugOfWarRatio = (totalPow > 0) ? (float)blueTotalPower / totalPow : 0.5f;
    clashFrontlineX = 640.f;

    float centerY = 460.f;
    int fs = Troop::getFrameSize();

    // Thiết lập vị trí Tướng xuất phát ở ngoài rìa màn hình
    // Tướng Xanh: Ở vị trí x = -60.f, đi trước toàn bộ lính Xanh, mặt hướng sang Phải
    if (player1) {
        player1->setPosition(-60.f, centerY);
        player1->updateHeadAnimation(0.f, 1.f, 0.f); // Mặt quay sang phải (animRow = 2)
    }

    // Tướng Đỏ (Player 2 hoặc Bot): Ở vị trí x = 1340.f, đi trước toàn bộ lính Đỏ, mặt hướng sang Trái
    Character* redLeader = (gameMode == GameMode::PVP && player2) ? (Character*)player2 : (Character*)bot;
    if (redLeader) {
        redLeader->setPosition(1340.f, centerY);
        redLeader->updateHeadAnimation(0.f, -1.f, 0.f); // Mặt quay sang trái (animRow = 1)
    }

    // Thiết lập vị trí xuất phát cho lính 2 bên ở ngoài (off-screen)
    // Phe Xanh (bước ra từ mép trái, xếp hàng ngay phía sau Tướng Xanh)
    for (size_t i = 0; i < blueClashArmy.size(); ++i) {
        int col = (int)(i / 5);
        int row = (int)(i % 5);
        blueClashArmy[i]->x = -60.f - 65.f - col * 44.f;
        blueClashArmy[i]->y = centerY + (row - 2) * 44.f;
        blueClashArmy[i]->sprite.setPosition(blueClashArmy[i]->x, blueClashArmy[i]->y);
        blueClashArmy[i]->sprite.setScale(1.f, 1.f);
        blueClashArmy[i]->animRow = 2; // Hướng sang phải
        blueClashArmy[i]->animFrame = 0;
        blueClashArmy[i]->animTimer = 0.f;
        blueClashArmy[i]->sprite.setTextureRect(sf::IntRect(0, 2 * fs, fs, fs));
    }

    // Phe Đỏ (bước ra từ mép phải, xếp hàng ngay ngắn phía sau Tướng Đỏ)
    for (size_t i = 0; i < redClashArmy.size(); ++i) {
        int col = (int)(i / 5);
        int row = (int)(i % 5);
        redClashArmy[i]->x = 1340.f + 65.f + col * 44.f;
        redClashArmy[i]->y = centerY + (row - 2) * 44.f;
        redClashArmy[i]->sprite.setPosition(redClashArmy[i]->x, redClashArmy[i]->y);
        redClashArmy[i]->sprite.setScale(1.f, 1.f); // Không dùng -1.f, animRow 1 đã là hướng trái!
        redClashArmy[i]->animRow = 1; // Hướng sang trái
        redClashArmy[i]->animFrame = 0;
        redClashArmy[i]->animTimer = 0.f;
        redClashArmy[i]->sprite.setTextureRect(sf::IntRect(0, 1 * fs, fs, fs));
    }

    // Nếu cả 2 bên đều không có lính
    if (blueClashArmy.empty() && redClashArmy.empty()) {
        currentState = GameState::RESULT;
        matchWinner = 0;
        resultText = u8"BẤT PHÂN THẮNG BẠI!\nKhông bên nào có binh lính ra trận!";
        if (gameMode == GameMode::PVE) {
            AudioManager::getInstance().playVoice("pve_draw.ogg");
        } else {
            AudioManager::getInstance().playVoice("pvp_draw.ogg");
        }
        saveMatchHistoryToCSV("match_history.csv");
        return;
    }
}

// ================================================================
//  buildTiledBackground – Xây dựng VertexArray từ tile cỏ
// ================================================================
void GameEngine::buildTiledBackground() {
    int tileSize = 128;
    int tilesX = (int)(WORLD_W / tileSize) + 1;
    int tilesY = (int)(WORLD_H / tileSize) + 1;

    bgVertices.setPrimitiveType(sf::Quads);
    bgVertices.resize(tilesX * tilesY * 4);

    for (int ty = 0; ty < tilesY; ty++) {
        for (int tx = 0; tx < tilesX; tx++) {
            int idx = (ty * tilesX + tx) * 4;
            float px = (float)(tx * tileSize);
            float py = (float)(ty * tileSize);

            bgVertices[idx + 0].position = sf::Vector2f(px, py);
            bgVertices[idx + 1].position = sf::Vector2f(px + tileSize, py);
            bgVertices[idx + 2].position = sf::Vector2f(px + tileSize, py + tileSize);
            bgVertices[idx + 3].position = sf::Vector2f(px, py + tileSize);

            bgVertices[idx + 0].texCoords = sf::Vector2f(0, 0);
            bgVertices[idx + 1].texCoords = sf::Vector2f((float)tileSize, 0);
            bgVertices[idx + 2].texCoords = sf::Vector2f((float)tileSize, (float)tileSize);
            bgVertices[idx + 3].texCoords = sf::Vector2f(0, (float)tileSize);
        }
    }
}

// ================================================================
//  clampCamera – Giới hạn tâm camera không ra ngoài rìa bản đồ
// ================================================================
void GameEngine::clampCamera(sf::View& cam, float targetX, float targetY) {
    float halfW = cam.getSize().x / 2.f;
    float halfH = cam.getSize().y / 2.f;
    if (targetX < halfW) targetX = halfW;
    if (targetX > WORLD_W - halfW) targetX = WORLD_W - halfW;
    if (targetY < halfH) targetY = halfH;
    if (targetY > WORLD_H - halfH) targetY = WORLD_H - halfH;
    cam.setCenter(targetX, targetY);
}

// ================================================================
//  render
// ================================================================
void GameEngine::render() {
    window.clear(sf::Color(20, 20, 20));

    switch (currentState) {
        case GameState::MENU:            renderMenu(); break;
        case GameState::HERO_SELECT_PVE: renderHeroSelectPve(); break;
        case GameState::HERO_SELECT_PVP: renderHeroSelectPvp(); break;
        case GameState::GATHERING:       renderGathering(); break;
        case GameState::CLASH:           renderClash(); break;
        case GameState::RESULT:          renderResult(); break;
    }

    if (isPaused && (currentState == GameState::GATHERING || currentState == GameState::CLASH)) {
        renderPauseMenu();
    }

    window.display();
}

void GameEngine::renderMenu() {
    window.setView(window.getDefaultView());

    // 1. Vẽ hình nền menu 1280x720 (Đã có logo SNAKE ARMY hoàng kim phía trên)
    try {
        sf::Sprite menuBg(ResourceManager::getInstance().getTexture("menu_bg.png"));
        window.draw(menuBg);
    } catch (...) {
        sf::RectangleShape bg(sf::Vector2f(1280.f, 720.f));
        bg.setFillColor(sf::Color(25, 20, 20));
        window.draw(bg);
    }

    // 2. Vẽ 5 nút tương tác chính bằng ui_button.png (300x60)
    const std::string btnKeys[5] = { "btn_pve", "btn_pvp", "btn_story", "btn_settings", "btn_exit" };
    sf::Texture* btnTex = nullptr;
    try {
        btnTex = &ResourceManager::getInstance().getTexture("ui_button.png");
    } catch (...) {}

    for (int i = 0; i < 5; ++i) {
        float btnY = 330.f + i * 70.f;
        bool isHovered = (hoveredButtonIndex == i && menuModal == MenuModal::NONE);

        if (btnTex) {
            sf::Sprite btnSprite(*btnTex);
            btnSprite.setOrigin(150.f, 30.f);
            btnSprite.setPosition(640.f, btnY);
            if (isHovered) {
                btnSprite.setScale(1.05f, 1.05f);
                btnSprite.setColor(sf::Color(255, 255, 220));
            } else {
                btnSprite.setScale(1.0f, 1.0f);
                btnSprite.setColor(sf::Color(230, 230, 230));
            }
            window.draw(btnSprite);
        } else {
            sf::RectangleShape rect(sf::Vector2f(300.f, 54.f));
            rect.setOrigin(150.f, 27.f);
            rect.setPosition(640.f, btnY);
            rect.setFillColor(isHovered ? sf::Color(160, 30, 30) : sf::Color(100, 20, 20));
            rect.setOutlineColor(isHovered ? sf::Color(255, 230, 100) : sf::Color(200, 170, 70));
            rect.setOutlineThickness(2.5f);
            window.draw(rect);
        }

        if (gameFont) {
            sf::String label = ConfigManager::getInstance().getSfText(btnKeys[i]);
            sf::Text text(label, *gameFont, 16);
            text.setStyle(sf::Text::Bold);
            if (isHovered) {
                text.setFillColor(sf::Color(255, 245, 120));
                text.setOutlineColor(sf::Color(60, 20, 0));
                text.setOutlineThickness(1.5f);
            } else {
                text.setFillColor(sf::Color(250, 240, 220));
                text.setOutlineColor(sf::Color(20, 10, 0));
                text.setOutlineThickness(1.2f);
            }
            sf::FloatRect tb = text.getLocalBounds();
            text.setOrigin(tb.width / 2.f, tb.height / 2.f + 2.f);
            text.setPosition(640.f, btnY - 2.f);
            window.draw(text);
        }
    }

    // 3. Nếu đang mở modal thì vẽ đè lên
    if (menuModal == MenuModal::STORY) {
        renderStoryModal();
    } else if (menuModal == MenuModal::SETTINGS) {
        renderSettingsModal();
    }
}

// ================================================================
//  renderStoryModal – Cửa sổ Cốt truyện & Luật chơi cổ điển
// ================================================================
void GameEngine::renderStoryModal() {
    // 1. Phủ mờ nền toàn màn hình
    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 190));
    window.draw(dim);

    // 2. Vẽ khung cuộn hoàng gia ui_panel.png (860x540) tại (210, 90)
    try {
        sf::Sprite panel(ResourceManager::getInstance().getTexture("ui_panel.png"));
        panel.setPosition(210.f, 90.f);
        window.draw(panel);
    } catch (...) {
        sf::RectangleShape p(sf::Vector2f(860.f, 540.f));
        p.setPosition(210.f, 90.f);
        p.setFillColor(sf::Color(40, 30, 20));
        p.setOutlineColor(sf::Color(255, 215, 0));
        p.setOutlineThickness(3.f);
        window.draw(p);
    }

    if (!gameFont) return;

    // 3. Hai Tab: CỐT TRUYỆN | LUẬT CHƠI
    bool isStoryTab = (storyTab == StoryTab::STORY);

    // Tab 1: CỐT TRUYỆN [Tab / 1]
    sf::RectangleShape tab1(sf::Vector2f(230.f, 38.f));
    tab1.setOrigin(115.f, 19.f);
    tab1.setPosition(470.f, 182.f);
    tab1.setFillColor(isStoryTab ? sf::Color(140, 30, 30) : sf::Color(60, 45, 35, 180));
    tab1.setOutlineColor(isStoryTab ? sf::Color(255, 220, 100) : sf::Color(130, 100, 70));
    tab1.setOutlineThickness(isStoryTab ? 2.5f : 1.5f);
    window.draw(tab1);

    sf::Text tab1Text(ConfigManager::getInstance().getSfText("tab_story"), *gameFont, 16);
    tab1Text.setStyle(sf::Text::Bold);
    tab1Text.setFillColor(isStoryTab ? sf::Color(255, 240, 150) : sf::Color(190, 170, 140));
    sf::FloatRect t1b = tab1Text.getLocalBounds();
    tab1Text.setOrigin(t1b.width / 2.f, t1b.height / 2.f + 2.f);
    tab1Text.setPosition(470.f, 180.f);
    window.draw(tab1Text);

    // Tab 2: LUẬT CHƠI [Tab / 2]
    sf::RectangleShape tab2(sf::Vector2f(230.f, 38.f));
    tab2.setOrigin(115.f, 19.f);
    tab2.setPosition(810.f, 182.f);
    tab2.setFillColor(!isStoryTab ? sf::Color(140, 30, 30) : sf::Color(60, 45, 35, 180));
    tab2.setOutlineColor(!isStoryTab ? sf::Color(255, 220, 100) : sf::Color(130, 100, 70));
    tab2.setOutlineThickness(!isStoryTab ? 2.5f : 1.5f);
    window.draw(tab2);

    sf::Text tab2Text(ConfigManager::getInstance().getSfText("tab_rules"), *gameFont, 16);
    tab2Text.setStyle(sf::Text::Bold);
    tab2Text.setFillColor(!isStoryTab ? sf::Color(255, 240, 150) : sf::Color(190, 170, 140));
    sf::FloatRect t2b = tab2Text.getLocalBounds();
    tab2Text.setOrigin(t2b.width / 2.f, t2b.height / 2.f + 2.f);
    tab2Text.setPosition(810.f, 180.f);
    window.draw(tab2Text);

    // 4. Nội dung chi tiết trên mặt giấy da parchment
    if (isStoryTab) {
        // --- Phần PvE ---
        sf::Text pveHead(ConfigManager::getInstance().getSfText("story_pve_title"), *gameFont, 16);
        pveHead.setStyle(sf::Text::Bold);
        pveHead.setFillColor(sf::Color(20, 60, 150));
        pveHead.setPosition(325.f, 222.f);
        window.draw(pveHead);

        sf::Text pveBody(ConfigManager::getInstance().getSfText("story_pve_body"), *gameFont, 14);
        pveBody.setStyle(sf::Text::Regular);
        pveBody.setFillColor(sf::Color(40, 25, 15));
        pveBody.setLineSpacing(1.25f);
        pveBody.setPosition(325.f, 248.f);
        window.draw(pveBody);

        // --- Phần PvP ---
        sf::Text pvpHead(ConfigManager::getInstance().getSfText("story_pvp_title"), *gameFont, 16);
        pvpHead.setStyle(sf::Text::Bold);
        pvpHead.setFillColor(sf::Color(150, 25, 25));
        pvpHead.setPosition(325.f, 352.f);
        window.draw(pvpHead);

        sf::Text pvpBody(ConfigManager::getInstance().getSfText("story_pvp_body"), *gameFont, 14);
        pvpBody.setStyle(sf::Text::Regular);
        pvpBody.setFillColor(sf::Color(40, 25, 15));
        pvpBody.setLineSpacing(1.25f);
        pvpBody.setPosition(325.f, 378.f);
        window.draw(pvpBody);
    } else {
        // --- Luật Phase 1 ---
        sf::Text p1Head(ConfigManager::getInstance().getSfText("rules_p1_title"), *gameFont, 16);
        p1Head.setStyle(sf::Text::Bold);
        p1Head.setFillColor(sf::Color(20, 60, 150));
        p1Head.setPosition(325.f, 222.f);
        window.draw(p1Head);

        sf::Text p1Body(ConfigManager::getInstance().getSfText("rules_p1_body"), *gameFont, 13);
        p1Body.setStyle(sf::Text::Regular);
        p1Body.setFillColor(sf::Color(40, 25, 15));
        p1Body.setLineSpacing(1.22f);
        p1Body.setPosition(325.f, 246.f);
        window.draw(p1Body);

        // --- Luật Phase 2 ---
        sf::Text p2Head(ConfigManager::getInstance().getSfText("rules_p2_title"), *gameFont, 16);
        p2Head.setStyle(sf::Text::Bold);
        p2Head.setFillColor(sf::Color(150, 25, 25));
        p2Head.setPosition(325.f, 352.f);
        window.draw(p2Head);

        sf::Text p2Body(ConfigManager::getInstance().getSfText("rules_p2_body"), *gameFont, 13);
        p2Body.setStyle(sf::Text::Regular);
        p2Body.setFillColor(sf::Color(40, 25, 15));
        p2Body.setLineSpacing(1.22f);
        p2Body.setPosition(325.f, 376.f);
        window.draw(p2Body);
    }

    // 5. Nút ĐÓNG [ESC] ở đáy khung cuộn
    sf::FloatRect closeBtnRect(530.f, 510.f, 220.f, 40.f);
    bool isCloseHovered = closeBtnRect.contains(mousePos);

    try {
        sf::Sprite closeBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        closeBtn.setOrigin(150.f, 30.f);
        closeBtn.setPosition(640.f, 530.f);
        closeBtn.setScale(0.72f, 0.72f);
        if (isCloseHovered) closeBtn.setColor(sf::Color(255, 255, 220));
        window.draw(closeBtn);
    } catch (...) {
        sf::RectangleShape cBtn(sf::Vector2f(220.f, 40.f));
        cBtn.setOrigin(110.f, 20.f);
        cBtn.setPosition(640.f, 530.f);
        cBtn.setFillColor(isCloseHovered ? sf::Color(150, 40, 40) : sf::Color(90, 20, 20));
        cBtn.setOutlineColor(sf::Color(255, 215, 0));
        cBtn.setOutlineThickness(2.f);
        window.draw(cBtn);
    }

    sf::Text closeText(ConfigManager::getInstance().getSfText("btn_close"), *gameFont, 15);
    closeText.setStyle(sf::Text::Bold);
    closeText.setFillColor(isCloseHovered ? sf::Color(255, 245, 120) : sf::Color(240, 230, 200));
    sf::FloatRect cb = closeText.getLocalBounds();
    closeText.setOrigin(cb.width / 2.f, cb.height / 2.f + 2.f);
    closeText.setPosition(640.f, 528.f);
    window.draw(closeText);
}

// ================================================================
//  renderSettingsModal – Cửa sổ Cài đặt Âm thanh & Ngôn ngữ
// ================================================================
void GameEngine::renderSettingsModal() {
    // 1. Phủ mờ nền toàn màn hình
    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 190));
    window.draw(dim);

    // 2. Vẽ khung cuộn ui_panel.png
    try {
        sf::Sprite panel(ResourceManager::getInstance().getTexture("ui_panel.png"));
        panel.setPosition(210.f, 90.f);
        window.draw(panel);
    } catch (...) {
        sf::RectangleShape p(sf::Vector2f(860.f, 540.f));
        p.setPosition(210.f, 90.f);
        p.setFillColor(sf::Color(40, 30, 20));
        p.setOutlineColor(sf::Color(255, 215, 0));
        p.setOutlineThickness(3.f);
        window.draw(p);
    }

    if (!gameFont) return;

    // 3. Tiêu đề Cài Đặt Hệ Thống
    sf::Text title(ConfigManager::getInstance().getSfText("settings_title"), *gameFont, 22);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(140, 25, 25));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(640.f, 168.f);
    window.draw(title);

    // Đường gạch trang trí dưới tiêu đề
    sf::RectangleShape decorLine(sf::Vector2f(360.f, 2.f));
    decorLine.setOrigin(180.f, 1.f);
    decorLine.setPosition(640.f, 190.f);
    decorLine.setFillColor(sf::Color(160, 120, 60));
    window.draw(decorLine);

    auto& cfg = ConfigManager::getInstance();

    // ================================================================
    //  HÀNG 1: ÂM NHẠC (MUSIC / BGM) - y = 230
    // ================================================================
    sf::Text musicLbl(cfg.getSfText("settings_music"), *gameFont, 18);
    musicLbl.setStyle(sf::Text::Bold);
    musicLbl.setFillColor(sf::Color(40, 25, 10));
    musicLbl.setPosition(320.f, 222.f);
    window.draw(musicLbl);

    // Nút Bật/Tắt Music
    sf::FloatRect mToggleRect(550.f, 215.f, 90.f, 38.f);
    bool mHover = mToggleRect.contains(mousePos);
    sf::RectangleShape mToggleBtn(sf::Vector2f(90.f, 38.f));
    mToggleBtn.setPosition(550.f, 215.f);
    mToggleBtn.setFillColor(cfg.isMusicEnabled() ? sf::Color(40, 130, 50) : sf::Color(140, 40, 40));
    mToggleBtn.setOutlineColor(mHover ? sf::Color(255, 230, 100) : sf::Color(180, 150, 90));
    mToggleBtn.setOutlineThickness(2.f);
    window.draw(mToggleBtn);

    sf::String mStatus = cfg.isMusicEnabled() ? cfg.getSfText("settings_on") : cfg.getSfText("settings_off");
    sf::Text mStatusText(mStatus, *gameFont, 15);
    mStatusText.setStyle(sf::Text::Bold);
    mStatusText.setFillColor(sf::Color::White);
    sf::FloatRect mstb = mStatusText.getLocalBounds();
    mStatusText.setOrigin(mstb.width / 2.f, mstb.height / 2.f + 2.f);
    mStatusText.setPosition(595.f, 232.f);
    window.draw(mStatusText);

    // Nút [-] Volume Music
    sf::FloatRect mMinusRect(670.f, 215.f, 38.f, 38.f);
    bool mMinusHover = mMinusRect.contains(mousePos);
    sf::RectangleShape mMinusBtn(sf::Vector2f(38.f, 38.f));
    mMinusBtn.setPosition(670.f, 215.f);
    mMinusBtn.setFillColor(mMinusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    mMinusBtn.setOutlineColor(sf::Color(200, 160, 90));
    mMinusBtn.setOutlineThickness(1.5f);
    window.draw(mMinusBtn);

    sf::Text mMinusText("-", *gameFont, 22);
    mMinusText.setStyle(sf::Text::Bold);
    mMinusText.setFillColor(sf::Color::White);
    sf::FloatRect mmtb = mMinusText.getLocalBounds();
    mMinusText.setOrigin(mmtb.width / 2.f, mmtb.height / 2.f + 3.f);
    mMinusText.setPosition(689.f, 231.f);
    window.draw(mMinusText);

    // Hiển thị % Âm lượng
    std::string mVolStr = std::to_string(cfg.getMusicVolume()) + "%";
    sf::Text mVolText(mVolStr, *gameFont, 16);
    mVolText.setStyle(sf::Text::Bold);
    mVolText.setFillColor(sf::Color(45, 25, 10));
    sf::FloatRect mvb = mVolText.getLocalBounds();
    mVolText.setOrigin(mvb.width / 2.f, mvb.height / 2.f + 2.f);
    mVolText.setPosition(755.f, 232.f);
    window.draw(mVolText);

    // Nút [+] Volume Music
    sf::FloatRect mPlusRect(800.f, 215.f, 38.f, 38.f);
    bool mPlusHover = mPlusRect.contains(mousePos);
    sf::RectangleShape mPlusBtn(sf::Vector2f(38.f, 38.f));
    mPlusBtn.setPosition(800.f, 215.f);
    mPlusBtn.setFillColor(mPlusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    mPlusBtn.setOutlineColor(sf::Color(200, 160, 90));
    mPlusBtn.setOutlineThickness(1.5f);
    window.draw(mPlusBtn);

    sf::Text mPlusText("+", *gameFont, 20);
    mPlusText.setStyle(sf::Text::Bold);
    mPlusText.setFillColor(sf::Color::White);
    sf::FloatRect mptb = mPlusText.getLocalBounds();
    mPlusText.setOrigin(mptb.width / 2.f, mptb.height / 2.f + 3.f);
    mPlusText.setPosition(819.f, 232.f);
    window.draw(mPlusText);

    // ================================================================
    //  HÀNG 2: HIỆU ỨNG ÂM THANH (SFX) - y = 285
    // ================================================================
    sf::Text sfxLbl(cfg.getSfText("settings_sfx"), *gameFont, 18);
    sfxLbl.setStyle(sf::Text::Bold);
    sfxLbl.setFillColor(sf::Color(40, 25, 10));
    sfxLbl.setPosition(320.f, 287.f);
    window.draw(sfxLbl);

    // Nút Bật/Tắt SFX
    sf::FloatRect sToggleRect(550.f, 280.f, 90.f, 38.f);
    bool sHover = sToggleRect.contains(mousePos);
    sf::RectangleShape sToggleBtn(sf::Vector2f(90.f, 38.f));
    sToggleBtn.setPosition(550.f, 280.f);
    sToggleBtn.setFillColor(cfg.isSfxEnabled() ? sf::Color(40, 130, 50) : sf::Color(140, 40, 40));
    sToggleBtn.setOutlineColor(sHover ? sf::Color(255, 230, 100) : sf::Color(180, 150, 90));
    sToggleBtn.setOutlineThickness(2.f);
    window.draw(sToggleBtn);

    sf::String sStatus = cfg.isSfxEnabled() ? cfg.getSfText("settings_on") : cfg.getSfText("settings_off");
    sf::Text sStatusText(sStatus, *gameFont, 15);
    sStatusText.setStyle(sf::Text::Bold);
    sStatusText.setFillColor(sf::Color::White);
    sf::FloatRect sstb = sStatusText.getLocalBounds();
    sStatusText.setOrigin(sstb.width / 2.f, sstb.height / 2.f + 2.f);
    sStatusText.setPosition(595.f, 297.f);
    window.draw(sStatusText);

    // Nút [-] Volume SFX
    sf::FloatRect sMinusRect(670.f, 280.f, 38.f, 38.f);
    bool sMinusHover = sMinusRect.contains(mousePos);
    sf::RectangleShape sMinusBtn(sf::Vector2f(38.f, 38.f));
    sMinusBtn.setPosition(670.f, 280.f);
    sMinusBtn.setFillColor(sMinusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    sMinusBtn.setOutlineColor(sf::Color(200, 160, 90));
    sMinusBtn.setOutlineThickness(1.5f);
    window.draw(sMinusBtn);

    sf::Text sMinusText("-", *gameFont, 22);
    sMinusText.setStyle(sf::Text::Bold);
    sMinusText.setFillColor(sf::Color::White);
    sf::FloatRect smtb = sMinusText.getLocalBounds();
    sMinusText.setOrigin(smtb.width / 2.f, smtb.height / 2.f + 3.f);
    sMinusText.setPosition(689.f, 296.f);
    window.draw(sMinusText);

    // Hiển thị % SFX
    std::string sVolStr = std::to_string(cfg.getSfxVolume()) + "%";
    sf::Text sVolText(sVolStr, *gameFont, 16);
    sVolText.setStyle(sf::Text::Bold);
    sVolText.setFillColor(sf::Color(45, 25, 10));
    sf::FloatRect svb = sVolText.getLocalBounds();
    sVolText.setOrigin(svb.width / 2.f, svb.height / 2.f + 2.f);
    sVolText.setPosition(755.f, 297.f);
    window.draw(sVolText);

    // Nút [+] Volume SFX
    sf::FloatRect sPlusRect(800.f, 280.f, 38.f, 38.f);
    bool sPlusHover = sPlusRect.contains(mousePos);
    sf::RectangleShape sPlusBtn(sf::Vector2f(38.f, 38.f));
    sPlusBtn.setPosition(800.f, 280.f);
    sPlusBtn.setFillColor(sPlusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    sPlusBtn.setOutlineColor(sf::Color(200, 160, 90));
    sPlusBtn.setOutlineThickness(1.5f);
    window.draw(sPlusBtn);

    sf::Text sPlusText("+", *gameFont, 20);
    sPlusText.setStyle(sf::Text::Bold);
    sPlusText.setFillColor(sf::Color::White);
    sf::FloatRect sptb = sPlusText.getLocalBounds();
    sPlusText.setOrigin(sptb.width / 2.f, sptb.height / 2.f + 3.f);
    sPlusText.setPosition(819.f, 297.f);
    window.draw(sPlusText);

    // ================================================================
    //  HÀNG 3: NGÔN NGỮ (LANGUAGE) - y = 350
    // ================================================================
    sf::Text langLbl(cfg.getSfText("settings_lang"), *gameFont, 18);
    langLbl.setStyle(sf::Text::Bold);
    langLbl.setFillColor(sf::Color(40, 25, 10));
    langLbl.setPosition(320.f, 357.f);
    window.draw(langLbl);

    sf::FloatRect langToggleRect(550.f, 350.f, 290.f, 40.f);
    bool langHover = langToggleRect.contains(mousePos);
    sf::RectangleShape langBtn(sf::Vector2f(290.f, 40.f));
    langBtn.setPosition(550.f, 350.f);
    langBtn.setFillColor(langHover ? sf::Color(140, 40, 40) : sf::Color(100, 25, 25));
    langBtn.setOutlineColor(langHover ? sf::Color(255, 230, 100) : sf::Color(200, 160, 80));
    langBtn.setOutlineThickness(2.f);
    window.draw(langBtn);

    sf::String currentLangStr = (cfg.getLanguage() == Language::VI) ? ConfigManager::toUtf8(u8"Tiếng Việt [Click đổi]") : ConfigManager::toUtf8("English [Click change]");
    sf::Text langText(currentLangStr, *gameFont, 15);
    langText.setStyle(sf::Text::Bold);
    langText.setFillColor(sf::Color(255, 235, 140));
    sf::FloatRect ltb = langText.getLocalBounds();
    langText.setOrigin(ltb.width / 2.f, ltb.height / 2.f + 2.f);
    langText.setPosition(695.f, 368.f);
    window.draw(langText);

    // ================================================================
    //  HÀNG 4: HƯỚNG DẪN PHÍM TẮT
    // ================================================================
    sf::Text hintText(cfg.getSfText("settings_hint"), *gameFont, 14);
    hintText.setStyle(sf::Text::Italic);
    hintText.setFillColor(sf::Color(90, 60, 40));
    sf::FloatRect htb = hintText.getLocalBounds();
    hintText.setOrigin(htb.width / 2.f, htb.height / 2.f);
    hintText.setPosition(640.f, 435.f);
    window.draw(hintText);

    // ================================================================
    //  NÚT ĐÓNG [ESC]
    // ================================================================
    sf::FloatRect closeBtnRect(530.f, 505.f, 220.f, 40.f);
    bool isCloseHovered = closeBtnRect.contains(mousePos);

    try {
        sf::Sprite closeBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        closeBtn.setOrigin(150.f, 30.f);
        closeBtn.setPosition(640.f, 525.f);
        closeBtn.setScale(0.72f, 0.72f);
        if (isCloseHovered) closeBtn.setColor(sf::Color(255, 255, 220));
        window.draw(closeBtn);
    } catch (...) {
        sf::RectangleShape cBtn(sf::Vector2f(220.f, 40.f));
        cBtn.setOrigin(110.f, 20.f);
        cBtn.setPosition(640.f, 525.f);
        cBtn.setFillColor(isCloseHovered ? sf::Color(150, 40, 40) : sf::Color(90, 20, 20));
        cBtn.setOutlineColor(sf::Color(255, 215, 0));
        cBtn.setOutlineThickness(2.f);
        window.draw(cBtn);
    }

    sf::Text closeText(ConfigManager::getInstance().getSfText("btn_close"), *gameFont, 15);
    closeText.setStyle(sf::Text::Bold);
    closeText.setFillColor(isCloseHovered ? sf::Color(255, 245, 120) : sf::Color(240, 230, 200));
    sf::FloatRect cb = closeText.getLocalBounds();
    closeText.setOrigin(cb.width / 2.f, cb.height / 2.f + 2.f);
    closeText.setPosition(640.f, 523.f);
    window.draw(closeText);
}

// ================================================================
//  wrapText – Tự động xuống dòng khi độ rộng vượt quá maxW
// ================================================================
static std::string wrapText(const std::string& input, const sf::Font* font, unsigned int charSize, float maxW) {
    if (!font || maxW <= 10.f) return input;
    std::string result = "";
    std::stringstream ss(input);
    std::string paragraph;
    bool firstPara = true;

    while (std::getline(ss, paragraph, '\n')) {
        if (!firstPara) result += "\n";
        firstPara = false;

        std::stringstream wordStream(paragraph);
        std::string word;
        std::string currentLine = "";

        while (wordStream >> word) {
            std::string test = currentLine.empty() ? word : (currentLine + " " + word);
            sf::Text t(ConfigManager::toUtf8(test), *font, charSize);
            if (t.getLocalBounds().width > maxW && !currentLine.empty()) {
                if (!result.empty() && result.back() != '\n') result += "\n";
                result += currentLine;
                currentLine = word;
            } else {
                currentLine = test;
            }
        }
        if (!currentLine.empty()) {
            if (!result.empty() && result.back() != '\n') result += "\n";
            result += currentLine;
        }
    }
    return result;
}

// ================================================================
//  renderHeroSelectPve – Màn hình Chọn Tướng Chế Độ PvE
// ================================================================
void GameEngine::renderHeroSelectPve() {
    window.setView(window.getDefaultView());

    // 1. Hình nền mờ hoàng gia
    try {
        sf::Sprite menuBg(ResourceManager::getInstance().getTexture("menu_bg.png"));
        window.draw(menuBg);
    } catch (...) {
        sf::RectangleShape bg(sf::Vector2f(1280.f, 720.f));
        bg.setFillColor(sf::Color(25, 20, 20));
        window.draw(bg);
    }

    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(10, 14, 22, 225));
    window.draw(dim);

    if (!gameFont) return;

    // 2. Banner Tiêu Đề Hoàng Gia
    try {
        sf::Sprite ribbon(ResourceManager::getInstance().getTexture("banner_ribbon.png"));
        ribbon.setOrigin(300.f, 45.f);
        ribbon.setPosition(640.f, 52.f);
        ribbon.setScale(0.95f, 0.95f);
        window.draw(ribbon);
    } catch (...) {}

    sf::Text title(ConfigManager::toUtf8(u8"CHỌN TƯỚNG XUẤT TRẬN (PVE)"), *gameFont, 23);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 235, 110));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(640.f, 48.f);
    window.draw(title);

    sf::Text sub(ConfigManager::toUtf8(u8"Chọn 1 trong 3 Tướng Lĩnh Phe Xanh để thống lĩnh đại quân chinh chiến!"), *gameFont, 14);
    sub.setFillColor(sf::Color(190, 220, 245));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setOrigin(sb.width / 2.f, sb.height / 2.f);
    sub.setPosition(640.f, 82.f);
    window.draw(sub);

    // 3. Thông số & Nội dung 3 Tướng Lĩnh
    struct HeroCardData {
        HeroClass hClass;
        std::string name;
        std::string title;
        std::string sheet;
        sf::Color themeCol;
        std::string speedStr;
        std::string cdStr;
        std::string traitStr;
        std::string skillName;
        std::string skillDesc;
    };

    const HeroCardData cards[3] = {
        {
            HeroClass::VANGUARD,
            u8"TIÊN PHONG",
            u8"Chiến Tướng Xung Kích",
            "player1_sheet.png",
            sf::Color(55, 190, 255),
            u8"225 (Cực Nhanh)",
            "7.0s",
            u8"Cắt đứt đuôi hàng ngũ",
            u8"[SPACE] ĐOẠT MỆNH TRẢM",
            u8"• Lướt kiếm thần tốc theo hướng đi.\n• Xuyên thấu đội hình, cắt lính địch.\n• Cướp toàn bộ lính bị cắt về phe ta."
        },
        {
            HeroClass::TACTICIAN,
            u8"QUÂN SƯ",
            u8"Kỳ Tài Trận Pháp",
            "hero_mage_sheet.png",
            sf::Color(215, 120, 255),
            u8"210 (Cân Bằng)",
            "8.5s",
            u8"Hút lính & vật phẩm (280px)",
            u8"[SPACE] BÁT QUÁI HẤP TINH",
            u8"• Triệu hồi bão xoáy trong 3.5 giây.\n• Tự động hút lính & đồ gần (280px).\n• Làm chậm 60% tốc độ Tướng địch."
        },
        {
            HeroClass::TITAN,
            u8"HỘ QUỐC",
            u8"Thần Tướng Kim Cang",
            "hero_titan_sheet.png",
            sf::Color(255, 195, 55),
            u8"195 (Chắc Chắn)",
            "9.5s",
            u8"Choáng địch & Bất tử 4s",
            u8"[SPACE] KIM CANG HỐNG",
            u8"• Gầm vang làm choáng địch 1.5s (260px).\n• Bật Khiên Hoàng Kim bất tử 4.0 giây.\n• Bảo hộ toàn diện Tướng và đại quân."
        }
    };

    float cardXPositions[3] = { 110.f, 480.f, 850.f };
    float cardY = 112.f;
    float cardW = 320.f;
    float cardH = 465.f;

    for (int i = 0; i < 3; ++i) {
        float cx = cardXPositions[i];
        bool isSelected = (p1SelectedHero == cards[i].hClass);
        bool isHovered = (heroSelectP1Hover == i);

        // Khung nền thẻ
        sf::RectangleShape box(sf::Vector2f(cardW, cardH));
        box.setPosition(cx, cardY);
        box.setFillColor(isSelected ? sf::Color(20, 36, 56, 240) : (isHovered ? sf::Color(24, 30, 42, 230) : sf::Color(14, 18, 28, 215)));
        box.setOutlineColor(isSelected ? cards[i].themeCol : (isHovered ? sf::Color(220, 220, 180) : sf::Color(65, 80, 105, 160)));
        box.setOutlineThickness(isSelected ? 3.5f : (isHovered ? 2.5f : 1.5f));
        window.draw(box);

        // Thanh Header của Thẻ
        sf::RectangleShape header(sf::Vector2f(cardW, 46.f));
        header.setPosition(cx, cardY);
        sf::Color headerCol = cards[i].themeCol;
        headerCol.a = isSelected ? 90 : 40;
        header.setFillColor(headerCol);
        window.draw(header);

        // Tên Tướng
        sf::Text nameTxt(ConfigManager::toUtf8(cards[i].name), *gameFont, 18);
        nameTxt.setStyle(sf::Text::Bold);
        nameTxt.setFillColor(isSelected ? sf::Color(255, 245, 160) : cards[i].themeCol);
        nameTxt.setPosition(cx + 15.f, cardY + 5.f);
        window.draw(nameTxt);

        // Danh hiệu Tướng
        sf::Text titleTxt(ConfigManager::toUtf8(cards[i].title), *gameFont, 12);
        titleTxt.setStyle(sf::Text::Italic);
        titleTxt.setFillColor(sf::Color(200, 210, 225));
        titleTxt.setPosition(cx + 15.f, cardY + 26.f);
        window.draw(titleTxt);

        // Bệ đỡ hào quang dưới chân Tướng
        sf::CircleShape pedestal(38.f);
        pedestal.setScale(1.2f, 0.45f);
        pedestal.setOrigin(38.f, 38.f);
        pedestal.setPosition(cx + cardW / 2.f, cardY + 165.f);
        sf::Color pedCol = cards[i].themeCol;
        pedCol.a = isSelected ? 120 : 50;
        pedestal.setFillColor(pedCol);
        window.draw(pedestal);

        // Mô hình Tướng di động (Sprite animation 4 frames Row 0)
        int curFrame = (int)(heroAnimTimer * 5.f) % 4;
        try {
            sf::Sprite heroSprite(ResourceManager::getInstance().getTexture(cards[i].sheet));
            heroSprite.setTextureRect(sf::IntRect(curFrame * 64, 0, 64, 64));
            heroSprite.setOrigin(32.f, 32.f);
            heroSprite.setPosition(cx + cardW / 2.f, cardY + 140.f);
            heroSprite.setScale(1.6f, 1.6f);
            window.draw(heroSprite);
        } catch (...) {}

        // Trạng thái Đang Chọn / Hướng dẫn
        if (isSelected) {
            sf::RectangleShape selBadge(sf::Vector2f(170.f, 26.f));
            selBadge.setOrigin(85.f, 13.f);
            selBadge.setPosition(cx + cardW / 2.f, cardY + 208.f);
            selBadge.setFillColor(sf::Color(30, 140, 70, 210));
            selBadge.setOutlineColor(sf::Color(100, 255, 150));
            selBadge.setOutlineThickness(1.5f);
            window.draw(selBadge);

            sf::Text selTxt(ConfigManager::toUtf8(u8"✓ ĐANG CHỌN"), *gameFont, 12);
            selTxt.setStyle(sf::Text::Bold);
            selTxt.setFillColor(sf::Color(255, 255, 230));
            sf::FloatRect stb = selTxt.getLocalBounds();
            selTxt.setOrigin(stb.width / 2.f, stb.height / 2.f + 1.f);
            selTxt.setPosition(cx + cardW / 2.f, cardY + 208.f);
            window.draw(selTxt);
        } else {
            std::string hint = u8"Click hoặc [Phím " + std::to_string(i + 1) + u8"] để chọn";
            sf::Text selHint(ConfigManager::toUtf8(hint), *gameFont, 12);
            selHint.setFillColor(sf::Color(170, 185, 205));
            sf::FloatRect shb = selHint.getLocalBounds();
            selHint.setOrigin(shb.width / 2.f, shb.height / 2.f);
            selHint.setPosition(cx + cardW / 2.f, cardY + 208.f);
            window.draw(selHint);
        }

        // Hộp thông số Tướng (Stats Box)
        sf::RectangleShape statsBox(sf::Vector2f(cardW - 24.f, 70.f));
        statsBox.setPosition(cx + 12.f, cardY + 232.f);
        statsBox.setFillColor(sf::Color(10, 15, 25, 200));
        statsBox.setOutlineColor(sf::Color(70, 85, 110, 130));
        statsBox.setOutlineThickness(1.f);
        window.draw(statsBox);

        sf::Text stat1(ConfigManager::toUtf8(u8"• Tốc độ chạy: " + cards[i].speedStr), *gameFont, 11.5f);
        stat1.setFillColor(sf::Color(220, 230, 245));
        stat1.setPosition(cx + 20.f, cardY + 237.f);
        window.draw(stat1);

        sf::Text stat2(ConfigManager::toUtf8(u8"• Thời gian hồi: " + cards[i].cdStr), *gameFont, 11.5f);
        stat2.setFillColor(sf::Color(220, 230, 245));
        stat2.setPosition(cx + 20.f, cardY + 256.f);
        window.draw(stat2);

        sf::Text stat3(ConfigManager::toUtf8(u8"• Đặc trưng: " + cards[i].traitStr), *gameFont, 11.5f);
        stat3.setStyle(sf::Text::Bold);
        stat3.setFillColor(cards[i].themeCol);
        stat3.setPosition(cx + 20.f, cardY + 275.f);
        window.draw(stat3);

        // Hộp Kỹ Năng Tuyệt Kỹ (Skill Box) - Rộng rãi, không bao giờ tràn
        sf::RectangleShape skillBox(sf::Vector2f(cardW - 24.f, 144.f));
        skillBox.setPosition(cx + 12.f, cardY + 310.f);
        skillBox.setFillColor(sf::Color(8, 12, 20, 230));
        sf::Color skillOutline = cards[i].themeCol;
        skillOutline.a = isSelected ? 200 : 90;
        skillBox.setOutlineColor(skillOutline);
        skillBox.setOutlineThickness(1.5f);
        window.draw(skillBox);

        // Tiêu đề kỹ năng có nền mạ màu
        sf::RectangleShape skHeader(sf::Vector2f(cardW - 24.f, 26.f));
        skHeader.setPosition(cx + 12.f, cardY + 310.f);
        sf::Color skHeadCol = cards[i].themeCol;
        skHeadCol.a = isSelected ? 65 : 35;
        skHeader.setFillColor(skHeadCol);
        window.draw(skHeader);

        sf::Text skTitle(ConfigManager::toUtf8(cards[i].skillName), *gameFont, 12.5f);
        skTitle.setStyle(sf::Text::Bold);
        skTitle.setFillColor(cards[i].themeCol);
        skTitle.setPosition(cx + 20.f, cardY + 314.f);
        window.draw(skTitle);

        std::string wrappedDesc = wrapText(cards[i].skillDesc, gameFont, 11, 276.f);
        sf::Text skDesc(ConfigManager::toUtf8(wrappedDesc), *gameFont, 11);
        skDesc.setLineSpacing(1.22f);
        skDesc.setFillColor(sf::Color(215, 225, 240));
        skDesc.setPosition(cx + 20.f, cardY + 344.f);
        window.draw(skDesc);

        // Chân thẻ ánh kim nếu đang chọn
        if (isSelected) {
            sf::RectangleShape bar(sf::Vector2f(cardW, 4.f));
            bar.setPosition(cx, cardY + cardH - 4.f);
            bar.setFillColor(cards[i].themeCol);
            window.draw(bar);
        }
    }

    // 4. Nút Hành Động Ở Dưới (Action Bar)
    // Nút Quay Lại [ESC]
    sf::FloatRect backBtnRect(110.f, 592.f, 180.f, 50.f);
    bool isBackHover = backBtnRect.contains(mousePos);
    sf::RectangleShape backBtn(sf::Vector2f(180.f, 50.f));
    backBtn.setPosition(110.f, 592.f);
    backBtn.setFillColor(isBackHover ? sf::Color(100, 30, 30, 220) : sf::Color(55, 20, 20, 200));
    backBtn.setOutlineColor(isBackHover ? sf::Color(255, 180, 160) : sf::Color(160, 70, 70));
    backBtn.setOutlineThickness(1.8f);
    window.draw(backBtn);

    sf::Text backText(ConfigManager::toUtf8(u8"QUAY LẠI [ESC]"), *gameFont, 14);
    backText.setStyle(sf::Text::Bold);
    backText.setFillColor(isBackHover ? sf::Color::White : sf::Color(230, 210, 210));
    sf::FloatRect bb = backText.getLocalBounds();
    backText.setOrigin(bb.width / 2.f, bb.height / 2.f + 1.f);
    backText.setPosition(200.f, 617.f);
    window.draw(backText);

    // Nút Xác Nhận Xuất Trận [ENTER]
    sf::FloatRect startBtnRect(490.f, 588.f, 300.f, 58.f);
    bool isStartHover = startBtnRect.contains(mousePos);

    try {
        sf::Sprite btnSprite(ResourceManager::getInstance().getTexture("ui_button.png"));
        btnSprite.setOrigin(150.f, 30.f);
        btnSprite.setPosition(640.f, 617.f);
        btnSprite.setScale(isStartHover ? 1.05f : 1.0f, isStartHover ? 1.05f : 1.0f);
        if (isStartHover) btnSprite.setColor(sf::Color(255, 255, 220));
        window.draw(btnSprite);
    } catch (...) {
        sf::RectangleShape sBtn(sf::Vector2f(300.f, 58.f));
        sBtn.setOrigin(150.f, 29.f);
        sBtn.setPosition(640.f, 617.f);
        sBtn.setFillColor(isStartHover ? sf::Color(170, 40, 40) : sf::Color(120, 25, 25));
        sBtn.setOutlineColor(sf::Color(255, 215, 0));
        sBtn.setOutlineThickness(2.5f);
        window.draw(sBtn);
    }

    sf::Text startText(ConfigManager::toUtf8(u8"XÁC NHẬN XUẤT TRẬN"), *gameFont, 16);
    startText.setStyle(sf::Text::Bold);
    startText.setFillColor(isStartHover ? sf::Color(255, 250, 130) : sf::Color(255, 230, 160));
    sf::FloatRect sb2 = startText.getLocalBounds();
    startText.setOrigin(sb2.width / 2.f, sb2.height / 2.f + 1.f);
    startText.setPosition(640.f, 616.f);
    window.draw(startText);

    // Gợi ý phím tắt phía dưới cùng
    sf::Text hintBot(ConfigManager::toUtf8(u8"Phím [1, 2, 3] hoặc Click chọn Tướng | [ENTER / SPACE] Xuất Trận | [ESC] Trở Về"), *gameFont, 13);
    hintBot.setFillColor(sf::Color(170, 185, 205));
    sf::FloatRect hbb = hintBot.getLocalBounds();
    hintBot.setOrigin(hbb.width / 2.f, hbb.height / 2.f);
    hintBot.setPosition(640.f, 672.f);
    window.draw(hintBot);
}

// ================================================================
//  renderHeroSelectPvp – Màn hình Chọn Tướng Chế Độ PvP
// ================================================================
void GameEngine::renderHeroSelectPvp() {
    window.setView(window.getDefaultView());

    // 1. Nền mờ hoàng gia
    try {
        sf::Sprite menuBg(ResourceManager::getInstance().getTexture("menu_bg.png"));
        window.draw(menuBg);
    } catch (...) {
        sf::RectangleShape bg(sf::Vector2f(1280.f, 720.f));
        bg.setFillColor(sf::Color(25, 20, 20));
        window.draw(bg);
    }

    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(10, 14, 22, 225));
    window.draw(dim);

    if (!gameFont) return;

    // 2. Banner Tiêu Đề
    try {
        sf::Sprite ribbon(ResourceManager::getInstance().getTexture("banner_ribbon.png"));
        ribbon.setOrigin(300.f, 45.f);
        ribbon.setPosition(640.f, 48.f);
        ribbon.setScale(0.95f, 0.95f);
        window.draw(ribbon);
    } catch (...) {}

    sf::Text title(ConfigManager::toUtf8(u8"ĐẠI HỘI TRANH HÙNG (PVP) - CHỌN TƯỚNG"), *gameFont, 22);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 235, 110));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(640.f, 44.f);
    window.draw(title);

    sf::Text sub(ConfigManager::toUtf8(u8"P1 chọn Tướng Phe Xanh (Bên Trái) | P2 chọn Tướng Phe Đỏ (Bên Phải)"), *gameFont, 13);
    sub.setFillColor(sf::Color(200, 220, 245));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setOrigin(sb.width / 2.f, sb.height / 2.f);
    sub.setPosition(640.f, 78.f);
    window.draw(sub);

    // Tiêu đề 2 bên: P1 Phe Xanh & P2 Phe Đỏ
    sf::Text p1HeadTxt(ConfigManager::toUtf8(u8"PLAYER 1 (PHE XANH) - PHÍM [1, 2, 3]"), *gameFont, 15);
    p1HeadTxt.setStyle(sf::Text::Bold);
    p1HeadTxt.setFillColor(sf::Color(90, 205, 255));
    sf::FloatRect p1hb = p1HeadTxt.getLocalBounds();
    p1HeadTxt.setOrigin(p1hb.width / 2.f, p1hb.height / 2.f);
    p1HeadTxt.setPosition(318.f, 102.f);
    window.draw(p1HeadTxt);

    sf::Text p2HeadTxt(ConfigManager::toUtf8(u8"PLAYER 2 (PHE ĐỎ) - PHÍM [NUM 1, 2, 3]"), *gameFont, 15);
    p2HeadTxt.setStyle(sf::Text::Bold);
    p2HeadTxt.setFillColor(sf::Color(255, 115, 105));
    sf::FloatRect p2hb = p2HeadTxt.getLocalBounds();
    p2HeadTxt.setOrigin(p2hb.width / 2.f, p2hb.height / 2.f);
    p2HeadTxt.setPosition(962.f, 102.f);
    window.draw(p2HeadTxt);

    // Đường phân chia trung tâm & Biểu tượng VS
    sf::RectangleShape divLine(sf::Vector2f(2.f, 460.f));
    divLine.setPosition(640.f, 116.f);
    divLine.setFillColor(sf::Color(170, 130, 65, 140));
    window.draw(divLine);

    try {
        sf::Sprite vsSwords(ResourceManager::getInstance().getTexture("icon_clash_swords.png"));
        vsSwords.setOrigin(32.f, 32.f);
        vsSwords.setPosition(640.f, 320.f);
        vsSwords.setScale(0.70f, 0.70f);
        window.draw(vsSwords);
    } catch (...) {}

    sf::Text vsTxt("VS", *gameFont, 18);
    vsTxt.setStyle(sf::Text::Bold);
    vsTxt.setFillColor(sf::Color(255, 215, 0));
    sf::FloatRect vsb = vsTxt.getLocalBounds();
    vsTxt.setOrigin(vsb.width / 2.f, vsb.height / 2.f);
    vsTxt.setPosition(640.f, 362.f);
    window.draw(vsTxt);

    int curFrame = (int)(heroAnimTimer * 5.f) % 4;

    // Helper vẽ thẻ tướng compact trong PvP - đảm bảo 100% không tràn khung
    auto drawPvPCard = [&](float cx, float cy, float cw, float ch,
                           HeroClass hClass, const std::string& name,
                           const std::string& sheet, sf::Color themeCol,
                           bool isSelected, bool isHovered,
                           const std::string& keyHint,
                           const std::string& skillName,
                           const std::string& keyBadge,
                           const std::string& speedInfo,
                           const std::string& cdInfo,
                           const std::string& bullet1,
                           const std::string& bullet2,
                           const std::string& bullet3) {
        // Nền thẻ
        sf::RectangleShape box(sf::Vector2f(cw, ch));
        box.setPosition(cx, cy);
        box.setFillColor(isSelected ? sf::Color(themeCol.r / 6, themeCol.g / 6, themeCol.b / 6 + 15, 240) : (isHovered ? sf::Color(25, 30, 42, 230) : sf::Color(14, 18, 28, 215)));
        box.setOutlineColor(isSelected ? themeCol : (isHovered ? sf::Color(220, 220, 180) : sf::Color(65, 80, 105, 150)));
        box.setOutlineThickness(isSelected ? 3.0f : (isHovered ? 2.0f : 1.2f));
        window.draw(box);

        // Header thẻ
        sf::RectangleShape header(sf::Vector2f(cw, 34.f));
        header.setPosition(cx, cy);
        sf::Color hCol = themeCol;
        hCol.a = isSelected ? 90 : 40;
        header.setFillColor(hCol);
        window.draw(header);

        sf::Text nameTxt(ConfigManager::toUtf8(name), *gameFont, 13.5f);
        nameTxt.setStyle(sf::Text::Bold);
        nameTxt.setFillColor(isSelected ? sf::Color(255, 245, 160) : themeCol);
        sf::FloatRect nb = nameTxt.getLocalBounds();
        nameTxt.setOrigin(nb.width / 2.f, nb.height / 2.f + 1.f);
        nameTxt.setPosition(cx + cw / 2.f, cy + 17.f);
        window.draw(nameTxt);

        // Pedestal & Sprite
        sf::CircleShape ped(26.f);
        ped.setScale(1.2f, 0.45f);
        ped.setOrigin(26.f, 26.f);
        ped.setPosition(cx + cw / 2.f, cy + 104.f);
        sf::Color pCol = themeCol;
        pCol.a = isSelected ? 120 : 50;
        ped.setFillColor(pCol);
        window.draw(ped);

        try {
            sf::Sprite heroSprite(ResourceManager::getInstance().getTexture(sheet));
            heroSprite.setTextureRect(sf::IntRect(curFrame * 64, 0, 64, 64));
            heroSprite.setOrigin(32.f, 32.f);
            heroSprite.setPosition(cx + cw / 2.f, cy + 82.f);
            heroSprite.setScale(1.25f, 1.25f);
            window.draw(heroSprite);
        } catch (...) {}

        // Trạng thái Đang Chọn / Phím tắt
        if (isSelected) {
            sf::RectangleShape selBadge(sf::Vector2f(136.f, 20.f));
            selBadge.setOrigin(68.f, 10.f);
            selBadge.setPosition(cx + cw / 2.f, cy + 128.f);
            selBadge.setFillColor(sf::Color(30, 140, 70, 220));
            selBadge.setOutlineColor(sf::Color(100, 255, 150));
            selBadge.setOutlineThickness(1.2f);
            window.draw(selBadge);

            sf::Text selTxt(ConfigManager::toUtf8(u8"✓ ĐÃ CHỌN"), *gameFont, 11);
            selTxt.setStyle(sf::Text::Bold);
            selTxt.setFillColor(sf::Color(255, 255, 230));
            sf::FloatRect stb = selTxt.getLocalBounds();
            selTxt.setOrigin(stb.width / 2.f, stb.height / 2.f + 1.f);
            selTxt.setPosition(cx + cw / 2.f, cy + 128.f);
            window.draw(selTxt);
        } else {
            sf::Text keyTxt(ConfigManager::toUtf8(keyHint), *gameFont, 11);
            keyTxt.setFillColor(sf::Color(160, 175, 195));
            sf::FloatRect kb = keyTxt.getLocalBounds();
            keyTxt.setOrigin(kb.width / 2.f, kb.height / 2.f);
            keyTxt.setPosition(cx + cw / 2.f, cy + 128.f);
            window.draw(keyTxt);
        }

        // Hộp thông số Mini (Stats Box)
        sf::RectangleShape stBox(sf::Vector2f(cw - 14.f, 44.f));
        stBox.setPosition(cx + 7.f, cy + 146.f);
        stBox.setFillColor(sf::Color(8, 12, 20, 210));
        stBox.setOutlineColor(sf::Color(65, 80, 105, 130));
        stBox.setOutlineThickness(1.f);
        window.draw(stBox);

        sf::Text spdTxt(ConfigManager::toUtf8(speedInfo), *gameFont, 10.f);
        spdTxt.setFillColor(sf::Color(220, 230, 245));
        spdTxt.setPosition(cx + 14.f, cy + 150.f);
        window.draw(spdTxt);

        sf::Text cdTxt(ConfigManager::toUtf8(cdInfo), *gameFont, 10.f);
        cdTxt.setFillColor(sf::Color(220, 230, 245));
        cdTxt.setPosition(cx + 14.f, cy + 168.f);
        window.draw(cdTxt);

        // Hộp kỹ năng Tuyệt Kỹ (Skill Box)
        sf::RectangleShape skBox(sf::Vector2f(cw - 14.f, 252.f));
        skBox.setPosition(cx + 7.f, cy + 196.f);
        skBox.setFillColor(sf::Color(8, 12, 20, 235));
        sf::Color skBorder = themeCol;
        skBorder.a = isSelected ? 190 : 80;
        skBox.setOutlineColor(skBorder);
        skBox.setOutlineThickness(1.2f);
        window.draw(skBox);

        // Header nhỏ trong skill box
        sf::RectangleShape skHeader(sf::Vector2f(cw - 14.f, 24.f));
        skHeader.setPosition(cx + 7.f, cy + 196.f);
        sf::Color skHCol = themeCol;
        skHCol.a = isSelected ? 55 : 30;
        skHeader.setFillColor(skHCol);
        window.draw(skHeader);

        sf::Text skName(ConfigManager::toUtf8(skillName), *gameFont, 11.f);
        skName.setStyle(sf::Text::Bold);
        skName.setFillColor(themeCol);
        sf::FloatRect snb = skName.getLocalBounds();
        skName.setOrigin(snb.width / 2.f, 0.f);
        skName.setPosition(cx + cw / 2.f, cy + 201.f);
        window.draw(skName);

        sf::Text skKey(ConfigManager::toUtf8(keyBadge), *gameFont, 10.f);
        skKey.setStyle(sf::Text::Italic);
        skKey.setFillColor(sf::Color(255, 220, 110));
        sf::FloatRect skb = skKey.getLocalBounds();
        skKey.setOrigin(skb.width / 2.f, 0.f);
        skKey.setPosition(cx + cw / 2.f, cy + 224.f);
        window.draw(skKey);

        // Vạch phân cách mỏng
        sf::RectangleShape sep(sf::Vector2f(cw - 28.f, 1.f));
        sep.setPosition(cx + 14.f, cy + 242.f);
        sep.setFillColor(sf::Color(100, 120, 150, 80));
        window.draw(sep);

        // 3 Gạch đầu dòng tính năng chiêu - wrapText an toàn tuyệt đối
        float bulletY = cy + 250.f;
        const std::string bullets[3] = { bullet1, bullet2, bullet3 };
        for (int b = 0; b < 3; ++b) {
            std::string wrapped = wrapText(bullets[b], gameFont, 10, 150.f);
            sf::Text bTxt(ConfigManager::toUtf8(wrapped), *gameFont, 10.f);
            bTxt.setLineSpacing(1.15f);
            bTxt.setFillColor(sf::Color(215, 225, 240));
            bTxt.setPosition(cx + 14.f, bulletY);
            window.draw(bTxt);

            bulletY += bTxt.getLocalBounds().height + 10.f;
        }

        // Chân thẻ ánh kim nếu đang chọn
        if (isSelected) {
            sf::RectangleShape bar(sf::Vector2f(cw, 3.5f));
            bar.setPosition(cx, cy + ch - 3.5f);
            bar.setFillColor(themeCol);
            window.draw(bar);
        }
    };

    // 3 Thẻ Phe Xanh (P1)
    float p1XPositions[3] = { 35.f, 227.f, 419.f };
    drawPvPCard(p1XPositions[0], 116.f, 182.f, 458.f,
                HeroClass::VANGUARD, u8"TIÊN PHONG", "player1_sheet.png",
                sf::Color(55, 190, 255), (p1SelectedHero == HeroClass::VANGUARD),
                (heroSelectP1Hover == 0), u8"[Phím 1]",
                u8"ĐOẠT MỆNH TRẢM", u8"[PHÍM SPACE]",
                u8"• Tốc độ: 225 (Nhanh)", u8"• Hồi chiêu: 7.0s",
                u8"• Lướt kiếm thần tốc xuyên hàng ngũ địch.",
                u8"• Cắt đứt đội hình, cướp lính tức thì.",
                u8"• Miễn nhiễm khống chế khi đang lướt kiếm.");

    drawPvPCard(p1XPositions[1], 116.f, 182.f, 458.f,
                HeroClass::TACTICIAN, u8"QUÂN SƯ", "hero_mage_sheet.png",
                sf::Color(215, 120, 255), (p1SelectedHero == HeroClass::TACTICIAN),
                (heroSelectP1Hover == 1), u8"[Phím 2]",
                u8"BÁT QUÁI HẤP TINH", u8"[PHÍM SPACE]",
                u8"• Tốc độ: 210 (Cân bằng)", u8"• Hồi chiêu: 8.5s",
                u8"• Tạo bão xoáy ma trận duy trì trong 3.5s.",
                u8"• Hút lính tự do & đồ xung quanh (280px).",
                u8"• Giảm 60% tốc độ chạy của Tướng địch.");

    drawPvPCard(p1XPositions[2], 116.f, 182.f, 458.f,
                HeroClass::TITAN, u8"HỘ QUỐC", "hero_titan_sheet.png",
                sf::Color(255, 195, 55), (p1SelectedHero == HeroClass::TITAN),
                (heroSelectP1Hover == 2), u8"[Phím 3]",
                u8"KIM CANG HỐNG", u8"[PHÍM SPACE]",
                u8"• Tốc độ: 195 (Chắc chắn)", u8"• Hồi chiêu: 9.5s",
                u8"• Tiếng gầm làm choáng địch 1.5s (260px).",
                u8"• Bật Khiên Kim Cang bất tử trong 4.0 giây.",
                u8"• Bảo vệ toàn diện cho Tướng và toàn quân.");

    // 3 Thẻ Phe Đỏ (P2)
    float p2XPositions[3] = { 679.f, 871.f, 1063.f };
    drawPvPCard(p2XPositions[0], 116.f, 182.f, 458.f,
                HeroClass::VANGUARD, u8"TIÊN PHONG", "player2_sheet.png",
                sf::Color(255, 105, 95), (p2SelectedHero == HeroClass::VANGUARD),
                (heroSelectP2Hover == 0), u8"[Num 1]",
                u8"ĐOẠT MỆNH TRẢM", u8"[R-SHIFT / NUM 0]",
                u8"• Tốc độ: 225 (Nhanh)", u8"• Hồi chiêu: 7.0s",
                u8"• Lướt kiếm thần tốc xuyên hàng ngũ địch.",
                u8"• Cắt đứt đội hình, cướp lính tức thì.",
                u8"• Miễn nhiễm khống chế khi đang lướt kiếm.");

    drawPvPCard(p2XPositions[1], 116.f, 182.f, 458.f,
                HeroClass::TACTICIAN, u8"QUÂN SƯ", "hero_mage_sheet_red.png",
                sf::Color(255, 120, 200), (p2SelectedHero == HeroClass::TACTICIAN),
                (heroSelectP2Hover == 1), u8"[Num 2]",
                u8"BÁT QUÁI HẤP TINH", u8"[R-SHIFT / NUM 0]",
                u8"• Tốc độ: 210 (Cân bằng)", u8"• Hồi chiêu: 8.5s",
                u8"• Tạo bão xoáy ma trận duy trì trong 3.5s.",
                u8"• Hút lính tự do & đồ xung quanh (280px).",
                u8"• Giảm 60% tốc độ chạy của Tướng địch.");

    drawPvPCard(p2XPositions[2], 116.f, 182.f, 458.f,
                HeroClass::TITAN, u8"HỘ QUỐC", "hero_titan_sheet_red.png",
                sf::Color(255, 190, 50), (p2SelectedHero == HeroClass::TITAN),
                (heroSelectP2Hover == 2), u8"[Num 3]",
                u8"KIM CANG HỐNG", u8"[R-SHIFT / NUM 0]",
                u8"• Tốc độ: 195 (Chắc chắn)", u8"• Hồi chiêu: 9.5s",
                u8"• Tiếng gầm làm choáng địch 1.5s (260px).",
                u8"• Bật Khiên Kim Cang bất tử trong 4.0 giây.",
                u8"• Bảo vệ toàn diện cho Tướng và toàn quân.");

    // 4. Nút Hành Động Ở Dưới (Action Bar)
    sf::FloatRect backBtnRect(60.f, 592.f, 160.f, 48.f);
    bool isBackHover = backBtnRect.contains(mousePos);
    sf::RectangleShape backBtn(sf::Vector2f(160.f, 48.f));
    backBtn.setPosition(60.f, 592.f);
    backBtn.setFillColor(isBackHover ? sf::Color(100, 30, 30, 220) : sf::Color(55, 20, 20, 200));
    backBtn.setOutlineColor(isBackHover ? sf::Color(255, 180, 160) : sf::Color(160, 70, 70));
    backBtn.setOutlineThickness(1.8f);
    window.draw(backBtn);

    sf::Text backText(ConfigManager::toUtf8(u8"QUAY LẠI [ESC]"), *gameFont, 14);
    backText.setStyle(sf::Text::Bold);
    backText.setFillColor(isBackHover ? sf::Color::White : sf::Color(230, 210, 210));
    sf::FloatRect bb = backText.getLocalBounds();
    backText.setOrigin(bb.width / 2.f, bb.height / 2.f + 1.f);
    backText.setPosition(140.f, 616.f);
    window.draw(backText);

    // Nút Khai Chiến [ENTER]
    sf::FloatRect startBtnRect(490.f, 588.f, 300.f, 56.f);
    bool isStartHover = startBtnRect.contains(mousePos);

    try {
        sf::Sprite btnSprite(ResourceManager::getInstance().getTexture("ui_button.png"));
        btnSprite.setOrigin(150.f, 30.f);
        btnSprite.setPosition(640.f, 616.f);
        btnSprite.setScale(isStartHover ? 1.05f : 1.0f, isStartHover ? 1.05f : 1.0f);
        if (isStartHover) btnSprite.setColor(sf::Color(255, 255, 220));
        window.draw(btnSprite);
    } catch (...) {
        sf::RectangleShape sBtn(sf::Vector2f(300.f, 56.f));
        sBtn.setOrigin(150.f, 28.f);
        sBtn.setPosition(640.f, 616.f);
        sBtn.setFillColor(isStartHover ? sf::Color(170, 40, 40) : sf::Color(120, 25, 25));
        sBtn.setOutlineColor(sf::Color(255, 215, 0));
        sBtn.setOutlineThickness(2.5f);
        window.draw(sBtn);
    }

    sf::Text startText(ConfigManager::toUtf8(u8"KHAI CHIẾN [ENTER]"), *gameFont, 16);
    startText.setStyle(sf::Text::Bold);
    startText.setFillColor(isStartHover ? sf::Color(255, 250, 130) : sf::Color(255, 230, 160));
    sf::FloatRect sb2 = startText.getLocalBounds();
    startText.setOrigin(sb2.width / 2.f, sb2.height / 2.f + 1.f);
    startText.setPosition(640.f, 615.f);
    window.draw(startText);

    // Gợi ý phía dưới cùng
    sf::Text hintBot(ConfigManager::toUtf8(u8"P1: Phím 1,2,3 hoặc Click | P2: Phím Numpad 1,2,3 hoặc Click | [ENTER / SPACE] Khai Chiến"), *gameFont, 13);
    hintBot.setFillColor(sf::Color(170, 185, 205));
    sf::FloatRect hbb = hintBot.getLocalBounds();
    hintBot.setOrigin(hbb.width / 2.f, hbb.height / 2.f);
    hintBot.setPosition(640.f, 672.f);
    window.draw(hintBot);
}

// ================================================================
//  renderPauseMenu – Menu Tạm Dừng trận đấu (ESC Menu)
//  Cho phép tiếp tục chơi, chỉnh âm thanh/ngôn ngữ, hoặc thoát về menu chính
// ================================================================
void GameEngine::renderPauseMenu() {
    window.setView(window.getDefaultView());

    // 1. Phủ mờ toàn màn hình
    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 195));
    window.draw(dim);

    // 2. Vẽ khung cuộn hoàng gia ui_panel.png
    try {
        sf::Sprite panel(ResourceManager::getInstance().getTexture("ui_panel.png"));
        panel.setPosition(210.f, 75.f);
        window.draw(panel);
    } catch (...) {
        sf::RectangleShape p(sf::Vector2f(860.f, 540.f));
        p.setPosition(210.f, 75.f);
        p.setFillColor(sf::Color(40, 30, 20));
        p.setOutlineColor(sf::Color(255, 215, 0));
        p.setOutlineThickness(3.f);
        window.draw(p);
    }

    if (!gameFont) return;
    auto& cfg = ConfigManager::getInstance();

    // 3. Tiêu đề TẠM DỪNG TRÒ CHƠI
    sf::Text title(cfg.getSfText("pause_title"), *gameFont, 22);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(135, 20, 20));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width / 2.f, tb.height / 2.f);
    title.setPosition(640.f, 175.f);
    window.draw(title);

    // Đường gạch trang trí vàng
    sf::RectangleShape decorLine(sf::Vector2f(340.f, 2.f));
    decorLine.setOrigin(170.f, 1.f);
    decorLine.setPosition(640.f, 193.f);
    decorLine.setFillColor(sf::Color(165, 120, 50));
    window.draw(decorLine);

    // 4. Nút [TIẾP TỤC TRẬN ĐẤU]
    sf::FloatRect resumeRect(495.f, 206.f, 290.f, 48.f);
    bool resumeHover = resumeRect.contains(mousePos);
    try {
        sf::Sprite rBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        rBtn.setOrigin(150.f, 30.f);
        rBtn.setPosition(640.f, 230.f);
        rBtn.setScale(0.95f, 0.78f);
        if (resumeHover) rBtn.setColor(sf::Color(255, 255, 210));
        window.draw(rBtn);
    } catch (...) {
        sf::RectangleShape rb(sf::Vector2f(290.f, 48.f));
        rb.setOrigin(145.f, 24.f);
        rb.setPosition(640.f, 230.f);
        rb.setFillColor(resumeHover ? sf::Color(50, 120, 50) : sf::Color(35, 90, 35));
        rb.setOutlineColor(sf::Color(255, 215, 0));
        rb.setOutlineThickness(2.f);
        window.draw(rb);
    }
    sf::Text resumeText(cfg.getSfText("pause_resume"), *gameFont, 16);
    resumeText.setStyle(sf::Text::Bold);
    resumeText.setFillColor(resumeHover ? sf::Color(255, 250, 140) : sf::Color(250, 240, 220));
    sf::FloatRect rtb = resumeText.getLocalBounds();
    resumeText.setOrigin(rtb.width / 2.f, rtb.height / 2.f + 2.f);
    resumeText.setPosition(640.f, 227.f);
    window.draw(resumeText);

    // ================================================================
    //  HÀNG 1: ÂM NHẠC (MUSIC / BGM) - y = 280
    // ================================================================
    float row1Y = 280.f;
    sf::Text musicLbl(cfg.getSfText("settings_music"), *gameFont, 16);
    musicLbl.setStyle(sf::Text::Bold);
    musicLbl.setFillColor(sf::Color(40, 25, 10));
    musicLbl.setPosition(320.f, row1Y + 7.f);
    window.draw(musicLbl);

    // Nút Bật/Tắt Music
    sf::FloatRect mToggleRect(540.f, row1Y, 90.f, 36.f);
    bool mHover = mToggleRect.contains(mousePos);
    sf::RectangleShape mToggleBtn(sf::Vector2f(90.f, 36.f));
    mToggleBtn.setPosition(540.f, row1Y);
    mToggleBtn.setFillColor(cfg.isMusicEnabled() ? sf::Color(40, 130, 50) : sf::Color(140, 40, 40));
    mToggleBtn.setOutlineColor(mHover ? sf::Color(255, 230, 100) : sf::Color(180, 150, 90));
    mToggleBtn.setOutlineThickness(2.f);
    window.draw(mToggleBtn);

    sf::String mStatus = cfg.isMusicEnabled() ? cfg.getSfText("settings_on") : cfg.getSfText("settings_off");
    sf::Text mStatusText(mStatus, *gameFont, 15);
    mStatusText.setStyle(sf::Text::Bold);
    mStatusText.setFillColor(sf::Color::White);
    sf::FloatRect mstb = mStatusText.getLocalBounds();
    mStatusText.setOrigin(mstb.width / 2.f, mstb.height / 2.f + 2.f);
    mStatusText.setPosition(585.f, row1Y + 16.f);
    window.draw(mStatusText);

    // Nút [-] Volume Music
    sf::FloatRect mMinusRect(660.f, row1Y, 36.f, 36.f);
    bool mMinusHover = mMinusRect.contains(mousePos);
    sf::RectangleShape mMinusBtn(sf::Vector2f(36.f, 36.f));
    mMinusBtn.setPosition(660.f, row1Y);
    mMinusBtn.setFillColor(mMinusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    mMinusBtn.setOutlineColor(sf::Color(200, 160, 90));
    mMinusBtn.setOutlineThickness(1.5f);
    window.draw(mMinusBtn);

    sf::Text mMinusText("-", *gameFont, 22);
    mMinusText.setStyle(sf::Text::Bold);
    mMinusText.setFillColor(sf::Color::White);
    sf::FloatRect mmtb = mMinusText.getLocalBounds();
    mMinusText.setOrigin(mmtb.width / 2.f, mmtb.height / 2.f + 3.f);
    mMinusText.setPosition(678.f, row1Y + 15.f);
    window.draw(mMinusText);

    // Hiển thị % Music
    std::string mVolStr = std::to_string(cfg.getMusicVolume()) + "%";
    sf::Text mVolText(mVolStr, *gameFont, 16);
    mVolText.setStyle(sf::Text::Bold);
    mVolText.setFillColor(sf::Color(45, 25, 10));
    sf::FloatRect mvb = mVolText.getLocalBounds();
    mVolText.setOrigin(mvb.width / 2.f, mvb.height / 2.f + 2.f);
    mVolText.setPosition(745.f, row1Y + 16.f);
    window.draw(mVolText);

    // Nút [+] Volume Music
    sf::FloatRect mPlusRect(790.f, row1Y, 36.f, 36.f);
    bool mPlusHover = mPlusRect.contains(mousePos);
    sf::RectangleShape mPlusBtn(sf::Vector2f(36.f, 36.f));
    mPlusBtn.setPosition(790.f, row1Y);
    mPlusBtn.setFillColor(mPlusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    mPlusBtn.setOutlineColor(sf::Color(200, 160, 90));
    mPlusBtn.setOutlineThickness(1.5f);
    window.draw(mPlusBtn);

    sf::Text mPlusText("+", *gameFont, 20);
    mPlusText.setStyle(sf::Text::Bold);
    mPlusText.setFillColor(sf::Color::White);
    sf::FloatRect mptb = mPlusText.getLocalBounds();
    mPlusText.setOrigin(mptb.width / 2.f, mptb.height / 2.f + 3.f);
    mPlusText.setPosition(808.f, row1Y + 16.f);
    window.draw(mPlusText);

    // ================================================================
    //  HÀNG 2: HIỆU ỨNG (SFX) - y = 332
    // ================================================================
    float row2Y = 332.f;
    sf::Text sfxLbl(cfg.getSfText("settings_sfx"), *gameFont, 16);
    sfxLbl.setStyle(sf::Text::Bold);
    sfxLbl.setFillColor(sf::Color(40, 25, 10));
    sfxLbl.setPosition(320.f, row2Y + 7.f);
    window.draw(sfxLbl);

    // Nút Bật/Tắt SFX
    sf::FloatRect sToggleRect(540.f, row2Y, 90.f, 36.f);
    bool sHover = sToggleRect.contains(mousePos);
    sf::RectangleShape sToggleBtn(sf::Vector2f(90.f, 36.f));
    sToggleBtn.setPosition(540.f, row2Y);
    sToggleBtn.setFillColor(cfg.isSfxEnabled() ? sf::Color(40, 130, 50) : sf::Color(140, 40, 40));
    sToggleBtn.setOutlineColor(sHover ? sf::Color(255, 230, 100) : sf::Color(180, 150, 90));
    sToggleBtn.setOutlineThickness(2.f);
    window.draw(sToggleBtn);

    sf::String sStatus = cfg.isSfxEnabled() ? cfg.getSfText("settings_on") : cfg.getSfText("settings_off");
    sf::Text sStatusText(sStatus, *gameFont, 15);
    sStatusText.setStyle(sf::Text::Bold);
    sStatusText.setFillColor(sf::Color::White);
    sf::FloatRect sstb = sStatusText.getLocalBounds();
    sStatusText.setOrigin(sstb.width / 2.f, sstb.height / 2.f + 2.f);
    sStatusText.setPosition(585.f, row2Y + 16.f);
    window.draw(sStatusText);

    // Nút [-] Volume SFX
    sf::FloatRect sMinusRect(660.f, row2Y, 36.f, 36.f);
    bool sMinusHover = sMinusRect.contains(mousePos);
    sf::RectangleShape sMinusBtn(sf::Vector2f(36.f, 36.f));
    sMinusBtn.setPosition(660.f, row2Y);
    sMinusBtn.setFillColor(sMinusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    sMinusBtn.setOutlineColor(sf::Color(200, 160, 90));
    sMinusBtn.setOutlineThickness(1.5f);
    window.draw(sMinusBtn);

    sf::Text sMinusText("-", *gameFont, 22);
    sMinusText.setStyle(sf::Text::Bold);
    sMinusText.setFillColor(sf::Color::White);
    sf::FloatRect smtb = sMinusText.getLocalBounds();
    sMinusText.setOrigin(smtb.width / 2.f, smtb.height / 2.f + 3.f);
    sMinusText.setPosition(678.f, row2Y + 15.f);
    window.draw(sMinusText);

    // Hiển thị % SFX
    std::string sVolStr = std::to_string(cfg.getSfxVolume()) + "%";
    sf::Text sVolText(sVolStr, *gameFont, 16);
    sVolText.setStyle(sf::Text::Bold);
    sVolText.setFillColor(sf::Color(45, 25, 10));
    sf::FloatRect svb = sVolText.getLocalBounds();
    sVolText.setOrigin(svb.width / 2.f, svb.height / 2.f + 2.f);
    sVolText.setPosition(745.f, row2Y + 16.f);
    window.draw(sVolText);

    // Nút [+] Volume SFX
    sf::FloatRect sPlusRect(790.f, row2Y, 36.f, 36.f);
    bool sPlusHover = sPlusRect.contains(mousePos);
    sf::RectangleShape sPlusBtn(sf::Vector2f(36.f, 36.f));
    sPlusBtn.setPosition(790.f, row2Y);
    sPlusBtn.setFillColor(sPlusHover ? sf::Color(120, 60, 30) : sf::Color(80, 40, 20));
    sPlusBtn.setOutlineColor(sf::Color(200, 160, 90));
    sPlusBtn.setOutlineThickness(1.5f);
    window.draw(sPlusBtn);

    sf::Text sPlusText("+", *gameFont, 20);
    sPlusText.setStyle(sf::Text::Bold);
    sPlusText.setFillColor(sf::Color::White);
    sf::FloatRect sptb = sPlusText.getLocalBounds();
    sPlusText.setOrigin(sptb.width / 2.f, sptb.height / 2.f + 3.f);
    sPlusText.setPosition(808.f, row2Y + 16.f);
    window.draw(sPlusText);

    // ================================================================
    //  HÀNG 3: NGÔN NGỮ (LANGUAGE) - y = 384
    // ================================================================
    float row3Y = 384.f;
    sf::Text langLbl(cfg.getSfText("settings_lang"), *gameFont, 16);
    langLbl.setStyle(sf::Text::Bold);
    langLbl.setFillColor(sf::Color(40, 25, 10));
    langLbl.setPosition(320.f, row3Y + 7.f);
    window.draw(langLbl);

    sf::FloatRect langToggleRect(540.f, row3Y, 286.f, 36.f);
    bool langHover = langToggleRect.contains(mousePos);
    sf::RectangleShape langBtn(sf::Vector2f(286.f, 36.f));
    langBtn.setPosition(540.f, row3Y);
    langBtn.setFillColor(langHover ? sf::Color(140, 40, 40) : sf::Color(110, 25, 25));
    langBtn.setOutlineColor(langHover ? sf::Color(255, 230, 100) : sf::Color(200, 160, 80));
    langBtn.setOutlineThickness(2.f);
    window.draw(langBtn);

    sf::String currentLangStr = (cfg.getLanguage() == Language::VI) ? ConfigManager::toUtf8(u8"Tiếng Việt [Click đổi]") : ConfigManager::toUtf8("English [Click change]");
    sf::Text langText(currentLangStr, *gameFont, 15);
    langText.setStyle(sf::Text::Bold);
    langText.setFillColor(sf::Color(255, 235, 140));
    sf::FloatRect ltb = langText.getLocalBounds();
    langText.setOrigin(ltb.width / 2.f, ltb.height / 2.f + 2.f);
    langText.setPosition(683.f, row3Y + 16.f);
    window.draw(langText);

    // 5. Nút [THOÁT RA MENU CHÍNH] - y = 448
    sf::FloatRect quitRect(495.f, 424.f, 290.f, 48.f);
    bool quitHover = quitRect.contains(mousePos);
    try {
        sf::Sprite qBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        qBtn.setOrigin(150.f, 30.f);
        qBtn.setPosition(640.f, 448.f);
        qBtn.setScale(0.95f, 0.78f);
        if (quitHover) qBtn.setColor(sf::Color(255, 220, 220));
        window.draw(qBtn);
    } catch (...) {
        sf::RectangleShape qb(sf::Vector2f(290.f, 48.f));
        qb.setOrigin(145.f, 24.f);
        qb.setPosition(640.f, 448.f);
        qb.setFillColor(quitHover ? sf::Color(160, 40, 40) : sf::Color(120, 25, 25));
        qb.setOutlineColor(sf::Color(255, 215, 0));
        qb.setOutlineThickness(2.f);
        window.draw(qb);
    }
    sf::Text quitText(cfg.getSfText("pause_main_menu"), *gameFont, 16);
    quitText.setStyle(sf::Text::Bold);
    quitText.setFillColor(quitHover ? sf::Color(255, 230, 140) : sf::Color(250, 230, 210));
    sf::FloatRect qtb = quitText.getLocalBounds();
    quitText.setOrigin(qtb.width / 2.f, qtb.height / 2.f + 2.f);
    quitText.setPosition(640.f, 445.f);
    window.draw(quitText);

    // 6. Gợi ý phím tắt phía dưới - y = 502
    sf::Text hintText(cfg.getSfText("pause_hint"), *gameFont, 13);
    hintText.setStyle(sf::Text::Italic);
    hintText.setFillColor(sf::Color(90, 60, 35));
    sf::FloatRect htb = hintText.getLocalBounds();
    hintText.setOrigin(htb.width / 2.f, htb.height / 2.f);
    hintText.setPosition(640.f, 502.f);
    window.draw(hintText);
}

// ================================================================
//  renderWorld – Vẽ toàn bộ thế giới trò chơi (dùng chung cho cả màn hình đơn và chia đôi)
// ================================================================
void GameEngine::renderWorld() {
    // 1. Lớp mặt đất tĩnh (Ground Layer)
    sf::RenderStates bgState;
    bgState.texture = &ResourceManager::getInstance().getTexture("background_tile.png");
    window.draw(bgVertices, bgState);

    // Viền bản đồ
    sf::RectangleShape border(sf::Vector2f(WORLD_W, WORLD_H));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(200, 50, 50, 180));
    border.setOutlineThickness(4.f);
    window.draw(border);

    // Vùng căn cứ Base
    if (baseBlue) baseBlue->draw(window);
    if (baseRed) baseRed->draw(window);

    // Vùng cảnh báo bom & sự kiện VIP (nằm sát mặt đất)
    for (auto b : bombs) b->draw(window);
    if (vipDrop) vipDrop->draw(window);

    // Lớp mặt đất: Vùng biển lửa rồng & Bóng rồng 2.5D
    for (auto de : dragonEvents) de->drawGround(window);

    // 2. Lớp thực thể động có chiều sâu (Y-Sorting Depth Layer)
    // Thực thể nào ở phía trên (Y nhỏ) vẽ trước, thực thể ở phía dưới (Y lớn) vẽ sau.
    // Tán cây sẽ CHE LẤP Tướng và Lính khi họ đi vào phía sau thân cây!
    struct RenderItem {
        float depthY;
        std::function<void()> drawFunc;
    };

    sf::RenderWindow& win = window;

    std::vector<RenderItem> renderList;
    renderList.reserve(160);

    // Vật cản (Đá, Cây, Bụi rậm, Tàn tích La Mã)
    for (auto o : obstacles) {
        // Điểm chân gốc cây/đá chạm đất (85% chiều cao)
        float depth = o->getY() + o->getHeight() * 0.85f;
        renderList.push_back({ depth, [o, &win]() { o->draw(win); } });
    }

    // Lính tự do trên bản đồ
    for (auto t : freeTroops) {
        renderList.push_back({ t->y + 20.f, [t, &win]() { win.draw(t->sprite); } });
    }

    // Items
    for (auto i : items) {
        renderList.push_back({ i->getY() + 16.f, [i, &win]() { i->draw(win); } });
    }

    // Tướng & Lính Player 1 (ẩn nấp trong bụi rậm -> mờ 55%)
    if (player1) {
        Troop* curr = player1->getArmy().getHead();
        while (curr) {
            Troop* t = curr;
            renderList.push_back({ t->y + 20.f, [t, &win, this]() {
                sf::Color c = t->getTeamColor();
                c.a = player1->isInBush() ? 115 : 255;
                t->sprite.setColor(c);
                win.draw(t->sprite);
            } });
            curr = curr->next;
        }
        renderList.push_back({ player1->getY() + 24.f, [this, &win]() {
            player1->drawSkillEffects(win);
            player1->drawHeadAndAura(win);
        }});
    }

    // Tướng & Lính Player 2 (PvP)
    if (player2 && gameMode == GameMode::PVP) {
        Troop* curr = player2->getArmy().getHead();
        while (curr) {
            Troop* t = curr;
            renderList.push_back({ t->y + 20.f, [t, &win, this]() {
                sf::Color c = t->getTeamColor();
                c.a = player2->isInBush() ? 115 : 255;
                t->sprite.setColor(c);
                win.draw(t->sprite);
            } });
            curr = curr->next;
        }
        renderList.push_back({ player2->getY() + 24.f, [this, &win]() {
            player2->drawSkillEffects(win);
            player2->drawHeadAndAura(win);
        }});
    }

    // Tướng & Lính Bot (PvE)
    if (bot && gameMode == GameMode::PVE) {
        Troop* curr = bot->getArmy().getHead();
        while (curr) {
            Troop* t = curr;
            renderList.push_back({ t->y + 20.f, [t, &win, this]() {
                sf::Color c = t->getTeamColor();
                c.a = bot->isInBush() ? 115 : 255;
                t->sprite.setColor(c);
                win.draw(t->sprite);
            } });
            curr = curr->next;
        }
        renderList.push_back({ bot->getY() + 24.f, [this, &win]() {
            bot->drawSkillEffects(win);
            bot->drawHeadAndAura(win);
        }});
    }

    // Sắp xếp theo chiều sâu Y tăng dần (phía trên vẽ trước, phía dưới vẽ sau)
    std::sort(renderList.begin(), renderList.end(), [](const RenderItem& a, const RenderItem& b) {
        return a.depthY < b.depthY;
    });

    // Vẽ lần lượt toàn bộ thực thể theo đúng thứ tự chiều sâu
    for (const auto& item : renderList) {
        item.drawFunc();
    }

    // 2.5. Vẽ hiệu ứng chém, tia lửa và chữ bay trong thế giới (World VFX)
    for (const auto& slash : clashSlashes) {
        float progress = 1.f - (slash.lifetime / slash.maxLifetime);
        float alpha = (1.f - progress) * 255.f;
        sf::Color col = slash.color;
        col.a = (sf::Uint8)alpha;

        sf::RectangleShape slashBar(sf::Vector2f(70.f * (1.f + progress * 0.4f), 6.f));
        slashBar.setOrigin(slashBar.getSize().x / 2.f, 3.f);
        slashBar.setPosition(slash.x, slash.y);
        slashBar.setRotation(slash.angle);
        slashBar.setFillColor(col);
        window.draw(slashBar);

        sf::RectangleShape slashCore(sf::Vector2f(55.f, 3.f));
        slashCore.setOrigin(slashCore.getSize().x / 2.f, 1.5f);
        slashCore.setPosition(slash.x, slash.y);
        slashCore.setRotation(slash.angle);
        slashCore.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
        window.draw(slashCore);
    }
    for (const auto& spark : clashSparks) {
        float alpha = (spark.lifetime / spark.maxLifetime) * 255.f;
        sf::Color col = spark.color;
        col.a = (sf::Uint8)alpha;
        sf::CircleShape dot(3.f);
        dot.setPosition(spark.x, spark.y);
        dot.setFillColor(col);
        window.draw(dot);
    }
    if (gameFont) {
        for (const auto& ft : clashFloatTexts) {
            float alpha = (ft.lifetime / ft.maxLifetime) * 255.f;
            sf::Text txt(ConfigManager::toUtf8(ft.text), *gameFont, 18);
            txt.setStyle(sf::Text::Bold);
            txt.setFillColor(sf::Color(ft.color.r, ft.color.g, ft.color.b, (sf::Uint8)alpha));
            txt.setOutlineColor(sf::Color(0, 0, 0, (sf::Uint8)alpha));
            txt.setOutlineThickness(2.f);
            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            txt.setPosition(ft.x, ft.y);
            window.draw(txt);
        }
    }

    // 3. Lớp bầu trời (Air Layer): Rồng bay xé gió trên cao, che lấp cây cối và người chơi
    for (auto de : dragonEvents) {
        de->drawAir(window);
    }
}

// ================================================================
//  renderGathering – Quản lý góc nhìn camera (Split-Screen hoặc Gộp chung)
// ================================================================
void GameEngine::renderGathering() {
    sf::View viewP1 = cameraP1;
    sf::View viewP2 = cameraP2;
    sf::View viewMerged = camera;

    if (screenShakeTimer > 0.f) {
        float offX = ((std::rand() % 200) / 100.f - 1.f) * screenShakeIntensity;
        float offY = ((std::rand() % 200) / 100.f - 1.f) * screenShakeIntensity;
        viewP1.move(offX, offY);
        viewP2.move(offX, offY);
        viewMerged.move(offX, offY);
    }

    if (gameMode == GameMode::PVP && isSplitScreen) {
        // === CHIA ĐÔI MÀN HÌNH (SPLIT-SCREEN) ===
        // Nửa trái: Góc nhìn Player 1
        window.setView(viewP1);
        renderWorld();

        // Nửa phải: Góc nhìn Player 2
        window.setView(viewP2);
        renderWorld();

        // Vẽ đường kẻ chia màn hình dọc ở giữa (Vẽ trên default view)
        window.setView(window.getDefaultView());

        // Đổ bóng viền tối để đường kẻ nổi bật trên mọi địa hình
        sf::RectangleShape dividerShadow(sf::Vector2f(6.f, 720.f));
        dividerShadow.setOrigin(3.f, 0.f);
        dividerShadow.setPosition(640.f, 0.f);
        dividerShadow.setFillColor(sf::Color(10, 10, 20, 220));
        window.draw(dividerShadow);

        // Đường kẻ vàng hoàng kim sang trọng
        sf::RectangleShape divider(sf::Vector2f(2.f, 720.f));
        divider.setOrigin(1.f, 0.f);
        divider.setPosition(640.f, 0.f);
        divider.setFillColor(sf::Color(255, 215, 0, 240));
        window.draw(divider);

        // Tag nhận diện người chơi ở góc dưới mỗi nửa màn hình
        if (gameFont) {
            sf::Text p1Tag("<< P1 (XANH)", *gameFont, 14);
            p1Tag.setFillColor(sf::Color(100, 170, 255, 180));
            p1Tag.setPosition(20.f, 720.f - 30.f);
            window.draw(p1Tag);

            sf::Text p2Tag("P2 (DO) >>", *gameFont, 14);
            p2Tag.setFillColor(sf::Color(255, 120, 100, 180));
            sf::FloatRect p2tb = p2Tag.getLocalBounds();
            p2Tag.setPosition(1260.f - p2tb.width, 720.f - 30.f);
            window.draw(p2Tag);
        }
    } else {
        // === MÀN HÌNH GỘP CHUNG (MERGED SCREEN) ===
        // PvE hoặc khi 2 tướng đi lại gần nhau nhìn thấy nhau:
        // Màn hình gộp liền mạch, KHÔNG CÓ đường kẻ chia đôi!
        window.setView(viewMerged);
        renderWorld();
    }

    // 3. HUD (vẽ ở default view)
    renderHUD();
}

void GameEngine::renderClash() {
    // 1. Áp dụng rung chấn màn hình
    sf::View clashView = window.getDefaultView();
    if (screenShakeTimer > 0.f) {
        float offX = ((std::rand() % 200) / 100.f - 1.f) * screenShakeIntensity;
        float offY = ((std::rand() % 200) / 100.f - 1.f) * screenShakeIntensity;
        clashView.move(offX, offY);
    }
    window.setView(clashView);

    // 2. Nền đấu trường sa trường từ file clash_arena_bg.png
    sf::Texture& arenaTex = ResourceManager::getInstance().getTexture("clash_arena_bg.png");
    sf::Sprite arenaSprite(arenaTex);
    if (arenaTex.getSize().x > 0 && arenaTex.getSize().y > 0) {
        arenaSprite.setScale(1280.f / arenaTex.getSize().x, 720.f / arenaTex.getSize().y);
    }
    window.draw(arenaSprite);

    // Vạch chiến tuyến động (Dynamic Frontline Marker)
    sf::RectangleShape frontlineMarker(sf::Vector2f(3.f, 360.f));
    frontlineMarker.setPosition(clashFrontlineX, 290.f);
    frontlineMarker.setFillColor(sf::Color(255, 80, 80, 140));
    window.draw(frontlineMarker);

    // 2. Vẽ lính đã chết đang bị văng ngửa ra sau (Knockback fading)
    for (auto& dead : clashDeadTroops) {
        sf::Color c = dead.sprite.getColor();
        c.a = (sf::Uint8)dead.alpha;
        dead.sprite.setColor(c);
        dead.sprite.setPosition(dead.x, dead.y);
        window.draw(dead.sprite);
    }

    // 3. Sắp xếp và vẽ toàn bộ lính 2 bên theo trục Y để có chiều sâu
    struct ClashDrawItem {
        float y;
        std::function<void()> draw;
    };
    std::vector<ClashDrawItem> drawList;
    drawList.reserve(100);

    // Lính Xanh
    for (auto t : blueClashArmy) {
        drawList.push_back({ t->y, [t, this]() {
            t->sprite.setColor(sf::Color(100, 160, 255));
            window.draw(t->sprite);
        }});
    }

    // Lính Đỏ
    for (auto t : redClashArmy) {
        drawList.push_back({ t->y, [t, this]() {
            t->sprite.setColor(sf::Color(255, 120, 100));
            window.draw(t->sprite);
        }});
    }

    // Leader Xanh (P1)
    if (player1) {
        drawList.push_back({ player1->getY() + 2.f, [this]() {
            player1->drawHeadAndAura(window);
        }});
    }

    // Leader Đỏ (P2 hoặc Bot)
    if (gameMode == GameMode::PVP && player2) {
        drawList.push_back({ player2->getY() + 2.f, [this]() {
            player2->drawHeadAndAura(window);
        }});
    } else if (bot) {
        drawList.push_back({ bot->getY() + 2.f, [this]() {
            bot->drawHeadAndAura(window);
        }});
    }

    std::sort(drawList.begin(), drawList.end(), [](const ClashDrawItem& a, const ClashDrawItem& b) {
        return a.y < b.y;
    });

    for (const auto& item : drawList) {
        item.draw();
    }

    // 4. Vẽ hiệu ứng chém (Slash Arcs)
    for (const auto& slash : clashSlashes) {
        float progress = 1.f - (slash.lifetime / slash.maxLifetime);
        float alpha = (1.f - progress) * 255.f;
        sf::Color col = slash.color;
        col.a = (sf::Uint8)alpha;

        sf::RectangleShape slashBar(sf::Vector2f(65.f * (1.f + progress * 0.4f), 5.f));
        slashBar.setOrigin(slashBar.getSize().x / 2.f, 2.5f);
        slashBar.setPosition(slash.x, slash.y);
        slashBar.setRotation(slash.angle);
        slashBar.setFillColor(col);
        window.draw(slashBar);

        // Lõi sáng trắng ở tâm
        sf::RectangleShape slashCore(sf::Vector2f(50.f, 2.5f));
        slashCore.setOrigin(slashCore.getSize().x / 2.f, 1.25f);
        slashCore.setPosition(slash.x, slash.y);
        slashCore.setRotation(slash.angle);
        slashCore.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
        window.draw(slashCore);
    }

    // 5. Vẽ tia lửa va chạm (Sparks)
    for (const auto& spark : clashSparks) {
        float alpha = (spark.lifetime / spark.maxLifetime) * 255.f;
        sf::Color col = spark.color;
        col.a = (sf::Uint8)alpha;
        sf::CircleShape dot(2.5f);
        dot.setPosition(spark.x, spark.y);
        dot.setFillColor(col);
        window.draw(dot);
    }

    // 6. Vẽ chữ bay (Floating Combat Text)
    if (gameFont) {
        for (const auto& ft : clashFloatTexts) {
            float alpha = (ft.lifetime / ft.maxLifetime) * 255.f;
            sf::Text txt(ConfigManager::toUtf8(ft.text), *gameFont, 16);
            txt.setStyle(sf::Text::Bold);
            txt.setFillColor(sf::Color(ft.color.r, ft.color.g, ft.color.b, (sf::Uint8)alpha));
            txt.setOutlineColor(sf::Color(0, 0, 0, (sf::Uint8)alpha));
            txt.setOutlineThickness(1.5f);
            sf::FloatRect bounds = txt.getLocalBounds();
            txt.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            txt.setPosition(ft.x, ft.y);
            window.draw(txt);
        }
    }

    // ================================================================
    //  7. HUD Đại Chiến (Vẽ trên default view cố định)
    // ================================================================
    window.setView(window.getDefaultView());

    // Thanh Tug-of-War
    float barWidth = 840.f;
    float barHeight = 36.f;
    float barX = (1280.f - barWidth) / 2.f;
    float barY = 60.f;

    // Nền thanh
    sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
    barBg.setPosition(barX, barY);
    barBg.setFillColor(sf::Color(25, 25, 30));
    barBg.setOutlineColor(sf::Color(255, 215, 0, 180));
    barBg.setOutlineThickness(2.f);
    window.draw(barBg);

    // Phần Xanh
    sf::RectangleShape blueBar(sf::Vector2f(barWidth * tugOfWarRatio, barHeight));
    blueBar.setPosition(barX, barY);
    blueBar.setFillColor(sf::Color(80, 160, 255));
    window.draw(blueBar);

    // Phần Đỏ
    sf::RectangleShape redBar(sf::Vector2f(barWidth * (1.f - tugOfWarRatio), barHeight));
    redBar.setPosition(barX + barWidth * tugOfWarRatio, barY);
    redBar.setFillColor(sf::Color(255, 100, 90));
    window.draw(redBar);

    // Cặp kiếm đọ sức tóe lửa tại chiến tuyến động
    float markerX = barX + barWidth * tugOfWarRatio;
    try {
        sf::Sprite swords(ResourceManager::getInstance().getTexture("icon_clash_swords.png"));
        swords.setOrigin(32.f, 32.f);
        swords.setPosition(markerX, barY + barHeight / 2.f);
        float pulse = 0.85f + 0.12f * std::sin(clashFightTimer * 16.f);
        swords.setScale(pulse, pulse);
        window.draw(swords);
    } catch (...) {
        sf::CircleShape marker(10.f, 3);
        marker.setOrigin(10.f, 10.f);
        marker.setPosition(markerX, barY + barHeight + 12.f);
        marker.setRotation(180.f);
        marker.setFillColor(sf::Color(255, 215, 0));
        window.draw(marker);
    }

    // Hiển thị quân số thời gian thực dưới thanh kéo co
    if (gameFont) {
        std::string blueArmyCountStr = u8"PHE XANH: " + std::to_string(blueClashArmy.size()) + u8" CHIẾN BINH";
        sf::Text blueTxt(ConfigManager::toUtf8(blueArmyCountStr), *gameFont, 14);
        blueTxt.setStyle(sf::Text::Bold);
        blueTxt.setFillColor(sf::Color(130, 200, 255));
        blueTxt.setPosition(barX + 10.f, barY + barHeight + 8.f);
        window.draw(blueTxt);

        std::string redArmyCountStr = u8"PHE ĐỎ: " + std::to_string(redClashArmy.size()) + u8" CHIẾN BINH";
        sf::Text redTxt(ConfigManager::toUtf8(redArmyCountStr), *gameFont, 14);
        redTxt.setStyle(sf::Text::Bold);
        redTxt.setFillColor(sf::Color(255, 130, 120));
        sf::FloatRect rtb = redTxt.getLocalBounds();
        redTxt.setPosition(barX + barWidth - rtb.width - 10.f, barY + barHeight + 8.f);
        window.draw(redTxt);
    }

    if (gameFont) {
        // Dải lụa hoàng gia rồng cuộn (banner_ribbon.png) làm khung trang trí tiêu đề Clash
        try {
            sf::Sprite clashRibbon(ResourceManager::getInstance().getTexture("banner_ribbon.png"));
            clashRibbon.setOrigin(300.f, 45.f);
            clashRibbon.setPosition(640.f, 28.f);
            clashRibbon.setScale(0.92f, 0.65f);
            window.draw(clashRibbon);
        } catch (...) {}

        // Tiêu đề cốt truyện nằm lồng vào dải lụa hoàng gia
        std::string titleStr = (gameMode == GameMode::PVE) ?
            u8"TỔNG LỰC PHẢN CÔNG - TIÊU DIỆT BẠO CHÚA!" :
            u8"ĐẠI HỘI ĐỌ SỨC - TRANH ĐOẠT VƯƠNG QUYỀN!";

        // Bóng đổ tiêu đề
        sf::Text cShadow(ConfigManager::toUtf8(titleStr), *gameFont, 18);
        cShadow.setStyle(sf::Text::Bold);
        cShadow.setFillColor(sf::Color(0, 0, 0, 210));
        sf::FloatRect csb = cShadow.getLocalBounds();
        cShadow.setOrigin(csb.width / 2.f, csb.height / 2.f);
        cShadow.setPosition(641.5f, 28.5f);
        window.draw(cShadow);

        // Chữ chính
        sf::Text clashTitle(ConfigManager::toUtf8(titleStr), *gameFont, 18);
        clashTitle.setStyle(sf::Text::Bold);
        clashTitle.setFillColor(sf::Color(255, 240, 160));
        sf::FloatRect ctb = clashTitle.getLocalBounds();
        clashTitle.setOrigin(ctb.width / 2.f, ctb.height / 2.f);
        clashTitle.setPosition(640.f, 27.f);
        window.draw(clashTitle);

        // Dòng phụ đề trạng thái (chỉ hiện khi Dàn trận, Xung phong hoặc Kết thúc)
        std::string subStr = "";
        sf::Color subColor = sf::Color::White;

        if (clashMarchTimer > 0.f) {
            subStr = u8"HAI BÊN DÀN TRẬN NGHÊNH CHIẾN...";
            subColor = sf::Color(255, 215, 0);
        } else if (clashChargeTimer > 0.f) {
            subStr = u8">> TOÀN QUÂN XUNG PHONG! <<";
            subColor = sf::Color(255, 120, 50);
        } else if (clashIsEnding) {
            subStr = u8"QUÉT SẠCH QUÂN ĐỊCH! THỪA THẮNG XÔNG LÊN!";
            subColor = sf::Color(100, 255, 120);
        }

        // Chỉ vẽ phụ đề khi có sự kiện đặc biệt, xóa hẳn khung trắng lực chiến thừa thãi lúc đánh nhau
        if (!subStr.empty()) {
            sf::Text subText(ConfigManager::toUtf8(subStr), *gameFont, 15);
            subText.setStyle(sf::Text::Bold);
            subText.setFillColor(subColor);
            sf::FloatRect stb = subText.getLocalBounds();

            sf::RectangleShape subBadge(sf::Vector2f(stb.width + 36.f, 28.f));
            subBadge.setOrigin((stb.width + 36.f) / 2.f, 14.f);
            subBadge.setPosition(640.f, 126.f);
            subBadge.setFillColor(sf::Color(15, 18, 26, 215));
            subBadge.setOutlineColor(sf::Color(subColor.r, subColor.g, subColor.b, 170));
            subBadge.setOutlineThickness(1.5f);
            window.draw(subBadge);

            subText.setOrigin(stb.width / 2.f, stb.height / 2.f + 2.f);
            subText.setPosition(640.f, 124.f);
            window.draw(subText);
        }
    }
}

void GameEngine::renderResult() {
    window.setView(window.getDefaultView());

    // 1. Phủ tối màn hình (Dim Backdrop)
    sf::RectangleShape dim(sf::Vector2f(1280.f, 720.f));
    dim.setFillColor(sf::Color(10, 12, 18, 235));
    window.draw(dim);

    // Vầng hào quang tỏa sáng theo phe thắng (Ambient Glow)
    sf::CircleShape glow(220.f);
    glow.setOrigin(220.f, 220.f);
    glow.setPosition(640.f, 150.f);
    if (matchWinner == 1) {
        glow.setFillColor(sf::Color(40, 140, 255, 45));
    } else if (matchWinner == 2) {
        glow.setFillColor(sf::Color(255, 60, 40, 45));
    } else {
        glow.setFillColor(sf::Color(255, 200, 50, 40));
    }
    window.draw(glow);

    // 2. Bảng cuộn gấm da hoàng gia (ui_panel.png 860x540)
    try {
        sf::Sprite panel(ResourceManager::getInstance().getTexture("ui_panel.png"));
        panel.setPosition(210.f, 95.f);
        window.draw(panel);
    } catch (...) {
        sf::RectangleShape p(sf::Vector2f(860.f, 540.f));
        p.setPosition(210.f, 95.f);
        p.setFillColor(sf::Color(40, 30, 20));
        p.setOutlineColor(sf::Color(255, 215, 0));
        p.setOutlineThickness(3.f);
        window.draw(p);
    }

    // 3. Huy hiệu Crest Chiến Thắng / Chiến Bại / Hòa (512x256)
    std::string badgeName = "badge_draw.png";
    if (matchWinner == 1) {
        badgeName = "badge_victory.png";
    } else if (matchWinner == 2) {
        if (gameMode == GameMode::PVE) {
            badgeName = "badge_defeat.png";
        } else {
            badgeName = "badge_victory.png";
        }
    }

    try {
        sf::Sprite badge(ResourceManager::getInstance().getTexture(badgeName));
        badge.setOrigin(256.f, 128.f);
        float bob = std::sin(elapsedTime * 2.5f) * 3.5f;
        badge.setPosition(640.f, 140.f + bob);
        badge.setScale(0.72f, 0.72f);
        window.draw(badge);
    } catch (...) {}

    if (!gameFont) return;

    // 4. Dòng cốt truyện phụ đề kết quả
    sf::Text sub(ConfigManager::toUtf8(resultText), *gameFont, 15);
    sub.setStyle(sf::Text::Bold);
    sub.setFillColor(sf::Color(70, 45, 25));
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setOrigin(sb.width / 2.f, sb.height / 2.f);
    sub.setPosition(640.f, 248.f);
    window.draw(sub);

    // Đường viền ngăn cách trang trí mạ vàng
    sf::RectangleShape divLine(sf::Vector2f(560.f, 2.f));
    divLine.setOrigin(280.f, 1.f);
    divLine.setPosition(640.f, 276.f);
    divLine.setFillColor(sf::Color(165, 125, 65, 180));
    window.draw(divLine);

    // 5. Bảng thống kê 2 cột đối xứng
    // CỘT TRÁI: Phe Xanh (P1)
    sf::RectangleShape p1Box(sf::Vector2f(314.f, 202.f));
    p1Box.setPosition(276.f, 288.f);
    p1Box.setFillColor(sf::Color(20, 60, 140, 35));
    p1Box.setOutlineColor(sf::Color(60, 120, 200, 160));
    p1Box.setOutlineThickness(1.5f);
    window.draw(p1Box);

    try {
        std::string p1Tex = "player1_sheet.png";
        if (p1SelectedHero == HeroClass::TACTICIAN) p1Tex = "hero_mage_sheet.png";
        else if (p1SelectedHero == HeroClass::TITAN) p1Tex = "hero_titan_sheet.png";

        sf::Sprite p1Head(ResourceManager::getInstance().getTexture(p1Tex));
        p1Head.setTextureRect(sf::IntRect(0, 0, 64, 64));
        p1Head.setOrigin(32.f, 32.f);
        p1Head.setPosition(310.f, 314.f);
        p1Head.setScale(0.38f, 0.38f);
        window.draw(p1Head);

        sf::Sprite p1Frame(ResourceManager::getInstance().getTexture("hud_frame_blue.png"));
        p1Frame.setOrigin(48.f, 48.f);
        p1Frame.setPosition(310.f, 314.f);
        p1Frame.setScale(0.36f, 0.36f);
        window.draw(p1Frame);
    } catch (...) {}

    std::string p1TitleStr = u8"PHE XANH (P1)";
    if (p1SelectedHero == HeroClass::VANGUARD) p1TitleStr = u8"TIÊN PHONG (P1)";
    else if (p1SelectedHero == HeroClass::TACTICIAN) p1TitleStr = u8"QUÂN SƯ (P1)";
    else if (p1SelectedHero == HeroClass::TITAN) p1TitleStr = u8"HỘ QUỐC (P1)";

    sf::Text p1Title(ConfigManager::toUtf8(p1TitleStr), *gameFont, 15);
    p1Title.setStyle(sf::Text::Bold);
    p1Title.setFillColor(sf::Color(25, 65, 150));
    p1Title.setPosition(338.f, 305.f);
    window.draw(p1Title);

    // Dòng 1: Tổng số lính
    try {
        sf::Sprite vIcon(ResourceManager::getInstance().getTexture("icon_base_vault.png"));
        vIcon.setOrigin(24.f, 24.f);
        vIcon.setPosition(305.f, 346.f);
        vIcon.setScale(0.38f, 0.38f);
        window.draw(vIcon);
    } catch (...) {}

    sf::Text p1TotTroopLbl(ConfigManager::toUtf8(u8"Tổng số lính:"), *gameFont, 14);
    p1TotTroopLbl.setStyle(sf::Text::Bold);
    p1TotTroopLbl.setFillColor(sf::Color(65, 40, 20));
    p1TotTroopLbl.setPosition(326.f, 338.f);
    window.draw(p1TotTroopLbl);

    sf::Text p1TotTroopVal(std::to_string(matchBlueTotalTroops) + ConfigManager::toUtf8(u8" quân"), *gameFont, 15);
    p1TotTroopVal.setStyle(sf::Text::Bold);
    p1TotTroopVal.setFillColor(sf::Color(20, 75, 180));
    sf::FloatRect p1ttb = p1TotTroopVal.getLocalBounds();
    p1TotTroopVal.setPosition(575.f - p1ttb.width, 337.f);
    window.draw(p1TotTroopVal);

    // Dòng 2: Chi tiết từng loại lính (Kiếm Sĩ / Hộ Vệ / Kỵ Binh)
    sf::Text p1SwordLbl(ConfigManager::toUtf8(u8"• Kiếm Sĩ (Lực 1):"), *gameFont, 13);
    p1SwordLbl.setFillColor(sf::Color(80, 50, 25));
    p1SwordLbl.setPosition(310.f, 362.f);
    window.draw(p1SwordLbl);

    sf::Text p1SwordVal(std::to_string(matchBlueSwordsman), *gameFont, 13);
    p1SwordVal.setStyle(sf::Text::Bold);
    p1SwordVal.setFillColor(sf::Color(170, 95, 15));
    sf::FloatRect p1swb = p1SwordVal.getLocalBounds();
    p1SwordVal.setPosition(575.f - p1swb.width, 362.f);
    window.draw(p1SwordVal);

    sf::Text p1GuardLbl(ConfigManager::toUtf8(u8"• Hộ Vệ   (Lực 2):"), *gameFont, 13);
    p1GuardLbl.setFillColor(sf::Color(80, 50, 25));
    p1GuardLbl.setPosition(310.f, 382.f);
    window.draw(p1GuardLbl);

    sf::Text p1GuardVal(std::to_string(matchBlueGuardian), *gameFont, 13);
    p1GuardVal.setStyle(sf::Text::Bold);
    p1GuardVal.setFillColor(sf::Color(35, 120, 60));
    sf::FloatRect p1gvb = p1GuardVal.getLocalBounds();
    p1GuardVal.setPosition(575.f - p1gvb.width, 382.f);
    window.draw(p1GuardVal);

    sf::Text p1CavLbl(ConfigManager::toUtf8(u8"• Kỵ Binh (Lực 3):"), *gameFont, 13);
    p1CavLbl.setFillColor(sf::Color(80, 50, 25));
    p1CavLbl.setPosition(310.f, 402.f);
    window.draw(p1CavLbl);

    sf::Text p1CavVal(std::to_string(matchBlueCavalry), *gameFont, 13);
    p1CavVal.setStyle(sf::Text::Bold);
    p1CavVal.setFillColor(sf::Color(145, 35, 130));
    sf::FloatRect p1cvb = p1CavVal.getLocalBounds();
    p1CavVal.setPosition(575.f - p1cvb.width, 402.f);
    window.draw(p1CavVal);

    // Dòng 3: Tổng Lực Chiến Cả Đội P1
    sf::RectangleShape p1TotBox(sf::Vector2f(290.f, 34.f));
    p1TotBox.setPosition(288.f, 432.f);
    p1TotBox.setFillColor(sf::Color(40, 100, 220, 55));
    p1TotBox.setOutlineColor(sf::Color(70, 130, 230, 140));
    p1TotBox.setOutlineThickness(1.f);
    window.draw(p1TotBox);

    sf::Text p1TotLbl(ConfigManager::toUtf8(u8"TỔNG LỰC CHIẾN:"), *gameFont, 13);
    p1TotLbl.setStyle(sf::Text::Bold);
    p1TotLbl.setFillColor(sf::Color(15, 45, 120));
    p1TotLbl.setPosition(300.f, 440.f);
    window.draw(p1TotLbl);

    sf::Text p1TotVal(std::to_string(matchBlueTotalPower) + ConfigManager::toUtf8(u8" ĐIỂM"), *gameFont, 15);
    p1TotVal.setStyle(sf::Text::Bold);
    p1TotVal.setFillColor(sf::Color(10, 45, 150));
    sf::FloatRect tvb = p1TotVal.getLocalBounds();
    p1TotVal.setPosition(570.f - tvb.width, 439.f);
    window.draw(p1TotVal);

    // CỘT GIỮA: Biểu tượng kiếm đọ sức & VS
    try {
        sf::Sprite vsSwords(ResourceManager::getInstance().getTexture("icon_clash_swords.png"));
        vsSwords.setOrigin(32.f, 32.f);
        vsSwords.setPosition(640.f, 385.f);
        vsSwords.setScale(0.65f, 0.65f);
        window.draw(vsSwords);
    } catch (...) {}

    sf::Text vsTxt("VS", *gameFont, 18);
    vsTxt.setStyle(sf::Text::Bold);
    vsTxt.setFillColor(sf::Color(160, 40, 30));
    sf::FloatRect vsb = vsTxt.getLocalBounds();
    vsTxt.setOrigin(vsb.width / 2.f, vsb.height / 2.f);
    vsTxt.setPosition(640.f, 430.f);
    window.draw(vsTxt);

    // CỘT PHẢI: Phe Đỏ (P2 hoặc Bot)
    sf::RectangleShape p2Box(sf::Vector2f(314.f, 202.f));
    p2Box.setPosition(690.f, 288.f);
    p2Box.setFillColor(sf::Color(160, 30, 20, 35));
    p2Box.setOutlineColor(sf::Color(210, 60, 50, 160));
    p2Box.setOutlineThickness(1.5f);
    window.draw(p2Box);

    try {
        std::string redTex = (gameMode == GameMode::PVP) ? "player2_sheet.png" : "bot_sheet.png";
        if (gameMode == GameMode::PVP) {
            if (p2SelectedHero == HeroClass::VANGUARD) redTex = "player2_sheet.png";
            else if (p2SelectedHero == HeroClass::TACTICIAN) redTex = "hero_mage_sheet_red.png";
            else if (p2SelectedHero == HeroClass::TITAN) redTex = "hero_titan_sheet_red.png";
        } else {
            if (p2SelectedHero == HeroClass::VANGUARD) redTex = "bot_sheet.png";
            else if (p2SelectedHero == HeroClass::TACTICIAN) redTex = "hero_mage_sheet_red.png";
            else if (p2SelectedHero == HeroClass::TITAN) redTex = "hero_titan_sheet_red.png";
        }

        sf::Sprite p2Head(ResourceManager::getInstance().getTexture(redTex));
        p2Head.setTextureRect(sf::IntRect(0, 0, 64, 64));
        p2Head.setOrigin(32.f, 32.f);
        p2Head.setPosition(724.f, 314.f);
        p2Head.setScale(0.38f, 0.38f);
        window.draw(p2Head);

        sf::Sprite p2Frame(ResourceManager::getInstance().getTexture("hud_frame_red.png"));
        p2Frame.setOrigin(48.f, 48.f);
        p2Frame.setPosition(724.f, 314.f);
        p2Frame.setScale(0.36f, 0.36f);
        window.draw(p2Frame);
    } catch (...) {}

    std::string redName = u8"BẠO CHÚA (BOT)";
    if (gameMode == GameMode::PVP) {
        if (p2SelectedHero == HeroClass::VANGUARD) redName = u8"TIÊN PHONG (P2)";
        else if (p2SelectedHero == HeroClass::TACTICIAN) redName = u8"QUÂN SƯ (P2)";
        else if (p2SelectedHero == HeroClass::TITAN) redName = u8"HỘ QUỐC (P2)";
    } else {
        if (p2SelectedHero == HeroClass::VANGUARD) redName = u8"TIÊN PHONG (BOT)";
        else if (p2SelectedHero == HeroClass::TACTICIAN) redName = u8"QUÂN SƯ (BOT)";
        else if (p2SelectedHero == HeroClass::TITAN) redName = u8"HỘ QUỐC (BOT)";
    }

    sf::Text p2Title(ConfigManager::toUtf8(redName), *gameFont, 15);
    p2Title.setStyle(sf::Text::Bold);
    p2Title.setFillColor(sf::Color(160, 30, 20));
    p2Title.setPosition(752.f, 305.f);
    window.draw(p2Title);

    // Dòng 1: Tổng số lính P2/Bot
    try {
        sf::Sprite vIcon(ResourceManager::getInstance().getTexture("icon_base_vault.png"));
        vIcon.setOrigin(24.f, 24.f);
        vIcon.setPosition(718.f, 346.f);
        vIcon.setScale(0.38f, 0.38f);
        window.draw(vIcon);
    } catch (...) {}

    sf::Text p2TotTroopLbl(ConfigManager::toUtf8(u8"Tổng số lính:"), *gameFont, 14);
    p2TotTroopLbl.setStyle(sf::Text::Bold);
    p2TotTroopLbl.setFillColor(sf::Color(65, 40, 20));
    p2TotTroopLbl.setPosition(739.f, 338.f);
    window.draw(p2TotTroopLbl);

    sf::Text p2TotTroopVal(std::to_string(matchRedTotalTroops) + ConfigManager::toUtf8(u8" quân"), *gameFont, 15);
    p2TotTroopVal.setStyle(sf::Text::Bold);
    p2TotTroopVal.setFillColor(sf::Color(170, 30, 20));
    sf::FloatRect p2ttb = p2TotTroopVal.getLocalBounds();
    p2TotTroopVal.setPosition(985.f - p2ttb.width, 337.f);
    window.draw(p2TotTroopVal);

    // Dòng 2: Chi tiết từng loại lính P2/Bot (Kiếm Sĩ / Hộ Vệ / Kỵ Binh)
    sf::Text p2SwordLbl(ConfigManager::toUtf8(u8"• Kiếm Sĩ (Lực 1):"), *gameFont, 13);
    p2SwordLbl.setFillColor(sf::Color(80, 50, 25));
    p2SwordLbl.setPosition(722.f, 362.f);
    window.draw(p2SwordLbl);

    sf::Text p2SwordVal(std::to_string(matchRedSwordsman), *gameFont, 13);
    p2SwordVal.setStyle(sf::Text::Bold);
    p2SwordVal.setFillColor(sf::Color(170, 95, 15));
    sf::FloatRect p2swb = p2SwordVal.getLocalBounds();
    p2SwordVal.setPosition(985.f - p2swb.width, 362.f);
    window.draw(p2SwordVal);

    sf::Text p2GuardLbl(ConfigManager::toUtf8(u8"• Hộ Vệ   (Lực 2):"), *gameFont, 13);
    p2GuardLbl.setFillColor(sf::Color(80, 50, 25));
    p2GuardLbl.setPosition(722.f, 382.f);
    window.draw(p2GuardLbl);

    sf::Text p2GuardVal(std::to_string(matchRedGuardian), *gameFont, 13);
    p2GuardVal.setStyle(sf::Text::Bold);
    p2GuardVal.setFillColor(sf::Color(35, 120, 60));
    sf::FloatRect p2gvb = p2GuardVal.getLocalBounds();
    p2GuardVal.setPosition(985.f - p2gvb.width, 382.f);
    window.draw(p2GuardVal);

    sf::Text p2CavLbl(ConfigManager::toUtf8(u8"• Kỵ Binh (Lực 3):"), *gameFont, 13);
    p2CavLbl.setFillColor(sf::Color(80, 50, 25));
    p2CavLbl.setPosition(722.f, 402.f);
    window.draw(p2CavLbl);

    sf::Text p2CavVal(std::to_string(matchRedCavalry), *gameFont, 13);
    p2CavVal.setStyle(sf::Text::Bold);
    p2CavVal.setFillColor(sf::Color(145, 35, 130));
    sf::FloatRect p2cvb = p2CavVal.getLocalBounds();
    p2CavVal.setPosition(985.f - p2cvb.width, 402.f);
    window.draw(p2CavVal);

    // Dòng 3: Tổng Lực Chiến Cả Đội P2/Bot
    sf::RectangleShape p2TotBox(sf::Vector2f(290.f, 34.f));
    p2TotBox.setPosition(702.f, 432.f);
    p2TotBox.setFillColor(sf::Color(200, 40, 30, 55));
    p2TotBox.setOutlineColor(sf::Color(220, 60, 50, 140));
    p2TotBox.setOutlineThickness(1.f);
    window.draw(p2TotBox);

    sf::Text p2TotLbl(ConfigManager::toUtf8(u8"TỔNG LỰC CHIẾN:"), *gameFont, 13);
    p2TotLbl.setStyle(sf::Text::Bold);
    p2TotLbl.setFillColor(sf::Color(110, 20, 15));
    p2TotLbl.setPosition(714.f, 440.f);
    window.draw(p2TotLbl);

    sf::Text p2TotVal(std::to_string(matchRedTotalPower) + ConfigManager::toUtf8(u8" ĐIỂM"), *gameFont, 15);
    p2TotVal.setStyle(sf::Text::Bold);
    p2TotVal.setFillColor(sf::Color(140, 20, 15));
    sf::FloatRect rtvb = p2TotVal.getLocalBounds();
    p2TotVal.setPosition(980.f - rtvb.width, 439.f);
    window.draw(p2TotVal);

    // 6. Nút bấm tương tác
    // Nút Tái Chiến (Replay)
    sf::FloatRect replayRect(360.f, 516.f, 240.f, 46.f);
    bool replayHover = replayRect.contains(mousePos);
    try {
        sf::Sprite rBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        rBtn.setOrigin(150.f, 30.f);
        rBtn.setPosition(480.f, 539.f);
        rBtn.setScale(0.80f, 0.77f);
        if (replayHover) rBtn.setColor(sf::Color(255, 255, 210));
        window.draw(rBtn);
    } catch (...) {
        sf::RectangleShape rb(sf::Vector2f(240.f, 46.f));
        rb.setOrigin(120.f, 23.f);
        rb.setPosition(480.f, 539.f);
        rb.setFillColor(replayHover ? sf::Color(50, 120, 50) : sf::Color(35, 90, 35));
        rb.setOutlineColor(sf::Color(255, 215, 0));
        rb.setOutlineThickness(2.f);
        window.draw(rb);
    }
    sf::Text replayTxt(ConfigManager::toUtf8(u8"TÁI CHIẾN (ENTER)"), *gameFont, 15);
    replayTxt.setStyle(sf::Text::Bold);
    replayTxt.setFillColor(replayHover ? sf::Color(255, 250, 140) : sf::Color(250, 240, 220));
    sf::FloatRect rtb = replayTxt.getLocalBounds();
    replayTxt.setOrigin(rtb.width / 2.f, rtb.height / 2.f + 2.f);
    replayTxt.setPosition(480.f, 537.f);
    window.draw(replayTxt);

    // Nút Menu Chính
    sf::FloatRect menuRect(680.f, 516.f, 240.f, 46.f);
    bool menuHover = menuRect.contains(mousePos);
    try {
        sf::Sprite mBtn(ResourceManager::getInstance().getTexture("ui_button.png"));
        mBtn.setOrigin(150.f, 30.f);
        mBtn.setPosition(800.f, 539.f);
        mBtn.setScale(0.80f, 0.77f);
        if (menuHover) mBtn.setColor(sf::Color(255, 220, 220));
        window.draw(mBtn);
    } catch (...) {
        sf::RectangleShape mb(sf::Vector2f(240.f, 46.f));
        mb.setOrigin(120.f, 23.f);
        mb.setPosition(800.f, 539.f);
        mb.setFillColor(menuHover ? sf::Color(160, 40, 40) : sf::Color(120, 25, 25));
        mb.setOutlineColor(sf::Color(255, 215, 0));
        mb.setOutlineThickness(2.f);
        window.draw(mb);
    }
    sf::Text menuTxt(ConfigManager::toUtf8(u8"MENU CHÍNH (ESC)"), *gameFont, 15);
    menuTxt.setStyle(sf::Text::Bold);
    menuTxt.setFillColor(menuHover ? sf::Color(255, 230, 140) : sf::Color(250, 230, 210));
    sf::FloatRect mtb = menuTxt.getLocalBounds();
    menuTxt.setOrigin(mtb.width / 2.f, mtb.height / 2.f + 2.f);
    menuTxt.setPosition(800.f, 537.f);
    window.draw(menuTxt);

    // Hint phím tắt dưới cùng
    sf::Text hint(ConfigManager::toUtf8(u8"Nhấn [ENTER] để chơi lại  •  [ESC] để về Menu  •  Click Chuột để chọn"), *gameFont, 12);
    hint.setStyle(sf::Text::Italic);
    hint.setFillColor(sf::Color(100, 70, 45));
    sf::FloatRect htb = hint.getLocalBounds();
    hint.setOrigin(htb.width / 2.f, htb.height / 2.f);
    hint.setPosition(640.f, 584.f);
    window.draw(hint);
}

// ================================================================
//  saveMatchHistoryToCSV – Lưu trữ dữ liệu trận đấu bền vững ra file CSV
//  Thỏa mãn yêu cầu YC05 về Lưu trữ bền vững (Persistence) trong OOP
// ================================================================
void GameEngine::saveMatchHistoryToCSV(const std::string& filename) {
    bool fileExists = false;
    {
        std::ifstream check(filename);
        if (check.good()) fileExists = true;
    }

    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) return;

    // Nếu file chưa tồn tại -> Ghi hàng tiêu đề (CSV Header)
    if (!fileExists) {
        out << "ThoiGian,CheDo,TuongPheXanh,TuongPheDo,"
            << "TongLinhXanh,LinhKiemXanh,LinhHoVeXanh,LinhKyBinhXanh,TongLucChienXanh,"
            << "TongLinhDo,LinhKiemDo,LinhHoVeDo,LinhKyBinhDo,TongLucChienDo,"
            << "KetQua\n";
    }

    // Thời gian kết thúc trận đấu
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&now_tm, &now_c);
#else
    localtime_r(&now_c, &now_tm);
#endif
    char timeBuffer[32];
    std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", &now_tm);

    std::string modeStr = (gameMode == GameMode::PVE) ? "PvE (Dau Bot)" : "PvP (2 Nguoi)";

    auto getHeroStr = [](HeroClass hc) -> std::string {
        if (hc == HeroClass::VANGUARD) return "Tien Phong";
        if (hc == HeroClass::TACTICIAN) return "Quan Su";
        return "Ho Quoc";
    };

    std::string p1Hero = getHeroStr(p1SelectedHero);
    std::string p2Hero = getHeroStr(p2SelectedHero);
    std::string winnerStr = (matchWinner == 1) ? "Phe Xanh Thang" :
                            (matchWinner == 2) ? "Phe Do Thang" : "Hoa";

    out << timeBuffer << ","
        << modeStr << ","
        << p1Hero << ","
        << p2Hero << ","
        << matchBlueTotalTroops << ","
        << matchBlueSwordsman << ","
        << matchBlueGuardian << ","
        << matchBlueCavalry << ","
        << matchBlueTotalPower << ","
        << matchRedTotalTroops << ","
        << matchRedSwordsman << ","
        << matchRedGuardian << ","
        << matchRedCavalry << ","
        << matchRedTotalPower << ","
        << winnerStr << "\n";

    out.close();
}

void GameEngine::renderHUD() {
    window.setView(window.getDefaultView());

    if (!gameFont) return;

    // Thanh nền mờ phía trên (Top Header Bar) - Chiều cao 54px
    sf::RectangleShape topBar(sf::Vector2f(1280.f, 54.f));
    topBar.setPosition(0.f, 0.f);
    topBar.setFillColor(sf::Color(12, 14, 22, 225));
    window.draw(topBar);

    // Đường viền dưới của top bar ánh kim
    sf::RectangleShape topBarBorder(sf::Vector2f(1280.f, 2.5f));
    topBarBorder.setPosition(0.f, 54.f);
    topBarBorder.setFillColor(sf::Color(255, 215, 0, 110));
    window.draw(topBarBorder);

    // Timer đếm ngược ở trung tâm màn hình
    std::stringstream ss;
    ss << (int)gatheringTimer << "s";
    sf::Text timer(ss.str(), *gameFont, 27);
    timer.setStyle(sf::Text::Bold);
    if (gatheringTimer <= 15.f) {
        float pulse = 1.0f + 0.08f * std::sin(elapsedTime * 9.f);
        timer.setScale(pulse, pulse);
        timer.setFillColor(sf::Color(255, 65, 65)); // Báo động đỏ khi sắp hết giờ
    } else {
        timer.setFillColor(sf::Color(255, 215, 0)); // Vàng hoàng gia
    }
    sf::FloatRect tb = timer.getLocalBounds();
    timer.setOrigin(tb.width / 2.f, tb.height / 2.f);
    timer.setPosition(640.f, 27.f);
    window.draw(timer);

    // Thông tin Phe Xanh (Player 1) - Bên trái kèm khung nguyệt quế và avatar
    if (player1) {
        try {
            std::string p1Tex = "player1_sheet.png";
            if (player1->getHeroClass() == HeroClass::TACTICIAN) p1Tex = "hero_mage_sheet.png";
            else if (player1->getHeroClass() == HeroClass::TITAN) p1Tex = "hero_titan_sheet.png";

            sf::Sprite p1Head(ResourceManager::getInstance().getTexture(p1Tex));
            p1Head.setTextureRect(sf::IntRect(0, 0, 64, 64));
            p1Head.setOrigin(32.f, 32.f);
            p1Head.setPosition(34.f, 27.f);
            p1Head.setScale(0.54f, 0.54f);
            window.draw(p1Head);

            sf::Sprite p1Frame(ResourceManager::getInstance().getTexture("hud_frame_blue.png"));
            p1Frame.setOrigin(48.f, 48.f);
            p1Frame.setPosition(34.f, 27.f);
            p1Frame.setScale(0.52f, 0.52f);
            window.draw(p1Frame);
        } catch (...) {}

        std::string p1Title = u8"TIÊN PHONG (XANH)";
        if (player1->getHeroClass() == HeroClass::TACTICIAN) p1Title = u8"QUÂN SƯ (XANH)";
        else if (player1->getHeroClass() == HeroClass::TITAN) p1Title = u8"HỘ QUỐC (XANH)";

        sf::Text p1Name(ConfigManager::toUtf8(p1Title), *gameFont, 14);
        p1Name.setStyle(sf::Text::Bold);
        p1Name.setFillColor(sf::Color(120, 210, 255));
        p1Name.setPosition(66.f, 7.f);
        window.draw(p1Name);

        std::string p1ArmyStr = u8"Quân: " + std::to_string(player1->getArmy().getCount());
        sf::Text p1Army(ConfigManager::toUtf8(p1ArmyStr), *gameFont, 13);
        p1Army.setStyle(sf::Text::Bold);
        p1Army.setFillColor(sf::Color(240, 245, 255));
        p1Army.setPosition(66.f, 27.f);
        window.draw(p1Army);

        float vaultX = 66.f + p1Army.getLocalBounds().width + 16.f;
        try {
            sf::Sprite vIcon(ResourceManager::getInstance().getTexture("icon_base_vault.png"));
            vIcon.setOrigin(24.f, 24.f);
            vIcon.setPosition(vaultX, 36.f);
            vIcon.setScale(0.48f, 0.48f);
            window.draw(vIcon);
        } catch (...) {}

        std::string p1VaultStr = ": " + std::to_string(baseBlue ? baseBlue->getStoredCount() : 0);
        sf::Text p1Vault(p1VaultStr, *gameFont, 13);
        p1Vault.setStyle(sf::Text::Bold);
        p1Vault.setFillColor(sf::Color(255, 220, 100));
        p1Vault.setPosition(vaultX + 13.f, 27.f);
        window.draw(p1Vault);
    }

    // Thông tin Phe Đỏ (Player 2 hoặc Bot) - Bên phải kèm khung nguyệt quế và avatar
    try {
        std::string redTex = (gameMode == GameMode::PVP) ? "player2_sheet.png" : "bot_sheet.png";
        if (gameMode == GameMode::PVP && player2) {
            if (player2->getHeroClass() == HeroClass::VANGUARD) redTex = "player2_sheet.png";
            else if (player2->getHeroClass() == HeroClass::TACTICIAN) redTex = "hero_mage_sheet_red.png";
            else if (player2->getHeroClass() == HeroClass::TITAN) redTex = "hero_titan_sheet_red.png";
        } else if (bot) {
            if (bot->getHeroClass() == HeroClass::VANGUARD) redTex = "bot_sheet.png";
            else if (bot->getHeroClass() == HeroClass::TACTICIAN) redTex = "hero_mage_sheet_red.png";
            else if (bot->getHeroClass() == HeroClass::TITAN) redTex = "hero_titan_sheet_red.png";
        }

        sf::Sprite redHead(ResourceManager::getInstance().getTexture(redTex));
        redHead.setTextureRect(sf::IntRect(0, 0, 64, 64));
        redHead.setOrigin(32.f, 32.f);
        redHead.setPosition(1246.f, 27.f);
        redHead.setScale(0.54f, 0.54f);
        window.draw(redHead);

        sf::Sprite redFrame(ResourceManager::getInstance().getTexture("hud_frame_red.png"));
        redFrame.setOrigin(48.f, 48.f);
        redFrame.setPosition(1246.f, 27.f);
        redFrame.setScale(0.52f, 0.52f);
        window.draw(redFrame);
    } catch (...) {}

    std::string redNameStr = u8"BẠO CHÚA (BOT)";
    if (gameMode == GameMode::PVP && player2) {
        if (player2->getHeroClass() == HeroClass::VANGUARD) redNameStr = u8"TIÊN PHONG (ĐỎ)";
        else if (player2->getHeroClass() == HeroClass::TACTICIAN) redNameStr = u8"QUÂN SƯ (ĐỎ)";
        else if (player2->getHeroClass() == HeroClass::TITAN) redNameStr = u8"HỘ QUỐC (ĐỎ)";
    } else if (bot) {
        if (bot->getHeroClass() == HeroClass::VANGUARD) redNameStr = u8"TIÊN PHONG (BOT)";
        else if (bot->getHeroClass() == HeroClass::TACTICIAN) redNameStr = u8"QUÂN SƯ (BOT)";
        else if (bot->getHeroClass() == HeroClass::TITAN) redNameStr = u8"HỘ QUỐC (BOT)";
    }

    sf::Text redName(ConfigManager::toUtf8(redNameStr), *gameFont, 14);
    redName.setStyle(sf::Text::Bold);
    redName.setFillColor(sf::Color(255, 125, 120));
    sf::FloatRect rnb = redName.getLocalBounds();
    redName.setPosition(1216.f - rnb.width, 7.f);
    window.draw(redName);

    int redStored = baseRed ? baseRed->getStoredCount() : 0;
    int redTroops = (gameMode == GameMode::PVP && player2) ? player2->getArmy().getCount() : (bot ? bot->getArmy().getCount() : 0);

    std::string redVaultStr = ": " + std::to_string(redStored);
    sf::Text redVault(redVaultStr, *gameFont, 13);
    redVault.setStyle(sf::Text::Bold);
    redVault.setFillColor(sf::Color(255, 220, 100));
    sf::FloatRect rvb = redVault.getLocalBounds();
    redVault.setPosition(1216.f - rvb.width, 27.f);
    window.draw(redVault);

    float redVaultIconX = 1216.f - rvb.width - 15.f;
    try {
        sf::Sprite vIcon(ResourceManager::getInstance().getTexture("icon_base_vault.png"));
        vIcon.setOrigin(24.f, 24.f);
        vIcon.setPosition(redVaultIconX, 36.f);
        vIcon.setScale(0.48f, 0.48f);
        window.draw(vIcon);
    } catch (...) {}

    std::string redArmyStr = u8"Quân: " + std::to_string(redTroops);
    sf::Text redArmy(ConfigManager::toUtf8(redArmyStr), *gameFont, 13);
    redArmy.setStyle(sf::Text::Bold);
    redArmy.setFillColor(sf::Color(255, 240, 240));
    sf::FloatRect rab = redArmy.getLocalBounds();
    redArmy.setPosition(redVaultIconX - 18.f - rab.width, 27.f);
    window.draw(redArmy);

    // Banner Sự Kiện Nổi Rồng Cuộn (banner_ribbon.png 600x90)
    if (bannerTimer > 0.f) {
        float alpha = 255.f;
        if (bannerTimer > bannerMaxDuration - 0.35f) {
            alpha = 255.f * (bannerMaxDuration - bannerTimer) / 0.35f;
        } else if (bannerTimer < 0.5f) {
            alpha = 255.f * (bannerTimer / 0.5f);
        }
        alpha = std::clamp(alpha, 0.f, 255.f);

        try {
            sf::Sprite ribbon(ResourceManager::getInstance().getTexture("banner_ribbon.png"));
            ribbon.setOrigin(300.f, 45.f);
            ribbon.setPosition(640.f, 96.f);
            ribbon.setScale(0.95f, 0.95f);
            ribbon.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
            window.draw(ribbon);
        } catch (...) {}

        // Bóng đổ chữ
        sf::Text bShadow(ConfigManager::toUtf8(bannerText), *gameFont, 17);
        bShadow.setStyle(sf::Text::Bold);
        bShadow.setFillColor(sf::Color(0, 0, 0, (sf::Uint8)(alpha * 0.9f)));
        sf::FloatRect bsb = bShadow.getLocalBounds();
        bShadow.setOrigin(bsb.width / 2.f, bsb.height / 2.f);
        bShadow.setPosition(641.5f, 95.5f);
        window.draw(bShadow);

        // Chữ chính
        sf::Text bText(ConfigManager::toUtf8(bannerText), *gameFont, 17);
        bText.setStyle(sf::Text::Bold);
        bText.setFillColor(sf::Color(bannerColor.r, bannerColor.g, bannerColor.b, (sf::Uint8)alpha));
        sf::FloatRect btb = bText.getLocalBounds();
        bText.setOrigin(btb.width / 2.f, btb.height / 2.f);
        bText.setPosition(640.f, 94.f);
        window.draw(bText);
    }

    // Cảnh báo Hỏa Long Giáng Lâm trên UI
    for (auto de : dragonEvents) {
        de->drawUI(window, gameFont);
    }

    // ================================================================
    //  KỸ NĂNG TƯỚNG (SKILL HUD)
    // ================================================================
    // 1. Skill Badge Player 1 (Góc dưới bên trái)
    if (player1) {
        float badgeX = 25.f;
        float badgeY = 665.f;
        float badgeW = 230.f;
        float badgeH = 42.f;

        bool ready = player1->canUseSkill();
        float progress = player1->getSkillCooldownProgress();

        sf::RectangleShape box(sf::Vector2f(badgeW, badgeH));
        box.setPosition(badgeX, badgeY);
        box.setFillColor(ready ? sf::Color(15, 30, 50, 220) : sf::Color(25, 25, 30, 200));
        box.setOutlineColor(ready ? sf::Color(100, 220, 255, 230) : sf::Color(100, 110, 130, 160));
        box.setOutlineThickness(ready ? 2.5f : 1.5f);
        window.draw(box);

        if (!ready) {
            sf::RectangleShape bar(sf::Vector2f(badgeW * progress, 3.5f));
            bar.setPosition(badgeX, badgeY + badgeH - 3.5f);
            bar.setFillColor(sf::Color(100, 200, 255));
            window.draw(bar);
        }

        std::string p1SkillLabel = "[SPACE] " + player1->getSkillName();
        sf::Text skillName(ConfigManager::toUtf8(p1SkillLabel), *gameFont, 12);
        skillName.setStyle(sf::Text::Bold);
        skillName.setFillColor(ready ? sf::Color(255, 245, 140) : sf::Color(180, 180, 190));
        skillName.setPosition(badgeX + 10.f, badgeY + 5.f);
        window.draw(skillName);

        std::string statusStr;
        if (ready) {
            statusStr = u8"SẴN SÀNG! [SPACE]";
        } else {
            int tenths = (int)(player1->getSkillTimer() * 10.f);
            statusStr = u8"Hồi chiêu: " + std::to_string(tenths / 10) + "." + std::to_string(tenths % 10) + "s";
        }
        sf::Text statusTxt(ConfigManager::toUtf8(statusStr), *gameFont, 11);
        statusTxt.setStyle(sf::Text::Bold);
        statusTxt.setFillColor(ready ? sf::Color(80, 255, 140) : sf::Color(255, 140, 100));
        statusTxt.setPosition(badgeX + 10.f, badgeY + 23.f);
        window.draw(statusTxt);
    }

    // 2. Skill Badge Player 2 (PvP) hoặc Bot (PvE) - Góc dưới bên phải
    if (gameMode == GameMode::PVP && player2) {
        float badgeW = 230.f;
        float badgeX = 1280.f - 25.f - badgeW;
        float badgeY = 665.f;
        float badgeH = 42.f;

        bool ready = player2->canUseSkill();
        float progress = player2->getSkillCooldownProgress();

        sf::RectangleShape box(sf::Vector2f(badgeW, badgeH));
        box.setPosition(badgeX, badgeY);
        box.setFillColor(ready ? sf::Color(50, 20, 20, 220) : sf::Color(30, 25, 25, 200));
        box.setOutlineColor(ready ? sf::Color(255, 120, 100, 230) : sf::Color(140, 110, 110, 160));
        box.setOutlineThickness(ready ? 2.5f : 1.5f);
        window.draw(box);

        if (!ready) {
            sf::RectangleShape bar(sf::Vector2f(badgeW * progress, 3.5f));
            bar.setPosition(badgeX, badgeY + badgeH - 3.5f);
            bar.setFillColor(sf::Color(255, 100, 90));
            window.draw(bar);
        }

        std::string p2SkillLabel = "[R-SHIFT] " + player2->getSkillName();
        sf::Text skillName(ConfigManager::toUtf8(p2SkillLabel), *gameFont, 12);
        skillName.setStyle(sf::Text::Bold);
        skillName.setFillColor(ready ? sf::Color(255, 245, 140) : sf::Color(190, 180, 180));
        skillName.setPosition(badgeX + 10.f, badgeY + 5.f);
        window.draw(skillName);

        std::string statusStr;
        if (ready) {
            statusStr = u8"SẴN SÀNG! [R-SHIFT]";
        } else {
            int tenths = (int)(player2->getSkillTimer() * 10.f);
            statusStr = u8"Hồi chiêu: " + std::to_string(tenths / 10) + "." + std::to_string(tenths % 10) + "s";
        }
        sf::Text statusTxt(ConfigManager::toUtf8(statusStr), *gameFont, 11);
        statusTxt.setStyle(sf::Text::Bold);
        statusTxt.setFillColor(ready ? sf::Color(80, 255, 140) : sf::Color(255, 140, 100));
        statusTxt.setPosition(badgeX + 10.f, badgeY + 23.f);
        window.draw(statusTxt);
    } else if (bot && gameMode == GameMode::PVE) {
        float badgeW = 230.f;
        float badgeX = 1280.f - 25.f - badgeW;
        float badgeY = 665.f;
        float badgeH = 42.f;

        bool ready = bot->canUseSkill();
        float progress = bot->getSkillCooldownProgress();

        sf::RectangleShape box(sf::Vector2f(badgeW, badgeH));
        box.setPosition(badgeX, badgeY);
        box.setFillColor(ready ? sf::Color(50, 15, 15, 220) : sf::Color(30, 25, 25, 200));
        box.setOutlineColor(ready ? sf::Color(255, 80, 70, 230) : sf::Color(130, 110, 110, 150));
        box.setOutlineThickness(ready ? 2.5f : 1.5f);
        window.draw(box);

        if (!ready) {
            sf::RectangleShape bar(sf::Vector2f(badgeW * progress, 3.5f));
            bar.setPosition(badgeX, badgeY + badgeH - 3.5f);
            bar.setFillColor(sf::Color(255, 90, 80));
            window.draw(bar);
        }

        std::string botSkillLabel = "BOT: " + bot->getSkillName();
        sf::Text skillName(ConfigManager::toUtf8(botSkillLabel), *gameFont, 12);
        skillName.setStyle(sf::Text::Bold);
        skillName.setFillColor(ready ? sf::Color(255, 200, 150) : sf::Color(180, 170, 170));
        skillName.setPosition(badgeX + 8.f, badgeY + 5.f);
        window.draw(skillName);

        std::string statusStr = ready ? u8"NGUY HIỂM: CHIÊU SẴN SÀNG!" : (u8"Tích tụ khí: " + std::to_string((int)bot->getSkillTimer()) + "s");
        sf::Text statusTxt(ConfigManager::toUtf8(statusStr), *gameFont, 11);
        statusTxt.setStyle(sf::Text::Bold);
        statusTxt.setFillColor(ready ? sf::Color(255, 60, 60) : sf::Color(190, 180, 180));
        statusTxt.setPosition(badgeX + 8.f, badgeY + 23.f);
        window.draw(statusTxt);
    }
}