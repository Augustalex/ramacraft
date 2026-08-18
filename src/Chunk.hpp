#pragma once

#include <vector>
#include <memory>
#include "Block.hpp"
#include "Math3D.hpp"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Y = 64;
constexpr int CHUNK_SIZE_Z = 16;

struct VoxelVertex {
    Vec3 pos;
    Vec2 texCoord;
    Vec3 normal;
    float ao = 1.0f;
    float light = 0.0f;
};

class World;

class Chunk {
public:
    Chunk(int cx, int cz, World* world);
    ~Chunk();

    int getChunkX() const { return m_chunkX; }
    int getChunkZ() const { return m_chunkZ; }

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);

    void markDirty() { m_isDirty = true; }
    bool isDirty() const { return m_isDirty; }

    void rebuildMesh();
    void renderOpaque();
    void renderTransparent();

private:
    int m_chunkX = 0;
    int m_chunkZ = 0;
    World* m_world = nullptr;

    BlockType m_blocks[CHUNK_SIZE_X][CHUNK_SIZE_Y][CHUNK_SIZE_Z];
    bool m_isDirty = true;

    // OpenGL Buffers
    GLuint m_vaoOpaque = 0;
    GLuint m_vboOpaque = 0;
    GLsizei m_opaqueVertexCount = 0;

    GLuint m_vaoTrans = 0;
    GLuint m_vboTrans = 0;
    GLsizei m_transVertexCount = 0;

    std::vector<VoxelVertex> m_opaqueMesh;
    std::vector<VoxelVertex> m_transMesh;

    void addFace(int x, int y, int z, int face, BlockType block, bool isTrans);
    float calculateAO(int x, int y, int z, int corner, int face);
    BlockType getNeighborBlock(int gx, int gy, int gz) const;
};
