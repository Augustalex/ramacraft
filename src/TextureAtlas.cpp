#include "TextureAtlas.hpp"
#include <cmath>
#include <cstring>
#include <random>

TextureAtlas& TextureAtlas::instance() {
    static TextureAtlas s_atlas;
    return s_atlas;
}

void TextureAtlas::setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (x < 0 || x >= ATLAS_SIZE || y < 0 || y >= ATLAS_SIZE) return;
    int idx = (y * ATLAS_SIZE + x) * 4;
    m_pixels[idx + 0] = r;
    m_pixels[idx + 1] = g;
    m_pixels[idx + 2] = b;
    m_pixels[idx + 3] = a;
}

void TextureAtlas::getTileUVs(int tileX, int tileY, float& u0, float& v0, float& u1, float& v1) const {
    float invSize = 1.0f / (float)ATLAS_SIZE;
    float padding = 0.0f; // Half-pixel can be added if needed
    u0 = (tileX * TILE_SIZE + padding) * invSize;
    v0 = (tileY * TILE_SIZE + padding) * invSize;
    u1 = ((tileX + 1) * TILE_SIZE - padding) * invSize;
    v1 = ((tileY + 1) * TILE_SIZE - padding) * invSize;
}

void TextureAtlas::init() {
    m_pixels.assign(ATLAS_SIZE * ATLAS_SIZE * 4, 0);
    generateProceduralTextures();

    if (m_textureID == 0) {
        glGenTextures(1, &m_textureID);
    }
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_pixels.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}

void TextureAtlas::cleanup() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

void TextureAtlas::drawTileHullAlloy(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t base = 48 + (uint8_t)((x * 7 + y * 13) % 15);
            // Hull plate bevel borders
            if (x == 0 || y == 0) base = 75;
            else if (x == TILE_SIZE - 1 || y == TILE_SIZE - 1) base = 30;
            // Cross panel seam
            if (x == 16 || y == 16) base = 25;
            // Rivets at corners
            if ((x == 3 || x == 28 || x == 13 || x == 19) && (y == 3 || y == 28 || y == 13 || y == 19)) {
                base = 95;
            }
            setPixel(startX + x, startY + y, base, base + 4, base + 8, 255);
        }
    }
}

