#include "Chunk.hpp"
#include "World.hpp"
#include "TextureAtlas.hpp"
#include <cstring>

Chunk::Chunk(int cx, int cz, World* world)
    : m_chunkX(cx), m_chunkZ(cz), m_world(world)
{
    std::memset(m_blocks, 0, sizeof(m_blocks));
}

Chunk::~Chunk() {
    if (m_vaoOpaque != 0) glDeleteVertexArrays(1, &m_vaoOpaque);
    if (m_vboOpaque != 0) glDeleteBuffers(1, &m_vboOpaque);
    if (m_vaoTrans != 0) glDeleteVertexArrays(1, &m_vaoTrans);
    if (m_vboTrans != 0) glDeleteBuffers(1, &m_vboTrans);
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= CHUNK_SIZE_X || y < 0 || y >= CHUNK_SIZE_Y || z < 0 || z >= CHUNK_SIZE_Z) {
        return BlockType::Air;
    }
    return m_blocks[x][y][z];
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (x >= 0 && x < CHUNK_SIZE_X && y >= 0 && y < CHUNK_SIZE_Y && z >= 0 && z < CHUNK_SIZE_Z) {
        m_blocks[x][y][z] = type;
        m_isDirty = true;
    }
}

BlockType Chunk::getNeighborBlock(int gx, int gy, int gz) const {
    if (gy < 0 || gy >= CHUNK_SIZE_Y) return BlockType::Air;

    int lx = gx - m_chunkX * CHUNK_SIZE_X;
    int lz = gz - m_chunkZ * CHUNK_SIZE_Z;

    if (lx >= 0 && lx < CHUNK_SIZE_X && lz >= 0 && lz < CHUNK_SIZE_Z) {
        return m_blocks[lx][gy][lz];
    }
    if (m_world) {
        return m_world->getBlock(gx, gy, gz);
    }
    return BlockType::Air;
}

float Chunk::calculateAO(int x, int y, int z, int corner, int face) {
    // Face normals: 0: +Y (Top), 1: -Y (Bottom), 2: -Z (North), 3: +Z (South), 4: -X (West), 5: +X (East)
    int gx = m_chunkX * CHUNK_SIZE_X + x;
    int gy = y;
    int gz = m_chunkZ * CHUNK_SIZE_Z + z;

    bool side1 = false, side2 = false, cornerBlock = false;

    // Check adjacent 3 blocks for corner AO
    switch (face) {
        case 0: // +Y (Top)
            if (corner == 0) { // -X, -Z
                side1 = BlockRegistry::get(getNeighborBlock(gx - 1, gy + 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy + 1, gz - 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx - 1, gy + 1, gz - 1)).isSolid;
            } else if (corner == 1) { // +X, -Z
                side1 = BlockRegistry::get(getNeighborBlock(gx + 1, gy + 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy + 1, gz - 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx + 1, gy + 1, gz - 1)).isSolid;
            } else if (corner == 2) { // +X, +Z
                side1 = BlockRegistry::get(getNeighborBlock(gx + 1, gy + 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy + 1, gz + 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx + 1, gy + 1, gz + 1)).isSolid;
            } else { // -X, +Z
                side1 = BlockRegistry::get(getNeighborBlock(gx - 1, gy + 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy + 1, gz + 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx - 1, gy + 1, gz + 1)).isSolid;
            }
            break;
        case 1: // -Y (Bottom)
            if (corner == 0) {
                side1 = BlockRegistry::get(getNeighborBlock(gx - 1, gy - 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy - 1, gz - 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx - 1, gy - 1, gz - 1)).isSolid;
            } else if (corner == 1) {
                side1 = BlockRegistry::get(getNeighborBlock(gx + 1, gy - 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy - 1, gz - 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx + 1, gy - 1, gz - 1)).isSolid;
            } else if (corner == 2) {
                side1 = BlockRegistry::get(getNeighborBlock(gx + 1, gy - 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy - 1, gz + 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx + 1, gy - 1, gz + 1)).isSolid;
            } else {
                side1 = BlockRegistry::get(getNeighborBlock(gx - 1, gy - 1, gz)).isSolid;
                side2 = BlockRegistry::get(getNeighborBlock(gx, gy - 1, gz + 1)).isSolid;
                cornerBlock = BlockRegistry::get(getNeighborBlock(gx - 1, gy - 1, gz + 1)).isSolid;
            }
            break;
        default:
            return 1.0f; // Simplified side AO for high performance
    }

    if (side1 && side2) return 0.25f;
    int count = (side1 ? 1 : 0) + (side2 ? 1 : 0) + (cornerBlock ? 1 : 0);
    return 1.0f - (float)count * 0.22f;
}

