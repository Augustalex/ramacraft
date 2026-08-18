#include "Biot.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "Audio.hpp"
#include "Projectile.hpp"
#include <cmath>
#include <random>
#include <iostream>

BiotManager& BiotManager::instance() {
    static BiotManager s_mgr;
    return s_mgr;
}

void BiotManager::init() {
    buildBeetleMesh();
}

void BiotManager::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
}

void BiotManager::buildBeetleMesh() {
    struct EntityVertex {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };

    std::vector<EntityVertex> verts;

    auto addBox = [&](const Vec3& min, const Vec3& max, int tx, int ty) {
        float u0, v0, u1, v1;
        TextureAtlas::instance().getTileUVs(tx, ty, u0, v0, u1, v1);

        // 6 faces of the box (CCW winding)
        // +Y (Top)
        verts.push_back({{min.x, max.y, max.z}, {u0, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {0, 1, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u0, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {0, 1, 0}});
        verts.push_back({{min.x, max.y, min.z}, {u0, v0}, {0, 1, 0}});

        // -Y (Bottom)
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, min.z}, {u1, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v0}, {0, -1, 0}});
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v0}, {0, -1, 0}});
        verts.push_back({{min.x, min.y, max.z}, {u0, v0}, {0, -1, 0}});

        // +Z (Front)
        verts.push_back({{min.x, min.y, max.z}, {u0, v1}, {0, 0, 1}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v1}, {0, 0, 1}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v0}, {0, 0, 1}});
        verts.push_back({{min.x, min.y, max.z}, {u0, v1}, {0, 0, 1}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v0}, {0, 0, 1}});
        verts.push_back({{min.x, max.y, max.z}, {u0, v0}, {0, 0, 1}});

        // -Z (Back)
        verts.push_back({{max.x, min.y, min.z}, {u0, v1}, {0, 0, -1}});
        verts.push_back({{min.x, min.y, min.z}, {u1, v1}, {0, 0, -1}});
        verts.push_back({{min.x, max.y, min.z}, {u1, v0}, {0, 0, -1}});
        verts.push_back({{max.x, min.y, min.z}, {u0, v1}, {0, 0, -1}});
        verts.push_back({{min.x, max.y, min.z}, {u1, v0}, {0, 0, -1}});
        verts.push_back({{max.x, max.y, min.z}, {u0, v0}, {0, 0, -1}});

        // -X (Left)
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, min.y, max.z}, {u1, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u1, v0}, {-1, 0, 0}});
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u1, v0}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, min.z}, {u0, v0}, {-1, 0, 0}});

        // +X (Right)
        verts.push_back({{max.x, min.y, max.z}, {u0, v1}, {1, 0, 0}});
        verts.push_back({{max.x, min.y, min.z}, {u1, v1}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {1, 0, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u0, v1}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, max.z}, {u0, v0}, {1, 0, 0}});
    };

    // 1. Armored Dorsal Carapace (Abdomen) (tx=0, ty=3 Biot shell)
    addBox({-0.34f, 0.16f, -0.44f}, {0.34f, 0.44f, 0.18f}, 0, 3);
    addBox({-0.28f, 0.38f, -0.38f}, {0.28f, 0.48f, 0.12f}, 0, 3); // Dorsal ridge plate

    // 2. Underbelly Chassis (tx=1, ty=3)
    addBox({-0.28f, 0.08f, -0.40f}, {0.28f, 0.18f, 0.22f}, 1, 3);

    // 3. Cephalothorax / Head (tx=2, ty=3 AI Robot core)
    addBox({-0.22f, 0.14f, 0.18f}, {0.22f, 0.38f, 0.46f}, 2, 3);

    // 4. Glowing Red Optical Eyes (tx=15, ty=0 Bright red beacon)
    addBox({-0.18f, 0.24f, 0.45f}, {-0.06f, 0.34f, 0.50f}, 15, 0); // Left eye
    addBox({0.06f, 0.24f, 0.45f}, {0.18f, 0.34f, 0.50f}, 15, 0);  // Right eye

    // 5. Mechanical Pincer Mandibles
    addBox({-0.14f, 0.12f, 0.46f}, {-0.06f, 0.22f, 0.62f}, 1, 3); // Left jaw
    addBox({0.06f, 0.12f, 0.46f}, {0.14f, 0.22f, 0.62f}, 1, 3);  // Right jaw

    // 6. Six articulated 3D spider legs (upper thigh + lower claw)
    // Left Legs
    // Front Left
    addBox({-0.52f, 0.22f, 0.20f}, {-0.28f, 0.32f, 0.32f}, 1, 3); // Thigh
    addBox({-0.62f, 0.00f, 0.28f}, {-0.48f, 0.24f, 0.36f}, 0, 3); // Claw down to ground
    // Mid Left
    addBox({-0.56f, 0.22f, -0.06f}, {-0.28f, 0.32f, 0.06f}, 1, 3);
    addBox({-0.66f, 0.00f, -0.08f}, {-0.52f, 0.24f, 0.08f}, 0, 3);
    // Rear Left
    addBox({-0.52f, 0.22f, -0.34f}, {-0.28f, 0.32f, -0.22f}, 1, 3);
    addBox({-0.62f, 0.00f, -0.40f}, {-0.48f, 0.24f, -0.26f}, 0, 3);

    // Right Legs
    // Front Right
    addBox({0.28f, 0.22f, 0.20f}, {0.52f, 0.32f, 0.32f}, 1, 3);
    addBox({0.48f, 0.00f, 0.28f}, {0.62f, 0.24f, 0.36f}, 0, 3);
    // Mid Right
    addBox({0.28f, 0.22f, -0.06f}, {0.56f, 0.32f, 0.06f}, 1, 3);
    addBox({0.52f, 0.00f, -0.08f}, {0.66f, 0.24f, 0.08f}, 0, 3);
    // Rear Right
    addBox({0.28f, 0.22f, -0.34f}, {0.52f, 0.32f, -0.22f}, 1, 3);
    addBox({0.48f, 0.00f, -0.40f}, {0.62f, 0.24f, -0.26f}, 0, 3);

    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    }

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(EntityVertex), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, normal));

    m_vertexCount = (GLsizei)verts.size();
    glBindVertexArray(0);
}

