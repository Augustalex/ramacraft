#pragma once

#include <SDL.h>
#include <memory>
#include "Shader.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Biot.hpp"
#include "Projectile.hpp"
#include "TextureAtlas.hpp"
#include "Audio.hpp"
#include "UI.hpp"
#include "Crafting.hpp"

class Game {
public:
    static Game& instance();

    bool init(int width = 1280, int height = 720);
    void cleanup();

    void run();
    void tick(); // Single frame step for Emscripten / main loop

    void handleEvent(const SDL_Event& event);
    void update(float dt);
    void render();

    bool isRunning() const { return m_running; }

private:
    Game() = default;

    SDL_Window* m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    int m_width = 1280;
    int m_height = 720;
    bool m_running = true;

    Uint64 m_lastTime = 0;

    // Engine Components
    Shader m_voxelShader;
    Shader m_entityShader;
    Shader m_uiShader;

    World m_world;
    Player m_player;

    bool m_relativeMouse = true;
    void setRelativeMouse(bool enabled);
};
