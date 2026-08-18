#include "World.hpp"
#include "Chunk.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "Audio.hpp"
#include "ItemEntity.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

// Simple deterministic noise for terrain generation
static float pseudoNoise(float x, float z) {
    float n = std::sin(x * 0.08f + z * 0.05f) * 1.5f +
              std::cos(x * 0.03f - z * 0.09f) * 2.2f +
              std::sin(x * 0.15f + z * 0.12f) * 0.8f;
    return n;
}

static float hash2D(int x, int z) {
    int n = x * 374761393 + z * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;
    return (float)(n & 0x7fffffff) / (float)0x7fffffff;
}

static inline int floorDiv(int a, int b) {
    int res = a / b;
    int rem = a % b;
    if (rem != 0 && ((a < 0) ^ (b < 0))) {
        res--;
    }
    return res;
}

static inline int floorMod(int a, int b) {
    int rem = a % b;
    if (rem < 0) rem += b;
    return rem;
}

World::World() {
}

World::~World() {
}

void World::init() {
    // Generate initial 360-degree closed cylinder (around player spawn Z)
    for (int cx = 0; cx < CHUNKS_X; ++cx) {
        for (int cz = -2; cz <= 6; ++cz) {
            getChunk(cx, cz);
        }
    }

    // Build initial meshes
    for (auto& pair : m_chunks) {
        pair.second->rebuildMesh();
    }
}

Chunk* World::getChunk(int cx, int cz) {
    int wrappedCx = floorMod(cx, CHUNKS_X);
    Vec3i key(wrappedCx, 0, cz);
    auto it = m_chunks.find(key);
    if (it != m_chunks.end()) {
        return it->second.get();
    }

    auto chunk = std::make_unique<Chunk>(wrappedCx, cz, this);
    generateTerrain(chunk.get());
    Chunk* ptr = chunk.get();
    m_chunks[key] = std::move(chunk);
    return ptr;
}

BlockType World::getBlock(int x, int y, int z) {
    if (y < 0 || y >= CHUNK_SIZE_Y) return BlockType::Air;

    int totalCirc = (int)CIRCUMFERENCE; // 1536
    int wrappedX = floorMod(x, totalCirc);
    int cz = floorDiv(z, CHUNK_SIZE_Z);
    int cx = wrappedX / CHUNK_SIZE_X;

    Vec3i key(cx, 0, cz);
    auto it = m_chunks.find(key);
    if (it == m_chunks.end()) {
        return BlockType::Air;
    }

    int lx = wrappedX % CHUNK_SIZE_X;
    int lz = floorMod(z, CHUNK_SIZE_Z);
    return it->second->getBlock(lx, y, lz);
}

void World::setBlock(int x, int y, int z, BlockType type) {
    if (y < 0 || y >= CHUNK_SIZE_Y) return;

    int totalCirc = (int)CIRCUMFERENCE; // 1536
    int wrappedX = floorMod(x, totalCirc);
    int cz = floorDiv(z, CHUNK_SIZE_Z);
    int cx = wrappedX / CHUNK_SIZE_X;

    Chunk* chunk = getChunk(cx, cz);
    int lx = wrappedX % CHUNK_SIZE_X;
    int lz = floorMod(z, CHUNK_SIZE_Z);
    chunk->setBlock(lx, y, lz, type);

    // If block is on chunk border, mark neighbor chunks dirty too
    if (lx == 0) { Chunk* nb = getChunk(cx - 1, cz); if (nb) nb->markDirty(); }
    if (lx == CHUNK_SIZE_X - 1) { Chunk* nb = getChunk(cx + 1, cz); if (nb) nb->markDirty(); }
    if (lz == 0) { Chunk* nb = getChunk(cx, cz - 1); if (nb) nb->markDirty(); }
    if (lz == CHUNK_SIZE_Z - 1) { Chunk* nb = getChunk(cx, cz + 1); if (nb) nb->markDirty(); }
}

void World::addTorch(const Vec3i& pos) {
    m_torches.push_back(pos);
}

void World::removeTorch(const Vec3i& pos) {
    m_torches.erase(std::remove(m_torches.begin(), m_torches.end(), pos), m_torches.end());
}