void Chunk::addFace(int x, int y, int z, int face, BlockType block, bool isTrans) {
    const BlockInfo& info = BlockRegistry::get(block);

    // World position of the voxel corner
    float wx = (float)(m_chunkX * CHUNK_SIZE_X + x);
    float wy = (float)y;
    float wz = (float)(m_chunkZ * CHUNK_SIZE_Z + z);

    int tx = info.texSideX, ty = info.texSideY;
    if (face == 0) { tx = info.texTopX; ty = info.texTopY; }
    else if (face == 1) { tx = info.texBottomX; ty = info.texBottomY; }

    float u0, v0, u1, v1;
    TextureAtlas::instance().getTileUVs(tx, ty, u0, v0, u1, v1);

    Vec3 normal;
    Vec3 p0, p1, p2, p3;
    float ao0 = 1.0f, ao1 = 1.0f, ao2 = 1.0f, ao3 = 1.0f;

    switch (face) {
        case 0: // Top (+Y)
            normal = {0, 1, 0};
            p0 = {wx, wy + 1.0f, wz + 1.0f};
            p1 = {wx + 1.0f, wy + 1.0f, wz + 1.0f};
            p2 = {wx + 1.0f, wy + 1.0f, wz};
            p3 = {wx, wy + 1.0f, wz};
            ao0 = calculateAO(x, y, z, 3, 0);
            ao1 = calculateAO(x, y, z, 2, 0);
            ao2 = calculateAO(x, y, z, 1, 0);
            ao3 = calculateAO(x, y, z, 0, 0);
            break;
        case 1: // Bottom (-Y)
            normal = {0, -1, 0};
            p0 = {wx, wy, wz};
            p1 = {wx + 1.0f, wy, wz};
            p2 = {wx + 1.0f, wy, wz + 1.0f};
            p3 = {wx, wy, wz + 1.0f};
            ao0 = calculateAO(x, y, z, 0, 1);
            ao1 = calculateAO(x, y, z, 1, 1);
            ao2 = calculateAO(x, y, z, 2, 1);
            ao3 = calculateAO(x, y, z, 3, 1);
            break;
        case 2: // North (-Z)
            normal = {0, 0, -1};
            p0 = {wx + 1.0f, wy, wz};
            p1 = {wx, wy, wz};
            p2 = {wx, wy + 1.0f, wz};
            p3 = {wx + 1.0f, wy + 1.0f, wz};
            break;
        case 3: // South (+Z)
            normal = {0, 0, 1};
            p0 = {wx, wy, wz + 1.0f};
            p1 = {wx + 1.0f, wy, wz + 1.0f};
            p2 = {wx + 1.0f, wy + 1.0f, wz + 1.0f};
            p3 = {wx, wy + 1.0f, wz + 1.0f};
            break;
        case 4: // West (-X)
            normal = {-1, 0, 0};
            p0 = {wx, wy, wz};
            p1 = {wx, wy, wz + 1.0f};
            p2 = {wx, wy + 1.0f, wz + 1.0f};
            p3 = {wx, wy + 1.0f, wz};
            break;
        case 5: // East (+X)
            normal = {1, 0, 0};
            p0 = {wx + 1.0f, wy, wz + 1.0f};
            p1 = {wx + 1.0f, wy, wz};
            p2 = {wx + 1.0f, wy + 1.0f, wz};
            p3 = {wx + 1.0f, wy + 1.0f, wz + 1.0f};
            break;
    }

    float light = info.isLightEmitter ? info.emitIntensity : 0.0f;

    auto& mesh = isTrans ? m_transMesh : m_opaqueMesh;

    // Two triangles for quad with proper CCW winding: (p0, p1, p2) and (p0, p2, p3)
    mesh.push_back({p0, {u0, v1}, normal, ao0, light});
    mesh.push_back({p1, {u1, v1}, normal, ao1, light});
    mesh.push_back({p2, {u1, v0}, normal, ao2, light});

    mesh.push_back({p0, {u0, v1}, normal, ao0, light});
    mesh.push_back({p2, {u1, v0}, normal, ao2, light});
    mesh.push_back({p3, {u0, v0}, normal, ao3, light});
}

