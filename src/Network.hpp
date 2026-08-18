#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "Math3D.hpp"
#include "Block.hpp"

#include "GLCommon.hpp"

class Shader;
class World;
class Player;

enum class NetworkRole {
    Offline,
    Host,
    Client
};

enum class PacketType : uint8_t {
    DiscoveryPing = 1,
    DiscoveryPong = 2,
    JoinRequest   = 3,
    JoinAccept    = 4,
    PlayerState   = 5,
    BlockSet      = 6,
    LaserShoot    = 7,
    PlayerLeave   = 8,
    GrenadeThrow  = 9,
    DamagePvP     = 10,
    Explosion     = 11
};

#pragma pack(push, 1)

struct PacketHeader {
    uint32_t magic = 0x52414D41; // 'RAMA'
    PacketType type;
    uint8_t playerId = 0;
};

struct PacketDiscoveryPing {
    PacketHeader header;
};

struct PacketDiscoveryPong {
    PacketHeader header;
    char serverName[32];
    uint16_t port;
    uint8_t currentPlayers;
    uint8_t maxPlayers;
};

struct PacketJoinRequest {
    PacketHeader header;
    char playerName[24];
};

struct PacketJoinAccept {
    PacketHeader header;
    uint8_t assignedId;
    float cylinderRadius;
    float circumference;
};

struct PacketPlayerState {
    PacketHeader header;
    Vec3 pos;
    Vec3 vel;
    Vec3 up;
    float pitch;
    float yaw;
    uint8_t selectedItem;
    uint8_t flags; // bit 0: jetpack, bit 1: mining, bit 2: flashlight
    float health;
};

struct PacketBlockSet {
    PacketHeader header;
    int32_t x;
    int32_t y;
    int32_t z;
    uint8_t blockType;
};

struct PacketLaserShoot {
    PacketHeader header;
    Vec3 origin;
    Vec3 dir;
    float speed;
    float damage;
    uint8_t isOverclocked;
};

struct PacketGrenadeThrow {
    PacketHeader header;
    Vec3 origin;
    Vec3 vel;
};

struct PacketDamagePvP {
    PacketHeader header;
    uint8_t targetPlayerId;
    float damage;
    Vec3 knockback;
};

struct PacketExplosion {
    PacketHeader header;
    Vec3 pos;
    float radius;
    float maxDamage;
};

struct PacketPlayerLeave {
    PacketHeader header;
};

#pragma pack(pop)

struct DiscoveredServer {
    std::string ip;
    uint16_t port;
    std::string name;
    int currentPlayers = 1;
    int maxPlayers = 8;
    float lastSeen = 0.0f;
};

struct RemotePlayer {
    uint8_t id = 0;
    std::string name = "Astronaut";
    Vec3 pos = {0, 20, 0};
    Vec3 targetPos = {0, 20, 0};
    Vec3 vel = {0, 0, 0};
    Vec3 up = {0, 1, 0};
    float pitch = 0.0f;
    float yaw = 0.0f;
    float targetYaw = 0.0f;
    float targetPitch = 0.0f;
    ItemType selectedItem = ItemType::RayGun;
    bool isJetpacking = false;
    bool isMining = false;
    bool isFlashlightOn = false;
    float health = 100.0f;
    float legAnim = 0.0f;
    float lastPacketTime = 0.0f;
};

class NetworkManager {
public:
    static NetworkManager& instance();

    bool init();
    void cleanup();

    bool startHost(int port = 7777, const std::string& serverName = "Rama Expedition");
    bool connectTo(const std::string& hostIp, int port = 7777);
    void disconnect();

    void update(float dt, World& world, Player& localPlayer);
    void renderRemotePlayers(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj);

    void sendBlockChange(int x, int y, int z, BlockType type);
    void sendShoot(const Vec3& origin, const Vec3& dir, float speed, float damage, bool overclocked);
    void sendGrenadeThrow(const Vec3& origin, const Vec3& vel);
    void sendDamagePvP(uint8_t targetPlayerId, float damage, const Vec3& knockback);
    void sendExplosion(const Vec3& pos, float radius, float maxDamage);

    NetworkRole getRole() const { return m_role; }
    bool isConnected() const { return m_role != NetworkRole::Offline; }
    bool isHost() const { return m_role == NetworkRole::Host; }
    bool isClient() const { return m_role == NetworkRole::Client; }
    uint8_t getLocalPlayerId() const { return m_localPlayerId; }

    const std::vector<DiscoveredServer>& getDiscoveredServers() const { return m_discoveredServers; }
    const std::unordered_map<uint8_t, RemotePlayer>& getRemotePlayers() const { return m_remotePlayers; }
    std::unordered_map<uint8_t, RemotePlayer>& getRemotePlayersMutable() { return m_remotePlayers; }

    std::string getStatusText() const;
    int getPort() const { return m_gamePort; }

private:
    NetworkManager() = default;

    NetworkRole m_role = NetworkRole::Offline;
    std::string m_serverName = "Rama Station";
    std::string m_hostIp = "127.0.0.1";
    int m_gamePort = 7777;
    int m_beaconPort = 7778;

    int m_gameSocket = -1;
    int m_beaconSocket = -1;

    uint8_t m_localPlayerId = 1;
    float m_stateBroadcastTimer = 0.0f;
    float m_beaconBroadcastTimer = 0.0f;
    float m_discoveryScanTimer = 0.0f;

    std::vector<DiscoveredServer> m_discoveredServers;
    std::unordered_map<uint8_t, RemotePlayer> m_remotePlayers;

    // Client connection addresses (Host mode)
    struct ClientEndpoint {
        uint32_t ip;
        uint16_t port;
        float lastSeen;
    };
    std::unordered_map<uint8_t, ClientEndpoint> m_clientEndpoints;

    // 3D Astronaut Model Mesh
    GLuint m_playerVAO = 0;
    GLuint m_playerVBO = 0;
    GLsizei m_playerVertexCount = 0;

    void buildAstronautMesh();
    void processIncomingPackets(World& world, Player& localPlayer);
    void broadcastBeacon();
    void sendDiscoveryPing();
    void sendPlayerState(const Player& localPlayer);
};
