#include "Projectile.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Biot.hpp"
#include "Shader.hpp"
#include "Audio.hpp"
#include "TextureAtlas.hpp"
#include "Player.hpp"
#include "Network.hpp"
#include <cmath>
#include <random>
#include <algorithm>

ProjectileManager& ProjectileManager::instance() {
    static ProjectileManager s_mgr;
    return s_mgr;
}

void ProjectileManager::init() {
    if (m_vao == 0) {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    }
}

void ProjectileManager::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    m_bolts.clear();
    m_grenades.clear();
    m_tracers.clear();
    m_particles.clear();
}

void ProjectileManager::spawnBolt(const Vec3& pos, const Vec3& dir, float speed, float damage, bool overclocked, uint8_t shooterId) {
    PlasmaBolt b;
    b.pos = pos;
    b.prevPos = pos;
    b.vel = dir.normalized() * speed;
    b.life = 2.5f;
    b.damage = damage;
    b.isOverclocked = overclocked;
    b.shooterId = shooterId;
    m_bolts.push_back(b);

    // Lingering red laser beam tracer
    spawnLaserTracer(pos, pos + dir.normalized() * 18.0f, Vec4(1.0f, 0.15f, 0.15f, 0.95f), 0.22f);

    AudioSystem::instance().playSound(SoundEffect::LaserFire);
}

void ProjectileManager::spawnGrenade(const Vec3& pos, const Vec3& vel, uint8_t throwerId) {
    Grenade g;
    g.pos = pos;
    g.prevPos = pos;
    g.vel = vel;
    g.fuse = 2.4f;
    g.maxFuse = 2.4f;
    g.throwerId = throwerId;
    m_grenades.push_back(g);
}

void ProjectileManager::spawnLaserTracer(const Vec3& start, const Vec3& end, const Vec4& color, float duration) {
    LaserTracer lt;
    lt.start = start;
    lt.end = end;
    lt.color = color;
    lt.maxLife = duration;
    lt.life = duration;
    m_tracers.push_back(lt);
}

void ProjectileManager::spawnSparks(const Vec3& pos, const Vec4& color, int count, float speed) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.pos = pos;
        float rx = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        float ry = ((float)(rand() % 2000) / 1000.0f - 0.5f);
        float rz = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        p.vel = Vec3(rx, ry, rz).normalized() * (speed * (0.5f + (rand() % 100) / 100.0f));
        p.color = color;
        p.size = 0.09f + (rand() % 50) / 1000.0f;
        p.maxLife = 0.35f + (rand() % 30) / 100.0f;
        p.life = p.maxLife;
        m_particles.push_back(p);
    }
}

void ProjectileManager::spawnJetpackExhaust(const Vec3& pos, const Vec3& dir) {
    for (int i = 0; i < 4; ++i) {
        Particle p;
        p.pos = pos + Vec3((rand() % 10 - 5) * 0.02f, 0, (rand() % 10 - 5) * 0.02f);
        Vec3 spread(
            ((float)(rand() % 200) / 100.0f - 1.0f) * 0.4f,
            -1.0f,
            ((float)(rand() % 200) / 100.0f - 1.0f) * 0.4f
        );
        p.vel = spread.normalized() * (4.0f + (rand() % 30) / 10.0f);
        // Orange and cyan high-power thruster flame
        p.color = (rand() % 2 == 0) ? Vec4(1.0f, 0.4f, 0.1f, 0.9f) : Vec4(0.2f, 0.8f, 1.0f, 0.9f);
        p.size = 0.15f;
        p.maxLife = 0.28f;
        p.life = p.maxLife;
        m_particles.push_back(p);
    }
}

