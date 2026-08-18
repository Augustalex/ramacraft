#pragma once

#include "Math3D.hpp"
#include "Block.hpp"
#include "Inventory.hpp"

#include "GLCommon.hpp"

class World;
class Shader;

class Player {
public:
    Player();

    void init(const Vec3& spawnPos);
    void handleInput(float dt, World& world);
    void update(float dt, World& world);

    // Camera & Transform (Voxel Space & 3D Render Space)
    Vec3 getPosition() const { return m_pos; }
    Vec3 getVelocity() const { return m_vel; }
    Vec3 getEyePosition() const { return m_pos + Vec3(0, m_eyeHeight, 0); }
    Vec3 getForward() const;
    Vec3 getRight() const;
    Vec3 getUp() const { return Vec3(0, 1, 0); }

    Vec3 getWorldPos3D() const;
    Vec3 getEyePosition3D() const;
    Vec3 getForward3D() const;
    Vec3 getRight3D() const;
    Vec3 getUp3D() const;

    float getPitch() const { return m_pitch; }
    float getYaw() const { return m_yaw; }
    void addRotation(float dPitch, float dYaw);

    Mat4 getViewMatrix() const;

    // Movement, Swimming & Jetpack
    bool isGrounded() const { return m_isGrounded; }
    bool isInWater() const { return m_inWater; }
    bool isUnderwater() const { return m_underwater; }
    bool isJetpackMode() const { return m_jetpackMode; }
    void toggleJetpackMode();
    bool isJetpackActive() const { return m_jetpackActive; }
    float getJetpackFuel() const { return m_jetpackFuel; }
    float getMaxFuel() const { return m_maxFuel; }
    float getThrustPower() const { return m_thrustPower; }

    // Health & Oxygen
    float getHealth() const { return m_health; }
    float getMaxHealth() const { return m_maxHealth; }
    float getOxygen() const { return m_oxygen; }
    float getDamageFlash() const { return m_damageFlash; }
    bool isFlashlightOn() const { return m_flashlightOn; }
    void toggleFlashlight();
    void takeDamage(float amount);
    void heal(float amount);
    void addVelocity(const Vec3& v) { m_vel += v; }
    void respawn(const Vec3& spawnPos = {0, 20.0f, 0});

    // Combat & Mining
    void fireRayGun(World& world);
    void throwGrenade(World& world);
    void startMining();
    void stopMining();
    bool isMining() const { return m_isMining; }
    float getMiningProgress() const { return m_miningProgress; }
    bool hasTarget() const { return m_hasTarget; }
    Vec3i getTargetBlock() const { return m_targetBlock; }
    Vec3i getTargetNormal() const { return m_targetNormal; }

    // Inventory & Building
    Inventory& getInventory() { return m_inventory; }
    const Inventory& getInventory() const { return m_inventory; }
    void placeSelectedBlock(World& world);

    // Viewmodel
    void renderViewmodel(const Shader& shader, const Mat4& proj);

    AABB getAABB() const {
        return AABB(m_pos - Vec3(0.35f, 0.0f, 0.35f), m_pos + Vec3(0.35f, 1.8f, 0.35f));
    }

private:
    Vec3 m_pos = {0, 20, 0};
    Vec3 m_vel = {0, 0, 0};
    Vec3 m_currentUp = {0, 1, 0};
    float m_pitch = 0.0f; // degrees
    float m_yaw = 0.0f;   // degrees
    float m_eyeHeight = 1.62f;

    bool m_isGrounded = false;
    bool m_inWater = false;
    bool m_underwater = false;
    bool m_jetpackMode = false;
    bool m_jetpackActive = false;
    float m_jetpackFuel = 100.0f;
    float m_maxFuel = 100.0f;
    float m_thrustPower = 14.0f;

    float m_health = 100.0f;
    float m_maxHealth = 100.0f;
    float m_oxygen = 100.0f;
    float m_damageFlash = 0.0f;

    bool m_flashlightOn = true;

    Inventory m_inventory;

    // Viewmodel bobbing & recoil
    float m_bobTime = 0.0f;
    float m_recoilAnim = 0.0f;
    GLuint m_viewmodelVAO = 0;
    GLuint m_viewmodelVBO = 0;
    int m_viewmodelVertCount = 0;

    // Mining state
    bool m_isMining = false;
    float m_miningProgress = 0.0f;
    Vec3i m_targetBlock = {0, 0, 0};
    Vec3i m_targetNormal = {0, 1, 0};
    bool m_hasTarget = false;
    float m_fireCooldown = 0.0f;

    void updatePhysics(float dt, World& world);
    void checkVoxelCollisions(float dt, World& world);
    void buildViewmodelMesh();
};