void BiotManager::spawnBiot(const Vec3& pos) {
    Biot b;
    b.pos = pos;
    b.vel = {0, 0, 0};
    b.health = 60.0f;
    b.maxHealth = 60.0f;
    b.isAlive = true;
    m_biots.push_back(b);
}

bool BiotManager::checkBoltCollision(const Vec3& boltPos, float radius, float damage) {
    for (auto& b : m_biots) {
        if (!b.isAlive) continue;
        float dist = (b.pos + Vec3(0, 0.3f, 0) - boltPos).length();
        if (dist < radius + 0.5f) {
            b.health -= damage;
            AudioSystem::instance().playSound(SoundEffect::BiotDamage);
            if (b.health <= 0.0f) {
                b.isAlive = false;
                AudioSystem::instance().playSound(SoundEffect::BiotScreech);
            }
            return true;
        }
    }
    return false;
}

void BiotManager::update(float dt, World& world, Player& player) {
    // 1. Spawning Biots at Night or in Dark Ruins
    m_spawnTimer += dt;
    if (m_spawnTimer > 3.5f) {
        m_spawnTimer = 0.0f;

        bool isNight = world.isNight();
        int maxBiots = isNight ? 12 : 3;

        if ((int)m_biots.size() < maxBiots) {
            // Spawn around player
            float angle = ((float)(rand() % 360)) * DEG2RAD;
            float dist = 22.0f + (rand() % 16);
            Vec3 pPos = player.getPosition();
            float spawnX = pPos.x + std::cos(angle) * dist;
            float spawnZ = pPos.z + std::sin(angle) * dist;

            // Find surface ground height
            int sy = 30;
            while (sy > 4 && world.getBlock((int)spawnX, sy, (int)spawnZ) == BlockType::Air) {
                --sy;
            }

            if (sy > 4 && world.getBlock((int)spawnX, sy, (int)spawnZ) != BlockType::CylindricalSeaWater) {
                spawnBiot({spawnX, (float)sy + 1.0f, spawnZ});
            }
        }
    }

    // 2. Update AI and Physics for Each Biot
    Vec3 playerPos = player.getPosition();
    bool flashlightOn = player.isFlashlightOn();

    for (auto& b : m_biots) {
        if (!b.isAlive) continue;

        Vec3 toPlayer = playerPos - b.pos;
        float distToPlayer = toPlayer.length();

        // Sight detection range: boosted if player has flashlight shining
        float sightRange = flashlightOn ? 45.0f : (world.isNight() ? 32.0f : 16.0f);

        if (distToPlayer < sightRange) {
            // Skitter towards player
            Vec3 dir = Vec3(toPlayer.x, 0.0f, toPlayer.z).normalized();
            float speed = world.isNight() ? 5.5f : 3.8f;
            b.vel.x = dir.x * speed;
            b.vel.z = dir.z * speed;

            // Face player
            b.yaw = std::atan2(dir.x, dir.z) * RAD2DEG;
            b.legAnim += dt * speed * 4.0f;

            // Attack player if in melee reach
            if (distToPlayer < 1.4f) {
                b.attackCooldown -= dt;
                if (b.attackCooldown <= 0.0f) {
                    b.attackCooldown = 0.8f;
                    player.takeDamage(15.0f);
                    AudioSystem::instance().playSound(SoundEffect::PlayerHurt);
                }
            }
        } else {
            // Idle wander
            b.vel.x *= 0.9f;
            b.vel.z *= 0.9f;
        }

        // Gravity & terrain collision
        b.vel.y -= 9.8f * dt;
        b.pos += b.vel * dt;

        int bx = (int)std::floor(b.pos.x);
        int by = (int)std::floor(b.pos.y);
        int bz = (int)std::floor(b.pos.z);

        // Ground floor check
        if (world.getBlock(bx, by, bz) != BlockType::Air) {
            b.pos.y = (float)(by + 1);
            b.vel.y = 0.0f;
        }

        // Step up 1 block obstacles
        if (world.getBlock(bx, by + 1, bz) != BlockType::Air) {
            b.pos.y += 1.0f;
            b.vel.y = 2.0f;
        }
    }

    // 3. Loot Drops for Killed Biots
    for (auto& b : m_biots) {
        if (!b.isAlive) {
            // Randomly drop valuable machine parts & biot shells
            int r = rand() % 100;
            if (r < 65) player.getInventory().addItem(ItemType::BiotShell, 1 + rand() % 2);
            if (r < 50) player.getInventory().addItem(ItemType::MicroActuator, 1 + rand() % 2);
            if (r < 35) player.getInventory().addItem(ItemType::RobotCore, 1);
            if (r < 45) player.getInventory().addItem(ItemType::EnergyCell, 1 + rand() % 2);
            ProjectileManager::instance().spawnSparks(b.pos + Vec3(0, 0.4f, 0), Vec4(0.8f, 0.1f, 0.1f, 1.0f), 24, 6.0f);
        }
    }

    // Remove dead biots
    m_biots.erase(
        std::remove_if(m_biots.begin(), m_biots.end(), [](const Biot& b) { return !b.isAlive; }),
        m_biots.end()
    );
}

void BiotManager::render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    if (m_biots.empty() || m_vertexCount == 0) return;

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", World::CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", 1.0f);

    glDisable(GL_CULL_FACE); // Disables backface culling so legs and joints are completely solid from all angles
    glBindVertexArray(m_vao);

    for (const auto& b : m_biots) {
        Mat4 model = Mat4::translation(b.pos) * Mat4::rotationY(b.yaw * DEG2RAD);
        shader.setMat4("uModel", model);
        shader.setVec4("uTint", Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }

    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
}
