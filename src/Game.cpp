#include "Game.hpp"
#include "ItemEntity.hpp"
#include "Network.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
static void emscriptenMainLoop() {
    Game::instance().tick();
}
#endif

Game& Game::instance() {
    static Game s_game;
    return s_game;
}

bool Game::init(int width, int height) {
    m_width = width;
    m_height = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

#ifdef __EMSCRIPTEN__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow(
        "RamaCraft - Voxel Survival inside Arthur C. Clarke's Rama",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_width, m_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!m_window) {
        std::cerr << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
        return false;
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "Failed to create OpenGL Context: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_GL_SetSwapInterval(1); // Enable VSync

    int drawW, drawH;
    SDL_GL_GetDrawableSize(m_window, &drawW, &drawH);
    glViewport(0, 0, drawW, drawH);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Initialize Global Subsystems
    BlockRegistry::init();
    AudioSystem::instance().init();
    TextureAtlas::instance().init();
    ProjectileManager::instance().init();
    BiotManager::instance().init();
    ItemEntityManager::instance().init();
    NetworkManager::instance().init();
    UI::instance().init();

    // Compile Shaders
    if (!m_voxelShader.loadFromSource(Shaders::VOXEL_VERT, Shaders::VOXEL_FRAG)) {
        std::cerr << "Failed to compile Voxel Shader!" << std::endl;
        return false;
    }
    if (!m_entityShader.loadFromSource(Shaders::ENTITY_VERT, Shaders::ENTITY_FRAG)) {
        std::cerr << "Failed to compile Entity Shader!" << std::endl;
        return false;
    }
    if (!m_uiShader.loadFromSource(Shaders::UI_VERT, Shaders::UI_FRAG)) {
        std::cerr << "Failed to compile UI Shader!" << std::endl;
        return false;
    }

    // Initialize World and Player
    m_world.init();
    m_player.init({0.0f, 17.5f, 4.0f});

    setRelativeMouse(true);
    m_lastTime = SDL_GetPerformanceCounter();

    std::cout << "RamaCraft initialized successfully inside Rama cylinder!" << std::endl;
    return true;
}

void Game::cleanup() {
    NetworkManager::instance().cleanup();
    UI::instance().cleanup();
    ItemEntityManager::instance().cleanup();
    BiotManager::instance().cleanup();
    ProjectileManager::instance().cleanup();
    TextureAtlas::instance().cleanup();
    AudioSystem::instance().cleanup();

    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    SDL_Quit();
}

void Game::setRelativeMouse(bool enabled) {
    m_relativeMouse = enabled;
    SDL_SetRelativeMouseMode(enabled ? SDL_TRUE : SDL_FALSE);
}

void Game::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_QUIT) {
        m_running = false;
    } else if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            int drawW, drawH;
            SDL_GL_GetDrawableSize(m_window, &drawW, &drawH);
            glViewport(0, 0, drawW, drawH);
            m_width = event.window.data1;
            m_height = event.window.data2;
        }
    } else if (event.type == SDL_KEYDOWN) {
        SDL_Keycode key = event.key.keysym.sym;
        if (key == SDLK_ESCAPE) {
            if (UI::instance().isMenuOpen()) {
                UI::instance().closeMenus();
                setRelativeMouse(true);
            } else {
                setRelativeMouse(!m_relativeMouse);
            }
        } else if (key == SDLK_e) {
            UI::instance().toggleInventory();
            setRelativeMouse(!UI::instance().isMenuOpen());
        } else if (key == SDLK_TAB) {
            m_player.toggleJetpackMode();
        } else if (key == SDLK_c) {
            UI::instance().toggleCrafting();
            setRelativeMouse(!UI::instance().isMenuOpen());
        } else if (key == SDLK_m) {
            UI::instance().toggleMultiplayer();
            setRelativeMouse(!UI::instance().isMenuOpen());
        } else if (key == SDLK_f) {
            m_player.toggleFlashlight();
        } else if (key == SDLK_k) {
            m_world.toggleCurvature();
        } else if (key >= SDLK_1 && key <= SDLK_9) {
            m_player.getInventory().setSelectedHotbarIndex(key - SDLK_1);
        }
    } else if (event.type == SDL_MOUSEWHEEL) {
        int idx = m_player.getInventory().getSelectedHotbarIndex();
        if (event.wheel.y > 0) idx = (idx - 1 + 9) % 9;
        else if (event.wheel.y < 0) idx = (idx + 1) % 9;
        m_player.getInventory().setSelectedHotbarIndex(idx);
    } else if (event.type == SDL_MOUSEMOTION) {
        if (m_relativeMouse && !UI::instance().isMenuOpen()) {
            float sensitivity = 0.15f;
            m_player.addRotation((float)event.motion.yrel * sensitivity, (float)event.motion.xrel * sensitivity);
        }
    } else if (event.type == SDL_MOUSEBUTTONDOWN) {
        if (UI::instance().isMenuOpen()) {
            UI::instance().handleMouseClick(event.button.x, event.button.y, event.button.button, m_player);
        } else {
            if (!m_relativeMouse) {
                setRelativeMouse(true);
            } else {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    const ItemStack& sel = m_player.getInventory().getSelectedItem();
                    if (sel.type == ItemType::RayGun) {
                        m_player.fireRayGun(m_world);
                    } else {
                        m_player.startMining();
                    }
                } else if (event.button.button == SDL_BUTTON_RIGHT) {
                    m_player.placeSelectedBlock(m_world);
                }
            }
        }
    } else if (event.type == SDL_MOUSEBUTTONUP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            m_player.stopMining();
        }
    }
}

