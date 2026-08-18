#pragma once

#include <vector>
#include "Math3D.hpp"
#include "Block.hpp"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class World;
class Shader;
class Player;

struct Biot {
    Vec3 pos;
    Vec3 vel;
    float yaw = 0.0f;
    float health = 60.0f;
    float maxHealth = 60.0f;
    float legAnim = 0.0f;
    float attackCooldown = 0.0f;
    bool isAlive = true;
    AABB getAABB() const {
        return AABB(pos - Vec3(0.5f, 0.0f, 0.5f), pos + Vec3(0.5f, 0.8f, 0.5f));
    }
};

class BiotManager {
public:
    static BiotManager& instance();

    void init();
    void cleanup();

    void spawnBiot(const Vec3& pos);
    void update(float dt, World& world, Player& player);
    void render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    bool checkBoltCollision(const Vec3& boltPos, float radius, float damage);
    std::vector<Biot>& getBiots() { return m_biots; }

private:
    BiotManager() = default;

    std::vector<Biot> m_biots;
    float m_spawnTimer = 0.0f;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLsizei m_vertexCount = 0;

    void buildBeetleMesh();
};
