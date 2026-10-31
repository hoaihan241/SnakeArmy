#include "TroopLinkedList.h"
#include <cmath>

// ================================================================
//  Constructor / Destructor
// ================================================================
TroopLinkedList::TroopLinkedList() : head(nullptr), tail(nullptr), count(0) {}

TroopLinkedList::~TroopLinkedList() {
    Troop* current = head;
    while (current != nullptr) {
        Troop* toDelete = current;
        current = current->next;
        delete toDelete;
    }
}

// ================================================================
//  addTroop – Tạo node mới và thêm vào đuôi DLL
// ================================================================
void TroopLinkedList::addTroop(float x, float y, TroopType type, Team team) {
    Troop* newTroop = new Troop(x, y, type, team);
    addTroopNode(newTroop);
}

void TroopLinkedList::addTroopNode(Troop* node) {
    node->next = nullptr;
    node->prev = nullptr;
    if (!head) {
        head = tail = node;
    } else {
        tail->next = node;
        node->prev = tail;
        tail = node;
    }
    count++;
}

// ================================================================
//  getTotalPower – Tổng sức mạnh chiến đấu toàn đoàn
// ================================================================
int TroopLinkedList::getTotalPower() const {
    int total = 0;
    Troop* curr = head;
    while (curr) {
        total += curr->power;
        curr = curr->next;
    }
    return total;
}

// ================================================================
//  THUẬT TOÁN CƯỚP LÍNH O(1)
//  
//  Khi Tướng A húc vào node 'from' thuộc đoàn lính của Tướng B:
//  - Cắt: Node trước 'from' trở thành tail mới của B
//  - Toàn bộ từ 'from' → tail cũ bị tách ra
//  - Trả về 'from' (head của đoạn bị cắt)
//  
//  Độ phức tạp: O(1) — chỉ đảo vài con trỏ!
// ================================================================
Troop* TroopLinkedList::stealFrom(Troop* from) {
    if (!from) return nullptr;

    // Đếm số node bị cướp
    int stolenCount = 0;
    Troop* temp = from;
    while (temp) {
        stolenCount++;
        temp = temp->next;
    }

    // Trường hợp 'from' là head → cướp toàn bộ
    if (from == head) {
        head = nullptr;
        tail = nullptr;
        count = 0;
    } else {
        // Cắt liên kết O(1)
        Troop* beforeFrom = from->prev;
        beforeFrom->next = nullptr;  // Node trước trở thành tail mới
        from->prev = nullptr;        // Đoạn bị cắt mất liên kết ngược
        tail = beforeFrom;           // Cập nhật tail
        count -= stolenCount;
    }

    return from; // Trả về head của đoạn bị cắt
}

// ================================================================
//  appendChain – Ghép đoạn DLL vào đuôi (sau khi cướp xong)
//  Độ phức tạp: O(1) — chỉ nối tail hiện tại với chainHead
// ================================================================
void TroopLinkedList::appendChain(Troop* chainHead, Troop* chainTail, int chainCount) {
    if (!chainHead) return;

    if (!head) {
        head = chainHead;
        tail = chainTail;
    } else {
        tail->next = chainHead;
        chainHead->prev = tail;
        tail = chainTail;
    }
    count += chainCount;
}

// ================================================================
//  BOM NỔ — PHƯƠNG ÁN B: Ngắt xích & Văng lính
//  
//  hitNode: node bị bom trúng trực tiếp → bị DIỆT (delete)
//  Các node SAU hitNode: bị đứt xích, văng ra đất thành lính tự do
//  Các node TRƯỚC hitNode: vẫn thuộc đoàn, nối lại bình thường
//  
//  Trả về: vector<Troop*> chứa các lính tự do bị văng ra
// ================================================================
std::vector<Troop*> TroopLinkedList::bombExplode(Troop* hitNode) {
    std::vector<Troop*> freedTroops;
    if (!hitNode) return freedTroops;

    // Thu thập các node SAU hitNode (sẽ thành lính tự do)
    Troop* afterHit = hitNode->next;
    while (afterHit) {
        Troop* freed = afterHit;
        afterHit = afterHit->next;

        // Ngắt liên kết hoàn toàn
        freed->prev = nullptr;
        freed->next = nullptr;
        freed->team = Team::NEUTRAL;
        freed->sprite.setColor(freed->getTeamColor());
        freedTroops.push_back(freed);
        count--;
    }

    // Xử lý phần TRƯỚC hitNode
    if (hitNode == head) {
        // hitNode là head → sau khi xóa, list rỗng
        head = nullptr;
        tail = nullptr;
    } else {
        // Nối lại: node trước hitNode trở thành tail
        Troop* beforeHit = hitNode->prev;
        beforeHit->next = nullptr;
        tail = beforeHit;
    }

    // Xóa node bị trúng bom
    count--;
    delete hitNode;

    return freedTroops;
}

