#include "Network.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "Shader.hpp"
#include "TextureAtlas.hpp"
#include "Projectile.hpp"
#include "Audio.hpp"

#include <iostream>
#include <cstring>
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>

NetworkManager& NetworkManager::instance() {
    static NetworkManager s_mgr;
    return s_mgr;
}

static bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

static bool enableBroadcast(int fd) {
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) == 0;
}

static bool enableReuseAddr(int fd) {
    int opt = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == 0;
}

bool NetworkManager::init() {
    buildAstronautMesh();
    return true;
}

void NetworkManager::cleanup() {
    disconnect();
    if (m_playerVAO != 0) {
        glDeleteVertexArrays(1, &m_playerVAO);
        m_playerVAO = 0;
    }
    if (m_playerVBO != 0) {
        glDeleteBuffers(1, &m_playerVBO);
        m_playerVBO = 0;
    }
}

bool NetworkManager::startHost(int port, const std::string& serverName) {
    disconnect();

    m_gamePort = port;
    m_serverName = serverName;
    m_role = NetworkRole::Host;
    m_localPlayerId = 1;

    // Create game socket
    m_gameSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_gameSocket < 0) {
        std::cerr << "Failed to create game socket" << std::endl;
        m_role = NetworkRole::Offline;
        return false;
    }

    enableReuseAddr(m_gameSocket);
    setNonBlocking(m_gameSocket);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(m_gamePort);

    if (bind(m_gameSocket, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind host game socket to port " << m_gamePort << std::endl;
        close(m_gameSocket);
        m_gameSocket = -1;
        m_role = NetworkRole::Offline;
        return false;
    }

    // Create beacon broadcast socket
    m_beaconSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_beaconSocket >= 0) {
        enableBroadcast(m_beaconSocket);
        enableReuseAddr(m_beaconSocket);
        setNonBlocking(m_beaconSocket);

        sockaddr_in baddr{};
        baddr.sin_family = AF_INET;
        baddr.sin_addr.s_addr = INADDR_ANY;
        baddr.sin_port = htons(m_beaconPort);
        bind(m_beaconSocket, (sockaddr*)&baddr, sizeof(baddr));
    }

    std::cout << "[Multiplayer] Hosting LAN Game on port " << m_gamePort << " ('" << m_serverName << "')" << std::endl;
    return true;
}

bool NetworkManager::connectTo(const std::string& hostIp, int port) {
    disconnect();

    m_hostIp = hostIp;
    m_gamePort = port;
    m_role = NetworkRole::Client;

    m_gameSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_gameSocket < 0) {
        std::cerr << "Failed to create client socket" << std::endl;
        m_role = NetworkRole::Offline;
        return false;
    }

    setNonBlocking(m_gameSocket);

    // Send Join Request to Host
    PacketJoinRequest req;
    req.header.type = PacketType::JoinRequest;
    req.header.playerId = 0;
    std::strncpy(req.playerName, "Explorer", sizeof(req.playerName) - 1);

    sockaddr_in hostAddr{};
    hostAddr.sin_family = AF_INET;
    hostAddr.sin_port = htons(m_gamePort);
    inet_pton(AF_INET, m_hostIp.c_str(), &hostAddr.sin_addr);

    sendto(m_gameSocket, &req, sizeof(req), 0, (sockaddr*)&hostAddr, sizeof(hostAddr));

    std::cout << "[Multiplayer] Connecting to host " << m_hostIp << ":" << m_gamePort << "..." << std::endl;
    return true;
}

