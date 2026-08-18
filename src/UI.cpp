#include "UI.hpp"
#include "Shader.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "Crafting.hpp"
#include "TextureAtlas.hpp"
#include "Biot.hpp"
#include "Network.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

UI& UI::instance() {
    static UI s_ui;
    return s_ui;
}

void UI::init() {
    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    }
}

void UI::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
}

void UI::update(float dt) {
    m_radarAngle += dt * 140.0f;
    if (m_radarAngle >= 360.0f) m_radarAngle -= 360.0f;
}

void UI::toggleInventory() {
    if (m_menuState == UIMenuState::InventoryScreen) {
        m_menuState = UIMenuState::HUDOnly;
    } else {
        m_menuState = UIMenuState::InventoryScreen;
        m_heldSlotIndex = -1;
    }
}

void UI::toggleCrafting() {
    if (m_menuState == UIMenuState::CraftingScreen) {
        m_menuState = UIMenuState::HUDOnly;
    } else {
        m_menuState = UIMenuState::CraftingScreen;
    }
}

void UI::closeMenus() {
    m_menuState = UIMenuState::HUDOnly;
    m_heldSlotIndex = -1;
}

void UI::drawQuad(float x, float y, float w, float h, const Vec4& color, float u0, float v0, float u1, float v1) {
    m_mesh.push_back({{x, y}, {u0, v0}, color});
    m_mesh.push_back({{x + w, y}, {u1, v0}, color});
    m_mesh.push_back({{x + w, y + h}, {u1, v1}, color});

    m_mesh.push_back({{x, y}, {u0, v0}, color});
    m_mesh.push_back({{x + w, y + h}, {u1, v1}, color});
    m_mesh.push_back({{x, y + h}, {u0, v1}, color});
}

void UI::drawText(const std::string& text, float x, float y, float scale, const Vec4& color) {
    float curX = x;
    float charW = 8.0f * scale;
    float charH = 16.0f * scale;

    float invAtlas = 1.0f / (float)TextureAtlas::ATLAS_SIZE;

    for (char c : text) {
        if (c < 32 || c > 126) {
            curX += charW;
            continue;
        }

        int idx = (int)c - 32;
        int gx = (idx % 64) * 8;
        int gy = 448 + (idx / 64) * 16;

        float u0 = (float)gx * invAtlas;
        float v0 = (float)gy * invAtlas;
        float u1 = (float)(gx + 8) * invAtlas;
        float v1 = (float)(gy + 16) * invAtlas;

        drawQuad(curX, y, charW, charH, color, u0, v0, u1, v1);
        curX += charW * 0.85f;
    }
}

void UI::drawItemIcon(ItemType item, float x, float y, float size) {
    if (item == ItemType::None) return;
    const ItemInfo& info = BlockRegistry::getItem(item);

    float u0, v0, u1, v1;
    TextureAtlas::instance().getTileUVs(info.iconX, info.iconY, u0, v0, u1, v1);
    drawQuad(x, y, size, size, Vec4(1, 1, 1, 1), u0, v0, u1, v1);
}

