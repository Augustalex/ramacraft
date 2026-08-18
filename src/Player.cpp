#include "Player.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Block.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "Audio.hpp"
#include "Projectile.hpp"
#include "Network.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

Player::Player() {
    m_inventory.initStartingGear();
}

void Player::init(const Vec3& spawnPos) {
    m_pos = spawnPos;
    m_vel = {0, 0, 0};
    m_currentUp = {0, 1, 0};
    m_pitch = 0.0f;
    m_yaw = 0.0f;
    m_health = 100.0f;
    m_oxygen = 100.0f;
    m_jetpackFuel = 100.0f;
    m_inventory.initStartingGear();
    buildViewmodelMesh();
}

Vec3 Player::getForward() const {
    float cosP = std::cos(m_pitch * DEG2RAD);
    float sinP = std::sin(m_pitch * DEG2RAD);
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);
    return Vec3(-sinY * cosP, -sinP, cosY * cosP).normalized();
}

Vec3 Player::getRight() const {
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);
    return Vec3(-cosY, 0.0f, -sinY).normalized();
}

Vec3 Player::getWorldPos3D() const {
    float theta = m_pos.x * (2.0f * PI / World::CIRCUMFERENCE);
    float r = World::CYLINDER_RADIUS - m_pos.y;
    return Vec3(std::sin(theta) * r, World::CYLINDER_RADIUS - std::cos(theta) * r, m_pos.z);
}

Vec3 Player::getEyePosition3D() const {
    float theta = m_pos.x * (2.0f * PI / World::CIRCUMFERENCE);
    float r = World::CYLINDER_RADIUS - (m_pos.y + m_eyeHeight);
    return Vec3(std::sin(theta) * r, World::CYLINDER_RADIUS - std::cos(theta) * r, m_pos.z);
}

Vec3 Player::getUp3D() const {
    float theta = m_pos.x * (2.0f * PI / World::CIRCUMFERENCE);
    return Vec3(-std::sin(theta), std::cos(theta), 0.0f);
}

Vec3 Player::getForward3D() const {
    float theta = m_pos.x * (2.0f * PI / World::CIRCUMFERENCE);
    Vec3 tangent(std::cos(theta), std::sin(theta), 0.0f);
    Vec3 up(-std::sin(theta), std::cos(theta), 0.0f);
    Vec3 longit(0.0f, 0.0f, 1.0f);

    float cosP = std::cos(m_pitch * DEG2RAD);
    float sinP = std::sin(m_pitch * DEG2RAD);
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);

    Vec3 fwd = tangent * (-sinY * cosP) + up * (-sinP) + longit * (cosY * cosP);
    return fwd.normalized();
}

Vec3 Player::getRight3D() const {
    Vec3 fwd = getForward3D();
    Vec3 up = getUp3D();
    return fwd.cross(up).normalized();
}

Mat4 Player::getViewMatrix() const {
    Vec3 eye = getEyePosition3D();
    Vec3 fwd = getForward3D();
    Vec3 up = getUp3D();
    return Mat4::lookAt(eye, eye + fwd, up);
}

void Player::addRotation(float dPitch, float dYaw) {
    m_pitch = std::clamp(m_pitch + dPitch, -89.0f, 89.0f);
    m_yaw += dYaw;
    while (m_yaw > 180.0f) m_yaw -= 360.0f;
    while (m_yaw < -180.0f) m_yaw += 360.0f;
}

void Player::toggleFlashlight() {
    m_flashlightOn = !m_flashlightOn;
    AudioSystem::instance().playSound(SoundEffect::BlockPlace);
}

void Player::toggleJetpackMode() {
    m_jetpackMode = !m_jetpackMode;
    AudioSystem::instance().playSound(SoundEffect::BlockPlace);
    if (m_jetpackMode) {
        m_isGrounded = false;
    } else {
        AudioSystem::instance().setJetpackHum(false);
        m_jetpackActive = false;
    }
}

void Player::takeDamage(float amount) {
    m_health = std::max(0.0f, m_health - amount);
}

void Player::heal(float amount) {
    m_health = std::min(m_maxHealth, m_health + amount);
}