void NetworkManager::disconnect() {
    if (m_role != NetworkRole::Offline && m_gameSocket >= 0) {
        PacketPlayerLeave leavePkt;
        leavePkt.header.type = PacketType::PlayerLeave;
        leavePkt.header.playerId = m_localPlayerId;

        if (m_role == NetworkRole::Client) {
            sockaddr_in hostAddr{};
            hostAddr.sin_family = AF_INET;
            hostAddr.sin_port = htons(m_gamePort);
            inet_pton(AF_INET, m_hostIp.c_str(), &hostAddr.sin_addr);
            sendto(m_gameSocket, &leavePkt, sizeof(leavePkt), 0, (sockaddr*)&hostAddr, sizeof(hostAddr));
        } else if (m_role == NetworkRole::Host) {
            for (const auto& pair : m_clientEndpoints) {
                sockaddr_in clientAddr{};
                clientAddr.sin_family = AF_INET;
                clientAddr.sin_addr.s_addr = pair.second.ip;
                clientAddr.sin_port = pair.second.port;
                sendto(m_gameSocket, &leavePkt, sizeof(leavePkt), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
            }
        }
    }

    if (m_gameSocket >= 0) {
        close(m_gameSocket);
        m_gameSocket = -1;
    }
    if (m_beaconSocket >= 0) {
        close(m_beaconSocket);
        m_beaconSocket = -1;
    }

    m_role = NetworkRole::Offline;
    m_remotePlayers.clear();
    m_clientEndpoints.clear();
}

void NetworkManager::broadcastBeacon() {
    if (m_role != NetworkRole::Host || m_beaconSocket < 0) return;

    PacketDiscoveryPong pong;
    pong.header.type = PacketType::DiscoveryPong;
    pong.header.playerId = 1;
    std::strncpy(pong.serverName, m_serverName.c_str(), sizeof(pong.serverName) - 1);
    pong.port = (uint16_t)m_gamePort;
    pong.currentPlayers = (uint8_t)(m_remotePlayers.size() + 1);
    pong.maxPlayers = 8;

    sockaddr_in bcastAddr{};
    bcastAddr.sin_family = AF_INET;
    bcastAddr.sin_port = htons(m_beaconPort);
    bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(m_beaconSocket, &pong, sizeof(pong), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));
}

void NetworkManager::sendDiscoveryPing() {
    int sock = (m_beaconSocket >= 0) ? m_beaconSocket : m_gameSocket;
    if (sock < 0) {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        enableBroadcast(sock);
        setNonBlocking(sock);
        m_beaconSocket = sock;
    }

    PacketDiscoveryPing ping;
    ping.header.type = PacketType::DiscoveryPing;

    sockaddr_in bcastAddr{};
    bcastAddr.sin_family = AF_INET;
    bcastAddr.sin_port = htons(m_beaconPort);
    bcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    sendto(sock, &ping, sizeof(ping), 0, (sockaddr*)&bcastAddr, sizeof(bcastAddr));
}

void NetworkManager::sendPlayerState(const Player& localPlayer) {
    if (m_gameSocket < 0 || m_role == NetworkRole::Offline) return;

    PacketPlayerState pkt;
    pkt.header.type = PacketType::PlayerState;
    pkt.header.playerId = m_localPlayerId;
    pkt.pos = localPlayer.getPosition();
    pkt.vel = localPlayer.getVelocity();
    pkt.up = localPlayer.getUp();
    pkt.pitch = localPlayer.getPitch();
    pkt.yaw = localPlayer.getYaw();
    pkt.selectedItem = (uint8_t)localPlayer.getInventory().getSelectedItem().type;
    pkt.flags = 0;
    if (localPlayer.isJetpackActive()) pkt.flags |= 1;
    if (localPlayer.isMining()) pkt.flags |= 2;
    if (localPlayer.isFlashlightOn()) pkt.flags |= 4;
    pkt.health = localPlayer.getHealth();

    if (m_role == NetworkRole::Client) {
        sockaddr_in hostAddr{};
        hostAddr.sin_family = AF_INET;
        hostAddr.sin_port = htons(m_gamePort);
        inet_pton(AF_INET, m_hostIp.c_str(), &hostAddr.sin_addr);
        sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&hostAddr, sizeof(hostAddr));
    } else if (m_role == NetworkRole::Host) {
        for (const auto& pair : m_clientEndpoints) {
            sockaddr_in clientAddr{};
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_addr.s_addr = pair.second.ip;
            clientAddr.sin_port = pair.second.port;
            sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
        }
    }
}

