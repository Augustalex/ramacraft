#include "Player.hpp"
#include "World.hpp"
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

Vec3 Player::getRight() const {
    Vec3 up = m_currentUp;
    Vec3 tangent;
    if (std::abs(up.y) > 0.7f) {
        tangent = Vec3(1, 0, 0);
    } else {
        tangent = Vec3(up.y, -up.x, 0.0f).normalized();
    }
    Vec3 longit(0.0f, 0.0f, -1.0f);
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);
    return (tangent * cosY + longit * sinY).normalized();
}

Vec3 Player::getForward() const {
    Vec3 up = m_currentUp;
    Vec3 tangent;
    if (std::abs(up.y) > 0.7f) {
        tangent = Vec3(1, 0, 0);
    } else {
        tangent = Vec3(up.y, -up.x, 0.0f).normalized();
    }
    Vec3 longit(0.0f, 0.0f, -1.0f);

    float cosP = std::cos(m_pitch * DEG2RAD);
    float sinP = std::sin(m_pitch * DEG2RAD);
    float cosY = std::cos(m_yaw * DEG2RAD);
    float sinY = std::sin(m_yaw * DEG2RAD);

    Vec3 fwd = tangent * (sinY * cosP) + up * (-sinP) + longit * (cosY * cosP);
    return fwd.normalized();
}

Vec3 Player::getEyePosition() const {
    return m_pos + m_currentUp * m_eyeHeight;
}

void Player::addRotation(float dPitch, float dYaw) {
    m_pitch = std::clamp(m_pitch + dPitch, -89.0f, 89.0f);
    m_yaw += dYaw;
    while (m_yaw > 180.0f) m_yaw -= 360.0f;
    while (m_yaw < -180.0f) m_yaw += 360.0f;
}

Mat4 Player::getViewMatrix() const {
    Vec3 eye = getEyePosition();
    Vec3 fwd = getForward();
    Vec3 up = getUp();
    return Mat4::lookAt(eye, eye + fwd, up);
}

void Player::toggleFlashlight() {
    m_flashlightOn = !m_flashlightOn;
    AudioSystem::instance().playSound(SoundEffect::BlockPlace);
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

    float axisY = World::CYLINDER_RADIUS; // 244.46199f
    float circ = World::CIRCUMFERENCE;    // 1536.0f

    Vec3 delta(m_pos.x, m_pos.y - axisY, 0.0f);
    float r = std::max(0.1f, std::sqrt(delta.x * delta.x + delta.y * delta.y));
    Vec3 rOut = delta / r;

    // Calculate height above the nearest terrain surface
    float theta = std::atan2(m_pos.x, axisY - m_pos.y);
    if (theta < 0.0f) theta += 2.0f * PI;

    float vx = (theta * circ) / (2.0f * PI);
    float vz = m_pos.z;

    int ivx = ((int)std::floor(vx) % (int)circ + (int)circ) % (int)circ;
    int ivz = (int)std::floor(vz);

    int groundY = 0;
    for (int y = 31; y >= 0; --y) {
        BlockType b = world.getBlock(ivx, y, ivz);
        if (BlockRegistry::get(b).isSolid) {
            groundY = y + 1;
            break;
        }
    }
    float maxRadius = axisY - (float)groundY;
    float heightAboveSurface = std::max(0.0f, maxRadius - r);

    // Gravity Zone: gravity only acts within 25 blocks of the ground/ceiling surface!
    // Above 25m, you are in pristine pure ZERO GRAVITY (0.0g).
    float gravityZone = 25.0f;
    bool inGravityZone = (heightAboveSurface < gravityZone);

    if (inGravityZone) {
        // Surface Up points inward toward the central axis
        Vec3 targetUp = Vec3(-rOut.x, -rOut.y, 0.0f);
        m_currentUp = (m_currentUp * 0.92f + targetUp * 0.08f).normalized();
    }
    // When in zero-g (h >= 25), m_currentUp DOES NOT ROTATE OR TWIST!

    Vec3 up = m_currentUp;
    Vec3 fwd = getForward();
    Vec3 right = getRight();

    bool moveFwd = state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP];
    bool moveBack = state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN];
    bool moveRight = state[SDL_SCANCODE_D] || state[SDL_SCANCODE_RIGHT];
    bool moveLeft = state[SDL_SCANCODE_A] || state[SDL_SCANCODE_LEFT];

    // Horizontal walk directions relative to surface Up
    Vec3 walkFwd = (fwd - up * fwd.dot(up)).normalized();
    Vec3 walkRight = (right - up * right.dot(up)).normalized();

    Vec3 walkDir(0, 0, 0);
    if (moveFwd) walkDir += walkFwd;
    if (moveBack) walkDir -= walkFwd;
    if (moveRight) walkDir += walkRight;
    if (moveLeft) walkDir -= walkRight;

    if (walkDir.lengthSq() > 0.001f) {
        walkDir = walkDir.normalized();
        m_bobTime += dt * (state[SDL_SCANCODE_LSHIFT] ? 14.0f : 9.5f);
    } else {
        m_bobTime += dt * 2.0f;
    }

    float speed = (state[SDL_SCANCODE_LSHIFT]) ? 16.0f : 8.0f;

    if (m_isGrounded) {
        // Ground walking physics
        m_vel = walkDir * speed;

        if (state[SDL_SCANCODE_SPACE]) {
            // Immediate upward leap impulse
            m_vel += up * 7.5f;
            m_isGrounded = false;
        }
    } else {
        // In-air / Jetpack physics
        if (state[SDL_SCANCODE_SPACE]) {
            m_jetpackActive = true;
            m_jetpackFuel = 100.0f;

            // 3D Directional Vector Propulsion: (W / UP ARROW rockets forward where looking!)
            if (moveFwd) {
                m_vel += fwd * (32.0f * dt);
            } else if (moveBack) {
                m_vel -= fwd * (26.0f * dt);
            }

            if (moveRight) {
                m_vel += right * (22.0f * dt);
            } else if (moveLeft) {
                m_vel -= right * (22.0f * dt);
            }

            // Always add upward lift thrust away from the surface
            m_vel += up * (22.0f * dt);

            AudioSystem::instance().setJetpackHum(true);
            ProjectileManager::instance().spawnJetpackExhaust(m_pos - up * 0.4f, up * -1.0f);
        } else {
            m_jetpackActive = false;
            AudioSystem::instance().setJetpackHum(false);

            if (inGravityZone) {
                // Surface gravity pulls purely toward the nearest surface hull
                float normH = heightAboveSurface / gravityZone;
                float gravityFactor = (1.0f - normH) * (1.0f - normH);
                float g = 6.0f * gravityFactor;
                m_vel += rOut * (g * dt);
            } else {
                // In pure zero-g: NO GRAVITY. Pure straight line inertial glide.
                m_vel *= 0.998f;
            }

            // Slight in-air steering
            if (walkDir.lengthSq() > 0.001f) {
                m_vel += walkDir * (speed * 0.06f);
            }
        }

        // Max Speed Clamping: Keeps jetpack fully controlled and prevents runaway velocity
        float maxSpeed = 24.0f; // 24 m/s (~86 km/h) max controlled flight speed
        float currentSpeed = m_vel.length();
        if (currentSpeed > maxSpeed) {
            m_vel = (m_vel / currentSpeed) * maxSpeed;
        }
    }

    checkVoxelCollisions(world);
}