// ================================================================
//  BREADCRUMB TRAIL ANIMATION — Lính đi nối đuôi chuẩn Snake
//  - Khi Tướng di chuyển: Vết bước chân được lưu vào trail.
//  - Khi Tướng dừng lại: trail đóng băng, lính vẫn đứng yên ở vị trí
//    cách Tướng một khoảng cố định (index * SPACING) phía sau.
//  - Hoàn toàn KHÔNG bị co cụm dồn cục vào người Tướng!
// ================================================================
void TroopLinkedList::updatePositions(float leaderX, float leaderY, float dt) {
    if (!head) {
        trail.clear();
        return;
    }

    sf::Vector2f leaderPos(leaderX, leaderY);

    // 1. Ghi nhận vị trí mới của Tướng vào trail
    if (trail.empty()) {
        trail.push_front(leaderPos);
    } else {
        float dx = leaderPos.x - trail.front().x;
        float dy = leaderPos.y - trail.front().y;
        float distFromFront = std::sqrt(dx * dx + dy * dy);

        // Khi Tướng di chuyển một khoảng >= 4px thì ghi nhận điểm mới vào trail
        if (distFromFront >= 4.0f) {
            trail.push_front(leaderPos);
        }
    }

    // 2. Định vị từng con lính trên trail
    const float SPACING = 34.0f; // Khoảng cách giữa các lính (lính 48px, cách nhau 34px nối đuôi cực đẹp)

    Troop* curr = head;
    int index = 1;

    while (curr) {
        float targetDist = (float)index * SPACING;

        // Tính khoảng cách từ Tướng đến điểm đầu trail
        float dxLead = leaderPos.x - trail.front().x;
        float dyLead = leaderPos.y - trail.front().y;
        float distToFront = std::sqrt(dxLead * dxLead + dyLead * dyLead);

        float distNeeded = targetDist - distToFront;
        if (distNeeded < 0.f) distNeeded = 0.f;

        sf::Vector2f targetPt = trail.front();
        sf::Vector2f faceDir(0.f, 1.f); // Hướng nhìn mặc định

        if (trail.size() <= 1) {
            // Chưa có nhiều điểm trail -> giả định lùi về phía sau Tướng
            targetPt = trail.front() - sf::Vector2f(0.f, distNeeded);
        } else {
            // Duyệt qua các đoạn của trail để tìm điểm ở khoảng cách distNeeded
            float accumulatedDist = 0.f;
            bool found = false;

            for (size_t i = 0; i + 1 < trail.size(); ++i) {
                float segDx = trail[i + 1].x - trail[i].x;
                float segDy = trail[i + 1].y - trail[i].y;
                float segLen = std::sqrt(segDx * segDx + segDy * segDy);

                if (segLen <= 0.001f) continue;

                if (accumulatedDist + segLen >= distNeeded) {
                    float t = (distNeeded - accumulatedDist) / segLen;
                    targetPt = trail[i] + sf::Vector2f(segDx * t, segDy * t);
                    faceDir = sf::Vector2f(-segDx, -segDy); // Hướng nhìn về phía trước trail (về phía Tướng)
                    found = true;
                    break;
                }
                accumulatedDist += segLen;
            }

            if (!found) {
                // Đi hết trail mà chưa đủ khoảng cách -> phóng tiếp theo hướng đoạn cuối cùng
                size_t lastIdx = trail.size() - 1;
                float endDx = trail[lastIdx].x - trail[lastIdx - 1].x;
                float endDy = trail[lastIdx].y - trail[lastIdx - 1].y;
                float endLen = std::sqrt(endDx * endDx + endDy * endDy);
                if (endLen > 0.001f) {
                    endDx /= endLen;
                    endDy /= endLen;
                } else {
                    endDx = 0.f; endDy = 1.f;
                }
                float extra = distNeeded - accumulatedDist;
                targetPt = trail[lastIdx] + sf::Vector2f(endDx * extra, endDy * extra);
                faceDir = sf::Vector2f(-endDx, -endDy);
            }
        }

        // Lerp mượt về targetPt
        float oldX = curr->x;
        float oldY = curr->y;
        float alpha = 14.0f * dt;
        if (alpha > 1.f) alpha = 1.f;

        curr->x += (targetPt.x - curr->x) * alpha;
        curr->y += (targetPt.y - curr->y) * alpha;
        curr->sprite.setPosition(curr->x, curr->y);

        // Hướng quay mặt và animation
        float moveDx = curr->x - oldX;
        float moveDy = curr->y - oldY;
        float moveLen = std::sqrt(moveDx * moveDx + moveDy * moveDy);

        if (moveLen > 0.15f) {
            // Đang di chuyển: quay mặt theo hướng đi và chạy animation
            curr->updateDirection(moveDx, moveDy);
            curr->updateAnimation(dt);
        } else {
            // Đứng yên: giữ nguyên hướng nhìn về phía Tướng
            curr->updateDirection(faceDir.x, faceDir.y);
        }

        curr = curr->next;
        index++;
    }

    // 3. Tỉa bớt trail không dùng đến để tiết kiệm bộ nhớ
    float maxTrailDist = (float)(count + 2) * SPACING + 80.f;
    float currentTrailDist = 0.f;
    size_t keepCount = trail.size();

    for (size_t i = 0; i + 1 < trail.size(); ++i) {
        float segDx = trail[i + 1].x - trail[i].x;
        float segDy = trail[i + 1].y - trail[i].y;
        currentTrailDist += std::sqrt(segDx * segDx + segDy * segDy);
        if (currentTrailDist > maxTrailDist) {
            keepCount = i + 2;
            break;
        }
    }

    while (trail.size() > keepCount && trail.size() > 4) {
        trail.pop_back();
    }
}