void Player::fireRayGun(World& world) {
    if (m_fireCooldown > 0.0f) return;

    ItemStack& sel = m_inventory.getSelectedItem();
    bool hasOverclock = m_inventory.hasItem(ItemType::OverclockedRayGun);

    if (sel.type == ItemType::RayGun || hasOverclock) {
        Vec3 eye = getEyePosition();
        Vec3 fwd = getForward();
        Vec3 right = getRight();

        m_recoilAnim = 1.0f; // Trigger viewmodel kickback animation

        if (hasOverclock) {
            Vec3 bPos1 = eye + right * 0.15f - Vec3(0, 0.1f, 0);
            Vec3 bPos2 = eye - right * 0.15f - Vec3(0, 0.1f, 0);
            ProjectileManager::instance().spawnBolt(bPos1, fwd, 55.0f, 40.0f, true);
            ProjectileManager::instance().spawnBolt(bPos2, fwd, 55.0f, 40.0f, true);
            NetworkManager::instance().sendShoot(bPos1, fwd, 55.0f, 40.0f, true);
            NetworkManager::instance().sendShoot(bPos2, fwd, 55.0f, 40.0f, true);
            m_fireCooldown = 0.14f;
        } else {
            Vec3 bPos = eye + right * 0.12f - Vec3(0, 0.1f, 0);
            ProjectileManager::instance().spawnBolt(bPos, fwd, 48.0f, 25.0f, false);
            NetworkManager::instance().sendShoot(bPos, fwd, 48.0f, 25.0f, false);
            m_fireCooldown = 0.20f;
        }
    }
}

void Player::startMining() {
    m_isMining = true;
}

void Player::stopMining() {
    m_isMining = false;
    m_miningProgress = 0.0f;
    AudioSystem::instance().setMiningBeamHum(false);
}

void Player::placeSelectedBlock(World& world) {
    if (!m_hasTarget) return;

    ItemStack& sel = m_inventory.getSelectedItem();
    const ItemInfo& info = BlockRegistry::getItem(sel.type);

    if (info.isPlaceable && sel.count > 0) {
        Vec3i placePos = m_targetBlock + m_targetNormal;
        world.setBlock(placePos.x, placePos.y, placePos.z, info.placeBlock);
        NetworkManager::instance().sendBlockChange(placePos.x, placePos.y, placePos.z, info.placeBlock);

        if (info.placeBlock == BlockType::Torch) {
            world.addTorch(placePos);
        }

        sel.count--;
        if (sel.count <= 0) sel.clear();

        AudioSystem::instance().playSound(SoundEffect::BlockPlace);
    }
}

