#pragma once
#include "Troop.h"
#include <vector>
#include <deque>

// ================================================================
//  TroopLinkedList – Doubly Linked List quản lý đoàn lính
//  Thành viên 2: Thuật toán cướp lính O(1), ngắt xích bom, Breadcrumb Trail
// ================================================================
class TroopLinkedList {
private:
    Troop* head;
    Troop* tail;
    int count;   // Số lượng node hiện tại
    std::deque<sf::Vector2f> trail; // Lưu lịch sử vết bước chân của Tướng

public:
    void clearTrail() { trail.clear(); }
    TroopLinkedList();
    ~TroopLinkedList();

    // === Thao tác cơ bản ===
    void addTroop(float x, float y, TroopType type, Team team);
    void addTroopNode(Troop* node); // Thêm node có sẵn vào đuôi
    int getCount() const { return count; }
    int getTotalPower() const;      // Tổng sức mạnh toàn đoàn
    Troop* getHead() const { return head; }
    Troop* getTail() const { return tail; }

    // === Thuật toán cốt lõi O(1) ===

    // Cướp lính: Cắt từ node 'from' đến tail, trả về danh sách cắt được
    // Đối thủ mất đoạn từ 'from' trở về sau
    // Trả về head của đoạn bị cắt, nullptr nếu không cắt được
    Troop* stealFrom(Troop* from);

    // Ghép một đoạn DLL vào đuôi danh sách hiện tại
    void appendChain(Troop* chainHead, Troop* chainTail, int chainCount);

    // === Bom nổ: Ngắt xích và văng lính (Phương án B) ===
    // Trả về vector các node lính bị văng ra (sau vị trí nổ), đã bị ngắt khỏi DLL
    // Node trúng bom bị delete, các node sau thành lính tự do
    std::vector<Troop*> bombExplode(Troop* hitNode);

    // === Di chuyển & Render ===
    // Lerp: P = P + (Target - P) * alpha * dt_factor
    void updatePositions(float leaderX, float leaderY, float dt);
    void draw(sf::RenderWindow& window, sf::Uint8 alpha = 255);

    // === Đổi phe toàn bộ đoàn (khi bị cướp) ===
    void setTeamAll(Team newTeam);

    // === Chuyển toàn bộ lính sang vector (dùng cho Phase 2 Clash) ===
    std::vector<Troop*> releaseAll();

    // === Operator Overloading (Kỹ thuật OOP nâng cao theo chuẩn BTL) ===
    // Nạp chồng toán tử += để thêm một node lính vào danh sách
    TroopLinkedList& operator+=(Troop* node);

    // Nạp chồng toán tử += để gộp toàn bộ đoàn quân khác vào danh sách hiện tại
    TroopLinkedList& operator+=(TroopLinkedList& other);

    // Nạp chồng toán tử so sánh tổng lực chiến giữa 2 đoàn quân
    bool operator>(const TroopLinkedList& other) const { return getTotalPower() > other.getTotalPower(); }
    bool operator<(const TroopLinkedList& other) const { return getTotalPower() < other.getTotalPower(); }
    bool operator==(const TroopLinkedList& other) const { return count == other.count && getTotalPower() == other.getTotalPower(); }

    // Nạp chồng toán tử xuất luồng ostream để in thông tin đoàn quân
    friend std::ostream& operator<<(std::ostream& os, const TroopLinkedList& list);
};