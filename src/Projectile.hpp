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

class ProjectileManager {
public:
    static ProjectileManager& instance();

    void init();
    void cleanup();

    void spawnBolt(const Vec3& pos, const Vec3& dir, float speed = 36.0f, float damage = 25.0f, bool overclocked = false);
    void spawnLaserTracer(const Vec3& start, const Vec3& end, const Vec4& color = Vec4(1.0f, 0.15f, 0.15f, 0.95f), float duration = 0.25f);
    void spawnSparks(const Vec3& pos, const Vec4& color, int count = 12, float speed = 4.0f);
    void spawnJetpackExhaust(const Vec3& pos, const Vec3& dir);

    void update(float dt, World& world, BiotManager& biots);
    void render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    const std::vector<PlasmaBolt>& getBolts() const { return m_bolts; }

private:
    ProjectileManager() = default;

    std::vector<PlasmaBolt> m_bolts;
    std::vector<LaserTracer> m_tracers;
    std::vector<Particle> m_particles;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;

    void rebuildMesh();
};
