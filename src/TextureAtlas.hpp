#pragma once

#include <cstdint>
#include <vector>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif

class TextureAtlas {
public:
    static const int ATLAS_SIZE = 512;
    static const int TILE_SIZE = 32;
    static const int TILES_PER_ROW = ATLAS_SIZE / TILE_SIZE; // 16

    static TextureAtlas& instance();

    void init();
    void cleanup();

    GLuint getTextureID() const { return m_textureID; }

    // Helper to get UV coordinates for a tile index (0..15, 0..15)
    void getTileUVs(int tileX, int tileY, float& u0, float& v0, float& u1, float& v1) const;

    // Get pixel buffer for inspection/export if needed
    const std::vector<uint8_t>& getPixels() const { return m_pixels; }

private:
    TextureAtlas() = default;
    GLuint m_textureID = 0;
    std::vector<uint8_t> m_pixels;

    void setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
    void generateProceduralTextures();

    void drawTileHullAlloy(int tx, int ty);
    void drawTileAlienRuin(int tx, int ty);
    void drawTileDarkMonolith(int tx, int ty);
    void drawTileMachinery(int tx, int ty);
    void drawTileReactor(int tx, int ty);
    void drawTileTitanium(int tx, int ty);
    void drawTileCobaltCrystal(int tx, int ty);
    void drawTileCarbonite(int tx, int ty);
    void drawTileSilica(int tx, int ty);
    void drawTileWater(int tx, int ty);
    void drawTileMossTop(int tx, int ty);
    void drawTileMossSide(int tx, int ty);
    void drawTileBaseWall(int tx, int ty);
    void drawTileGlass(int tx, int ty);
    void drawTileAirlock(int tx, int ty);
    void drawTileTorch(int tx, int ty);
    void drawTileTurret(int tx, int ty);
    void drawTileFabricator(int tx, int ty);
    void drawTileSteamVent(int tx, int ty);

    void drawTileGrassTop(int tx, int ty);
    void drawTileDirt(int tx, int ty);
    void drawTileGrassSide(int tx, int ty);
    void drawTileStone(int tx, int ty);
    void drawTileSand(int tx, int ty);
    void drawTileWoodTop(int tx, int ty);
    void drawTileWoodSide(int tx, int ty);
    void drawTileLeaves(int tx, int ty);

    void drawItemIcons();
    void drawFontGlyphs();
};