void Chunk::rebuildMesh() {
    m_opaqueMesh.clear();
    m_transMesh.clear();

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
            for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
                BlockType b = m_blocks[x][y][z];
                if (b == BlockType::Air) continue;

                const BlockInfo& info = BlockRegistry::get(b);
                bool isTrans = info.isTransparent;

                int gx = m_chunkX * CHUNK_SIZE_X + x;
                int gy = y;
                int gz = m_chunkZ * CHUNK_SIZE_Z + z;

                // Check 6 adjacent neighbors
                // +Y
                BlockType nTop = getNeighborBlock(gx, gy + 1, gz);
                if (BlockRegistry::get(nTop).isTransparent && nTop != b) {
                    addFace(x, y, z, 0, b, isTrans);
                }
                // -Y
                BlockType nBot = getNeighborBlock(gx, gy - 1, gz);
                if (BlockRegistry::get(nBot).isTransparent && nBot != b) {
                    addFace(x, y, z, 1, b, isTrans);
                }
                // -Z
                BlockType nNorth = getNeighborBlock(gx, gy, gz - 1);
                if (BlockRegistry::get(nNorth).isTransparent && nNorth != b) {
                    addFace(x, y, z, 2, b, isTrans);
                }
                // +Z
                BlockType nSouth = getNeighborBlock(gx, gy, gz + 1);
                if (BlockRegistry::get(nSouth).isTransparent && nSouth != b) {
                    addFace(x, y, z, 3, b, isTrans);
                }
                // -X
                BlockType nWest = getNeighborBlock(gx - 1, gy, gz);
                if (BlockRegistry::get(nWest).isTransparent && nWest != b) {
                    addFace(x, y, z, 4, b, isTrans);
                }
                // +X
                BlockType nEast = getNeighborBlock(gx + 1, gy, gz);
                if (BlockRegistry::get(nEast).isTransparent && nEast != b) {
                    addFace(x, y, z, 5, b, isTrans);
                }
            }
        }
    }

    // Upload Opaque Mesh
    if (m_vaoOpaque == 0) {
        glGenVertexArrays(1, &m_vaoOpaque);
        glGenBuffers(1, &m_vboOpaque);
    }
    glBindVertexArray(m_vaoOpaque);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboOpaque);
    glBufferData(GL_ARRAY_BUFFER, m_opaqueMesh.size() * sizeof(VoxelVertex), m_opaqueMesh.data(), GL_DYNAMIC_DRAW);

    // Vertex attributes
    glEnableVertexAttribArray(0); // Pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, pos));
    glEnableVertexAttribArray(1); // TexCoord
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, texCoord));
    glEnableVertexAttribArray(2); // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, normal));
    glEnableVertexAttribArray(3); // AO
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, ao));
    glEnableVertexAttribArray(4); // Light
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, light));

    m_opaqueVertexCount = (GLsizei)m_opaqueMesh.size();

    // Upload Transparent Mesh
    if (m_vaoTrans == 0) {
        glGenVertexArrays(1, &m_vaoTrans);
        glGenBuffers(1, &m_vboTrans);
    }
    glBindVertexArray(m_vaoTrans);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboTrans);
    glBufferData(GL_ARRAY_BUFFER, m_transMesh.size() * sizeof(VoxelVertex), m_transMesh.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, pos));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, texCoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, normal));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, ao));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(VoxelVertex), (void*)offsetof(VoxelVertex, light));

    m_transVertexCount = (GLsizei)m_transMesh.size();

    glBindVertexArray(0);
    m_isDirty = false;
}

void Chunk::renderOpaque() {
    if (m_opaqueVertexCount > 0 && m_vaoOpaque != 0) {
        glBindVertexArray(m_vaoOpaque);
        glDrawArrays(GL_TRIANGLES, 0, m_opaqueVertexCount);
    }
}

void Chunk::renderTransparent() {
    if (m_transVertexCount > 0 && m_vaoTrans != 0) {
        glBindVertexArray(m_vaoTrans);
        glDrawArrays(GL_TRIANGLES, 0, m_transVertexCount);
    }
}
