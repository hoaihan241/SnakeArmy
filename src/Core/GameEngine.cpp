#include "GameEngine.h"
#include "../Items/SpeedItem.h"
#include "../Items/ShieldItem.h"

GameEngine::GameEngine() : window(sf::VideoMode(800, 600), "Snake Army 2.0 - Full OOP"), currentState(GameState::MENU) {
    window.setFramerateLimit(60);
    player1 = new HumanPlayer(400.0f, 300.0f);
    bot1 = new BotPlayer(100.0f, 100.0f);
    obstacles.push_back(new Obstacle(200, 200, 50, 100));
    items.push_back(new SpeedItem(500, 100));
    items.push_back(new ShieldItem(600, 400));
}

GameEngine::~GameEngine() {
    delete player1; delete bot1;
    for(auto o : obstacles) delete o;
    for(auto i : items) delete i;
}

void GameEngine::run() {
    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processInput(); update(dt); render();
    }
}

void GameEngine::processInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) window.close();
        if (currentState == GameState::MENU && event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter) currentState = GameState::PLAYING;
        }
    }
}

void GameEngine::update(float dt) {
    if (currentState == GameState::PLAYING) {
        player1->update(dt);
        bot1->update(dt);
        checkCollisions();
    }
}

void GameEngine::checkCollisions() {
    // Kiem tra da hinh Item (Polymorphism Demo)
}

void GameEngine::render() {
    window.clear(sf::Color(34, 139, 34));
    if (currentState == GameState::MENU) {
        sf::RectangleShape btn(sf::Vector2f(300, 50)); btn.setPosition(250, 275);
        btn.setFillColor(sf::Color::White); window.draw(btn);
    } else if (currentState == GameState::PLAYING) {
        for(auto o : obstacles) o->draw(window);
        for(auto i : items) i->draw(window);
        player1->draw(window);
        bot1->draw(window);
    }
    window.display();
}