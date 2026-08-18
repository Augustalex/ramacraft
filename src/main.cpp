#include "Game.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "   RAMACRAFT: VOXEL SURVIVAL IN RAMA    " << std::endl;
    std::cout << "  Minecraft meets Rendezvous with Rama  " << std::endl;
    std::cout << "========================================" << std::endl;

    if (!Game::instance().init(1280, 720)) {
        std::cerr << "Failed to initialize RamaCraft!" << std::endl;
        return 1;
    }

    Game::instance().run();
    Game::instance().cleanup();

    return 0;
}
