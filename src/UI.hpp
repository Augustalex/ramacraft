#pragma once

#include <string>
#include <vector>
#include "Math3D.hpp"
#include "Block.hpp"

#include "GLCommon.hpp"

class Shader;
class Player;
class World;

enum class UIMenuState {
    HUDOnly,
    InventoryScreen,
    CraftingScreen,
    MultiplayerScreen
};

class UI {
public:
    static UI& instance();

    void init();
    void cleanup();

    void update(float dt);
    void render(const Shader& uiShader, int screenWidth, int screenHeight, const Player& player, const World& world);

    void handleMouseClick(int mouseX, int mouseY, int button, Player& player);
    void toggleInventory();
    void toggleCrafting();
    void toggleMultiplayer();
    void closeMenus();

    UIMenuState getMenuState() const { return m_menuState; }
    bool isMenuOpen() const { return m_menuState != UIMenuState::HUDOnly; }

private:
    UI() = default;

    UIMenuState m_menuState = UIMenuState::HUDOnly;
    int m_selectedRecipe = 0;
    int m_heldSlotIndex = -1;
    float m_radarAngle = 0.0f;
    std::string m_directConnectIp = "127.0.0.1";

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    struct UIVertex {
        Vec2 pos;
        Vec2 uv;
        Vec4 color;
    };

    std::vector<UIVertex> m_mesh;

    void drawQuad(float x, float y, float w, float h, const Vec4& color, float u0 = 0.0f, float v0 = 0.0f, float u1 = 0.0f, float v1 = 0.0f);
    void drawText(const std::string& text, float x, float y, float scale, const Vec4& color);
    void drawItemIcon(ItemType item, float x, float y, float size);

    void renderHUD(int screenW, int screenH, const Player& player, const World& world);
    void renderInventory(int screenW, int screenH, Player& player);
    void renderCrafting(int screenW, int screenH, Player& player);
    void renderMultiplayer(int screenW, int screenH);
};
