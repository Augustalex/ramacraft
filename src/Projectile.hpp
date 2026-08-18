#pragma once

#include <vector>
#include "Math3D.hpp"
#include "Block.hpp"

#include "GLCommon.hpp"

class Shader;
class World;
class BiotManager;

struct PlasmaBolt {
    Vec3 pos;
    Vec3 prevPos;
    Vec3 vel;
    float life = 3.0f;
    float damage = 25.0f;
    bool isOverclocked = false;
    uint8_t shooterId = 0;
};

struct Grenade {
    Vec3 pos;
    Vec3 prevPos;
    Vec3 vel;
    float fuse = 2.4f;
    float maxFuse = 2.4f;
    uint8_t throwerId = 0;
};

struct LaserTracer {
    Vec3 start;
    Vec3 end;
    float life = 0.25f;
    float maxLife = 0.25f;
    Vec4 color = Vec4(1.0f, 0.15f, 0.15f, 0.9f);
};

struct Particle {
    Vec3 pos;
    Vec3 vel;
    Vec4 color;
    float size = 0.1f;
    float life = 0.5f;
    float maxLife = 0.5f;
};

class Player;

class ProjectileManager {
public:
    static ProjectileManager& instance();

    void init();
    void cleanup();

    void spawnBolt(const Vec3& pos, const Vec3& dir, float speed = 36.0f, float damage = 25.0f, bool overclocked = false, uint8_t shooterId = 0);
    void spawnGrenade(const Vec3& pos, const Vec3& vel, uint8_t throwerId = 0);
    void triggerExplosion(const Vec3& pos, float radius, float maxDamage, World& world, BiotManager& biots, Player* localPlayer = nullptr, bool syncNetwork = true);

    void spawnLaserTracer(const Vec3& start, const Vec3& end, const Vec4& color = Vec4(1.0f, 0.15f, 0.15f, 0.95f), float duration = 0.25f);
    void spawnSparks(const Vec3& pos, const Vec4& color, int count = 12, float speed = 4.0f);
    void spawnJetpackExhaust(const Vec3& pos, const Vec3& dir);
    void spawnExplosionParticles(const Vec3& pos, float radius);

    void update(float dt, World& world, BiotManager& biots, Player& localPlayer);
    void render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    const std::vector<PlasmaBolt>& getBolts() const { return m_bolts; }
    const std::vector<Grenade>& getGrenades() const { return m_grenades; }

private:
    ProjectileManager() = default;

    std::vector<PlasmaBolt> m_bolts;
    std::vector<Grenade> m_grenades;
    std::vector<LaserTracer> m_tracers;
    std::vector<Particle> m_particles;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    void rebuildMesh();
};