void Player::update(float dt, World& world) {
    if (m_fireCooldown > 0.0f) {
        m_fireCooldown -= dt;
    }

    // Recoil recovery
    m_recoilAnim = std::max(0.0f, m_recoilAnim - dt * 6.5f);

    // Target raycast
    Ray lookRay(getEyePosition(), getForward());
    float hitDist = 0.0f;
    m_hasTarget = world.raycast(lookRay, 6.0f, m_targetBlock, m_targetNormal, hitDist);

    // Continuous mining beam
    if (m_isMining && m_hasTarget) {
        BlockType bt = world.getBlock(m_targetBlock.x, m_targetBlock.y, m_targetBlock.z);
        if (bt != BlockType::Air && bt != BlockType::CylindricalSeaWater) {
            const BlockInfo& info = BlockRegistry::get(bt);
            ItemStack& cur = m_inventory.getSelectedItem();
            
            float drillSpeed = 2.0f;
            if (cur.type == ItemType::MiningDrill) {
                drillSpeed = 7.0f; // Dedicated Plasma Mining Drill: ultra-fast extraction
            } else if (m_inventory.hasItem(ItemType::OverclockedRayGun)) {
                drillSpeed = 4.5f;
            }

            m_miningProgress += (dt * drillSpeed) / info.hardness;
            AudioSystem::instance().setMiningBeamHum(true);

            Vec3 sparkPos = lookRay.at(hitDist);
            Vec4 sparkColor = (cur.type == ItemType::MiningDrill) ? Vec4(1.0f, 0.5f, 0.1f, 1.0f) : Vec4(0.3f, 0.8f, 1.0f, 1.0f);
            ProjectileManager::instance().spawnSparks(sparkPos, sparkColor, 3, 2.5f);

            // Laser mining beam tracer
            ProjectileManager::instance().spawnLaserTracer(getEyePosition() + getRight() * 0.12f - Vec3(0, 0.1f, 0), sparkPos, sparkColor, 0.05f);

            if (m_miningProgress >= 1.0f) {
                world.setBlock(m_targetBlock.x, m_targetBlock.y, m_targetBlock.z, BlockType::Air);
                NetworkManager::instance().sendBlockChange(m_targetBlock.x, m_targetBlock.y, m_targetBlock.z, BlockType::Air);
                if (bt == BlockType::Torch) {
                    world.removeTorch(m_targetBlock);
                }

                // Drop 3D physical item entity
                if (info.dropItem != ItemType::None) {
                    int count = info.dropMin + (rand() % (info.dropMax - info.dropMin + 1));
                    world.spawnDroppedItem(Vec3((float)m_targetBlock.x + 0.5f, (float)m_targetBlock.y + 0.5f, (float)m_targetBlock.z + 0.5f), info.dropItem, count);
                }

                ProjectileManager::instance().spawnSparks(sparkPos, Vec4(1.0f, 0.7f, 0.2f, 1.0f), 18, 4.5f);
                AudioSystem::instance().playSound(SoundEffect::BlockBreak);

                m_miningProgress = 0.0f;
            }
        }
    } else {
        m_miningProgress = 0.0f;
        AudioSystem::instance().setMiningBeamHum(false);
    }

    // Jetpack fuel management - INFINITE RANGE & POWER
    m_maxFuel = 100.0f;
    m_jetpackFuel = 100.0f;
    m_thrustPower = 28.0f; // High-powered plasma thrust

    // Check Water / Swimming / Underwater status
    Vec3 eye = getEyePosition();
    m_inWater = (world.getBlock((int)std::floor(m_pos.x), (int)std::floor(m_pos.y), (int)std::floor(m_pos.z)) == BlockType::CylindricalSeaWater);
    m_underwater = (world.getBlock((int)std::floor(eye.x), (int)std::floor(eye.y), (int)std::floor(eye.z)) == BlockType::CylindricalSeaWater);

    if (m_underwater) {
        bool hasRecycler = m_inventory.hasItem(ItemType::OxygenRecycler);
        if (!hasRecycler) {
            m_oxygen = std::max(0.0f, m_oxygen - 5.0f * dt);
            if (m_oxygen <= 0.0f) {
                takeDamage(8.0f * dt); // Drowning damage
            }
        }
        // Spawn rising bubble particles
        if (rand() % 15 == 0) {
            ProjectileManager::instance().spawnSparks(eye + Vec3(0, -0.2f, 0), Vec4(0.4f, 0.8f, 1.0f, 0.6f), 2, 0.8f);
        }
    } else {
        m_oxygen = std::min(100.0f, m_oxygen + 20.0f * dt); // Fast oxygen replenishment
    }

    updatePhysics(dt, world);
}

