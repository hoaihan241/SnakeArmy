#pragma once
#include "Troop.h"
class TroopLinkedList {
private:
    Troop* head;
    Troop* tail;
public:
    TroopLinkedList();
    ~TroopLinkedList();
    void addTroop(float x, float y, int power);
    void updatePositions(float leaderX, float leaderY);
    void draw(sf::RenderWindow& window);
};