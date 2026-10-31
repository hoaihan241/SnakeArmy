#pragma once
#include "GameObject.h"
#include <string>

// ================================================================
//  Obstacle – Chướng ngại vật tĩnh (đá, tường, tàn tích)
//  Thành viên 3: Địa hình & Vật cản
// ================================================================
class Obstacle : public GameObject {
protected:
    sf::Sprite sprite;
    float width, height;

public:
    Obstacle(float startX, float startY, float w, float h, const std::string& textureName = "obstacle.png");
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    sf::FloatRect getBounds() const override;
    float getWidth() const { return width; }
    float getHeight() const { return height; }

    // Hitbox va chạm thực tế (dùng để cản Tướng không đi xuyên qua)
    virtual sf::FloatRect getCollisionBounds() const;
    virtual bool isPassable() const { return false; }
    virtual bool isBush() const { return false; }
};

// ================================================================
//  Tree – Cây cối (Kế thừa từ Obstacle)
//  Hitbox va chạm nằm ở gốc thân cây, cho phép Tướng đi qua sau tán lá
// ================================================================
class Tree : public Obstacle {
private:
    int variant;

public:
    Tree(float startX, float startY, int var = 1, float size = 96.f);
    sf::FloatRect getCollisionBounds() const override;
};

// ================================================================
//  RuinsPillar – Cột đá La Mã gãy đổ cháy xém (ruins_pillar.png)
// ================================================================
class RuinsPillar : public Obstacle {
public:
    RuinsPillar(float startX, float startY, float size = 110.f);
    sf::FloatRect getCollisionBounds() const override;
};

// ================================================================
//  RuinsWall – Đoạn tường thành nứt toác có vết móng vuốt rồng (ruins_wall.png)
// ================================================================
class RuinsWall : public Obstacle {
public:
    RuinsWall(float startX, float startY, float w = 150.f, float h = 115.f);
    sf::FloatRect getCollisionBounds() const override;
};

// ================================================================
//  Bush – Bụi rậm ẩn nấp / Ngụy trang (bush.png)
//  Cho phép đi xuyên qua; khi ở bên trong sẽ được ngụy trang (Stealth)
// ================================================================
class Bush : public Obstacle {
public:
    Bush(float startX, float startY, float size = 115.f);
    sf::FloatRect getCollisionBounds() const override;
    bool isPassable() const override { return true; }
    bool isBush() const override { return true; }
    bool isInside(float px, float py) const;
};