void Player::updatePhysics(float dt, World& world) {
    const Uint8* state = SDL_GetKeyboardState(nullptr);

    bool moveFwd = state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP];
    bool moveBack = state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN];
    bool moveRight = state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT];
    bool moveLeft = state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT];

    bool pressSpace = (state[SDL_SCANCODE_SPACE] != 0);
    bool pressShift = (state[SDL_SCANCODE_LSHIFT] != 0 || state[SDL_SCANCODE_RSHIFT] != 0);
    bool pressCtrl  = (state[SDL_SCANCODE_LCTRL]  != 0 || state[SDL_SCANCODE_RCTRL]  != 0);

    float cosP = std::cos(m_pitch * DEG2RAD);
    float sinP = std::sin(m_pitch * DEG2RAD);
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);

    // Forward and Right vectors in voxel plane (Yaw = 0 -> Forward = +Z, Right = -X)
    Vec3 fwdVec(-sinY, 0.0f, cosY);
    Vec3 rightVec(-cosY, 0.0f, -sinY);

    // 3D Camera view directions in voxel coordinates
    Vec3 lookFwd(-sinY * cosP, -sinP, cosY * cosP);
    Vec3 lookRight(-cosY, 0.0f, -sinY);
    Vec3 lookUp(-sinY * sinP, cosP, cosY * sinP);

    Vec3 inputDir(0, 0, 0);
    if (moveFwd) inputDir += fwdVec;
    if (moveBack) inputDir -= fwdVec;
    if (moveRight) inputDir += rightVec;
    if (moveLeft) inputDir -= rightVec;

    if (inputDir.lengthSq() > 0.001f) {
        inputDir = inputDir.normalized();
        m_bobTime += dt * (pressShift ? 14.0f : 9.5f);
    } else {
        m_bobTime += dt * 2.0f;
    }

    if (m_jetpackMode) {
        // ==========================================
        // JETPACK FLIGHT MODE (TOGGLED WITH TAB)
        // ==========================================
        // Normal walking is disabled; full 3D thruster controls active!
        m_isGrounded = false;
        m_jetpackFuel = 100.0f;

        float thrustPower = 28.0f;
        Vec3 jetpackThrust(0, 0, 0);
        bool isThrusting = false;

        // 1. Space thrusts towards where looking
        if (pressSpace) {
            jetpackThrust += lookFwd * thrustPower;
            isThrusting = true;
        }

        // 2. Shift thrusts upwards relative to view
        if (pressShift) {
            jetpackThrust += lookUp * thrustPower;
            isThrusting = true;
        }

        // 3. Ctrl thrusts downwards relative to view
        if (pressCtrl) {
            jetpackThrust -= lookUp * thrustPower;
            isThrusting = true;
        }

        // 4. WASD directional steering in flight
        if (inputDir.lengthSq() > 0.001f) {
            jetpackThrust += inputDir * (thrustPower * 0.5f);
            isThrusting = true;
        }

        m_jetpackActive = isThrusting;
        AudioSystem::instance().setJetpackHum(isThrusting);

        if (isThrusting) {
            m_vel += jetpackThrust * dt;
            ProjectileManager::instance().spawnJetpackExhaust(getWorldPos3D() - Vec3(0, 0.4f, 0), Vec3(0, -1, 0));
        }

        // Inertial damping in jetpack mode
        m_vel.x *= 0.985f;
        m_vel.y *= 0.985f;
        m_vel.z *= 0.985f;

    } else {
        // ==========================================
        // NORMAL WALKING & SURVIVAL MODE
        // ==========================================
        // Jetpack is completely OFF! No forward thruster, no hum, no auto-firing when falling.
        m_jetpackActive = false;
        AudioSystem::instance().setJetpackHum(false);

        float walkSpeed = (m_isGrounded && pressShift) ? 8.5f : 5.0f;

        // In survival mode, horizontal movement is directly controlled by input
        m_vel.x = inputDir.x * walkSpeed;
        m_vel.z = inputDir.z * walkSpeed;

        if (m_isGrounded) {
            if (pressSpace) {
                m_vel.y = 7.1f; // Exact 1.5-block jump height
                m_isGrounded = false;
                AudioSystem::instance().playSound(SoundEffect::BlockPlace);
            }
        } else {
            // Natural Minecraft-style Gravity & Drag (Zero Jetpack firing)
            float groundGravZone = 60.0f;
            float ceilingGravZone = (World::CYLINDER_RADIUS * 2.0f - 60.0f);

            if (m_pos.y <= groundGravZone) {
                m_vel.y -= 16.0f * dt;
                m_vel.y *= 0.985f; // standard air drag
            } else if (m_pos.y >= ceilingGravZone) {
                m_vel.y += 16.0f * dt;
                m_vel.y *= 0.985f;
            } else {
                // Pure zero-g mid-air
                m_vel.y *= 0.99f;
            }
        }
    }

    // Terminal velocity clamps
    m_vel.x = std::clamp(m_vel.x, -24.0f, 24.0f);
    m_vel.y = std::clamp(m_vel.y, -26.0f, 24.0f);
    m_vel.z = std::clamp(m_vel.z, -24.0f, 24.0f);

    checkVoxelCollisions(dt, world);
}