void ProjectileManager::spawnExplosionParticles(const Vec3& pos, float radius) {
    // 1. Fiery explosion core
    for (int i = 0; i < 50; ++i) {
        Particle p;
        p.pos = pos + Vec3((rand() % 100 - 50) * 0.01f, (rand() % 100 - 50) * 0.01f, (rand() % 100 - 50) * 0.01f);
        float rx = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        float ry = ((float)(rand() % 2000) / 1000.0f - 0.2f);
        float rz = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        p.vel = Vec3(rx, ry, rz).normalized() * (8.0f + (rand() % 100) * 0.12f);
        p.color = (rand() % 3 == 0) ? Vec4(1.0f, 0.9f, 0.2f, 1.0f) : ((rand() % 2 == 0) ? Vec4(1.0f, 0.35f, 0.05f, 1.0f) : Vec4(0.9f, 0.1f, 0.05f, 1.0f));
        p.size = 0.35f + (rand() % 30) * 0.01f;
        p.maxLife = 0.65f + (rand() % 40) * 0.01f;
        p.life = p.maxLife;
        m_particles.push_back(p);
    }

    // 2. Lingering smoke cloud
    for (int i = 0; i < 35; ++i) {
        Particle p;
        p.pos = pos + Vec3((rand() % 100 - 50) * 0.02f, (rand() % 100 - 50) * 0.02f, (rand() % 100 - 50) * 0.02f);
        float rx = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        float ry = ((float)(rand() % 2000) / 1000.0f) * 0.8f;
        float rz = ((float)(rand() % 2000) / 1000.0f - 1.0f);
        p.vel = Vec3(rx, ry, rz).normalized() * (3.0f + (rand() % 40) * 0.1f);
        p.color = Vec4(0.35f, 0.35f, 0.38f, 0.75f);
        p.size = 0.5f + (rand() % 40) * 0.01f;
        p.maxLife = 1.1f + (rand() % 60) * 0.01f;
        p.life = p.maxLife;
        m_particles.push_back(p);
    }
}