void UI::renderHUD(int screenW, int screenH, const Player& player, const World& world) {
    // 0. Underwater Screen Overlay
    if (player.isUnderwater()) {
        drawQuad(0, 0, (float)screenW, (float)screenH, Vec4(0.04f, 0.25f, 0.55f, 0.42f));
    }

    // 1. Crosshair
    float cx = (float)screenW * 0.5f;
    float cy = (float)screenH * 0.5f;
    Vec4 chColor(1.0f, 1.0f, 1.0f, 0.8f);
    drawQuad(cx - 1.0f, cy - 8.0f, 2.0f, 16.0f, chColor);
    drawQuad(cx - 8.0f, cy - 1.0f, 16.0f, 2.0f, chColor);

    // 2. Health, Fuel, Oxygen Status Bars (Top Left)
    float barX = 20.0f;
    float barY = 20.0f;
    float barW = 200.0f;
    float barH = 14.0f;

    // Health Bar
    drawQuad(barX - 2, barY - 2, barW + 4, barH + 4, Vec4(0.1f, 0.1f, 0.15f, 0.8f));
    float hpPct = std::clamp(player.getHealth() / 100.0f, 0.0f, 1.0f);
    drawQuad(barX, barY, barW * hpPct, barH, Vec4(0.9f, 0.2f, 0.2f, 1.0f));
    drawText("SUIT INTEGRITY: " + std::to_string((int)player.getHealth()) + "%", barX + 6, barY + 1, 0.8f, Vec4(1, 1, 1, 1));

    // Jetpack Status / Mode Indicator (TAB to toggle)
    barY += 22.0f;
    drawQuad(barX - 2, barY - 2, barW + 4, barH + 4, Vec4(0.1f, 0.1f, 0.15f, 0.8f));
    bool jpMode = player.isJetpackMode();
    Vec4 jpBarCol = jpMode ? Vec4(0.15f, 0.75f, 0.35f, 1.0f) : Vec4(0.25f, 0.3f, 0.35f, 0.8f);
    drawQuad(barX, barY, barW, barH, jpBarCol);
    std::string jpText = jpMode ? "JETPACK: FLIGHT ON [TAB]" : "JETPACK: OFF [TAB]";
    drawText(jpText, barX + 6, barY + 1, 0.8f, Vec4(1, 1, 1, 1));

    // Oxygen Supply Bar (shown if swimming or low)
    if (player.isInWater() || player.getOxygen() < 98.0f) {
        barY += 22.0f;
        drawQuad(barX - 2, barY - 2, barW + 4, barH + 4, Vec4(0.1f, 0.1f, 0.15f, 0.8f));
        float oxyPct = std::clamp(player.getOxygen() / 100.0f, 0.0f, 1.0f);
        Vec4 oxyColor = (oxyPct < 0.25f) ? Vec4(1.0f, 0.2f, 0.2f, 1.0f) : Vec4(0.2f, 0.85f, 1.0f, 1.0f);
        drawQuad(barX, barY, barW * oxyPct, barH, oxyColor);
        drawText("OXYGEN SUPPLY: " + std::to_string((int)player.getOxygen()) + "%", barX + 6, barY + 1, 0.8f, Vec4(1, 1, 1, 1));
    }

    // Rama Sun Status & Day/Night Indicator (Top Center-Right)
    float trX = (float)screenW - 320.0f;
    float trY = 20.0f;
    drawQuad(trX - 8, trY - 6, 175.0f, 54.0f, Vec4(0.05f, 0.08f, 0.12f, 0.85f));

    if (world.isDay()) {
        drawText("RAMA SUNS: DAY", trX, trY, 0.85f, Vec4(1.0f, 0.9f, 0.3f, 1.0f));
    } else {
        drawText("RAMA SUNS: NIGHT", trX, trY, 0.85f, Vec4(0.9f, 0.2f, 0.3f, 1.0f));
        drawText("BIOT THREAT: HIGH", trX, trY + 16, 0.7f, Vec4(1.0f, 0.3f, 0.3f, 1.0f));
    }

    std::string flashStr = player.isFlashlightOn() ? "[F] LIGHT: ON" : "[F] LIGHT: OFF";
    drawText(flashStr, trX, trY + 32, 0.7f, player.isFlashlightOn() ? Vec4(0.3f, 1.0f, 0.5f, 1.0f) : Vec4(0.6f, 0.6f, 0.6f, 1.0f));

    // Feature 7: Holographic Visor Radar Scanner (Top Right)
    float radCenterX = (float)screenW - 65.0f;
    float radCenterY = 65.0f;
    float radRadius = 45.0f;

    // Radar circular backing
    drawQuad(radCenterX - radRadius - 4, radCenterY - radRadius - 4, (radRadius + 4) * 2, (radRadius + 4) * 2, Vec4(0.05f, 0.10f, 0.16f, 0.85f));
    drawQuad(radCenterX - radRadius, radCenterY - radRadius, radRadius * 2, radRadius * 2, Vec4(0.02f, 0.05f, 0.09f, 0.9f));

    // Radar crosshair & range rings
    drawQuad(radCenterX - radRadius, radCenterY, radRadius * 2, 1.0f, Vec4(0.2f, 0.5f, 0.7f, 0.5f));
    drawQuad(radCenterX, radCenterY - radRadius, 1.0f, radRadius * 2, Vec4(0.2f, 0.5f, 0.7f, 0.5f));
    drawQuad(radCenterX - radRadius * 0.5f, radCenterY - radRadius * 0.5f, radRadius, radRadius, Vec4(0.15f, 0.4f, 0.6f, 0.25f));

    // Rotating Radar Sweep Line
    float sweepRad = m_radarAngle * DEG2RAD;
    float sweepX = radCenterX + std::cos(sweepRad) * radRadius;
    float sweepY = radCenterY + std::sin(sweepRad) * radRadius;
    drawQuad(radCenterX, radCenterY, sweepX - radCenterX, sweepY - radCenterY, Vec4(0.3f, 0.8f, 1.0f, 0.4f));

    // Player position (Center Cyan Dot)
    drawQuad(radCenterX - 2, radCenterY - 2, 4, 4, Vec4(0.3f, 1.0f, 0.6f, 1.0f));

    // Biots Threat Pings on Radar
    Vec3 pPos = player.getPosition();
    float pYawRad = player.getYaw() * DEG2RAD;

    auto& biots = BiotManager::instance().getBiots();
    for (const auto& b : biots) {
        if (!b.isAlive) continue;
        float dx = b.pos.x - pPos.x;
        float dz = b.pos.z - pPos.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        if (dist < 40.0f) {
            // Rotate relative to player yaw
            float relX = dx * std::cos(-pYawRad) - dz * std::sin(-pYawRad);
            float relZ = dx * std::sin(-pYawRad) + dz * std::cos(-pYawRad);

            float scale = radRadius / 40.0f;
            float blipX = radCenterX + relX * scale;
            float blipY = radCenterY - relZ * scale;

            // Pulsing red threat dot
            float pulse = 0.6f + 0.4f * std::sin(m_radarAngle * 0.1f);
            drawQuad(blipX - 2, blipY - 2, 4, 4, Vec4(1.0f, 0.2f, 0.2f, pulse));
        }
    }

    drawText("RADAR SCANNER", radCenterX - 45.0f, radCenterY + radRadius + 6.0f, 0.65f, Vec4(0.4f, 0.8f, 1.0f, 0.8f));

    // 3. Mining Beam Progress Bar (Center, below crosshair)
    if (player.isMining() && player.hasTarget()) {
        float mw = 120.0f, mh = 8.0f;
        float mx = cx - mw * 0.5f, my = cy + 24.0f;
        drawQuad(mx - 2, my - 2, mw + 4, mh + 4, Vec4(0.1f, 0.1f, 0.1f, 0.8f));
        drawQuad(mx, my, mw * player.getMiningProgress(), mh, Vec4(0.3f, 0.8f, 1.0f, 1.0f));
    }

    // Target Block Name Tooltip
    if (player.hasTarget()) {
        Vec3i tb = player.getTargetBlock();
        BlockType btype = const_cast<World&>(world).getBlock(tb.x, tb.y, tb.z);
        if (btype != BlockType::Air) {
            std::string bname = BlockRegistry::get(btype).name;
            drawText(bname, cx - (float)bname.length() * 4.0f, cy + 38.0f, 0.8f, Vec4(0.8f, 0.9f, 1.0f, 0.9f));
        }
    }

    // 4. Hotbar (Bottom Center)
    const Inventory& inv = player.getInventory();
    float slotSize = 46.0f;
    float slotPad = 5.0f;
    float hotbarW = 9 * slotSize + 8 * slotPad;
    float hx = (screenW - hotbarW) * 0.5f;
    float hy = (float)screenH - slotSize - 22.0f;

    // 1. Hotbar Outer Glass Panel
    drawQuad(hx - 8, hy - 8, hotbarW + 16, slotSize + 16, Vec4(0.04f, 0.06f, 0.10f, 0.92f));
    drawQuad(hx - 6, hy - 6, hotbarW + 12, slotSize + 12, Vec4(0.08f, 0.12f, 0.18f, 0.60f));

    for (int i = 0; i < 9; ++i) {
        float sx = hx + i * (slotSize + slotPad);
        bool isSel = (i == inv.getSelectedHotbarIndex());

        // Slot frame: vibrant glowing cyan for selected, sleek steel-blue for inactive
        if (isSel) {
            drawQuad(sx - 2, hy - 2, slotSize + 4, slotSize + 4, Vec4(0.0f, 0.85f, 1.0f, 1.0f)); // Outer glow
            drawQuad(sx, hy, slotSize, slotSize, Vec4(0.06f, 0.16f, 0.28f, 0.95f));              // Inner active
        } else {
            drawQuad(sx, hy, slotSize, slotSize, Vec4(0.18f, 0.24f, 0.34f, 0.85f));
            drawQuad(sx + 2, hy + 2, slotSize - 4, slotSize - 4, Vec4(0.05f, 0.07f, 0.11f, 0.95f));
        }

        const ItemStack& item = inv.getSlot(i);
        if (!item.isEmpty()) {
            drawItemIcon(item.type, sx + 6, hy + 6, slotSize - 12);
            if (item.count > 1) {
                std::string countStr = std::to_string(item.count);
                float tw = (float)countStr.length() * 8.0f;
                drawQuad(sx + slotSize - tw - 4, hy + slotSize - 14, tw + 3, 11, Vec4(0, 0, 0, 0.75f));
                drawText(countStr, sx + slotSize - tw - 2, hy + slotSize - 13, 0.75f, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
        }

        // Slot shortcut number [1..9]
        std::string numStr = std::to_string(i + 1);
        drawText(numStr, sx + 4, hy + 3, 0.65f, isSel ? Vec4(0.4f, 1.0f, 1.0f, 1.0f) : Vec4(0.5f, 0.65f, 0.8f, 0.7f));
    }

    // 2. Floating Item Tooltip Banner above active slot
    const ItemStack& curItem = const_cast<Player&>(player).getInventory().getSelectedItem();
    if (!curItem.isEmpty()) {
        const ItemInfo& itemInfo = BlockRegistry::getItem(curItem.type);
        std::string banner = "[" + std::to_string(inv.getSelectedHotbarIndex() + 1) + "] " + itemInfo.name;
        if (!itemInfo.description.empty()) {
            banner += " - " + itemInfo.description;
        }

        float bannerW = (float)banner.length() * 7.5f + 16.0f;
        float bx = cx - bannerW * 0.5f;
        float by = hy - 24.0f;

        drawQuad(bx - 2, by - 2, bannerW + 4, 20.0f, Vec4(0.0f, 0.75f, 1.0f, 0.45f));
        drawQuad(bx, by, bannerW, 18.0f, Vec4(0.04f, 0.07f, 0.12f, 0.90f));
        drawText(banner, bx + 8.0f, by + 4.0f, 0.75f, Vec4(0.3f, 0.9f, 1.0f, 1.0f));
    }

    // 3. Quick shortcuts reminder footer
    drawText("[1-9 / Wheel] Toolbar   [L-Click] Tool/Fire   [R-Click] Place   [Space] Jetpack   [E] Cargo   [C] Fabricator", 18.0f, (float)screenH - 18.0f, 0.68f, Vec4(0.55f, 0.70f, 0.85f, 0.80f));
}

void UI::renderInventory(int screenW, int screenH, Player& player) {
    // Dim background overlay
    drawQuad(0, 0, (float)screenW, (float)screenH, Vec4(0, 0, 0, 0.6f));

    float winW = 460.0f;
    float winH = 340.0f;
    float wx = ((float)screenW - winW) * 0.5f;
    float wy = ((float)screenH - winH) * 0.5f;

    // Window Frame
    drawQuad(wx, wy, winW, winH, Vec4(0.12f, 0.15f, 0.22f, 0.95f));
    drawQuad(wx + 4, wy + 4, winW - 8, winH - 8, Vec4(0.06f, 0.08f, 0.12f, 0.95f));

    drawText("SUIT STORAGE & CARGO MANIFEST [E to Close]", wx + 18, wy + 16, 0.95f, Vec4(0.3f, 0.8f, 1.0f, 1.0f));

    Inventory& inv = player.getInventory();
    float slotSize = 42.0f;
    float slotPad = 4.0f;

    // 1. Main Inventory Grid (3 rows of 9 slots: indices 9..35)
    float gridX = wx + 24.0f;
    float gridY = wy + 50.0f;

    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 9; ++c) {
            int slotIdx = 9 + r * 9 + c;
            float sx = gridX + c * (slotSize + slotPad);
            float sy = gridY + r * (slotSize + slotPad);

            drawQuad(sx, sy, slotSize, slotSize, Vec4(0.2f, 0.25f, 0.35f, 0.8f));
            drawQuad(sx + 2, sy + 2, slotSize - 4, slotSize - 4, Vec4(0.04f, 0.05f, 0.08f, 0.9f));

            const ItemStack& item = inv.getSlot(slotIdx);
            if (!item.isEmpty()) {
                drawItemIcon(item.type, sx + 5, sy + 5, slotSize - 10);
                if (item.count > 1) {
                    drawText(std::to_string(item.count), sx + slotSize - 16, sy + slotSize - 14, 0.8f, Vec4(1, 1, 1, 1));
                }
            }
        }
    }

    // 2. Hotbar Grid (bottom row: indices 0..8)
    float hbY = gridY + 3 * (slotSize + slotPad) + 16.0f;
    drawText("HOTBAR (SLOTS 1-9):", gridX, hbY - 14.0f, 0.75f, Vec4(0.6f, 0.7f, 0.8f, 0.8f));

    for (int c = 0; c < 9; ++c) {
        int slotIdx = c;
        float sx = gridX + c * (slotSize + slotPad);
        float sy = hbY;

        bool isSel = (c == inv.getSelectedHotbarIndex());
        Vec4 frameCol = isSel ? Vec4(0.3f, 0.8f, 1.0f, 1.0f) : Vec4(0.2f, 0.25f, 0.35f, 0.8f);
        drawQuad(sx, sy, slotSize, slotSize, frameCol);
        drawQuad(sx + 2, sy + 2, slotSize - 4, slotSize - 4, Vec4(0.04f, 0.05f, 0.08f, 0.9f));

        const ItemStack& item = inv.getSlot(slotIdx);
        if (!item.isEmpty()) {
            drawItemIcon(item.type, sx + 5, sy + 5, slotSize - 10);
            if (item.count > 1) {
                drawText(std::to_string(item.count), sx + slotSize - 16, sy + slotSize - 14, 0.8f, Vec4(1, 1, 1, 1));
            }
        }
    }
}