void Player::checkVoxelCollisions(float dt, World& world) {
    float halfW = 0.3f;
    float height = 1.8f;
    float eps = 0.001f;

    float dx = m_vel.x * dt;
    float dy = m_vel.y * dt;
    float dz = m_vel.z * dt;

    float origDy = dy;
    float origDx = dx;
    float origDz = dz;

    // Broadphase bounding box for query
    float minQX = std::min(m_pos.x - halfW, m_pos.x - halfW + dx) - 1.0f;
    float maxQX = std::max(m_pos.x + halfW, m_pos.x + halfW + dx) + 1.0f;
    float minQY = std::min(m_pos.y, m_pos.y + dy) - 1.0f;
    float maxQY = std::max(m_pos.y + height, m_pos.y + height + dy) + 1.0f;
    float minQZ = std::min(m_pos.z - halfW, m_pos.z - halfW + dz) - 1.0f;
    float maxQZ = std::max(m_pos.z + halfW, m_pos.z + halfW + dz) + 1.0f;

    struct BlockAABB {
        float minX, minY, minZ;
        float maxX, maxY, maxZ;
    };
    std::vector<BlockAABB> blocks;

    int qMinX = (int)std::floor(minQX);
    int qMaxX = (int)std::floor(maxQX);
    int qMinY = std::max(0, (int)std::floor(minQY));
    int qMaxY = std::min((int)CHUNK_SIZE_Y - 1, (int)std::floor(maxQY));
    int qMinZ = (int)std::floor(minQZ);
    int qMaxZ = (int)std::floor(maxQZ);

    for (int y = qMinY; y <= qMaxY; ++y) {
        for (int x = qMinX; x <= qMaxX; ++x) {
            for (int z = qMinZ; z <= qMaxZ; ++z) {
                BlockType bt = world.getBlock(x, y, z);
                if (BlockRegistry::get(bt).isSolid) {
                    blocks.push_back({(float)x, (float)y, (float)z, (float)(x + 1), (float)(y + 1), (float)(z + 1)});
                }
            }
        }
    }

    // 1. Move along Y-axis (Swept AABB)
    for (const auto& b : blocks) {
        if (m_pos.x + halfW > b.minX + eps && m_pos.x - halfW < b.maxX - eps &&
            m_pos.z + halfW > b.minZ + eps && m_pos.z - halfW < b.maxZ - eps) {
            if (dy < 0.0f && m_pos.y >= b.maxY - eps) {
                float dist = b.maxY - m_pos.y;
                if (dist > dy) {
                    dy = dist;
                }
            } else if (dy > 0.0f && m_pos.y + height <= b.minY + eps) {
                float dist = b.minY - (m_pos.y + height);
                if (dist < dy) {
                    dy = dist;
                }
            }
        }
    }
    m_pos.y += dy;

    if (origDy < 0.0f && dy > origDy) {
        m_isGrounded = true;
        m_vel.y = 0.0f;
    } else {
        m_isGrounded = false;
        if (origDy > 0.0f && dy < origDy) {
            m_vel.y = 0.0f;
        }
    }

    // 2. Move along X-axis (Swept AABB)
    for (const auto& b : blocks) {
        if (m_pos.y + height > b.minY + eps && m_pos.y < b.maxY - eps &&
            m_pos.z + halfW > b.minZ + eps && m_pos.z - halfW < b.maxZ - eps) {
            if (dx < 0.0f && m_pos.x - halfW >= b.maxX - eps) {
                float dist = b.maxX - (m_pos.x - halfW);
                if (dist > dx) {
                    dx = dist;
                }
            } else if (dx > 0.0f && m_pos.x + halfW <= b.minX + eps) {
                float dist = b.minX - (m_pos.x + halfW);
                if (dist < dx) {
                    dx = dist;
                }
            }
        }
    }
    m_pos.x += dx;
    m_pos.x = std::fmod(m_pos.x + World::CIRCUMFERENCE, World::CIRCUMFERENCE);
    if (origDx != dx) {
        m_vel.x = 0.0f;
    }

    // 3. Move along Z-axis (Swept AABB)
    for (const auto& b : blocks) {
        if (m_pos.y + height > b.minY + eps && m_pos.y < b.maxY - eps &&
            m_pos.x + halfW > b.minX + eps && m_pos.x - halfW < b.maxX - eps) {
            if (dz < 0.0f && m_pos.z - halfW >= b.maxZ - eps) {
                float dist = b.maxZ - (m_pos.z - halfW);
                if (dist > dz) {
                    dz = dist;
                }
            } else if (dz > 0.0f && m_pos.z + halfW <= b.minZ + eps) {
                float dist = b.minZ - (m_pos.z + halfW);
                if (dist < dz) {
                    dz = dist;
                }
            }
        }
    }
    m_pos.z += dz;
    if (origDz != dz) {
        m_vel.z = 0.0f;
    }

    // Spindle collision
    float axisY = World::CYLINDER_RADIUS;
    float distFromSpindle = std::abs(m_pos.y - axisY);
    if (distFromSpindle < 16.0f) {
        if (m_pos.y < axisY) {
            m_pos.y = axisY - 16.0f;
            if (m_vel.y > 0.0f) m_vel.y = 0.0f;
        } else {
            m_pos.y = axisY + 16.0f;
            if (m_vel.y < 0.0f) m_vel.y = 0.0f;
        }
    }

    // Bedrock ground floor safeguard
    if (m_pos.y < 4.0f) {
        m_pos.y = 4.0f;
        m_vel.y = 0.0f;
        m_isGrounded = true;
    }
}