void Player::checkVoxelCollisions(World& world) {
    // 4-step continuous sub-stepping: completely prevents tunneling/clipping at high speeds
    int subSteps = 4;
    float subDt = 0.016f / (float)subSteps;

    float axisY = World::CYLINDER_RADIUS; // 244.46199f
    float circ = World::CIRCUMFERENCE;    // 1536.0f
    float spindleRadius = 16.0f;

    m_isGrounded = false;

    for (int step = 0; step < subSteps; ++step) {
        m_pos += m_vel * subDt;

        Vec3 delta(m_pos.x, m_pos.y - axisY, 0.0f);
        float r = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (r < 0.1f) r = 0.1f;
        Vec3 rOut = delta / r;

        // 1. Solid Central Spindle Collision (radius = 16 blocks)
        if (r <= spindleRadius + 0.9f) {
            float landRadius = spindleRadius + 0.9f;
            m_pos.x = rOut.x * landRadius;
            m_pos.y = axisY + rOut.y * landRadius;
            m_isGrounded = true;

            float inDot = m_vel.dot(rOut);
            if (inDot < 0.0f) {
                m_vel -= rOut * inDot; // Stop moving into spindle
            }
            continue;
        }

        // 2. Convert current 3D position to cylinder voxel space
        float theta = std::atan2(m_pos.x, axisY - m_pos.y);
        if (theta < 0.0f) theta += 2.0f * PI;

        float vx = (theta * circ) / (2.0f * PI);
        float vz = m_pos.z;

        int ivx = ((int)std::floor(vx) % (int)circ + (int)circ) % (int)circ;
        int ivz = (int)std::floor(vz);

        // Search for solid terrain height at this circumferential column
        int groundY = 0;
        for (int y = 63; y >= 0; --y) {
            BlockType b = world.getBlock(ivx, y, ivz);
            if (BlockRegistry::get(b).isSolid) {
                groundY = y + 1;
                break;
            }
        }

        float maxRadius = axisY - (float)groundY;

        // 3. Ground / Ceiling Hull Landing Check
        if (r >= maxRadius) {
            m_pos.x = rOut.x * maxRadius;
            m_pos.y = axisY + rOut.y * maxRadius;
            m_isGrounded = true;

            float outDot = m_vel.dot(rOut);
            if (outDot > 0.0f) {
                m_vel -= rOut * outDot; // Stop moving into hull
            }
        }
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