void UI::renderCrafting(int screenW, int screenH, Player& player) {
    drawQuad(0, 0, (float)screenW, (float)screenH, Vec4(0, 0, 0, 0.6f));

    float winW = 620.0f;
    float winH = 420.0f;
    float wx = ((float)screenW - winW) * 0.5f;
    float wy = ((float)screenH - winH) * 0.5f;

    // Main window frame
    drawQuad(wx, wy, winW, winH, Vec4(0.12f, 0.15f, 0.22f, 0.95f));
    drawQuad(wx + 4, wy + 4, winW - 8, winH - 8, Vec4(0.06f, 0.08f, 0.12f, 0.95f));

    drawText("RAMA BASE EXPANSION & TECH FABRICATOR [C to Close]", wx + 18, wy + 16, 0.95f, Vec4(0.3f, 0.8f, 1.0f, 1.0f));

    const auto& recipes = CraftingSystem::instance().getRecipes();
    Inventory& inv = player.getInventory();

    // 1. Recipe List (Left Column)
    float listX = wx + 20.0f;
    float listY = wy + 50.0f;
    float itemH = 28.0f;

    for (size_t i = 0; i < recipes.size() && i < 12; ++i) {
        const auto& r = recipes[i];
        bool canCraft = CraftingSystem::instance().canCraft(r, inv);
        bool isSel = ((int)i == m_selectedRecipe);

        float ry = listY + i * itemH;
        Vec4 btnCol = isSel ? Vec4(0.2f, 0.4f, 0.6f, 0.9f) : Vec4(0.1f, 0.12f, 0.18f, 0.8f);
        drawQuad(listX, ry, 260.0f, itemH - 4, btnCol);

        Vec4 textCol = canCraft ? Vec4(0.4f, 1.0f, 0.6f, 1.0f) : Vec4(0.6f, 0.6f, 0.6f, 0.8f);
        if (isSel) textCol = Vec4(1.0f, 1.0f, 1.0f, 1.0f);

        drawItemIcon(r.outputItem, listX + 4, ry + 2, 20.0f);
        drawText(r.name, listX + 30.0f, ry + 4, 0.75f, textCol);
    }

    // 2. Recipe Detail & Requirements (Right Column)
    if (m_selectedRecipe >= 0 && m_selectedRecipe < (int)recipes.size()) {
        const auto& selRecipe = recipes[m_selectedRecipe];
        float detX = wx + 295.0f;
        float detY = wy + 50.0f;

        // Detail panel backing
        drawQuad(detX, detY, 305.0f, 345.0f, Vec4(0.08f, 0.1f, 0.15f, 0.9f));

        // Output Icon & Name
        drawItemIcon(selRecipe.outputItem, detX + 16, detY + 16, 48.0f);
        drawText(selRecipe.name, detX + 74, detY + 18, 0.9f, Vec4(0.3f, 0.8f, 1.0f, 1.0f));
        drawText("Creates: " + std::to_string(selRecipe.outputCount) + "x Unit", detX + 74, detY + 36, 0.75f, Vec4(0.7f, 0.8f, 0.9f, 0.8f));

        // Description
        drawText(selRecipe.description, detX + 16, detY + 80, 0.7f, Vec4(0.75f, 0.85f, 0.95f, 0.9f));

        // Required Materials List
        drawText("REQUIRED MATERIALS:", detX + 16, detY + 120, 0.8f, Vec4(0.9f, 0.8f, 0.4f, 1.0f));
        float reqY = detY + 144.0f;
        for (const auto& ing : selRecipe.ingredients) {
            int have = inv.countItem(ing.item);
            bool hasEnough = have >= ing.count;
            std::string ingName = BlockRegistry::getItem(ing.item).name;
            std::string status = ingName + " : " + std::to_string(have) + "/" + std::to_string(ing.count);

            Vec4 ingCol = hasEnough ? Vec4(0.3f, 1.0f, 0.5f, 1.0f) : Vec4(1.0f, 0.4f, 0.4f, 1.0f);
            drawItemIcon(ing.item, detX + 20, reqY, 20.0f);
            drawText(status, detX + 48, reqY + 2, 0.75f, ingCol);
            reqY += 26.0f;
        }

        // CRAFT BUTTON
        bool canCraft = CraftingSystem::instance().canCraft(selRecipe, inv);
        float btnW = 260.0f, btnH = 38.0f;
        float bx = detX + 22.0f, by = detY + 285.0f;
        Vec4 btnBg = canCraft ? Vec4(0.15f, 0.6f, 0.3f, 1.0f) : Vec4(0.3f, 0.3f, 0.35f, 0.6f);
        drawQuad(bx, by, btnW, btnH, btnBg);
        drawText(canCraft ? "FABRICATE ITEM" : "INSUFFICIENT MATERIALS", bx + (canCraft ? 50.0f : 20.0f), by + 10.0f, 0.9f, Vec4(1, 1, 1, 1));
    }
}

