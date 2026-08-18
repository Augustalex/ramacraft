#include "Game.hpp"
#include "Network.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "   RAMACRAFT: VOXEL SURVIVAL IN RAMA    " << std::endl;
    std::cout << "  Minecraft meets Rendezvous with Rama  " << std::endl;
    std::cout << "========================================" << std::endl;

    if (!Game::instance().init(1280, 720)) {
        std::cerr << "Failed to initialize RamaCraft!" << std::endl;
        return 1;
    }

    // Process CLI arguments for direct multiplayer launch
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--host" || arg == "-h") {
            int port = 7777;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = std::stoi(argv[++i]);
            }
            NetworkManager::instance().startHost(port, "Rama Expedition");
        } else if (arg == "--join" || arg == "-j") {
            std::string host = "127.0.0.1";
            int port = 7777;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                host = argv[++i];
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                port = std::stoi(argv[++i]);
            }
            NetworkManager::instance().connectTo(host, port);
        }
    }

    Game::instance().run();
    Game::instance().cleanup();

    return 0;
}