void NetworkManager::sendBlockChange(int x, int y, int z, BlockType type) {
    if (m_gameSocket < 0 || m_role == NetworkRole::Offline) return;

    PacketBlockSet pkt;
    pkt.header.type = PacketType::BlockSet;
    pkt.header.playerId = m_localPlayerId;
    pkt.x = x;
    pkt.y = y;
    pkt.z = z;
    pkt.blockType = (uint8_t)type;

    if (m_role == NetworkRole::Client) {
        sockaddr_in hostAddr{};
        hostAddr.sin_family = AF_INET;
        hostAddr.sin_port = htons(m_gamePort);
        inet_pton(AF_INET, m_hostIp.c_str(), &hostAddr.sin_addr);
        sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&hostAddr, sizeof(hostAddr));
    } else if (m_role == NetworkRole::Host) {
        for (const auto& pair : m_clientEndpoints) {
            sockaddr_in clientAddr{};
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_addr.s_addr = pair.second.ip;
            clientAddr.sin_port = pair.second.port;
            sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
        }
    }
}

void NetworkManager::sendShoot(const Vec3& origin, const Vec3& dir, float speed, float damage, bool overclocked) {
    if (m_gameSocket < 0 || m_role == NetworkRole::Offline) return;

    PacketLaserShoot pkt;
    pkt.header.type = PacketType::LaserShoot;
    pkt.header.playerId = m_localPlayerId;
    pkt.origin = origin;
    pkt.dir = dir;
    pkt.speed = speed;
    pkt.damage = damage;
    pkt.isOverclocked = overclocked ? 1 : 0;

    if (m_role == NetworkRole::Client) {
        sockaddr_in hostAddr{};
        hostAddr.sin_family = AF_INET;
        hostAddr.sin_port = htons(m_gamePort);
        inet_pton(AF_INET, m_hostIp.c_str(), &hostAddr.sin_addr);
        sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&hostAddr, sizeof(hostAddr));
    } else if (m_role == NetworkRole::Host) {
        for (const auto& pair : m_clientEndpoints) {
            sockaddr_in clientAddr{};
            clientAddr.sin_family = AF_INET;
            clientAddr.sin_addr.s_addr = pair.second.ip;
            clientAddr.sin_port = pair.second.port;
            sendto(m_gameSocket, &pkt, sizeof(pkt), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
        }
    }
}