void UI::render(const Shader& uiShader, int screenWidth, int screenHeight, const Player& player, const World& world) {
    m_mesh.clear();

    if (m_menuState == UIMenuState::HUDOnly) {
        renderHUD(screenWidth, screenHeight, player, world);
    } else if (m_menuState == UIMenuState::InventoryScreen) {
        renderInventory(screenWidth, screenHeight, const_cast<Player&>(player));
    } else if (m_menuState == UIMenuState::CraftingScreen) {
        renderCrafting(screenWidth, screenHeight, const_cast<Player&>(player));
    } else if (m_menuState == UIMenuState::MultiplayerScreen) {
        renderMultiplayer(screenWidth, screenHeight);
    }

    if (m_mesh.empty()) return;

    uiShader.use();
    Mat4 proj = Mat4::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    uiShader.setMat4("uProjection", proj);
    uiShader.setInt("uUseTexture", 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_mesh.size() * sizeof(UIVertex), m_mesh.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(UIVertex), (void*)offsetof(UIVertex, color));

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_mesh.size());

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}

void UI::handleMouseClick(int mouseX, int mouseY, int button, Player& player) {
    if (m_menuState == UIMenuState::CraftingScreen) {
        float winW = 620.0f, winH = 420.0f;
        float wx = (1280.0f - winW) * 0.5f;
        float wy = (720.0f - winH) * 0.5f;

        const auto& recipes = CraftingSystem::instance().getRecipes();
        float listX = wx + 20.0f;
        float listY = wy + 50.0f;
        float itemH = 28.0f;

        // Check recipe list item clicks
        for (size_t i = 0; i < recipes.size() && i < 12; ++i) {
            float ry = listY + i * itemH;
            if (mouseX >= listX && mouseX <= listX + 260.0f && mouseY >= ry && mouseY <= ry + itemH) {
                m_selectedRecipe = (int)i;
                return;
            }
        }

        // Check craft button click
        float detX = wx + 295.0f;
        float detY = wy + 50.0f;
        float bx = detX + 22.0f, by = detY + 285.0f;
        float btnW = 260.0f, btnH = 38.0f;

        if (mouseX >= bx && mouseX <= bx + btnW && mouseY >= by && mouseY <= by + btnH) {
            if (m_selectedRecipe >= 0 && m_selectedRecipe < (int)recipes.size()) {
                CraftingSystem::instance().craft(recipes[m_selectedRecipe], player.getInventory());
            }
        }
    } else if (m_menuState == UIMenuState::InventoryScreen) {
        float winW = 460.0f, winH = 340.0f;
        float wx = (1280.0f - winW) * 0.5f;
        float wy = (720.0f - winH) * 0.5f;

        float gridX = wx + 24.0f;
        float gridY = wy + 50.0f;
        float slotSize = 42.0f;
        float slotPad = 4.0f;

        int clickedSlot = -1;

        // Check main inventory slots
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 9; ++c) {
                float sx = gridX + c * (slotSize + slotPad);
                float sy = gridY + r * (slotSize + slotPad);
                if (mouseX >= sx && mouseX <= sx + slotSize && mouseY >= sy && mouseY <= sy + slotSize) {
                    clickedSlot = 9 + r * 9 + c;
                    break;
                }
            }
        }

        // Check hotbar slots
        float hbY = gridY + 3 * (slotSize + slotPad) + 16.0f;
        for (int c = 0; c < 9; ++c) {
            float sx = gridX + c * (slotSize + slotPad);
            float sy = hbY;
            if (mouseX >= sx && mouseX <= sx + slotSize && mouseY >= sy && mouseY <= sy + slotSize) {
                clickedSlot = c;
                break;
            }
        }

        if (clickedSlot != -1) {
            if (m_heldSlotIndex == -1) {
                m_heldSlotIndex = clickedSlot;
            } else {
                player.getInventory().swapSlots(m_heldSlotIndex, clickedSlot);
                m_heldSlotIndex = -1;
            }
        }
    } else if (m_menuState == UIMenuState::MultiplayerScreen) {
        float winW = 680.0f, winH = 480.0f;
        float wx = (1280.0f - winW) * 0.5f;
        float wy = (720.0f - winH) * 0.5f;

        // 1. Host Button Click
        float hostBtnX = wx + 30.0f, hostBtnY = wy + 115.0f;
        float hostBtnW = 280.0f, hostBtnH = 36.0f;
        if (mouseX >= hostBtnX && mouseX <= hostBtnX + hostBtnW && mouseY >= hostBtnY && mouseY <= hostBtnY + hostBtnH) {
            NetworkManager::instance().startHost(7777, "Rama Expedition");
            return;
        }

        // 2. Disconnect Button Click
        float discBtnX = wx + 340.0f, discBtnY = wy + 115.0f;
        float discBtnW = 280.0f, discBtnH = 36.0f;
        if (mouseX >= discBtnX && mouseX <= discBtnX + discBtnW && mouseY >= discBtnY && mouseY <= discBtnY + discBtnH) {
            NetworkManager::instance().disconnect();
            return;
        }

        // 3. Discovered Servers List Clicks
        const auto& servers = NetworkManager::instance().getDiscoveredServers();
        float listY = wy + 200.0f;
        for (size_t i = 0; i < servers.size() && i < 5; ++i) {
            float sy = listY + i * 40.0f;
            float joinBtnX = wx + 540.0f, joinBtnY = sy + 4.0f;
            float joinBtnW = 90.0f, joinBtnH = 28.0f;
            if (mouseX >= joinBtnX && mouseX <= joinBtnX + joinBtnW && mouseY >= joinBtnY && mouseY <= joinBtnY + joinBtnH) {
                NetworkManager::instance().connectTo(servers[i].ip, servers[i].port);
                return;
            }
        }

        // 4. Localhost Direct Connect Button
        float locBtnX = wx + 30.0f, locBtnY = wy + 415.0f;
        float locBtnW = 320.0f, locBtnH = 32.0f;
        if (mouseX >= locBtnX && mouseX <= locBtnX + locBtnW && mouseY >= locBtnY && mouseY <= locBtnY + locBtnH) {
            NetworkManager::instance().connectTo("127.0.0.1", 7777);
            return;
        }
    }
}

