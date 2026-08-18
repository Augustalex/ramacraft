#include "Projectile.hpp"
#include "World.hpp"
#include "Biot.hpp"
#include "Shader.hpp"
#include "Audio.hpp"
#include "TextureAtlas.hpp"
#include <cmath>
#include <random>

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
}

void ProjectileManager::spawnBolt(const Vec3& pos, const Vec3& dir, float speed, float damage, bool overclocked) {
    PlasmaBolt b;
    b.pos = pos;
    b.prevPos = pos;
    b.vel = dir.normalized() * speed;
    b.life = 2.5f;
    b.damage = damage;
    b.isOverclocked = overclocked;
    m_bolts.push_back(b);

    // Lingering red laser beam tracer
    spawnLaserTracer(pos, pos + dir.normalized() * 18.0f, Vec4(1.0f, 0.15f, 0.15f, 0.95f), 0.22f);

    AudioSystem::instance().playSound(SoundEffect::LaserFire);
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

void ProjectileManager::update(float dt, World& world, BiotManager& biots) {
    // 1. Update Plasma Bolts
    for (auto& b : m_bolts) {
        b.prevPos = b.pos;
        b.pos += b.vel * dt;
        b.life -= dt;

        // Trailing glowing red sparks
        if (rand() % 2 == 0) {
            spawnSparks(b.pos, Vec4(1.0f, 0.2f, 0.2f, 0.8f), 1, 1.0f);
        }

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

    // Remove expired bolts
    m_bolts.erase(
        std::remove_if(m_bolts.begin(), m_bolts.end(), [](const PlasmaBolt& b) { return b.life <= 0.0f; }),
        m_bolts.end()
    );

    // 2. Update Laser Tracers
    for (auto& lt : m_tracers) {
        lt.life -= dt;
    }
    m_tracers.erase(
        std::remove_if(m_tracers.begin(), m_tracers.end(), [](const LaserTracer& lt) { return lt.life <= 0.0f; }),
        m_tracers.end()
    );

    // 3. Update Particles
    for (auto& p : m_particles) {
        p.pos += p.vel * dt;
        p.life -= dt;
        p.vel.y -= 3.5f * dt; // low gravity falling
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(), [](const Particle& p) { return p.life <= 0.0f; }),
        m_particles.end()
    );
}

void ProjectileManager::render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    if (m_bolts.empty() && m_particles.empty() && m_tracers.empty()) return;

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
        TextureAtlas::instance().getTileUVs(15, 0, u0, v0, u1, v1); // Glowing beacon texture

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

    // 1. Render Red Laser Bolts (with visible elongated beam & head flare)
    for (const auto& b : m_bolts) {
        Vec3 tail = b.pos - b.vel.normalized() * 1.4f;
        addLaserBeamSegment(tail, b.pos, b.isOverclocked ? 0.28f : 0.20f);
        addBillboardQuad(b.pos, b.isOverclocked ? 0.45f : 0.35f, 15, 0);
    }

    // 2. Render Lingering Laser Tracers
    for (const auto& lt : m_tracers) {
        float alpha = lt.life / lt.maxLife;
        addLaserBeamSegment(lt.start, lt.end, 0.12f * alpha);
    }

    // 3. Render Sparks & Flames
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
    shader.setVec4("uTint", Vec4(1.0f, 0.2f, 0.2f, 1.0f)); // Bright vibrant ruby red tint!

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
