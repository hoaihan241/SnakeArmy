#include "Troop.h"
#include "../Core/ResourceManager.h"

// ================================================================
//  Troop Constructor
//  Khởi tạo node lính với sprite từ ResourceManager (Singleton)
// ================================================================
Troop::Troop(float startX, float startY, TroopType troopType, Team startTeam)
    : x(startX), y(startY),
      power(getDefaultPower(troopType)),
      type(troopType), team(startTeam),
      next(nullptr), prev(nullptr),
      animFrame(0), animRow(0), animTimer(0.f)
{
    // Load texture từ ResourceManager Singleton
    sf::Texture& tex = ResourceManager::getInstance().getTexture(getSheetName(troopType));
    sprite.setTexture(tex);

    // Cắt frame đầu tiên (frame 0, hướng xuống)
    int fs = getFrameSize();
    sprite.setTextureRect(sf::IntRect(0, 0, fs, fs));

    // Đặt origin vào tâm sprite
    sprite.setOrigin(fs / 2.f, fs / 2.f);
    sprite.setScale(1.0f, 1.0f); // Ảnh mới 48x48 pixel-perfect 1:1, không bị mờ
    sprite.setPosition(x, y);

    // Áp dụng màu phe
    sprite.setColor(getTeamColor());
}

// ================================================================
//  Static helpers
// ================================================================
std::string Troop::getSheetName(TroopType type) {
    switch (type) {
        case TroopType::GUARDIAN:  return "troop_guardian_sheet.png";
        case TroopType::CAVALRY:  return "troop_cavalry_sheet.png";
        default:                  return "troop_swordsman_sheet.png";
    }
}

int Troop::getDefaultPower(TroopType type) {
    switch (type) {
        case TroopType::GUARDIAN:  return 2;
        case TroopType::CAVALRY:  return 3;
        default:                  return 1;
    }
}

int Troop::getFrameSize() {
    return 48; // Ảnh gốc 192x192 chia 4 frame = 48x48 px
}

sf::Color Troop::getTeamColor() const {
    switch (team) {
        case Team::BLUE: return sf::Color(100, 150, 255);    // Xanh dương nhạt
        case Team::RED:  return sf::Color(255, 120, 100);    // Đỏ nhạt
        default:         return sf::Color(255, 255, 255);    // Trung tính (giữ nguyên)
    }
}

// ================================================================
//  Animation: Chuyển frame sprite sheet
//  Tốc độ: ~0.15s / frame ≈ 6-7 FPS animation
// ================================================================
void Troop::updateAnimation(float dt) {
    animTimer += dt;
    if (animTimer >= 0.15f) {
        animTimer = 0.f;
        animFrame = (animFrame + 1) % 4; // 4 frame / hướng
        int fs = getFrameSize();
        sprite.setTextureRect(sf::IntRect(animFrame * fs, animRow * fs, fs, fs));
    }
}

// ================================================================
//  Cập nhật hướng sprite dựa trên vector di chuyển (dx, dy)
//  Hàng 0=↓, 1=←, 2=→, 3=↑
// ================================================================
void Troop::updateDirection(float dx, float dy) {
    // Chọn hướng dựa trên trục nào có delta lớn hơn
    if (std::abs(dx) > std::abs(dy)) {
        animRow = (dx < 0) ? 1 : 2; // Trái hoặc Phải
    } else if (std::abs(dy) > 0.01f) {
        animRow = (dy < 0) ? 3 : 0; // Lên hoặc Xuống
    }
    // Nếu cả hai ≈ 0 thì giữ nguyên hướng cũ
}

// ================================================================
//  Operator Overloading: Xuất thông tin chiến binh ra luồng ostream
// ================================================================
std::ostream& operator<<(std::ostream& os, const Troop& t) {
    std::string typeStr = (t.type == TroopType::SWORDSMAN) ? "KiemSi" :
                          (t.type == TroopType::GUARDIAN)  ? "HoVe" : "KyBinh";
    std::string teamStr = (t.team == Team::BLUE) ? "PheXanh" :
                          (t.team == Team::RED)  ? "PheDo" : "TuDo";
    os << "[Linh: " << typeStr << " | LucChien: " << t.power
       << " | Phe: " << teamStr << " | ViTri: (" << t.x << ", " << t.y << ")]";
    return os;
}