void NetworkManager::processIncomingPackets(World& world, Player& localPlayer) {
    uint8_t buffer[2048];
    sockaddr_in fromAddr{};
    socklen_t fromLen = sizeof(fromAddr);

    // 1. Process Beacon Socket (Discovery Ping / Pong)
    if (m_beaconSocket >= 0) {
        while (true) {
            ssize_t bytes = recvfrom(m_beaconSocket, buffer, sizeof(buffer), 0, (sockaddr*)&fromAddr, &fromLen);
            if (bytes < (ssize_t)sizeof(PacketHeader)) break;

            const PacketHeader* hdr = reinterpret_cast<const PacketHeader*>(buffer);
            if (hdr->magic != 0x52414D41) continue;

            if (hdr->type == PacketType::DiscoveryPing && m_role == NetworkRole::Host) {
                // Respond with pong
                PacketDiscoveryPong pong;
                pong.header.type = PacketType::DiscoveryPong;
                pong.header.playerId = 1;
                std::strncpy(pong.serverName, m_serverName.c_str(), sizeof(pong.serverName) - 1);
                pong.port = (uint16_t)m_gamePort;
                pong.currentPlayers = (uint8_t)(m_remotePlayers.size() + 1);
                pong.maxPlayers = 8;
                sendto(m_beaconSocket, &pong, sizeof(pong), 0, (sockaddr*)&fromAddr, fromLen);
            } else if (hdr->type == PacketType::DiscoveryPong) {
                const PacketDiscoveryPong* pong = reinterpret_cast<const PacketDiscoveryPong*>(buffer);
                char ipStr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &fromAddr.sin_addr, ipStr, sizeof(ipStr));

                bool exists = false;
                for (auto& s : m_discoveredServers) {
                    if (s.ip == ipStr && s.port == pong->port) {
                        s.name = pong->serverName;
                        s.currentPlayers = pong->currentPlayers;
                        s.maxPlayers = pong->maxPlayers;
                        s.lastSeen = 0.0f;
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    DiscoveredServer ds;
                    ds.ip = ipStr;
                    ds.port = pong->port;
                    ds.name = pong->serverName;
                    ds.currentPlayers = pong->currentPlayers;
                    ds.maxPlayers = pong->maxPlayers;
                    ds.lastSeen = 0.0f;
                    m_discoveredServers.push_back(ds);
                }
            }
        }
    }

    // 2. Process Game Socket
    if (m_gameSocket >= 0) {
        while (true) {
            ssize_t bytes = recvfrom(m_gameSocket, buffer, sizeof(buffer), 0, (sockaddr*)&fromAddr, &fromLen);
            if (bytes < (ssize_t)sizeof(PacketHeader)) break;

            const PacketHeader* hdr = reinterpret_cast<const PacketHeader*>(buffer);
            if (hdr->magic != 0x52414D41) continue;

            // Handle Join Request (Host mode)
            if (hdr->type == PacketType::JoinRequest && m_role == NetworkRole::Host) {
                const PacketJoinRequest* req = reinterpret_cast<const PacketJoinRequest*>(buffer);
                uint8_t newId = 2;
                while (m_remotePlayers.find(newId) != m_remotePlayers.end() || m_clientEndpoints.find(newId) != m_clientEndpoints.end()) {
                    newId++;
                }

                ClientEndpoint ep;
                ep.ip = fromAddr.sin_addr.s_addr;
                ep.port = fromAddr.sin_port;
                ep.lastSeen = 0.0f;
                m_clientEndpoints[newId] = ep;

                RemotePlayer rp;
                rp.id = newId;
                rp.name = req->playerName;
                rp.pos = {0, 18.0f, 4.0f};
                rp.targetPos = rp.pos;
                m_remotePlayers[newId] = rp;

                // Send Accept back
                PacketJoinAccept ack;
                ack.header.type = PacketType::JoinAccept;
                ack.header.playerId = 1;
                ack.assignedId = newId;
                ack.cylinderRadius = World::CYLINDER_RADIUS;
                ack.circumference = World::CIRCUMFERENCE;
                sendto(m_gameSocket, &ack, sizeof(ack), 0, (sockaddr*)&fromAddr, fromLen);

                // Send local player state immediately
                sendPlayerState(localPlayer);

                AudioSystem::instance().playSound(SoundEffect::CraftItem);
                std::cout << "[Multiplayer] Player '" << req->playerName << "' connected (ID " << (int)newId << ")" << std::endl;
            }
            // Handle Join Accept (Client mode)
            else if (hdr->type == PacketType::JoinAccept && m_role == NetworkRole::Client) {
                const PacketJoinAccept* ack = reinterpret_cast<const PacketJoinAccept*>(buffer);
                m_localPlayerId = ack->assignedId;
                AudioSystem::instance().playSound(SoundEffect::CraftItem);
                std::cout << "[Multiplayer] Successfully connected to Rama server! Assigned Player ID: " << (int)m_localPlayerId << std::endl;
            }
            // Handle Player State
            else if (hdr->type == PacketType::PlayerState) {
                const PacketPlayerState* state = reinterpret_cast<const PacketPlayerState*>(buffer);
                uint8_t pid = state->header.playerId;

                if (pid != m_localPlayerId) {
                    auto& rp = m_remotePlayers[pid];
                    rp.id = pid;
                    rp.targetPos = state->pos;
                    rp.vel = state->vel;
                    rp.up = state->up;
                    rp.targetPitch = state->pitch;
                    rp.targetYaw = state->yaw;
                    rp.selectedItem = (ItemType)state->selectedItem;
                    rp.isJetpacking = (state->flags & 1) != 0;
                    rp.isMining = (state->flags & 2) != 0;
                    rp.isFlashlightOn = (state->flags & 4) != 0;
                    rp.health = state->health;
                    rp.lastPacketTime = 0.0f;

                    // If Host: forward state packet to all other clients
                    if (m_role == NetworkRole::Host) {
                        for (const auto& pair : m_clientEndpoints) {
                            if (pair.first != pid) {
                                sockaddr_in clientAddr{};
                                clientAddr.sin_family = AF_INET;
                                clientAddr.sin_addr.s_addr = pair.second.ip;
                                clientAddr.sin_port = pair.second.port;
                                sendto(m_gameSocket, state, sizeof(*state), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                            }
                        }
                    }
                }
            }
            // Handle Block Set
            else if (hdr->type == PacketType::BlockSet) {
                const PacketBlockSet* bset = reinterpret_cast<const PacketBlockSet*>(buffer);
                world.setBlock(bset->x, bset->y, bset->z, (BlockType)bset->blockType);
                if (bset->blockType == (uint8_t)BlockType::Torch) {
                    world.addTorch({bset->x, bset->y, bset->z});
                }

                // If Host: relay block set to other clients
                if (m_role == NetworkRole::Host) {
                    for (const auto& pair : m_clientEndpoints) {
                        if (pair.first != hdr->playerId) {
                            sockaddr_in clientAddr{};
                            clientAddr.sin_family = AF_INET;
                            clientAddr.sin_addr.s_addr = pair.second.ip;
                            clientAddr.sin_port = pair.second.port;
                            sendto(m_gameSocket, bset, sizeof(*bset), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                        }
                    }
                }
            }
            // Handle Laser Shoot
            else if (hdr->type == PacketType::LaserShoot) {
                const PacketLaserShoot* shoot = reinterpret_cast<const PacketLaserShoot*>(buffer);
                ProjectileManager::instance().spawnBolt(shoot->origin, shoot->dir, shoot->speed, shoot->damage, shoot->isOverclocked != 0);

                if (m_role == NetworkRole::Host) {
                    for (const auto& pair : m_clientEndpoints) {
                        if (pair.first != hdr->playerId) {
                            sockaddr_in clientAddr{};
                            clientAddr.sin_family = AF_INET;
                            clientAddr.sin_addr.s_addr = pair.second.ip;
                            clientAddr.sin_port = pair.second.port;
                            sendto(m_gameSocket, shoot, sizeof(*shoot), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                        }
                    }
                }
            }
            // Handle Player Leave
            else if (hdr->type == PacketType::PlayerLeave) {
                uint8_t pid = hdr->playerId;
                m_remotePlayers.erase(pid);
                m_clientEndpoints.erase(pid);
                std::cout << "[Multiplayer] Player " << (int)pid << " disconnected." << std::endl;

                if (m_role == NetworkRole::Host) {
                    for (const auto& pair : m_clientEndpoints) {
                        if (pair.first != pid) {
                            sockaddr_in clientAddr{};
                            clientAddr.sin_family = AF_INET;
                            clientAddr.sin_addr.s_addr = pair.second.ip;
                            clientAddr.sin_port = pair.second.port;
                            sendto(m_gameSocket, buffer, bytes, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                        }
                    }
                }
            }
        }
    }
}

void NetworkManager::update(float dt, World& world, Player& localPlayer) {
    // 1. Discovery scan & beacon broadcast
    m_discoveryScanTimer += dt;
    if (m_discoveryScanTimer > 1.2f) {
        m_discoveryScanTimer = 0.0f;
        sendDiscoveryPing();
    }

    if (m_role == NetworkRole::Host) {
        m_beaconBroadcastTimer += dt;
        if (m_beaconBroadcastTimer > 1.0f) {
            m_beaconBroadcastTimer = 0.0f;
            broadcastBeacon();
        }
    }

    // Prune stale discovered servers
    for (auto& s : m_discoveredServers) s.lastSeen += dt;
    m_discoveredServers.erase(
        std::remove_if(m_discoveredServers.begin(), m_discoveredServers.end(), [](const DiscoveredServer& s) { return s.lastSeen > 5.0f; }),
        m_discoveredServers.end()
    );

    // 2. Process incoming packets
    processIncomingPackets(world, localPlayer);

    // 3. Broadcast local player state (20Hz)
    if (m_role != NetworkRole::Offline) {
        m_stateBroadcastTimer += dt;
        if (m_stateBroadcastTimer >= 0.05f) {
            m_stateBroadcastTimer = 0.0f;
            sendPlayerState(localPlayer);
        }
    }

    // 4. Update Remote Player interpolation and animations
    for (auto& pair : m_remotePlayers) {
        auto& rp = pair.second;
        rp.lastPacketTime += dt;

        // Smooth position and rotation lerp
        rp.pos = Vec3::lerp(rp.pos, rp.targetPos, std::min(1.0f, 18.0f * dt));
        rp.yaw = rp.yaw + (rp.targetYaw - rp.yaw) * std::min(1.0f, 18.0f * dt);
        rp.pitch = rp.pitch + (rp.targetPitch - rp.pitch) * std::min(1.0f, 18.0f * dt);

        if (rp.vel.lengthSq() > 0.1f) {
            rp.legAnim += dt * 10.0f;
        }

        // Remote jetpack particle exhaust
        if (rp.isJetpacking) {
            ProjectileManager::instance().spawnJetpackExhaust(rp.pos - rp.up * 0.4f, rp.up * -1.0f);
        }
    }

    // Prune timed out remote players (> 8 seconds no packet)
    std::vector<uint8_t> timedOut;
    for (const auto& pair : m_remotePlayers) {
        if (pair.second.lastPacketTime > 8.0f) {
            timedOut.push_back(pair.first);
        }
    }
    for (uint8_t id : timedOut) {
        m_remotePlayers.erase(id);
        m_clientEndpoints.erase(id);
    }
}

void NetworkManager::buildAstronautMesh() {
    struct EntityVertex {
        Vec3 pos;
        Vec2 uv;
        Vec3 normal;
    };

    std::vector<EntityVertex> verts;

    auto addBox = [&](const Vec3& min, const Vec3& max, int tx, int ty) {
        float u0, v0, u1, v1;
        TextureAtlas::instance().getTileUVs(tx, ty, u0, v0, u1, v1);

        // Top (+Y)
        verts.push_back({{min.x, max.y, max.z}, {u0, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {0, 1, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u0, v1}, {0, 1, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {0, 1, 0}});
        verts.push_back({{min.x, max.y, min.z}, {u0, v0}, {0, 1, 0}});

        // Bottom (-Y)
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, min.z}, {u1, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v0}, {0, -1, 0}});
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {0, -1, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v0}, {0, -1, 0}});
        verts.push_back({{min.x, min.y, max.z}, {u0, v0}, {0, -1, 0}});

        // Front (+Z)
        verts.push_back({{min.x, min.y, max.z}, {u0, v1}, {0, 0, 1}});
        verts.push_back({{max.x, min.y, max.z}, {u1, v1}, {0, 0, 1}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v0}, {0, 0, 1}});
        verts.push_back({{min.x, min.y, max.z}, {u0, v1}, {0, 0, 1}});
        verts.push_back({{max.x, max.y, max.z}, {u1, v0}, {0, 0, 1}});
        verts.push_back({{min.x, max.y, max.z}, {u0, v0}, {0, 0, 1}});

        // Back (-Z)
        verts.push_back({{max.x, min.y, min.z}, {u0, v1}, {0, 0, -1}});
        verts.push_back({{min.x, min.y, min.z}, {u1, v1}, {0, 0, -1}});
        verts.push_back({{min.x, max.y, min.z}, {u1, v0}, {0, 0, -1}});
        verts.push_back({{max.x, min.y, min.z}, {u0, v1}, {0, 0, -1}});
        verts.push_back({{min.x, max.y, min.z}, {u1, v0}, {0, 0, -1}});
        verts.push_back({{max.x, max.y, min.z}, {u0, v0}, {0, 0, -1}});

        // Left (-X)
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, min.y, max.z}, {u1, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u1, v0}, {-1, 0, 0}});
        verts.push_back({{min.x, min.y, min.z}, {u0, v1}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, max.z}, {u1, v0}, {-1, 0, 0}});
        verts.push_back({{min.x, max.y, min.z}, {u0, v0}, {-1, 0, 0}});

        // Right (+X)
        verts.push_back({{max.x, min.y, max.z}, {u0, v1}, {1, 0, 0}});
        verts.push_back({{max.x, min.y, min.z}, {u1, v1}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {1, 0, 0}});
        verts.push_back({{max.x, min.y, max.z}, {u0, v1}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, min.z}, {u1, v0}, {1, 0, 0}});
        verts.push_back({{max.x, max.y, max.z}, {u0, v0}, {1, 0, 0}});
    };

    // 1. Torso / Spacesuit Chest (tx=0, ty=0 Hull alloy white/gray)
    addBox({-0.25f, 0.7f, -0.15f}, {0.25f, 1.35f, 0.15f}, 0, 0);

    // 2. Life Support Unit / Jetpack on Back (tx=1, ty=3 Titanium)
    addBox({-0.20f, 0.8f, -0.28f}, {0.20f, 1.30f, -0.15f}, 1, 3);
    // Twin Thruster Nozzles
    addBox({-0.18f, 0.68f, -0.26f}, {-0.06f, 0.8f, -0.17f}, 4, 0);
    addBox({0.06f, 0.68f, -0.26f}, {0.18f, 0.8f, -0.17f}, 4, 0);

    // 3. Astronaut Helmet (tx=0, ty=0)
    addBox({-0.20f, 1.35f, -0.20f}, {0.20f, 1.75f, 0.20f}, 0, 0);

    // 4. Gold Solar Reflective Visor (tx=14, ty=0 Glowing gold/amber glass)
    addBox({-0.16f, 1.42f, 0.18f}, {0.16f, 1.68f, 0.22f}, 14, 0);

    // 5. Arms
    // Left Arm
    addBox({-0.40f, 0.7f, -0.12f}, {-0.25f, 1.30f, 0.12f}, 0, 0);
    // Right Arm (holding weapon forward)
    addBox({0.25f, 0.7f, -0.12f}, {0.40f, 1.30f, 0.12f}, 0, 0);
    addBox({0.26f, 0.85f, 0.12f}, {0.38f, 1.05f, 0.55f}, 2, 3); // Weapon barrel

    // 6. Left & Right Legs
    addBox({-0.22f, 0.0f, -0.12f}, {-0.03f, 0.7f, 0.12f}, 0, 0);
    addBox({0.03f, 0.0f, -0.12f}, {0.22f, 0.7f, 0.12f}, 0, 0);

    if (m_playerVAO == 0) {
        glGenVertexArrays(1, &m_playerVAO);
        glGenBuffers(1, &m_playerVBO);
    }

    glBindVertexArray(m_playerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_playerVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(EntityVertex), verts.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(EntityVertex), (void*)offsetof(EntityVertex, normal));

    m_playerVertexCount = (GLsizei)verts.size();
    glBindVertexArray(0);
}

