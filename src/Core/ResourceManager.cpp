#include "ResourceManager.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

// ================================================================
//  detectBasePath – Tự tìm thư mục gốc project
//  exe nằm ở bin/main.exe → basePath = thư mục cha của bin/
//  Nếu không tìm được → dùng thư mục hiện tại (CWD)
// ================================================================
void ResourceManager::detectBasePath() {
#ifdef _WIN32
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath);

    // Cắt tên file exe ra → lấy thư mục bin/
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }

    // Nhảy lên 1 cấp từ bin/ → thư mục gốc project
    size_t parentSlash = exeDir.find_last_of("\\/");
    if (parentSlash != std::string::npos) {
        basePath = exeDir.substr(0, parentSlash + 1); // Giữ dấu /
    } else {
        basePath = ""; // Fallback: dùng CWD
    }
#else
    basePath = ""; // Linux/Mac: dùng CWD
#endif

    // Kiểm tra xem basePath + "assets/" có tồn tại không
    if (!basePath.empty() && std::filesystem::exists(basePath + "assets")) {
        std::cout << "[ResourceManager] Base path: " << basePath << std::endl;
    } else {
        // Fallback: thử CWD
        basePath = "";
        std::cout << "[ResourceManager] Using CWD as base path" << std::endl;
    }
}

// ================================================================
//  getFullPath – Ghép basePath + relative path
// ================================================================
std::string ResourceManager::getFullPath(const std::string& relativePath) const {
    return basePath + relativePath;
}

// ================================================================
//  preloadAll – Load tất cả asset 1 lần duy nhất khi khởi động
//  Tránh load trùng lặp, tối ưu tốc độ khung hình
// ================================================================
void ResourceManager::preloadAll() {
    // Tự detect đường dẫn gốc
    detectBasePath();

    // Danh sách toàn bộ sprite cần load
    const std::string spriteFiles[] = {
        // Tướng (Leaders) — Sprite Sheet 256x256 (3 Hệ Tướng Xanh & Đỏ)
        "player1_sheet.png",         // Tiên Phong Xanh
        "player2_sheet.png",         // Tiên Phong Đỏ
        "bot_sheet.png",             // Bot Tiên Phong Đỏ
        "hero_mage_sheet.png",       // Quân Sư Xanh
        "hero_mage_sheet_red.png",   // Quân Sư Đỏ
        "hero_titan_sheet.png",      // Hộ Quốc Xanh
        "hero_titan_sheet_red.png",  // Hộ Quốc Đỏ
        // Quân lính (Troops) — Sprite Sheet 128x128
        "troop_swordsman_sheet.png",
        "troop_guardian_sheet.png",
        "troop_cavalry_sheet.png",
        // Căn cứ (Bases) — 256x256
        "base_blue.png",
        "base_red.png",
        // Địa hình & Cây cối
        "obstacle.png",
        "tree1.png",
        "tree2.png",
        "background_tile.png",
        // Vật phẩm (Items) — Sprite Sheet 128x32
        "item_speed_sheet.png",
        "item_shield_sheet.png",
        "item_magnet_sheet.png",
        // Sự kiện & Hiệu ứng
        "bomb.png",
        "explosion_sheet.png",
        "warning_zone.png",
        "shield_aura.png",
        // Đấu trường sa trường Phase 2
        "clash_arena_bg.png",
        // Giao diện Menu & UI
        "menu_bg.png",
        "ui_panel.png",
        "ui_button.png",
        // Địa hình mới & Cổ Long Sa Trường
        "bush.png",
        "ruins_pillar.png",
        "ruins_wall.png",
        "dragon_sheet.png",
        "dragon_fire_sheet.png",
        // Giao diện HUD & Màn hình Kết quả mới
        "badge_victory.png",
        "badge_defeat.png",
        "badge_draw.png",
        "hud_frame_blue.png",
        "hud_frame_red.png",
        "icon_base_vault.png",
        "icon_clash_swords.png",
        "banner_ribbon.png"
    };

    for (const auto& file : spriteFiles) {
        sf::Texture tex;
        std::string fullPath = getFullPath("assets/sprites/" + file);
        if (tex.loadFromFile(fullPath)) {
            textures[file] = tex;
            std::cout << "[ResourceManager] Loaded: " << file << std::endl;
        } else {
            std::cerr << "[ResourceManager] FAILED to load: " << fullPath << std::endl;
        }
    }

    // Load font
    sf::Font font;
    std::string fontPath = getFullPath("assets/fonts/arial.ttf");
    if (font.loadFromFile(fontPath)) {
        fonts["arial.ttf"] = font;
        std::cout << "[ResourceManager] Loaded font: arial.ttf" << std::endl;
    } else {
        std::cerr << "[ResourceManager] FAILED to load font: " << fontPath << std::endl;
    }
}

// ================================================================
//  getTexture – Lấy texture đã preload, throw nếu chưa load
// ================================================================
sf::Texture& ResourceManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it == textures.end()) {
        throw std::runtime_error("[ResourceManager] Texture not found: " + name);
    }
    return it->second;
}

// ================================================================
//  getFont – Lấy font đã preload, throw nếu chưa load
// ================================================================
sf::Font& ResourceManager::getFont(const std::string& name) {
    auto it = fonts.find(name);
    if (it == fonts.end()) {
        throw std::runtime_error("[ResourceManager] Font not found: " + name);
    }
    return it->second;
}