void Game::update(float dt) {
    dt = std::min(dt, 0.05f);

    m_player.update(dt, m_world);
    m_world.update(dt, m_player.getPosition());
    ItemEntityManager::instance().update(dt, m_world, m_player);
    BiotManager::instance().update(dt, m_world, m_player);
    ProjectileManager::instance().update(dt, m_world, BiotManager::instance());
    NetworkManager::instance().update(dt, m_world, m_player);
    UI::instance().update(dt);
}

void Game::render() {
    int drawW, drawH;
    SDL_GL_GetDrawableSize(m_window, &drawW, &drawH);
    glViewport(0, 0, drawW, drawH);

    Vec3 fogColor = m_world.getFogColor();
    glClearColor(fogColor.x, fogColor.y, fogColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (float)drawW / (float)drawH;
    // Colossal 2000m far plane for cylinder
    Mat4 proj = Mat4::perspective(75.0f * DEG2RAD, aspect, 0.1f, 2000.0f);
    Mat4 view = m_player.getViewMatrix();
    Vec3 playerEye3D = m_player.getEyePosition3D();
    Vec3 playerPos3D = m_player.getWorldPos3D();

    // Bind texture atlas to unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureAtlas::instance().getTextureID());

    // 1. Setup Flashlight & Lighting Uniforms on Shaders
    auto setLighting = [&](const Shader& s) {
        s.use();
        s.setInt("uAtlas", 0);
        s.setVec3("uFlashlightPos", playerEye3D);
        s.setVec3("uFlashlightDir", m_player.getForward3D());
        s.setVec3("uFlashlightColor", {1.0f, 0.95f, 0.88f});
        s.setFloat("uFlashlightEnable", m_player.isFlashlightOn() ? 1.0f : 0.0f);
        s.setVec3("uAmbientColor", m_world.getAmbientColor());
        s.setVec3("uSunColor", m_world.getSunColor());
        s.setFloat("uSunIntensity", m_world.getSunIntensity());
        s.setVec3("uFogColor", m_world.getFogColor());
        s.setFloat("uFogDensity", m_world.getFogDensity());
    };

    setLighting(m_voxelShader);
    setLighting(m_entityShader);

    // 2. Render Opaque Voxel World Chunks (Full 360-degree Cylinder Ring)
    m_world.render(m_voxelShader, playerPos3D, view, proj);

    // 3. Render Rama Central Spindle & 6 Linear Sun Rods
    m_world.renderRamaSpindleAndSuns(m_entityShader, playerPos3D, view, proj);

    // 4. Render Floating 3D Pickups
    ItemEntityManager::instance().render(m_entityShader, playerPos3D, view, proj);

    // 5. Render Biots (Robot Beetles)
    BiotManager::instance().render(m_entityShader, playerPos3D, view, proj);

    // 6. Render Remote Multiplayer Players (Astronauts)
    NetworkManager::instance().renderRemotePlayers(m_entityShader, playerPos3D, view, proj);

    // 7. Render Projectiles & Sparks
    ProjectileManager::instance().render(m_entityShader, playerPos3D, view, proj);

    // 8. Render Transparent Chunks (Cylindrical Sea water, glass)
    m_world.renderTransparent(m_voxelShader, playerPos3D, view, proj);

    // 9. Render First-Person 3D Viewmodel & Weapon Sway
    if (!UI::instance().isMenuOpen()) {
        Mat4 vmProj = Mat4::perspective(65.0f * DEG2RAD, aspect, 0.05f, 10.0f);
        m_player.renderViewmodel(m_entityShader, vmProj);
    }

    // 10. Render 2D UI / HUD
    UI::instance().render(m_uiShader, m_width, m_height, m_player, m_world);

    SDL_GL_SwapWindow(m_window);
}

void Game::tick() {
    Uint64 now = SDL_GetPerformanceCounter();
    float dt = (float)(now - m_lastTime) / (float)SDL_GetPerformanceFrequency();
    m_lastTime = now;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }

    update(dt);
    render();
}

void Game::run() {
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscriptenMainLoop, 0, 1);
#else
    while (m_running) {
        tick();
    }
#endif
}
