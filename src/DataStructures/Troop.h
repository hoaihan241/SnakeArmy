#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

// ================================================================
//  Troop – 1 NODE trong Doubly Linked List
//  Mỗi node = 1 con lính trên bản đồ.
//  Thành viên 2: Cấu trúc dữ liệu & Giải thuật
// ================================================================

// Loại lính — ảnh hưởng đến power và sprite
enum class TroopType {
    SWORDSMAN,   // Kiếm Sĩ   — Power 1, phổ biến
    GUARDIAN,    // Hộ Vệ     — Power 2, trung bình
    CAVALRY      // Kỵ Binh   — Power 3, hiếm
};

// Phe — quyết định màu nhuộm setColor()
enum class Team {
    NEUTRAL,     // Lính tự do trên map (trắng/xám gốc)
    BLUE,        // Phe Xanh (Player 1)
    RED          // Phe Đỏ   (Player 2 / Bot)
};

struct Troop {
    // === Vị trí & Thuộc tính ===
    float x, y;           // Tọa độ hiện tại (dùng cho Lerp)
    int power;            // Sức mạnh chiến đấu (1, 2, 3)
    TroopType type;       // Loại lính
    Team team;            // Phe hiện tại

    // === Con trỏ DLL ===
    Troop* next;
    Troop* prev;

    // === Đồ họa: Sprite Sheet Animation ===
    sf::Sprite sprite;
    int animFrame;        // Frame hiện tại (0–3)
    int animRow;          // Hàng hướng (0=↓, 1=←, 2=→, 3=↑)
    float animTimer;      // Đếm thời gian chuyển frame

    // === Constructor & Methods ===
    Troop(float startX, float startY, TroopType troopType, Team startTeam = Team::NEUTRAL);

    // Lấy tên file sprite sheet dựa trên TroopType
    static std::string getSheetName(TroopType type);

    // Lấy power mặc định theo TroopType
    static int getDefaultPower(TroopType type);

    // Lấy kích thước 1 frame (pixel) dựa trên TroopType
    static int getFrameSize();

    // Lấy màu nhuộm theo phe
    sf::Color getTeamColor() const;

    // Cập nhật animation frame
    void updateAnimation(float dt);

    // Cập nhật hướng sprite dựa trên hướng di chuyển
    void updateDirection(float dx, float dy);

    // === Operator Overloading (Kỹ thuật OOP nâng cao) ===
    bool operator==(const Troop& other) const { return type == other.type && power == other.power; }
    friend std::ostream& operator<<(std::ostream& os, const Troop& t);
};