void TextureAtlas::drawTileAlienRuin(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t base = 35 + (uint8_t)((x * 17 ^ y * 23) % 20);
            uint8_t r = base, g = base + 5, b = base + 10;
            // Ancient geometric glyph patterns
            bool glyph = (x > 6 && x < 25 && y > 6 && y < 25) &&
                         ((x == 8 || x == 23 || y == 8 || y == 23) ||
                          (x == y && x > 10 && x < 21) ||
                          (x + y == 31 && x > 10 && x < 21));
            if (glyph) {
                r = 10; g = 140; b = 180; // Glowing turquoise runes
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileDarkMonolith(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t base = 15 + (uint8_t)((x * 3 + y * 5) % 8);
            uint8_t r = base, g = base, b = base + 4;
            // Subtle vertical metallic streaks
            if (x % 8 == 0) { r += 12; g += 12; b += 22; }
            if (x == 15 && y >= 4 && y <= 27) { r = 180; g = 30; b = 30; } // Center red monolith optical line
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileMachinery(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t r = 40, g = 44, b = 50;
            // Grill vents
            if (y >= 4 && y <= 20 && y % 3 == 0 && x >= 4 && x <= 27) {
                r = 15; g = 15; b = 18;
            }
            // Blinking status indicators
            if (y >= 23 && y <= 27) {
                if (x >= 4 && x <= 7) { r = 255; g = 50; b = 30; } // Red LED
                else if (x >= 10 && x <= 13) { r = 40; g = 240; b = 80; } // Green LED
                else if (x >= 16 && x <= 19) { r = 30; g = 180; b = 255; } // Blue LED
                else if (x >= 22 && x <= 25) { r = 255; g = 200; b = 20; } // Yellow LED
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileReactor(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            float dx = (float)(x - 16);
            float dy = (float)(y - 16);
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 10.0f) {
                // Glowing plasma reactor core
                float t = 1.0f - (dist / 10.0f);
                uint8_t r = (uint8_t)(255);
                uint8_t g = (uint8_t)(100 + 155 * t);
                uint8_t b = (uint8_t)(30 + 100 * (t * t));
                setPixel(startX + x, startY + y, r, g, b, 255);
            } else {
                // Reactor containment housing
                uint8_t dark = (dist < 12.0f) ? 80 : 35;
                setPixel(startX + x, startY + y, dark + 10, dark + 5, dark, 255);
            }
        }
    }
}

void TextureAtlas::drawTileTitanium(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t rock = 60 + (uint8_t)((x * 11 + y * 19) % 25);
            uint8_t r = rock, g = rock, b = rock + 5;
            // Shiny metallic crystalline veins
            if (((x % 7 == 2 || x % 7 == 3) && (y % 6 == 1 || y % 6 == 2)) ||
                (x >= 12 && x <= 20 && y >= 14 && y <= 18)) {
                r = 210; g = 220; b = 240; // Polished titanium sheen
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileCobaltCrystal(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t base = 40 + (uint8_t)((x * 5 + y * 9) % 20);
            uint8_t r = base, g = base, b = base + 15;
            // Glowing geometric cobalt shards
            int cx = (x - 16), cy = (y - 16);
            if (std::abs(cx) + std::abs(cy) < 11 || (std::abs(x - 8) + std::abs(y - 8) < 5) || (std::abs(x - 24) + std::abs(y - 24) < 5)) {
                r = 30; g = 140 + (x * 4) % 80; b = 255;
                if (std::abs(cx) + std::abs(cy) < 5) {
                    r = 180; g = 230; b = 255; // Core facet highlight
                }
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileCarbonite(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t c = 25 + (uint8_t)((x * 13 ^ y * 7) % 18);
            // Carbon weave texture
            if ((x + y) % 4 == 0) c += 15;
            setPixel(startX + x, startY + y, c, c, c, 255);
        }
    }
}

void TextureAtlas::drawTileSilica(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t s = 140 + (uint8_t)((x * 37 + y * 53) % 40);
            uint8_t r = s + 10, g = s, b = s - 20;
            if ((x * 19 + y * 31) % 17 == 0) {
                r = 250; g = 250; b = 255; // Silica sparkle
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileWater(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            float wave = std::sin(x * 0.4f) * std::cos(y * 0.4f);
            uint8_t r = (uint8_t)(20 + wave * 10);
            uint8_t g = (uint8_t)(80 + wave * 30);
            uint8_t b = (uint8_t)(190 + wave * 45);
            setPixel(startX + x, startY + y, r, g, b, 190); // Semi-transparent
        }
    }
}

void TextureAtlas::drawTileMossTop(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t noise = (uint8_t)((x * 29 + y * 47) % 50);
            uint8_t r = 10 + noise / 3;
            uint8_t g = 170 + noise;
            uint8_t b = 110 + noise;
            if ((x + y * 3) % 11 == 0) {
                r = 120; g = 255; b = 200; // Spore bioluminescence
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileMossSide(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            if (y < 8 + (x % 5)) {
                // Moss top drip
                uint8_t r = 20, g = 180 + (x * 7) % 50, b = 120;
                setPixel(startX + x, startY + y, r, g, b, 255);
            } else {
                // Lower rock
                uint8_t base = 50 + (uint8_t)((x * 13 + y * 17) % 25);
                setPixel(startX + x, startY + y, base, base + 5, base + 10, 255);
            }
        }
    }
}

void TextureAtlas::drawTileBaseWall(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t r = 210, g = 215, b = 225; // Clean sci-fi white habitat panel
            if (x == 0 || y == 0 || x == 31 || y == 31) {
                r = 130; g = 140; b = 160; // Panel edge
            }
            // Blue LED indicator strip
            if (y == 15 || y == 16) {
                r = 30; g = 150; b = 255;
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileGlass(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            if (x == 0 || y == 0 || x == 31 || y == 31) {
                setPixel(startX + x, startY + y, 90, 140, 180, 230); // Frame
            } else if (x == y || x == y - 1 || x == y - 8) {
                setPixel(startX + x, startY + y, 220, 245, 255, 120); // Glare line
            } else {
                setPixel(startX + x, startY + y, 60, 160, 210, 50); // Glass tint
            }
        }
    }
}

void TextureAtlas::drawTileAirlock(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t r = 160, g = 160, b = 170;
            // Hazard diagonal stripes on borders
            if (x < 4 || x >= 28 || y < 4 || y >= 28) {
                bool yellow = ((x + y) / 4) % 2 == 0;
                r = yellow ? 240 : 30;
                g = yellow ? 200 : 30;
                b = yellow ? 20 : 30;
            } else {
                // Center circular reinforced viewport
                float dx = (float)(x - 16), dy = (float)(y - 16);
                float d = std::sqrt(dx * dx + dy * dy);
                if (d < 8.0f) {
                    r = 40; g = 120; b = 180; // Blue port
                }
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileTorch(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            if (x >= 14 && x <= 17 && y >= 12 && y <= 30) {
                // Metal mounting rod
                setPixel(startX + x, startY + y, 70, 75, 80, 255);
            } else if (x >= 12 && x <= 19 && y >= 2 && y <= 11) {
                // Plasma flame
                float dx = (float)(x - 15.5f), dy = (float)(y - 7.0f);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < 4.5f) {
                    uint8_t r = 255;
                    uint8_t g = (uint8_t)(150 + (4.5f - dist) * 20);
                    uint8_t b = (uint8_t)(50 + (4.5f - dist) * 35);
                    setPixel(startX + x, startY + y, r, g, b, 255);
                } else {
                    setPixel(startX + x, startY + y, 0, 0, 0, 0);
                }
            } else {
                setPixel(startX + x, startY + y, 0, 0, 0, 0); // Transparent background for torch
            }
        }
    }
}

void TextureAtlas::drawTileTurret(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t r = 60, g = 65, b = 75;
            // Gun barrels & optics
            if ((x >= 8 && x <= 11) || (x >= 20 && x <= 23)) {
                if (y >= 4 && y <= 18) { r = 20; g = 20; b = 25; }
            }
            if (x >= 14 && x <= 17 && y >= 14 && y <= 17) {
                r = 255; g = 30; b = 30; // Red sensor eye
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileFabricator(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t r = 50, g = 55, b = 65;
            // Holographic workstation grid
            if (y >= 6 && y <= 18 && x >= 6 && x <= 25) {
                r = 20; g = 180; b = 240;
                if ((x + y) % 4 == 0) { r = 180; g = 240; b = 255; }
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileSteamVent(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t base = 45;
            float dx = (float)(x - 16), dy = (float)(y - 16);
            if (dx * dx + dy * dy < 80.0f) {
                base = ((x + y) % 2 == 0) ? 10 : 200;
            }
            setPixel(startX + x, startY + y, base, base + 5, base + 15, 255);
        }
    }
}

void TextureAtlas::drawTileGrassTop(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t noise = (uint8_t)((x * 37 ^ y * 61) % 45);
            uint8_t r = 35 + noise / 3;
            uint8_t g = 145 + noise;
            uint8_t b = 45 + noise / 2;
            // Blade highlights
            if ((x * 13 + y * 23) % 11 == 0) {
                r += 20; g += 30; b += 10;
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileDirt(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t noise = (uint8_t)((x * 47 ^ y * 31) % 35);
            uint8_t r = 85 + noise;
            uint8_t g = 55 + noise / 2;
            uint8_t b = 35 + noise / 3;
            if ((x * 17 + y * 29) % 19 == 0) {
                r += 30; g += 25; b += 20; // Pebble
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileGrassSide(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int grassDrip = 6 + ((x * 19 + 7) % 7);
            if (y < grassDrip) {
                // Lush green grass rim
                uint8_t noise = (uint8_t)((x * 37 + y * 13) % 40);
                uint8_t r = 40 + noise / 3;
                uint8_t g = 140 + noise;
                uint8_t b = 40 + noise / 2;
                setPixel(startX + x, startY + y, r, g, b, 255);
            } else {
                // Rich brown dirt body
                uint8_t noise = (uint8_t)((x * 47 ^ y * 31) % 35);
                uint8_t r = 85 + noise;
                uint8_t g = 55 + noise / 2;
                uint8_t b = 35 + noise / 3;
                setPixel(startX + x, startY + y, r, g, b, 255);
            }
        }
    }
}

void TextureAtlas::drawTileStone(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t s = 100 + (uint8_t)((x * 53 ^ y * 79) % 35);
            uint8_t r = s, g = s + 2, b = s + 5;
            // Stone mineral flecks
            if ((x * 23 + y * 41) % 17 == 0) {
                r = 160; g = 165; b = 175;
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileSand(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t s = 190 + (uint8_t)((x * 31 + y * 43) % 35);
            uint8_t r = s + 20, g = s, b = s - 55;
            if ((x + y * 2) % 13 == 0) {
                r += 15; g += 15; b += 10;
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileWoodTop(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            float dx = (float)(x - 16), dy = (float)(y - 16);
            float d = std::sqrt(dx * dx + dy * dy);
            int ring = (int)(d * 1.5f) % 3;
            uint8_t r = 160 - ring * 25;
            uint8_t g = 115 - ring * 20;
            uint8_t b = 70 - ring * 15;
            if (d > 14.0f) {
                r = 80; g = 50; b = 30; // Bark outer rim
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileWoodSide(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            uint8_t noise = (uint8_t)((x * 19 ^ y * 7) % 25);
            uint8_t r = 90 + noise;
            uint8_t g = 60 + noise / 2;
            uint8_t b = 35 + noise / 3;
            if (x % 6 == 0) {
                r = 60; g = 40; b = 25; // Vertical bark crevice
            }
            setPixel(startX + x, startY + y, r, g, b, 255);
        }
    }
}

void TextureAtlas::drawTileLeaves(int tx, int ty) {
    int startX = tx * TILE_SIZE;
    int startY = ty * TILE_SIZE;
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            bool leaf = ((x * 23 ^ y * 41) % 5) != 0;
            if (leaf) {
                uint8_t noise = (uint8_t)((x * 17 + y * 29) % 50);
                uint8_t r = 25 + noise / 3;
                uint8_t g = 135 + noise;
                uint8_t b = 35 + noise / 4;
                setPixel(startX + x, startY + y, r, g, b, 255);
            } else {
                setPixel(startX + x, startY + y, 0, 0, 0, 0); // Transparent leaf cutout
            }
        }
    }
}

void TextureAtlas::drawItemIcons() {
    // We draw detailed item icons on rows 2, 3, 4 of the atlas
    auto drawBox = [this](int tx, int ty, int x0, int y0, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
        int sx = tx * TILE_SIZE;
        int sy = ty * TILE_SIZE;
        for (int y = y0; y < y0 + h; ++y) {
            for (int x = x0; x < x0 + w; ++x) {
                setPixel(sx + x, sy + y, r, g, b, 255);
            }
        }
    };

    // Item: Ray Gun (tx=0, ty=2)
    {
        int sx = 0 * TILE_SIZE, sy = 2 * TILE_SIZE;
        // Gun barrel, body, handle, emitter crystal
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                if (y >= 10 && y <= 16 && x >= 4 && x <= 26) {
                    setPixel(sx + x, sy + y, 70, 75, 85, 255); // Barrel
                } else if (y >= 16 && y <= 26 && x >= 6 && x <= 12) {
                    setPixel(sx + x, sy + y, 40, 40, 45, 255); // Grip
                } else if (y >= 11 && y <= 15 && x >= 24 && x <= 28) {
                    setPixel(sx + x, sy + y, 30, 200, 255, 255); // Blue laser lens
                } else if (y >= 11 && y <= 15 && x >= 14 && x <= 18) {
                    setPixel(sx + x, sy + y, 255, 140, 30, 255); // Plasma cell
                }
            }
        }
    }

    // Item: Flashlight (tx=1, ty=2)
    {
        int sx = 1 * TILE_SIZE, sy = 2 * TILE_SIZE;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                if (x >= 8 && x <= 22 && y >= 12 && y <= 18) {
                    setPixel(sx + x, sy + y, 90, 95, 100, 255); // Cylinder body
                } else if (x >= 22 && x <= 26 && y >= 9 && y <= 21) {
                    setPixel(sx + x, sy + y, 255, 240, 160, 255); // Glowing head
                }
            }
        }
    }

    // Item: Jetpack (tx=2, ty=2)
    {
        int sx = 2 * TILE_SIZE, sy = 2 * TILE_SIZE;
        for (int y = 0; y < TILE_SIZE; ++y) {
            for (int x = 0; x < TILE_SIZE; ++x) {
                // Twin tanks
                if ((x >= 6 && x <= 12 && y >= 6 && y <= 22) || (x >= 18 && x <= 24 && y >= 6 && y <= 22)) {
                    setPixel(sx + x, sy + y, 160, 170, 185, 255);
                } else if (x >= 10 && x <= 20 && y >= 10 && y <= 18) {
                    setPixel(sx + x, sy + y, 60, 65, 75, 255); // Cross harness
                } else if ((x >= 7 && x <= 11 && y >= 23 && y <= 28) || (x >= 19 && x <= 23 && y >= 23 && y <= 28)) {
                    setPixel(sx + x, sy + y, 255, 120, 20, 255); // Exhaust flame
                }
            }
        }
    }

    // Item: Titanium Ingot (tx=4, ty=2)
    drawBox(4, 2, 6, 10, 20, 12, 190, 205, 220);

    // Item: Cobalt Crystal (tx=5, ty=2)
    drawBox(5, 2, 10, 6, 12, 20, 30, 160, 255);

    // Item: Alien Circuit (tx=9, ty=2)
    drawBox(9, 2, 6, 6, 20, 20, 20, 120, 70); // Green board with gold traces

    // Item: Servo Motor (tx=10, ty=2)
    drawBox(10, 2, 8, 8, 16, 16, 140, 145, 155);

    // Item: Power Matrix (tx=11, ty=2)
    drawBox(11, 2, 6, 6, 20, 20, 255, 120, 30);

    // Item: Biot Shell (tx=0, ty=3)
    drawBox(0, 3, 6, 8, 20, 16, 120, 60, 180); // Chitin purple-black

    // Item: Micro Actuator (tx=1, ty=3)
    drawBox(1, 3, 8, 8, 16, 16, 80, 180, 200);

    // Item: Robot Core (tx=2, ty=3)
    drawBox(2, 3, 6, 6, 20, 20, 220, 40, 40); // Glowing red AI core

    // Item: Energy Cell (tx=3, ty=3)
    drawBox(3, 3, 8, 6, 16, 20, 40, 240, 120); // High-charge battery
}

void TextureAtlas::drawFontGlyphs() {
    // 8x8 font table stored in tile rows (ty = 14 and 15) for rendering HUD text
    // We create a standard 5x7 / 8x8 bitmap font for letters, numbers, and symbols
    static const uint8_t font5x7[][7] = {
        // Space (32)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        // ! (33)
        {0x04, 0x04, 0x04, 0x04, 0x00, 0x04, 0x00},
        // " (34)
        {0x0A, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00},
        // # (35)
        {0x0A, 0x1F, 0x0A, 0x0A, 0x1F, 0x0A, 0x00},
        // $ (36)
        {0x04, 0x0F, 0x14, 0x0E, 0x05, 0x1E, 0x04},
        // % (37)
        {0x19, 0x19, 0x02, 0x04, 0x08, 0x13, 0x13},
        // & (38)
        {0x08, 0x14, 0x14, 0x08, 0x15, 0x12, 0x0D},
        // ' (39)
        {0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00},
        // ( (40)
        {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02},
        // ) (41)
        {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08},
        // * (42)
        {0x00, 0x04, 0x15, 0x0E, 0x15, 0x04, 0x00},
        // + (43)
        {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00},
        // , (44)
        {0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x08},
        // - (45)
        {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00},
        // . (46)
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00},
        // / (47)
        {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00},
        // 0..9 (48..57)
        {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}, // 0
        {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}, // 1
        {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, // 2
        {0x1F, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0E}, // 3
        {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
        {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
        {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
        {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}, // 8
        {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}, // 9
        // : (58)
        {0x00, 0x04, 0x00, 0x00, 0x04, 0x00, 0x00},
        // ; (59)
        {0x00, 0x04, 0x00, 0x00, 0x04, 0x04, 0x08},
        // < (60)
        {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02},
        // = (61)
        {0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00, 0x00},
        // > (62)
        {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08},
        // ? (63)
        {0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04},
        // @ (64)
        {0x0E, 0x11, 0x01, 0x0D, 0x15, 0x15, 0x0E},
        // A..Z (65..90)
        {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // A
        {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}, // B
        {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}, // C
        {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}, // D
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}, // E
        {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}, // F
        {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}, // G
        {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}, // H
        {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}, // I
        {0x07, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0C}, // J
        {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}, // L
        {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
        {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // N
        {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // O
        {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}, // P
        {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}, // Q
        {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}, // R
        {0x0E, 0x11, 0x10, 0x0E, 0x01, 0x11, 0x0E}, // S
        {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}, // U
        {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}, // V
        {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}, // W
        {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}, // X
        {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}, // Y
        {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}, // Z
        // [ \ ] ^ _ `
        {0x0E, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0E}, // [
        {0x10, 0x08, 0x04, 0x02, 0x01, 0x00, 0x00}, // '\'
        {0x0E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0E}, // ]
        {0x04, 0x0A, 0x11, 0x00, 0x00, 0x00, 0x00}, // ^
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}, // _
        {0x08, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00}  // `
    };

    // Render characters into bottom two rows (y=448..511)
    int numChars = sizeof(font5x7) / sizeof(font5x7[0]);
    for (int i = 0; i < numChars; ++i) {
        int charX = (i % 64) * 8;
        int charY = 448 + (i / 64) * 16;
        for (int row = 0; row < 7; ++row) {
            uint8_t bits = font5x7[i][row];
            for (int col = 0; col < 5; ++col) {
                if (bits & (0x10 >> col)) {
                    setPixel(charX + col + 1, charY + row + 2, 255, 255, 255, 255);
                }
            }
        }
    }
}

void TextureAtlas::generateProceduralTextures() {
    // Row 0: Primary terrain blocks
    drawTileHullAlloy(0, 0);
    drawTileAlienRuin(1, 0);
    drawTileDarkMonolith(2, 0);
    drawTileMachinery(3, 0);
    drawTileReactor(4, 0);
    drawTileTitanium(5, 0);
    drawTileCobaltCrystal(6, 0);
    drawTileCarbonite(7, 0);
    drawTileSilica(8, 0);
    drawTileWater(9, 0);
    drawTileMossTop(10, 0);
    drawTileMossSide(11, 0);
    drawTileBaseWall(12, 0);
    drawTileGlass(13, 0);
    drawTileAirlock(14, 0);
    drawTileTorch(15, 0);

    // Row 1: Interactive stations, specialized blocks & Earth natural terrain
    drawTileTurret(0, 1);
    drawTileFabricator(1, 1);
    drawTileSteamVent(2, 1);
    drawTileGrassTop(3, 1);
    drawTileDirt(4, 1);
    drawTileGrassSide(5, 1);
    drawTileStone(6, 1);
    drawTileSand(7, 1);
    drawTileWoodTop(8, 1);
    drawTileWoodSide(9, 1);
    drawTileLeaves(10, 1);

    // Row 2, 3: Inventory Item Icons
    drawItemIcons();

    // Bottom rows: Font atlas
    drawFontGlyphs();
}
