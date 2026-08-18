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
class Player;
class Shader;

struct ItemEntity {
    Vec3 pos;
    Vec3 vel;
    ItemType item = ItemType::None;
    int count = 1;
    float rotation = 0.0f;
    float age = 0.0f;
    bool isAlive = true;
};

class ItemEntityManager {
public:
    static ItemEntityManager& instance();

    void init();
    void cleanup();

    void spawnItem(const Vec3& pos, ItemType item, int count = 1);
    void update(float dt, World& world, Player& player);
    void render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    void clearAll() { m_items.clear(); }
    const std::vector<ItemEntity>& getItems() const { return m_items; }

private:
    ItemEntityManager() = default;

    std::vector<ItemEntity> m_items;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLsizei m_vertexCount = 0;

    void buildItemMesh();
};