// ================================================================
//  Draw – Vẽ toàn bộ lính trong DLL
// ================================================================
void TroopLinkedList::draw(sf::RenderWindow& window, sf::Uint8 alpha) {
    Troop* curr = head;
    while (curr) {
        sf::Color c = curr->getTeamColor();
        c.a = alpha;
        curr->sprite.setColor(c);
        window.draw(curr->sprite);
        curr = curr->next;
    }
}

// ================================================================
//  setTeamAll – Đổi phe + nhuộm màu toàn bộ đoàn
//  Gọi sau khi cướp lính thành công
// ================================================================
void TroopLinkedList::setTeamAll(Team newTeam) {
    Troop* curr = head;
    while (curr) {
        curr->team = newTeam;
        curr->sprite.setColor(curr->getTeamColor());
        curr = curr->next;
    }
}

// ================================================================
//  releaseAll – Tách toàn bộ node ra khỏi DLL, trả về vector
//  Dùng cho Phase 2 Clash: Đổ lính ra chiến trường
// ================================================================
std::vector<Troop*> TroopLinkedList::releaseAll() {
    std::vector<Troop*> all;
    Troop* curr = head;
    while (curr) {
        Troop* node = curr;
        curr = curr->next;
        node->prev = nullptr;
        node->next = nullptr;
        all.push_back(node);
    }
    head = nullptr;
    tail = nullptr;
    count = 0;
    trail.clear();
    return all;
}

// ================================================================
//  Operator Overloading: += (Thêm 1 node lính vào đuôi)
// ================================================================
TroopLinkedList& TroopLinkedList::operator+=(Troop* node) {
    if (node) {
        addTroopNode(node);
    }
    return *this;
}

// ================================================================
//  Operator Overloading: += (Gộp toàn bộ đoàn quân khác vào danh sách)
// ================================================================
TroopLinkedList& TroopLinkedList::operator+=(TroopLinkedList& other) {
    if (other.head) {
        appendChain(other.head, other.tail, other.count);
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
        other.trail.clear();
    }
    return *this;
}

// ================================================================
//  Operator Overloading: << (Xuất thông tin toàn đoàn quân)
// ================================================================
std::ostream& operator<<(std::ostream& os, const TroopLinkedList& list) {
    os << "TroopLinkedList [SoLuong: " << list.count
       << " | TongLucChien: " << list.getTotalPower() << "]\n";
    Troop* curr = list.head;
    int idx = 1;
    while (curr) {
        os << "  " << idx++ << ". " << *curr << "\n";
        curr = curr->next;
    }
    return os;
}