void NetworkManager::renderRemotePlayers(const Shader& shader, const Vec3& playerPos, const Mat4& view, const Mat4& proj) {
    if (m_remotePlayers.empty() || m_playerVertexCount == 0) return;

    shader.use();
    shader.setMat4("uView", view);
    shader.setMat4("uProjection", proj);
    shader.setVec3("uPlayerPos", playerPos);
    shader.setFloat("uCylinderRadius", World::CYLINDER_RADIUS);
    shader.setFloat("uCurvatureEnable", 1.0f);

    glBindVertexArray(m_playerVAO);

    for (const auto& pair : m_remotePlayers) {
        const auto& rp = pair.second;

        // Calculate model transform aligned with the cylinder Up and Yaw rotation
        Mat4 model = Mat4::translation(rp.pos) *
                     Mat4::rotationY(rp.yaw * DEG2RAD) *
                     Mat4::rotationX(rp.pitch * DEG2RAD * 0.3f);

        shader.setMat4("uModel", model);
        shader.setVec4("uTint", Vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glDrawArrays(GL_TRIANGLES, 0, m_playerVertexCount);
    }

    glBindVertexArray(0);
}

std::string NetworkManager::getStatusText() const {
    if (m_role == NetworkRole::Host) {
        return "Hosting on LAN Port " + std::to_string(m_gamePort) + " (" + std::to_string(m_remotePlayers.size() + 1) + " players)";
    } else if (m_role == NetworkRole::Client) {
        return "Connected to " + m_hostIp + ":" + std::to_string(m_gamePort) + " (Player #" + std::to_string(m_localPlayerId) + ")";
    }
    return "Offline (Single Player)";
}
