#include "ItemEntity.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "Audio.hpp"
#include <cmath>
#include <random>
#include <algorithm>

ItemEntityManager& ItemEntityManager::instance() {
    static ItemEntityManager s_mgr;
    return s_mgr;
}

void ItemEntityManager::init() {
    buildItemMesh();
}

void ItemEntityManager::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
}

void ItemEntityManager::buildItemMesh() {
    struct Vertex {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };

    std::vector<Vertex> verts;
    float h = 0.16f; // Mini cube half-size

    auto addFace = [&](const Vec3& p0, const Vec3& p1, const Vec3& p2, const Vec3& p3, const Vec3& n) {
        // Will use default unit UVs, scaled per item in rendering or default texture
        verts.push_back({p0, {0.0f, 1.0f}, n});
        verts.push_back({p1, {1.0f, 1.0f}, n});
        verts.push_back({p2, {1.0f, 0.0f}, n});

        verts.push_back({p0, {0.0f, 1.0f}, n});
        verts.push_back({p2, {1.0f, 0.0f}, n});
        verts.push_back({p3, {0.0f, 0.0f}, n});
    };

    // +Y (Top)
    addFace({-h, h, h}, {h, h, h}, {h, h, -h}, {-h, h, -h}, {0, 1, 0});
    // -Y (Bottom)
    addFace({-h, -h, -h}, {h, -h, -h}, {h, -h, h}, {-h, -h, h}, {0, -1, 0});
    // +Z (Front)
    addFace({-h, -h, h}, {h, -h, h}, {h, h, h}, {-h, h, h}, {0, 0, 1});
    // -Z (Back)
    addFace({h, -h, -h}, {-h, -h, -h}, {-h, h, -h}, {h, h, -h}, {0, 0, -1});
    // -X (Left)
    addFace({-h, -h, -h}, {-h, -h, h}, {-h, h, h}, {-h, h, -h}, {-1, 0, 0});
    // +X (Right)
    addFace({h, -h, h}, {h, -h, -h}, {h, h, -h}, {h, h, h}, {1, 0, 0});

    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    m_vertexCount = (GLsizei)verts.size();
    glBindVertexArray(0);
}

void ItemEntityManager::spawnItem(const Vec3& pos, ItemType item, int count) {
    if (item == ItemType::None || count <= 0) return;

    ItemEntity e;
    e.pos = pos + Vec3(0, 0.2f, 0);
    float rx = ((float)(rand() % 200) / 100.0f - 1.0f) * 1.5f;
    float rz = ((float)(rand() % 200) / 100.0f - 1.0f) * 1.5f;
    e.vel = Vec3(rx, 3.5f, rz);
    e.item = item;
    e.count = count;
    e.rotation = (float)(rand() % 360);
    e.age = 0.0f;
    e.isAlive = true;
    m_items.push_back(e);
}

void ItemEntityManager::update(float dt, World& world, Player& player) {
    Vec3 playerPos = player.getPosition() + Vec3(0, 0.9f, 0);

    for (auto& item : m_items) {
        if (!item.isAlive) continue;

        item.age += dt;
        item.rotation += 90.0f * dt;

        Vec3 toPlayer = playerPos - item.pos;
        float dist = toPlayer.length();

        // Magnetic attraction when player is within 3.5 units
        if (dist < 3.5f) {
            Vec3 pullDir = toPlayer.normalized();
            float pullSpeed = 9.0f * (1.0f - dist / 4.0f) + 3.0f;
            item.vel = Vec3::lerp(item.vel, pullDir * pullSpeed, 12.0f * dt);

            // Pickup on contact
            if (dist < 0.65f) {
                if (player.getInventory().addItem(item.item, item.count)) {
                    item.isAlive = false;
                    AudioSystem::instance().playSound(SoundEffect::BlockPlace);
                    continue;
                }
            }
        } else {
            // Low gravity & friction
            item.vel.y -= 8.0f * dt;
            item.vel.x *= 0.92f;
            item.vel.z *= 0.92f;
        }

        item.pos += item.vel * dt;

        // Voxel ground collision
        int bx = (int)std::floor(item.pos.x);
        int by = (int)std::floor(item.pos.y);
        int bz = (int)std::floor(item.pos.z);

        if (world.getBlock(bx, by, bz) != BlockType::Air) {
            item.pos.y = (float)(by + 1) + 0.15f + std::sin(item.age * 3.0f) * 0.04f;
            item.vel.y = 0.0f;
        }
    }

    m_items.erase(
        std::remove_if(m_items.begin(), m_items.end(), [](const ItemEntity& e) { return !e.isAlive; }),
        m_items.end()
    );
}

void ItemEntityManager::render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    if (m_items.empty() || m_vertexCount == 0) return;

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", World::CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", 1.0f);

    glBindVertexArray(m_vao);

    for (const auto& item : m_items) {
        float bobY = std::sin(item.age * 3.0f) * 0.08f;
        Mat4 model = Mat4::translation(item.pos + Vec3(0, bobY, 0)) *
                     Mat4::rotationY(item.rotation * DEG2RAD) *
                     Mat4::rotationX(15.0f * DEG2RAD);

        shader.setMat4("uModel", model);
        shader.setVec4("uTint", Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }

    glBindVertexArray(0);
}