void UI::toggleMultiplayer() {
    if (m_menuState == UIMenuState::MultiplayerScreen) {
        m_menuState = UIMenuState::HUDOnly;
    } else {
        m_menuState = UIMenuState::MultiplayerScreen;
    }
}

void UI::renderMultiplayer(int screenW, int screenH) {
    float winW = 680.0f, winH = 480.0f;
    float wx = (screenW - winW) * 0.5f;
    float wy = (screenH - winH) * 0.5f;

    // Outer Glow Border & Window Backing
    drawQuad(wx - 4, wy - 4, winW + 8, winH + 8, Vec4(0.1f, 0.5f, 0.8f, 0.4f));
    drawQuad(wx, wy, winW, winH, Vec4(0.04f, 0.06f, 0.10f, 0.96f));

    // Title Bar
    drawQuad(wx, wy, winW, 44.0f, Vec4(0.08f, 0.16f, 0.28f, 0.95f));
    drawText("RAMA EXPEDITION - WIFI LAN MULTIPLAYER", wx + 20.0f, wy + 14.0f, 0.95f, Vec4(0.3f, 0.9f, 1.0f, 1.0f));

    // 1. Connection Status Banner
    std::string status = NetworkManager::instance().getStatusText();
    Vec4 statusCol = NetworkManager::instance().isConnected() ? Vec4(0.3f, 1.0f, 0.5f, 1.0f) : Vec4(0.8f, 0.8f, 0.8f, 0.8f);
    drawQuad(wx + 20.0f, wy + 56.0f, winW - 40.0f, 44.0f, Vec4(0.08f, 0.12f, 0.18f, 0.9f));
    drawText("NETWORK STATUS: " + status, wx + 36.0f, wy + 70.0f, 0.85f, statusCol);

    // Host & Disconnect Buttons
    float hostBtnX = wx + 30.0f, hostBtnY = wy + 115.0f;
    float hostBtnW = 280.0f, hostBtnH = 36.0f;
    drawQuad(hostBtnX, hostBtnY, hostBtnW, hostBtnH, Vec4(0.15f, 0.55f, 0.35f, 0.95f));
    drawText("HOST LAN EXPEDITION (PORT 7777)", hostBtnX + 24.0f, hostBtnY + 11.0f, 0.75f, Vec4(1, 1, 1, 1));

    float discBtnX = wx + 340.0f, discBtnY = wy + 115.0f;
    float discBtnW = 280.0f, discBtnH = 36.0f;
    drawQuad(discBtnX, discBtnY, discBtnW, discBtnH, Vec4(0.55f, 0.2f, 0.2f, 0.95f));
    drawText("DISCONNECT / SINGLEPLAYER", discBtnX + 38.0f, discBtnY + 11.0f, 0.75f, Vec4(1, 1, 1, 1));

    // 2. Discovered LAN Servers List
    drawText("DISCOVERED LAN SERVERS ON YOUR WIFI:", wx + 24.0f, wy + 175.0f, 0.8f, Vec4(0.9f, 0.85f, 0.4f, 1.0f));
    float listY = wy + 200.0f;
    const auto& servers = NetworkManager::instance().getDiscoveredServers();

    if (servers.empty()) {
        drawQuad(wx + 20.0f, listY, winW - 40.0f, 180.0f, Vec4(0.06f, 0.08f, 0.12f, 0.8f));
        drawText("Searching WiFi subnet for active RamaCraft hosts...", wx + 130.0f, listY + 70.0f, 0.8f, Vec4(0.5f, 0.6f, 0.7f, 0.8f));
        drawText("(Click 'HOST LAN EXPEDITION' above on one machine to start)", wx + 110.0f, listY + 95.0f, 0.7f, Vec4(0.4f, 0.5f, 0.6f, 0.7f));
    } else {
        for (size_t i = 0; i < servers.size() && i < 5; ++i) {
            float sy = listY + i * 40.0f;
            drawQuad(wx + 20.0f, sy, winW - 40.0f, 34.0f, Vec4(0.1f, 0.14f, 0.22f, 0.9f));
            drawText(servers[i].name, wx + 36.0f, sy + 10.0f, 0.8f, Vec4(1, 1, 1, 1));
            drawText(servers[i].ip + ":" + std::to_string(servers[i].port), wx + 280.0f, sy + 10.0f, 0.75f, Vec4(0.7f, 0.8f, 0.9f, 0.8f));
            drawText(std::to_string(servers[i].currentPlayers) + "/" + std::to_string(servers[i].maxPlayers) + " Players", wx + 440.0f, sy + 10.0f, 0.75f, Vec4(0.4f, 1.0f, 0.6f, 0.9f));

            // Join Button
            drawQuad(wx + 540.0f, sy + 4.0f, 90.0f, 26.0f, Vec4(0.2f, 0.6f, 0.9f, 0.95f));
            drawText("JOIN", wx + 568.0f, sy + 9.0f, 0.75f, Vec4(1, 1, 1, 1));
        }
    }

    // 3. Direct Loopback Connect & Hotkey Info
    drawQuad(wx + 30.0f, wy + 415.0f, 320.0f, 32.0f, Vec4(0.12f, 0.3f, 0.45f, 0.9f));
    drawText("DIRECT CONNECT (127.0.0.1:7777)", wx + 48.0f, wy + 424.0f, 0.7f, Vec4(0.9f, 0.95f, 1.0f, 1.0f));

    drawText("Press [M] to close menu and return to game", wx + 380.0f, wy + 424.0f, 0.7f, Vec4(0.6f, 0.7f, 0.8f, 0.8f));
}