void ProjectileManager::triggerExplosion(const Vec3& pos, float radius, float maxDamage, World& world, BiotManager& biots, Player* localPlayer, bool syncNetwork) {
    AudioSystem::instance().playSound(SoundEffect::Explosion);
    spawnExplosionParticles(pos, radius);

    // 1. Crater Destruction (Break non-bedrock blocks in spherical radius)
    int minBx = (int)std::floor(pos.x - 2.8f);
    int maxBx = (int)std::floor(pos.x + 2.8f);
    int minBy = std::max(4, (int)std::floor(pos.y - 2.8f)); // Preserve bedrock floor at y <= 4
    int maxBy = std::min((int)CHUNK_SIZE_Y - 1, (int)std::floor(pos.y + 2.8f));
    int minBz = (int)std::floor(pos.z - 2.8f);
    int maxBz = (int)std::floor(pos.z + 2.8f);

    for (int y = minBy; y <= maxBy; ++y) {
        for (int x = minBx; x <= maxBx; ++x) {
            for (int z = minBz; z <= maxBz; ++z) {
                float distSq = (Vec3((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f) - pos).lengthSq();
                if (distSq <= 2.8f * 2.8f) {
                    BlockType bt = world.getBlock(x, y, z);
                    if (bt != BlockType::Air && bt != BlockType::CylindricalSeaWater) {
                        const BlockInfo& info = BlockRegistry::get(bt);
                        if (info.hardness >= 0.0f && info.hardness < 50.0f) { // don't destroy indestructible monoliths
                            world.setBlock(x, y, z, BlockType::Air);
                            if (syncNetwork) {
                                NetworkManager::instance().sendBlockChange(x, y, z, BlockType::Air);
                            }
                            if (info.dropItem != ItemType::None && rand() % 3 == 0) {
                                world.spawnDroppedItem(Vec3((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f), info.dropItem, 1);
                            }
                        }
                    }
                }
            }
        }
    }

    // 2. Damage & Knockback to Local Player
    if (localPlayer) {
        Vec3 toPlayer = localPlayer->getEyePosition() - pos;
        float d = toPlayer.length();
        if (d < radius) {
            float pct = 1.0f - (d / radius);
            float dmg = pct * maxDamage;
            Vec3 knockback = toPlayer.normalized() * (pct * 20.0f) + Vec3(0, 5.0f * pct, 0);
            localPlayer->takeDamage(dmg);
            localPlayer->addVelocity(knockback);
        }
    }

    // 3. PvP Damage & Knockback to Remote Players
    auto& remotes = NetworkManager::instance().getRemotePlayersMutable();
    for (auto& pair : remotes) {
        RemotePlayer& rp = pair.second;
        Vec3 toRemote = (rp.pos + Vec3(0, 0.9f, 0)) - pos;
        float d = toRemote.length();
        if (d < radius) {
            float pct = 1.0f - (d / radius);
            float dmg = pct * maxDamage;
            Vec3 knockback = toRemote.normalized() * (pct * 20.0f) + Vec3(0, 5.0f * pct, 0);
            NetworkManager::instance().sendDamagePvP(rp.id, dmg, knockback);
        }
    }

    // 4. Damage to Biots
    // Biots will be hit via splash damage
    for (int i = 0; i < 6; ++i) {
        Vec3 spreadPos = pos + Vec3((rand() % 100 - 50) * 0.05f * radius, 0, (rand() % 100 - 50) * 0.05f * radius);
        biots.checkBoltCollision(spreadPos, radius * 0.5f, maxDamage * 0.6f);
    }

    // 5. Broadcast explosion over network
    if (syncNetwork) {
        NetworkManager::instance().sendExplosion(pos, radius, maxDamage);
    }
}

void ProjectileManager::update(float dt, World& world, BiotManager& biots, Player& localPlayer) {
    uint8_t localId = NetworkManager::instance().getLocalPlayerId();
    auto& remotePlayers = NetworkManager::instance().getRemotePlayersMutable();

    // 1. Update Plasma Bolts (PvP & PvE)
    for (auto& b : m_bolts) {
        b.prevPos = b.pos;
        b.pos += b.vel * dt;
        b.life -= dt;

        // Trailing glowing red sparks
        if (rand() % 2 == 0) {
            spawnSparks(b.pos, Vec4(1.0f, 0.2f, 0.2f, 0.8f), 1, 1.0f);
        }

        // PvP: Check collision against Local Player (if bolt was fired by remote player)
        if (b.shooterId != 0 && b.shooterId != localId) {
            Vec3 toLocal = (localPlayer.getPosition() + Vec3(0, 0.9f, 0)) - b.pos;
            if (toLocal.lengthSq() < 0.65f * 0.65f) {
                b.life = 0.0f;
                localPlayer.takeDamage(b.damage);
                localPlayer.addVelocity(b.vel.normalized() * 8.0f);
                Vec4 sparkCol = b.isOverclocked ? Vec4(0.3f, 0.9f, 1.0f, 1.0f) : Vec4(1.0f, 0.15f, 0.15f, 1.0f);
                spawnSparks(b.pos, sparkCol, 25, 7.0f);
                continue;
            }
        }

        // PvP: Check collision against Remote Players
        bool hitRemote = false;
        for (auto& pair : remotePlayers) {
            RemotePlayer& rp = pair.second;
            if (b.shooterId == rp.id) continue; // Don't hit self

            Vec3 toRemote = (rp.pos + Vec3(0, 0.9f, 0)) - b.pos;
            if (toRemote.lengthSq() < 0.75f * 0.75f) {
                b.life = 0.0f;
                hitRemote = true;
                Vec4 sparkCol = b.isOverclocked ? Vec4(0.3f, 0.9f, 1.0f, 1.0f) : Vec4(1.0f, 0.15f, 0.15f, 1.0f);
                spawnSparks(b.pos, sparkCol, 22, 6.5f);
                AudioSystem::instance().playSound(SoundEffect::PlayerHurt);

                // Send Damage over LAN
                NetworkManager::instance().sendDamagePvP(rp.id, b.damage, b.vel.normalized() * 10.0f);
                break;
            }
        }
        if (hitRemote) continue;

        // Check Biot collision
        if (biots.checkBoltCollision(b.pos, 0.7f, b.damage)) {
            b.life = 0.0f;
            Vec4 sparkCol = b.isOverclocked ? Vec4(0.3f, 0.9f, 1.0f, 1.0f) : Vec4(1.0f, 0.15f, 0.15f, 1.0f);
            spawnSparks(b.pos, sparkCol, 22, 6.0f);
            continue;
        }

        // Check World Voxel collision
        int bx = (int)std::floor(b.pos.x);
        int by = (int)std::floor(b.pos.y);
        int bz = (int)std::floor(b.pos.z);
        BlockType bt = world.getBlock(bx, by, bz);
        if (bt != BlockType::Air && bt != BlockType::CylindricalSeaWater) {
            b.life = 0.0f;
            Vec4 sparkCol = b.isOverclocked ? Vec4(0.2f, 0.8f, 1.0f, 1.0f) : Vec4(1.0f, 0.2f, 0.1f, 1.0f);
            spawnSparks(b.pos, sparkCol, 16, 5.0f);
            AudioSystem::instance().playSound(SoundEffect::BlockBreak);
        }
    }

    m_bolts.erase(
        std::remove_if(m_bolts.begin(), m_bolts.end(), [](const PlasmaBolt& b) { return b.life <= 0.0f; }),
        m_bolts.end()
    );

    // 2. Update Grenades (Bouncing Physics & Fuse)
    for (auto& g : m_grenades) {
        g.prevPos = g.pos;
        g.vel.y -= 18.0f * dt; // Gravity
        g.vel.x *= 0.99f;
        g.vel.z *= 0.99f;

        Vec3 nextPos = g.pos + g.vel * dt;

        // Voxel collision bounce test
        int bx = (int)std::floor(nextPos.x);
        int by = (int)std::floor(nextPos.y);
        int bz = (int)std::floor(nextPos.z);
        BlockType bt = world.getBlock(bx, by, bz);

        if (BlockRegistry::get(bt).isSolid) {
            // Reflect velocity with damping
            g.vel = g.vel * (-0.52f);
            AudioSystem::instance().playSound(SoundEffect::GrenadeBounce);
            spawnSparks(g.pos, Vec4(1.0f, 0.5f, 0.1f, 0.9f), 4, 2.0f);
        } else {
            g.pos = nextPos;
        }

        g.pos.x = std::fmod(g.pos.x + World::CIRCUMFERENCE, World::CIRCUMFERENCE);

        // LED blinking & smoke trail
        g.fuse -= dt;
        float blinkRate = (g.fuse < 0.8f) ? 0.08f : 0.25f;
        if (std::fmod(g.fuse, blinkRate) < dt * 1.5f) {
            spawnSparks(g.pos, Vec4(1.0f, 0.1f, 0.1f, 1.0f), 2, 0.8f);
        }

        // Detonation
        if (g.fuse <= 0.0f) {
            triggerExplosion(g.pos, 5.5f, 95.0f, world, biots, &localPlayer, true);
        }
    }

    m_grenades.erase(
        std::remove_if(m_grenades.begin(), m_grenades.end(), [](const Grenade& g) { return g.fuse <= 0.0f; }),
        m_grenades.end()
    );

    // 3. Update Laser Tracers
    for (auto& lt : m_tracers) {
        lt.life -= dt;
    }
    m_tracers.erase(
        std::remove_if(m_tracers.begin(), m_tracers.end(), [](const LaserTracer& lt) { return lt.life <= 0.0f; }),
        m_tracers.end()
    );

    // 4. Update Particles
    for (auto& p : m_particles) {
        p.pos += p.vel * dt;
        p.life -= dt;
        p.vel.y -= 4.0f * dt;
    }
    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& p) { return p.life <= 0.0f; }),
        m_particles.end()
    );
}

