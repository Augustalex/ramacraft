#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include "Math3D.hpp"
#include "Block.hpp"

class Chunk;
class Shader;

struct PointLight {
    Vec3 pos;
    Vec3 color;
    float radius = 10.0f;
};

class World {
public:
    static constexpr float CIRCUMFERENCE = 1536.0f;         // 1536 blocks around circumference (96 chunks x 16)
    static constexpr float CYLINDER_RADIUS = 244.46199f;    // True 2x cylinder radius R = CIRCUMFERENCE / (2 * PI)
    static constexpr int CHUNKS_X = 96;                     // 96 chunks form the colossal 360-degree ring
    static constexpr int RENDER_DISTANCE_Z = 16;            // 33 chunks along length of Rama (528m render distance!)

    World();
    ~World();

    void init();
    void update(float dt, const Vec3& playerPos);
    void render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);
    void renderTransparent(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);
    void renderRamaSpindleAndSuns(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    BlockType getBlock(int x, int y, int z);
    void setBlock(int x, int y, int z, BlockType type);

    bool raycast(const Ray& ray, float maxDist, Vec3i& outBlock, Vec3i& outNormal, float& outDist);

    // Dynamic light management
    void addTorch(const Vec3i& pos);
    void removeTorch(const Vec3i& pos);
    const std::vector<PointLight>& getPointLights() const { return m_pointLights; }

    // Day/Night & Rama Sun cycle
    float getTimeOfDay() const { return m_timeOfDay; }
    void setTimeOfDay(float t) { m_timeOfDay = t; }
    bool isDay() const { return m_timeOfDay < 0.5f; }
    bool isNight() const { return !isDay(); }
    float getSunIntensity() const { return m_sunIntensity; }
    Vec3 getSunColor() const { return m_sunColor; }
    Vec3 getSunDir() const { return m_sunDir; }
    Vec3 getAmbientColor() const { return m_ambientColor; }
    Vec3 getFogColor() const { return m_fogColor; }
    float getFogDensity() const { return m_fogDensity; }

    // Curvature toggle
    bool isCurvatureEnabled() const { return m_curvatureEnabled; }
    void toggleCurvature() { m_curvatureEnabled = !m_curvatureEnabled; }

    Chunk* getChunk(int cx, int cz);

    // Drop item spawning on break
    void spawnDroppedItem(const Vec3& pos, ItemType item, int count);

private:
    std::unordered_map<Vec3i, std::unique_ptr<Chunk>, Vec3i::Hash> m_chunks;
    std::vector<PointLight> m_pointLights;
    std::vector<Vec3i> m_torches;

    float m_timeOfDay = 0.15f; // 0..1 (starts in morning)
    float m_dayDuration = 180.0f; // 3 minutes per Rama full cycle

    float m_sunIntensity = 1.0f;
    Vec3 m_sunColor = {1.0f, 0.95f, 0.85f};
    Vec3 m_sunDir = {0.0f, 1.0f, 0.0f};
    Vec3 m_ambientColor = {0.15f, 0.18f, 0.22f};
    Vec3 m_fogColor = {0.05f, 0.08f, 0.12f};
    float m_fogDensity = 0.015f;
    bool m_curvatureEnabled = true;

    void updateEnvironment(float dt);
    void generateTerrain(Chunk* chunk);
    void generateAncientRuins(Chunk* chunk, int cx, int cz, int groundY);
    void generateMachineryComplex(Chunk* chunk, int cx, int cz, int groundY);
    void generateCavesAndMinerals(Chunk* chunk);
};
