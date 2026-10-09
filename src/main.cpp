#include "Core/GameEngine.h"
#include <iostream>
int main() {
    try {
        GameEngine game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
    }
    return 0;
}