void ProjectileManager::render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    if (m_bolts.empty() && m_particles.empty() && m_tracers.empty() && m_grenades.empty()) return;

    struct ParticleVertex {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };

    std::vector<ParticleVertex> verts;

    auto addBillboardQuad = [&](const Vec3& center, float size, int tx, int ty) {
        float h = size * 0.5f;
        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
        TextureAtlas::instance().getTileUVs(tx, ty, u0, v0, u1, v1);

        Vec3 p0 = center + Vec3(-h, -h, 0);
        Vec3 p1 = center + Vec3(h, -h, 0);
        Vec3 p2 = center + Vec3(h, h, 0);
        Vec3 p3 = center + Vec3(-h, h, 0);

        verts.push_back({p0, {u0, v1}, {0, 1, 0}});
        verts.push_back({p1, {u1, v1}, {0, 1, 0}});
        verts.push_back({p2, {u1, v0}, {0, 1, 0}});

        verts.push_back({p0, {u0, v1}, {0, 1, 0}});
        verts.push_back({p2, {u1, v0}, {0, 1, 0}});
        verts.push_back({p3, {u0, v0}, {0, 1, 0}});
    };

    auto addLaserBeamSegment = [&](const Vec3& start, const Vec3& end, float width) {
        Vec3 dir = (end - start);
        float len = dir.length();
        if (len < 0.001f) return;
        dir = dir.normalized();

        Vec3 side = Vec3(-dir.z, 0, dir.x).normalized() * (width * 0.5f);
        if (side.lengthSq() < 0.001f) side = Vec3(width * 0.5f, 0, 0);

        float u0 = 0.0f, v0 = 0.0f, u1 = 1.0f, v1 = 1.0f;
        TextureAtlas::instance().getTileUVs(15, 0, u0, v0, u1, v1);

        Vec3 p0 = start - side;
        Vec3 p1 = start + side;
        Vec3 p2 = end + side;
        Vec3 p3 = end - side;

        verts.push_back({p0, {u0, v1}, {0, 1, 0}});
        verts.push_back({p1, {u1, v1}, {0, 1, 0}});
        verts.push_back({p2, {u1, v0}, {0, 1, 0}});

        verts.push_back({p0, {u0, v1}, {0, 1, 0}});
        verts.push_back({p2, {u1, v0}, {0, 1, 0}});
        verts.push_back({p3, {u0, v0}, {0, 1, 0}});
    };

    // 1. Render Plasma Bolts
    for (const auto& b : m_bolts) {
        Vec3 tail = b.pos - b.vel.normalized() * 1.4f;
        addLaserBeamSegment(tail, b.pos, b.isOverclocked ? 0.28f : 0.20f);
        addBillboardQuad(b.pos, b.isOverclocked ? 0.45f : 0.35f, 15, 0);
    }

    // 2. Render Grenades (3D spherical canister with flashing LED)
    for (const auto& g : m_grenades) {
        addBillboardQuad(g.pos, 0.38f, 9, 3); // Grenade icon tile
        // Blinking red detonation LED
        if (std::fmod(g.fuse, 0.2f) < 0.1f) {
            addBillboardQuad(g.pos + Vec3(0, 0.12f, 0), 0.18f, 15, 0);
        }
    }

    // 3. Render Lingering Laser Tracers
    for (const auto& lt : m_tracers) {
        float alpha = lt.life / lt.maxLife;
        addLaserBeamSegment(lt.start, lt.end, 0.12f * alpha);
    }

    // 4. Render Sparks, Fireballs & Particles
    for (const auto& p : m_particles) {
        addBillboardQuad(p.pos, p.size, 15, 0);
    }

    if (verts.empty()) return;

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", World::CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", 1.0f);
    shader.setMat4("uModel", Mat4::identity());
    shader.setVec4("uTint", Vec4(1.0f, 0.3f, 0.2f, 1.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Glowing additive plasma blend

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(ParticleVertex), verts.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex), (void*)offsetof(ParticleVertex, normal));

    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

    glBindVertexArray(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_BLEND);
}