void World::generateTerrain(Chunk* chunk) {
    int cx = chunk->getChunkX();
    int cz = chunk->getChunkZ();

    int worldZBase = cz * CHUNK_SIZE_Z;
    int worldXBase = cx * CHUNK_SIZE_X;

    for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
            int gx = worldXBase + lx;
            int gz = worldZBase + lz;

            // -------------------------------------------------------------
            // Authentic Rama Longitudinal Z-Zones:
            // -------------------------------------------------------------
            
            // Zone 0: North Endcap Wall & Alpha Ladder Cliff (gz <= 0)
            if (gz <= 0) {
                // North polar endcap wall: colossal solid wall rising to the hub
                int wallBase = (gz < -4) ? 63 : (16 + std::abs(gz) * 6);
                wallBase = std::min(63, wallBase);

                for (int y = 0; y <= wallBase; ++y) {
                    chunk->setBlock(lx, y, lz, (gz < -6) ? BlockType::DarkMonolith : BlockType::RamaHullAlloy);
                }

                // Alpha Central Ladder & Stepped Stairway (gx around 0, descending from wall to plain)
                if (gx >= -4 && gx <= 4 && gz >= -8 && gz <= 0) {
                    int stepLevel = 16 + (0 - gz) * 4;
                    for (int y = 0; y <= stepLevel && y < 64; ++y) {
                        chunk->setBlock(lx, y, lz, BlockType::DarkMonolith);
                    }
                    // Ladder center shaft
                    if (gx == 0) {
                        for (int y = 16; y <= 60; ++y) {
                            chunk->setBlock(lx, y, lz, BlockType::RamaHullAlloy);
                        }
                    }
                    // Landing torches / beacons
                    if ((gx == -3 || gx == 3) && gz % 2 == 0 && stepLevel + 1 < 64) {
                        chunk->setBlock(lx, stepLevel + 1, lz, BlockType::Torch);
                    }
                }
            }
            // Zone 2: The Cylindrical Sea (gz in [110, 175])
            else if (gz >= 110 && gz <= 175) {
                int seaBed = 7 + (int)(std::sin(gx * 0.12f) * 2.0f);
                int seaLevel = 13;

                // Central Island ("New York" Island in the book: gz in [138, 148], gx in [-12, 12])
                bool isIsland = (gz >= 138 && gz <= 148 && gx >= -12 && gx <= 12);

                for (int y = 0; y <= 4; ++y) {
                    chunk->setBlock(lx, y, lz, BlockType::RamaHullAlloy);
                }

                if (isIsland) {
                    int islandH = 15 + (int)(hash2D(gx, gz) * 4.0f);
                    for (int y = 5; y < islandH - 2; ++y) {
                        chunk->setBlock(lx, y, lz, BlockType::NaturalStone);
                    }
                    for (int y = islandH - 2; y < islandH; ++y) {
                        chunk->setBlock(lx, y, lz, BlockType::Dirt);
                    }
                    chunk->setBlock(lx, islandH, lz, BlockType::GrassBlock);
                    if (hash2D(gx * 3, gz * 7) > 0.7f) {
                        chunk->setBlock(lx, islandH + 1, lz, BlockType::CobaltCrystal);
                    }
                } else {
                    for (int y = 5; y <= seaBed; ++y) {
                        chunk->setBlock(lx, y, lz, BlockType::Sand);
                    }
                    if (hash2D(gx, gz) > 0.93f) {
                        chunk->setBlock(lx, seaBed + 1, lz, BlockType::CobaltCrystal);
                    }
                    for (int y = seaBed + 1; y <= seaLevel; ++y) {
                        if (chunk->getBlock(lx, y, lz) == BlockType::Air) {
                            chunk->setBlock(lx, y, lz, BlockType::CylindricalSeaWater);
                        }
                    }
                }
            }
            // Zone 1: Northern Natural Wilderness Plain (gz in [1, 109])
            // & Zone 3: Southern Plains (gz > 175)
            else {
                float noise = pseudoNoise((float)gx, (float)gz);
                int baseHeight = 15 + (int)(noise * 0.9f);

                // Bedrock Hull
                for (int y = 0; y <= 3; ++y) {
                    chunk->setBlock(lx, y, lz, BlockType::RamaHullAlloy);
                }
                // Deep Natural Stone stratum
                for (int y = 4; y < std::max(4, baseHeight - 3); ++y) {
                    chunk->setBlock(lx, y, lz, BlockType::NaturalStone);
                }
                // Rich dark soil
                for (int y = std::max(4, baseHeight - 3); y < baseHeight; ++y) {
                    chunk->setBlock(lx, y, lz, BlockType::Dirt);
                }

                // Surface layer: Beaches near sea, lush grass in wilderness
                if (gz >= 104 && gz <= 109) {
                    // Golden Sand beach
                    chunk->setBlock(lx, baseHeight, lz, BlockType::Sand);
                } else if (gz > 175 && gz <= 180) {
                    chunk->setBlock(lx, baseHeight, lz, BlockType::Sand);
                } else if (gz > 180) {
                    // South hemisphere near ancient cities
                    float h = hash2D(gx, gz);
                    chunk->setBlock(lx, baseHeight, lz, (h > 0.4f) ? BlockType::AlienRuinWall : BlockType::RamaHullAlloy);
                } else {
                    // Northern Wilderness: Natural rich emerald grass
                    float h = hash2D(gx, gz);
                    if (h > 0.88f) {
                        chunk->setBlock(lx, baseHeight, lz, BlockType::BioluminescentMoss);
                    } else {
                        chunk->setBlock(lx, baseHeight, lz, BlockType::GrassBlock);
                    }

                    // Steam vents
                    if (h > 0.988f && gz > 15 && gz < 95) {
                        chunk->setBlock(lx, baseHeight, lz, BlockType::SteamVent);
                    }

                    // Natural Earth/Rama Tree groves
                    float treeNoise = hash2D(gx * 37 + 13, gz * 43 + 19);
                    if (treeNoise > 0.965f && gz > 6 && gz < 102 && lx >= 2 && lx < CHUNK_SIZE_X - 2 && lz >= 2 && lz < CHUNK_SIZE_Z - 2) {
                        int trunkH = 4 + (int)(hash2D(gx, gz) * 3.0f);
                        // Trunk
                        for (int ty = 1; ty <= trunkH; ++ty) {
                            chunk->setBlock(lx, baseHeight + ty, lz, BlockType::WoodLog);
                        }
                        // Foliage Canopy
                        for (int ox = -2; ox <= 2; ++ox) {
                            for (int oz = -2; oz <= 2; ++oz) {
                                for (int oy = trunkH - 1; oy <= trunkH + 2; ++oy) {
                                    if (std::abs(ox) == 2 && std::abs(oz) == 2 && oy == trunkH + 2) continue;
                                    if (chunk->getBlock(lx + ox, baseHeight + oy, lz + oz) == BlockType::Air) {
                                        chunk->setBlock(lx + ox, baseHeight + oy, lz + oz, BlockType::FoliageLeaves);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Sub-surface mineral veins & caverns
    generateCavesAndMinerals(chunk);

    // Ancient Alien Cities & Monolith Complexes (STRICTLY SOUTH OF THE CYLINDER SEA: gz >= 180)
    int avgZ = cz * CHUNK_SIZE_Z + 8;
    if (avgZ >= 180) {
        float cityChance = hash2D(cx * 13, cz * 19);
        if (cityChance > 0.55f) {
            generateAncientRuins(chunk, cx, cz, 16);
        } else if (cityChance > 0.35f) {
            generateMachineryComplex(chunk, cx, cz, 16);
        }
    }
}

void World::generateCavesAndMinerals(Chunk* chunk) {
    int cx = chunk->getChunkX();
    int cz = chunk->getChunkZ();

    for (int lx = 0; lx < CHUNK_SIZE_X; ++lx) {
        for (int lz = 0; lz < CHUNK_SIZE_Z; ++lz) {
            int gx = cx * CHUNK_SIZE_X + lx;
            int gz = cz * CHUNK_SIZE_Z + lz;

            for (int y = 4; y < 14; ++y) {
                float n3d = std::sin(gx * 0.2f) * std::cos(y * 0.3f) + std::sin(gz * 0.2f) * std::cos(gx * 0.15f);
                if (n3d > 1.25f) {
                    // Cave pocket
                    chunk->setBlock(lx, y, lz, BlockType::Air);
                } else if (chunk->getBlock(lx, y, lz) == BlockType::RamaHullAlloy || chunk->getBlock(lx, y, lz) == BlockType::NaturalStone) {
                    // Mineral Veins
                    float r = hash2D(gx * 19 + y * 7, gz * 23 + y * 13);
                    if (r > 0.95f) {
                        chunk->setBlock(lx, y, lz, BlockType::CobaltCrystal);
                    } else if (r > 0.88f) {
                        chunk->setBlock(lx, y, lz, BlockType::TitaniumOre);
                    } else if (r > 0.82f) {
                        chunk->setBlock(lx, y, lz, BlockType::CarboniteBlock);
                    }
                }
            }
        }
    }
}

void World::generateAncientRuins(Chunk* chunk, int cx, int cz, int groundY) {
    // Generate towering geometric Rama alien monoliths and stepped temple complexes
    int startX = 2;
    int startZ = 2;
    int towerHeight = 12 + (int)(hash2D(cx * 3, cz * 7) * 14.0f);

    for (int y = groundY; y < groundY + towerHeight; ++y) {
        for (int x = startX; x < startX + 10; ++x) {
            for (int z = startZ; z < startZ + 10; ++z) {
                // Hollow tower walls
                bool isWall = (x == startX || x == startX + 9 || z == startZ || z == startZ + 9);
                bool isFloor = ((y - groundY) % 5 == 0);
                
                if (isWall) {
                    if (y % 4 == 2 && (x == startX + 4 || x == startX + 5 || z == startZ + 4 || z == startZ + 5)) {
                        chunk->setBlock(x, y, z, BlockType::ReinforcedGlass); // Viewport windows
                    } else {
                        chunk->setBlock(x, y, z, (y % 6 == 0) ? BlockType::DarkMonolith : BlockType::AlienRuinWall);
                    }
                } else if (isFloor) {
                    chunk->setBlock(x, y, z, BlockType::DarkMonolith);
                } else {
                    // Interior ruin artifacts & ancient machines
                    if ((y - groundY) % 5 == 1 && x == startX + 4 && z == startZ + 4) {
                        chunk->setBlock(x, y, z, BlockType::AncientMachinery);
                    }
                }
            }
        }
    }

    // Ancient obelisk beside tower
    int obX = startX + 12, obZ = startZ + 12;
    if (obX < CHUNK_SIZE_X - 1 && obZ < CHUNK_SIZE_Z - 1) {
        for (int y = groundY; y < groundY + 18; ++y) {
            chunk->setBlock(obX, y, obZ, BlockType::DarkMonolith);
        }
    }
}

void World::generateMachineryComplex(Chunk* chunk, int cx, int cz, int groundY) {
    // Generate industrial alien machinery hubs with broken reactors and consoles
    int startX = 3, startZ = 3;

    // Machinery platform
    for (int x = startX; x < startX + 8; ++x) {
        for (int z = startZ; z < startZ + 8; ++z) {
            chunk->setBlock(x, groundY, z, BlockType::RamaHullAlloy);

            // Machinery racks
            if (x == startX + 1 || x == startX + 6) {
                chunk->setBlock(x, groundY + 1, z, BlockType::AncientMachinery);
                if (z % 2 == 0) {
                    chunk->setBlock(x, groundY + 2, z, BlockType::AncientMachinery);
                }
            }
        }
    }

    // Center broken reactor core
    chunk->setBlock(startX + 3, groundY + 1, startZ + 3, BlockType::BrokenReactor);
    chunk->setBlock(startX + 4, groundY + 1, startZ + 3, BlockType::BrokenReactor);
    chunk->setBlock(startX + 3, groundY + 1, startZ + 4, BlockType::BrokenReactor);
    chunk->setBlock(startX + 4, groundY + 1, startZ + 4, BlockType::BrokenReactor);

    // Steam vents
    chunk->setBlock(startX + 1, groundY, startZ + 1, BlockType::SteamVent);
    chunk->setBlock(startX + 6, groundY, startZ + 6, BlockType::SteamVent);
}

void World::update(float dt, const Vec3& playerPos) {
    // Advance Day/Night Cycle
    float prevTime = m_timeOfDay;
    m_timeOfDay += dt / m_dayDuration;
    if (m_timeOfDay >= 1.0f) m_timeOfDay -= 1.0f;

    // Siren chime on Day/Night transition
    if ((prevTime < 0.5f && m_timeOfDay >= 0.5f) || (prevTime < 0.98f && m_timeOfDay < 0.05f)) {
        AudioSystem::instance().playSound(SoundEffect::AlarmSiren);
    }

    updateEnvironment(dt);

    int playerChunkZ = floorDiv((int)std::floor(playerPos.z), CHUNK_SIZE_Z);

    // 1. Unload distant chunks
    std::vector<Vec3i> toErase;
    for (auto& pair : m_chunks) {
        if (std::abs(pair.first.z - playerChunkZ) > RENDER_DISTANCE_Z + 4) {
            toErase.push_back(pair.first);
        }
    }
    for (const auto& key : toErase) {
        m_chunks.erase(key);
    }

    // 2. Load visible chunks
    for (int cx = 0; cx < CHUNKS_X; ++cx) {
        for (int cz = playerChunkZ - RENDER_DISTANCE_Z; cz <= playerChunkZ + RENDER_DISTANCE_Z; ++cz) {
            getChunk(cx, cz);
        }
    }

    // 3. Collect dirty chunk pointers before rebuilding to avoid iterator invalidation
    std::vector<Chunk*> dirtyChunks;
    for (auto& pair : m_chunks) {
        if (pair.second->isDirty()) {
            dirtyChunks.push_back(pair.second.get());
        }
    }

    int rebuilt = 0;
    for (Chunk* chunk : dirtyChunks) {
        chunk->rebuildMesh();
        if (++rebuilt >= 32) break;
    }

    // Update dynamic light list (torches near player)
    m_pointLights.clear();
    for (const auto& t : m_torches) {
        // Convert torch voxel coordinates to 3D Cartesian coordinates
        float theta = ((float)t.x + 0.5f) * (2.0f * PI / CIRCUMFERENCE);
        float r = CYLINDER_RADIUS - ((float)t.y + 0.5f);
        Vec3 tPos(std::sin(theta) * r, CYLINDER_RADIUS - std::cos(theta) * r, (float)t.z + 0.5f);

        float distSq = (tPos - playerPos).lengthSq();
        if (distSq < 90.0f * 90.0f && m_pointLights.size() < 16) {
            PointLight pl;
            pl.pos = tPos;
            pl.color = {1.0f, 0.75f, 0.35f};
            pl.radius = 16.0f;
            m_pointLights.push_back(pl);
        }
    }
}

void World::updateEnvironment(float dt) {
    // Rama linear sun strips lighting model
    if (isDay()) {
        // Day phase: Rama sun strips are active overhead
        float dayProgress = m_timeOfDay / 0.5f; // 0..1
        float sunPower = std::sin(dayProgress * PI);
        m_sunIntensity = 0.2f + 1.0f * sunPower;
        m_sunColor = {1.0f, 0.96f, 0.88f};
        m_ambientColor = {0.20f + 0.14f * sunPower, 0.22f + 0.16f * sunPower, 0.28f + 0.18f * sunPower};
        m_fogColor = {0.05f + 0.08f * sunPower, 0.08f + 0.11f * sunPower, 0.13f + 0.15f * sunPower};
        m_fogDensity = 0.0030f; // Far crisp visibility with atmospheric depth
    } else {
        // Night phase: Rama sun strips power off -> deep space darkness & fog
        m_sunIntensity = 0.03f;
        m_sunColor = {0.1f, 0.15f, 0.3f};
        m_ambientColor = {0.04f, 0.05f, 0.08f};
        m_fogColor = {0.012f, 0.018f, 0.032f};
        m_fogDensity = 0.0045f;
    }
}

void World::render(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", m_curvatureEnabled ? 1.0f : 0.0f);

    shader.setVec3("uSunDir", m_sunDir);
    shader.setVec3("uSunColor", m_sunColor);
    shader.setFloat("uSunIntensity", m_sunIntensity);
    shader.setVec3("uAmbientColor", m_ambientColor);
    shader.setVec3("uFogColor", m_fogColor);
    shader.setFloat("uFogDensity", m_fogDensity);

    // Pass Point lights
    shader.setInt("uNumPointLights", (int)m_pointLights.size());
    for (size_t i = 0; i < m_pointLights.size() && i < 16; ++i) {
        std::string prefix = "uPointLightPos[" + std::to_string(i) + "]";
        shader.setVec3(prefix, m_pointLights[i].pos);
        prefix = "uPointLightColor[" + std::to_string(i) + "]";
        shader.setVec3(prefix, m_pointLights[i].color);
        prefix = "uPointLightRadius[" + std::to_string(i) + "]";
        shader.setFloat(prefix, m_pointLights[i].radius);
    }

    Mat4 model = Mat4::identity();
    shader.setMat4("uModel", model);

    int playerChunkZ = floorDiv((int)std::floor(playerPos.z), CHUNK_SIZE_Z);

    // Render all chunks around the 360-degree cylinder ring
    for (int cx = 0; cx < CHUNKS_X; ++cx) {
        for (int cz = playerChunkZ - RENDER_DISTANCE_Z; cz <= playerChunkZ + RENDER_DISTANCE_Z; ++cz) {
            Vec3i key(cx, 0, cz);
            auto it = m_chunks.find(key);
            if (it != m_chunks.end()) {
                it->second->renderOpaque();
            }
        }
    }
}

void World::renderTransparent(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", World::CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", m_curvatureEnabled ? 1.0f : 0.0f);
    shader.setMat4("uModel", Mat4::identity());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    int playerChunkZ = floorDiv((int)std::floor(playerPos.z), CHUNK_SIZE_Z);

    for (int cx = 0; cx < CHUNKS_X; ++cx) {
        for (int cz = playerChunkZ - RENDER_DISTANCE_Z; cz <= playerChunkZ + RENDER_DISTANCE_Z; ++cz) {
            Vec3i key(cx, 0, cz);
            auto it = m_chunks.find(key);
            if (it != m_chunks.end()) {
                it->second->renderTransparent();
            }
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void World::spawnDroppedItem(const Vec3& pos, ItemType item, int count) {
    ItemEntityManager::instance().spawnItem(pos, item, count);
}

void World::renderRamaSpindleAndSuns(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    // 1. Render Rama Central Spindle (colossal rotational axis along Z at Y = CYLINDER_RADIUS)
    static GLuint s_spindleVAO = 0;
    static GLuint s_spindleVBO = 0;
    static int s_spindleVertCount = 0;

    if (s_spindleVAO == 0) {
        struct SpindleVertex {
            Vec3 pos;
            Vec2 uv;
            Vec3 normal;
        };
        std::vector<SpindleVertex> verts;

        // Long central hexagonal spindle along Z at Y = CYLINDER_RADIUS
        int segments = 8;
        float radius = 16.0f;
        float length = 2000.0f;

        for (int i = 0; i < segments; ++i) {
            float a0 = (float)i * (2.0f * PI / (float)segments);
            float a1 = (float)(i + 1) * (2.0f * PI / (float)segments);

            float x0 = std::cos(a0) * radius;
            float y0 = std::sin(a0) * radius + CYLINDER_RADIUS;
            float x1 = std::cos(a1) * radius;
            float y1 = std::sin(a1) * radius + CYLINDER_RADIUS;

            Vec3 n0 = Vec3(std::cos((a0 + a1) * 0.5f), std::sin((a0 + a1) * 0.5f), 0.0f);

            Vec3 p0 = {x0, y0, -length * 0.5f};
            Vec3 p1 = {x1, y1, -length * 0.5f};
            Vec3 p2 = {x1, y1, length * 0.5f};
            Vec3 p3 = {x0, y0, length * 0.5f};

            float u0, v0, u1, v1;
            TextureAtlas::instance().getTileUVs(0, 0, u0, v0, u1, v1); // Hull alloy

            verts.push_back({p0, {u0, v1}, n0});
            verts.push_back({p1, {u1, v1}, n0});
            verts.push_back({p2, {u1, v0}, n0});

            verts.push_back({p0, {u0, v1}, n0});
            verts.push_back({p2, {u1, v0}, n0});
            verts.push_back({p3, {u0, v0}, n0});
        }

        // 6 Linear Sun Strips running along the perimeter of Rama's interior ceiling
        float sunRadius = CYLINDER_RADIUS - 2.0f;
        float stripWidth = 8.0f;

        for (int s = 0; s < 6; ++s) {
            float angle = (float)s * (2.0f * PI / 6.0f);
            float cx = std::sin(angle) * sunRadius;
            float cy = CYLINDER_RADIUS - std::cos(angle) * sunRadius;

            Vec3 p0 = {cx - stripWidth * 0.5f, cy, -length * 0.5f};
            Vec3 p1 = {cx + stripWidth * 0.5f, cy, -length * 0.5f};
            Vec3 p2 = {cx + stripWidth * 0.5f, cy, length * 0.5f};
            Vec3 p3 = {cx - stripWidth * 0.5f, cy, length * 0.5f};

            float u0, v0, u1, v1;
            TextureAtlas::instance().getTileUVs(4, 0, u0, v0, u1, v1); // Reactor core glow tile

            verts.push_back({p0, {u0, v1}, {0, -1, 0}});
            verts.push_back({p1, {u1, v1}, {0, -1, 0}});
            verts.push_back({p2, {u1, v0}, {0, -1, 0}});

            verts.push_back({p0, {u0, v1}, {0, -1, 0}});
            verts.push_back({p2, {u1, v0}, {0, -1, 0}});
            verts.push_back({p3, {u0, v0}, {0, -1, 0}});
        }

        glGenVertexArrays(1, &s_spindleVAO);
        glGenBuffers(1, &s_spindleVBO);

        glBindVertexArray(s_spindleVAO);
        glBindBuffer(GL_ARRAY_BUFFER, s_spindleVBO);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(SpindleVertex), verts.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SpindleVertex), (void*)offsetof(SpindleVertex, pos));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SpindleVertex), (void*)offsetof(SpindleVertex, uv));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(SpindleVertex), (void*)offsetof(SpindleVertex, normal));

        s_spindleVertCount = (int)verts.size();
        glBindVertexArray(0);
    }

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", 0.0f); // Spindle and suns are already mathematically in 3D cylinder space

    Mat4 model = Mat4::translation({0.0f, 0.0f, playerPos.z});
    shader.setMat4("uModel", model);

    Vec4 tint = isDay() ? Vec4(1.0f, 0.95f, 0.8f, 1.0f) * m_sunIntensity : Vec4(0.15f, 0.2f, 0.3f, 0.8f);
    shader.setVec4("uTint", tint);

    glBindVertexArray(s_spindleVAO);
    glDrawArrays(GL_TRIANGLES, 0, s_spindleVertCount);
    glBindVertexArray(0);
}

bool World::raycast(const Ray& ray, float maxDist, Vec3i& outBlock, Vec3i& outNormal, float& outDist) {
    float step = 0.08f;
    float dist = 0.0f;
    Vec3 prevPos = ray.origin;

    float axisY = CYLINDER_RADIUS; // 122.230996f
    float circ = CIRCUMFERENCE;    // 768.0f

    while (dist < maxDist) {
        dist += step;
        Vec3 curPos = ray.origin + ray.dir * dist;

        // Convert 3D Cartesian position to cylinder voxel coordinates
        float theta = std::atan2(curPos.x, axisY - curPos.y);
        if (theta < 0.0f) theta += 2.0f * PI;

        float vx = (theta * circ) / (2.0f * PI);
        float r = std::sqrt(curPos.x * curPos.x + (curPos.y - axisY) * (curPos.y - axisY));
        float vy = axisY - r;
        float vz = curPos.z;

        int ix = ((int)std::floor(vx) % (int)circ + (int)circ) % (int)circ;
        int iy = (int)std::floor(vy);
        int iz = (int)std::floor(vz);

        if (iy >= 0 && iy < CHUNK_SIZE_Y) {
            BlockType b = getBlock(ix, iy, iz);
            if (b != BlockType::Air && b != BlockType::CylindricalSeaWater) {
                outBlock = Vec3i(ix, iy, iz);
                outDist = dist;

                float prevTheta = std::atan2(prevPos.x, axisY - prevPos.y);
                if (prevTheta < 0.0f) prevTheta += 2.0f * PI;
                float prevVx = (prevTheta * circ) / (2.0f * PI);
                float prevR = std::sqrt(prevPos.x * prevPos.x + (prevPos.y - axisY) * (prevPos.y - axisY));
                float prevVy = axisY - prevR;
                float prevVz = prevPos.z;

                int dx = ix - (int)std::floor(prevVx);
                int dy = iy - (int)std::floor(prevVy);
                int dz = iz - (int)std::floor(prevVz);

                if (std::abs(dy) >= std::abs(dx) && std::abs(dy) >= std::abs(dz)) {
                    outNormal = Vec3i(0, (dy > 0 ? -1 : 1), 0);
                } else if (std::abs(dx) >= std::abs(dz)) {
                    outNormal = Vec3i((dx > 0 ? -1 : 1), 0, 0);
                } else {
                    outNormal = Vec3i(0, 0, (dz > 0 ? -1 : 1));
                }
                return true;
            }
        }
        prevPos = curPos;
    }
    return false;
}
