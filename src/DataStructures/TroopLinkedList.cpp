#include "TroopLinkedList.h"
TroopLinkedList::TroopLinkedList() : head(nullptr), tail(nullptr) {}
TroopLinkedList::~TroopLinkedList() {
    Troop* current = head;
    while (current != nullptr) { Troop* toDelete = current; current = current->next; delete toDelete; }
}
void TroopLinkedList::addTroop(float x, float y, int power) {
    Troop* newTroop = new Troop(x, y, power);
    if (!head) { head = tail = newTroop; } else { tail->next = newTroop; newTroop->prev = tail; tail = newTroop; }
}
void TroopLinkedList::updatePositions(float leaderX, float leaderY) {
    float tx = leaderX, ty = leaderY;
    Troop* curr = head;
    while (curr) {
        curr->x += (tx - curr->x) * 0.1f; curr->y += (ty - curr->y) * 0.1f;
        curr->shape.setPosition(curr->x, curr->y);
        tx = curr->x; ty = curr->y; curr = curr->next;
    }
}
void TroopLinkedList::draw(sf::RenderWindow& window) {
    Troop* curr = head;
    while (curr) { window.draw(curr->shape); curr = curr->next; }
}