#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "Math3D.hpp"

enum class BlockType : uint8_t {
    Air = 0,
    RamaHullAlloy = 1,       // Dark cylindrical exterior/interior base hull
    AlienRuinWall = 2,       // Patterned alien megalith stone/metal
    DarkMonolith = 3,        // Deep black alien obsidian/alloy with glowing glyphs
    AncientMachinery = 4,    // Breakable alien electronics & computers
    BrokenReactor = 5,       // Alien generator/fusion core (drops high-tier parts)
    TitaniumOre = 6,         // Metallic mineral vein
    CobaltCrystal = 7,       // Glowing blue energy crystal
    CarboniteBlock = 8,      // Hard dark carbonaceous mineral
    SilicaSand = 9,          // Fine ground glass/silicon soil
    CylindricalSeaWater = 10,// Water of the Cylindrical Sea
    BioluminescentMoss = 11, // Glowing green/cyan alien flora
    BaseHabitatWall = 12,    // Human player crafted clean sci-fi white/blue base wall
    ReinforcedGlass = 13,    // Transparent high-durability visor glass
    AirlockDoor = 14,        // Sealable habitat door
    Torch = 15,              // Placeable plasma glow beacon / torch
    BiotDefenseTurret = 16,  // Automated defense against night robot beetles
    FabricatorBench = 17,    // Base expansion & advanced tech workbench
    SteamVent = 18,          // Geothermal/alien hull exhaust vent
    GrassBlock = 19,         // Lush green Earth-inspired vegetation grass
    Dirt = 20,               // Rich dark soil
    NaturalStone = 21,       // Mountain stone / bedrock
    Sand = 22,               // Golden shore beach sand
    WoodLog = 23,            // Tree trunk log
    FoliageLeaves = 24,      // Lush green canopy leaves
    Count
};

enum class ItemType : uint16_t {
    None = 0,

    // Tools & Gear
    RayGun = 1,
    Flashlight = 2,
    Jetpack = 3,
    MiningDrill = 4,
    Grenade = 5,

    // Raw Materials & Minerals
    TitaniumOreItem = 10,
    TitaniumIngot = 11,
    CobaltCrystalItem = 12,
    CarboniteChunk = 13,
    SilicaDust = 14,
    SiliconWafer = 15,

    // Salvaged Machine Parts (from breaking alien machinery & reactors)
    AlienCircuit = 30,
    ServoMotor = 31,
    PowerMatrix = 32,
    PlasteelSheet = 33,
    OpticLens = 34,
    FusionBattery = 35,

    // Robot Beetle (Biot) Salvaged Drops
    BiotShell = 50,
    MicroActuator = 51,
    RobotCore = 52,
    EnergyCell = 53,

    // Base Building / Placeable Blocks
    BlockItem_RamaHull = 101,
    BlockItem_AlienRuin = 102,
    BlockItem_DarkMonolith = 103,
    BlockItem_AncientMachinery = 104,
    BlockItem_BrokenReactor = 105,
    BlockItem_TitaniumOre = 106,
    BlockItem_CobaltCrystal = 107,
    BlockItem_Carbonite = 108,
    BlockItem_SilicaSand = 109,
    BlockItem_Moss = 111,
    BlockItem_BaseWall = 112,
    BlockItem_ReinforcedGlass = 113,
    BlockItem_AirlockDoor = 114,
    BlockItem_Torch = 115,
    BlockItem_Turret = 116,
    BlockItem_Fabricator = 117,
    BlockItem_Grass = 118,
    BlockItem_Dirt = 119,
    BlockItem_Stone = 120,
    BlockItem_Sand = 121,
    BlockItem_Wood = 122,
    BlockItem_Leaves = 123,

    // Upgrades
    HighCapJetpack = 200,
    OverclockedRayGun = 201,
    OxygenRecycler = 202,
    RadarScanner = 203
};

struct BlockInfo {
    std::string name;
    bool isSolid = true;
    bool isTransparent = false;
    bool isLiquid = false;
    bool isLightEmitter = false;
    Vec3 emitColor = {0.0f, 0.0f, 0.0f};
    float emitIntensity = 0.0f;
    float hardness = 1.0f; // Mining time/durability
    ItemType dropItem = ItemType::None;
    int dropMin = 1;
    int dropMax = 1;
    // Texture atlas tile coordinates (X, Y in 16x16 atlas grid)
    // top, bottom, sides
    uint8_t texTopX = 0, texTopY = 0;
    uint8_t texBottomX = 0, texBottomY = 0;
    uint8_t texSideX = 0, texSideY = 0;
};

struct ItemInfo {
    std::string name;
    std::string description;
    BlockType placeBlock = BlockType::Air;
    bool isPlaceable = false;
    bool isTool = false;
    int maxStack = 64;
    uint8_t iconX = 0, iconY = 0; // Icon coords in atlas
};

class BlockRegistry {
public:
    static void init();
    static const BlockInfo& get(BlockType type);
    static const ItemInfo& getItem(ItemType type);
    static BlockType itemToBlock(ItemType item);
    static ItemType blockToItem(BlockType block);
};
