#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../Entities/HumanPlayer.h"
#include "../Entities/BotPlayer.h"
#include "../Entities/Obstacle.h"
#include "../Items/Item.h"

enum class GameState { MENU, PLAYING, SETTINGS, GAME_OVER };

class GameEngine {
private:
    sf::RenderWindow window;
    GameState currentState;
    HumanPlayer* player1;
    BotPlayer* bot1;
    std::vector<Obstacle*> obstacles;
    std::vector<Item*> items;

    void processInput();
    void update(float dt);
    void render();
    void checkCollisions();

public:
    GameEngine();
    ~GameEngine();
    void run();
};