void Player::buildViewmodelMesh() {
    struct VMVertex {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };

    std::vector<VMVertex> verts;

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

    // 3D Ray Gun Model: Chassis, Barrel, Grip, Emitter Lens, Orange Core
    addBox({-0.03f, -0.04f, -0.22f}, {0.03f, 0.04f, 0.08f}, 0, 0); // Receiver
    addBox({-0.02f, -0.02f, -0.32f}, {0.02f, 0.02f, -0.22f}, 12, 0); // Forward barrel
    addBox({-0.025f, -0.12f, 0.0f}, {0.025f, -0.04f, 0.06f}, 7, 0); // Grip
    addBox({-0.015f, -0.015f, -0.34f}, {0.015f, 0.015f, -0.32f}, 6, 0); // Glowing cyan lens
    addBox({-0.025f, 0.01f, -0.12f}, {0.025f, 0.045f, -0.02f}, 4, 0); // Plasma cell

    if (m_viewmodelVAO == 0) {
        glGenVertexArrays(1, &m_viewmodelVAO);
        glGenBuffers(1, &m_viewmodelVBO);
    }

    glBindVertexArray(m_viewmodelVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_viewmodelVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(VMVertex), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VMVertex), (void*)offsetof(VMVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VMVertex), (void*)offsetof(VMVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VMVertex), (void*)offsetof(VMVertex, normal));

    m_viewmodelVertCount = (int)verts.size();
    glBindVertexArray(0);
}

void Player::renderViewmodel(const Shader& shader, const Mat4& proj) {
    if (m_viewmodelVertCount == 0) return;

    // Viewmodel bobbing math
    float bobX = std::cos(m_bobTime * 0.5f) * 0.012f;
    float bobY = std::sin(m_bobTime) * 0.015f;
    float recoilZ = m_recoilAnim * 0.06f;
    float recoilPitch = m_recoilAnim * 8.0f;

    // Hand position in camera space
    Vec3 handOffset(0.24f + bobX, -0.18f + bobY, -0.35f + recoilZ);

    Mat4 vmModel = Mat4::translation(handOffset) *
                   Mat4::rotationX(recoilPitch * DEG2RAD) *
                   Mat4::rotationY(-6.0f * DEG2RAD);

    // Disable depth test clear or draw on top
    glDisable(GL_DEPTH_TEST);

    shader.use();
    shader.setMat4("uProjection", proj);
    shader.setMat4("uView", Mat4::identity()); // Camera local space
    shader.setMat4("uModel", vmModel);
    shader.setVec3("uPlayerPos", {0, 0, 0});
    shader.setFloat("uCylinderRadius", 1000.0f);
    shader.setFloat("uCurvatureEnable", 0.0f);
    shader.setVec4("uTint", Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glBindVertexArray(m_viewmodelVAO);
    glDrawArrays(GL_TRIANGLES, 0, m_viewmodelVertCount);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}
