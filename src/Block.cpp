#include "Block.hpp"
#include <unordered_map>

static std::vector<BlockInfo> s_blocks;
static std::unordered_map<ItemType, ItemInfo> s_items;

void BlockRegistry::init() {
    s_blocks.resize((size_t)BlockType::Count);

    // Air
    {
        BlockInfo b;
        b.name = "Air";
        b.isSolid = false;
        b.isTransparent = true;
        s_blocks[(size_t)BlockType::Air] = b;
    }

    // RamaHullAlloy
    {
        BlockInfo b;
        b.name = "Rama Hull Alloy";
        b.isSolid = true;
        b.hardness = 3.0f;
        b.dropItem = ItemType::BlockItem_RamaHull;
        b.texTopX = 0; b.texTopY = 0;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 0; b.texSideY = 0;
        s_blocks[(size_t)BlockType::RamaHullAlloy] = b;
    }

    // AlienRuinWall
    {
        BlockInfo b;
        b.name = "Alien Ruin Wall";
        b.isSolid = true;
        b.hardness = 2.5f;
        b.dropItem = ItemType::BlockItem_AlienRuin;
        b.texTopX = 1; b.texTopY = 0;
        b.texBottomX = 1; b.texBottomY = 0;
        b.texSideX = 1; b.texSideY = 0;
        s_blocks[(size_t)BlockType::AlienRuinWall] = b;
    }

    // DarkMonolith
    {
        BlockInfo b;
        b.name = "Dark Monolith";
        b.isSolid = true;
        b.hardness = 4.0f;
        b.dropItem = ItemType::BlockItem_DarkMonolith;
        b.texTopX = 2; b.texTopY = 0;
        b.texBottomX = 2; b.texBottomY = 0;
        b.texSideX = 2; b.texSideY = 0;
        s_blocks[(size_t)BlockType::DarkMonolith] = b;
    }

    // AncientMachinery
    {
        BlockInfo b;
        b.name = "Ancient Machinery";
        b.isSolid = true;
        b.hardness = 2.0f;
        b.dropItem = ItemType::AlienCircuit; // Breaks into electronic parts!
        b.dropMin = 1; b.dropMax = 3;
        b.texTopX = 3; b.texTopY = 0;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 3; b.texSideY = 0;
        s_blocks[(size_t)BlockType::AncientMachinery] = b;
    }

    // BrokenReactor
    {
        BlockInfo b;
        b.name = "Broken Reactor";
        b.isSolid = true;
        b.hardness = 3.5f;
        b.isLightEmitter = true;
        b.emitColor = {1.0f, 0.4f, 0.1f};
        b.emitIntensity = 0.8f;
        b.dropItem = ItemType::PowerMatrix;
        b.dropMin = 1; b.dropMax = 2;
        b.texTopX = 4; b.texTopY = 0;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 4; b.texSideY = 0;
        s_blocks[(size_t)BlockType::BrokenReactor] = b;
    }

    // TitaniumOre
    {
        BlockInfo b;
        b.name = "Titanium Ore";
        b.isSolid = true;
        b.hardness = 2.8f;
        b.dropItem = ItemType::TitaniumOreItem;
        b.dropMin = 1; b.dropMax = 2;
        b.texTopX = 5; b.texTopY = 0;
        b.texBottomX = 5; b.texBottomY = 0;
        b.texSideX = 5; b.texSideY = 0;
        s_blocks[(size_t)BlockType::TitaniumOre] = b;
    }

    // CobaltCrystal
    {
        BlockInfo b;
        b.name = "Cobalt Crystal";
        b.isSolid = true;
        b.isLightEmitter = true;
        b.emitColor = {0.2f, 0.6f, 1.0f};
        b.emitIntensity = 1.0f;
        b.hardness = 1.8f;
        b.dropItem = ItemType::CobaltCrystalItem;
        b.dropMin = 1; b.dropMax = 3;
        b.texTopX = 6; b.texTopY = 0;
        b.texBottomX = 6; b.texBottomY = 0;
        b.texSideX = 6; b.texSideY = 0;
        s_blocks[(size_t)BlockType::CobaltCrystal] = b;
    }

    // CarboniteBlock
    {
        BlockInfo b;
        b.name = "Carbonite Block";
        b.isSolid = true;
        b.hardness = 2.2f;
        b.dropItem = ItemType::CarboniteChunk;
        b.dropMin = 1; b.dropMax = 2;
        b.texTopX = 7; b.texTopY = 0;
        b.texBottomX = 7; b.texBottomY = 0;
        b.texSideX = 7; b.texSideY = 0;
        s_blocks[(size_t)BlockType::CarboniteBlock] = b;
    }

    // SilicaSand
    {
        BlockInfo b;
        b.name = "Silica Ground";
        b.isSolid = true;
        b.hardness = 1.0f;
        b.dropItem = ItemType::SilicaDust;
        b.dropMin = 1; b.dropMax = 4;
        b.texTopX = 8; b.texTopY = 0;
        b.texBottomX = 8; b.texBottomY = 0;
        b.texSideX = 8; b.texSideY = 0;
        s_blocks[(size_t)BlockType::SilicaSand] = b;
    }

    // CylindricalSeaWater
    {
        BlockInfo b;
        b.name = "Cylindrical Sea Water";
        b.isSolid = false;
        b.isTransparent = true;
        b.isLiquid = true;
        b.hardness = 0.0f;
        b.dropItem = ItemType::None;
        b.texTopX = 9; b.texTopY = 0;
        b.texBottomX = 9; b.texBottomY = 0;
        b.texSideX = 9; b.texSideY = 0;
        s_blocks[(size_t)BlockType::CylindricalSeaWater] = b;
    }

    // BioluminescentMoss
    {
        BlockInfo b;
        b.name = "Bioluminescent Moss";
        b.isSolid = true;
        b.isLightEmitter = true;
        b.emitColor = {0.1f, 0.9f, 0.5f};
        b.emitIntensity = 0.6f;
        b.hardness = 0.8f;
        b.dropItem = ItemType::BlockItem_Moss;
        b.texTopX = 10; b.texTopY = 0;
        b.texBottomX = 8;  b.texBottomY = 0;
        b.texSideX = 11; b.texSideY = 0;
        s_blocks[(size_t)BlockType::BioluminescentMoss] = b;
    }

    // BaseHabitatWall
    {
        BlockInfo b;
        b.name = "Habitat Wall";
        b.isSolid = true;
        b.hardness = 1.5f;
        b.dropItem = ItemType::BlockItem_BaseWall;
        b.texTopX = 12; b.texTopY = 0;
        b.texBottomX = 12; b.texBottomY = 0;
        b.texSideX = 12; b.texSideY = 0;
        s_blocks[(size_t)BlockType::BaseHabitatWall] = b;
    }

    // ReinforcedGlass
    {
        BlockInfo b;
        b.name = "Reinforced Glass";
        b.isSolid = true;
        b.isTransparent = true;
        b.hardness = 1.2f;
        b.dropItem = ItemType::BlockItem_ReinforcedGlass;
        b.texTopX = 13; b.texTopY = 0;
        b.texBottomX = 13; b.texBottomY = 0;
        b.texSideX = 13; b.texSideY = 0;
        s_blocks[(size_t)BlockType::ReinforcedGlass] = b;
    }

    // AirlockDoor
    {
        BlockInfo b;
        b.name = "Airlock Door";
        b.isSolid = true;
        b.hardness = 2.0f;
        b.dropItem = ItemType::BlockItem_AirlockDoor;
        b.texTopX = 14; b.texTopY = 0;
        b.texBottomX = 14; b.texBottomY = 0;
        b.texSideX = 14; b.texSideY = 0;
        s_blocks[(size_t)BlockType::AirlockDoor] = b;
    }

    // Torch
    {
        BlockInfo b;
        b.name = "Plasma Torch";
        b.isSolid = false;
        b.isTransparent = true;
        b.isLightEmitter = true;
        b.emitColor = {1.0f, 0.75f, 0.35f};
        b.emitIntensity = 1.2f;
        b.hardness = 0.2f;
        b.dropItem = ItemType::BlockItem_Torch;
        b.texTopX = 15; b.texTopY = 0;
        b.texBottomX = 15; b.texBottomY = 0;
        b.texSideX = 15; b.texSideY = 0;
        s_blocks[(size_t)BlockType::Torch] = b;
    }

    // BiotDefenseTurret
    {
        BlockInfo b;
        b.name = "Biot Defense Turret";
        b.isSolid = true;
        b.isLightEmitter = true;
        b.emitColor = {0.2f, 0.8f, 1.0f};
        b.emitIntensity = 0.5f;
        b.hardness = 2.5f;
        b.dropItem = ItemType::BlockItem_Turret;
        b.texTopX = 0; b.texTopY = 1;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 0; b.texSideY = 1;
        s_blocks[(size_t)BlockType::BiotDefenseTurret] = b;
    }

    // FabricatorBench
    {
        BlockInfo b;
        b.name = "Fabricator Station";
        b.isSolid = true;
        b.hardness = 2.0f;
        b.dropItem = ItemType::BlockItem_Fabricator;
        b.texTopX = 1; b.texTopY = 1;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 1; b.texSideY = 1;
        s_blocks[(size_t)BlockType::FabricatorBench] = b;
    }

    // SteamVent
    {
        BlockInfo b;
        b.name = "Hull Steam Vent";
        b.isSolid = true;
        b.hardness = 3.0f;
        b.dropItem = ItemType::BlockItem_RamaHull;
        b.texTopX = 2; b.texTopY = 1;
        b.texBottomX = 0; b.texBottomY = 0;
        b.texSideX = 2; b.texSideY = 1;
        s_blocks[(size_t)BlockType::SteamVent] = b;
    }

    // GrassBlock (Lush green earth-inspired terrain)
    {
        BlockInfo b;
        b.name = "Grass Block";
        b.isSolid = true;
        b.hardness = 0.8f;
        b.dropItem = ItemType::BlockItem_Dirt;
        b.texTopX = 3; b.texTopY = 1;
        b.texBottomX = 4; b.texBottomY = 1;
        b.texSideX = 5; b.texSideY = 1;
        s_blocks[(size_t)BlockType::GrassBlock] = b;
    }

    // Dirt
    {
        BlockInfo b;
        b.name = "Dirt";
        b.isSolid = true;
        b.hardness = 0.7f;
        b.dropItem = ItemType::BlockItem_Dirt;
        b.texTopX = 4; b.texTopY = 1;
        b.texBottomX = 4; b.texBottomY = 1;
        b.texSideX = 4; b.texSideY = 1;
        s_blocks[(size_t)BlockType::Dirt] = b;
    }

    // NaturalStone
    {
        BlockInfo b;
        b.name = "Stone";
        b.isSolid = true;
        b.hardness = 2.0f;
        b.dropItem = ItemType::BlockItem_Stone;
        b.texTopX = 6; b.texTopY = 1;
        b.texBottomX = 6; b.texBottomY = 1;
        b.texSideX = 6; b.texSideY = 1;
        s_blocks[(size_t)BlockType::NaturalStone] = b;
    }

    // Sand
    {
        BlockInfo b;
        b.name = "Golden Sand";
        b.isSolid = true;
        b.hardness = 0.6f;
        b.dropItem = ItemType::BlockItem_Sand;
        b.texTopX = 7; b.texTopY = 1;
        b.texBottomX = 7; b.texBottomY = 1;
        b.texSideX = 7; b.texSideY = 1;
        s_blocks[(size_t)BlockType::Sand] = b;
    }

    // WoodLog
    {
        BlockInfo b;
        b.name = "Wood Log";
        b.isSolid = true;
        b.hardness = 1.4f;
        b.dropItem = ItemType::BlockItem_Wood;
        b.texTopX = 8; b.texTopY = 1;
        b.texBottomX = 8; b.texBottomY = 1;
        b.texSideX = 9; b.texSideY = 1;
        s_blocks[(size_t)BlockType::WoodLog] = b;
    }

    // FoliageLeaves
    {
        BlockInfo b;
        b.name = "Foliage Leaves";
        b.isSolid = true;
        b.isTransparent = false; // Rendered in opaque mesh with alpha-test cutout
        b.hardness = 0.3f;
        b.dropItem = ItemType::BlockItem_Leaves;
        b.texTopX = 10; b.texTopY = 1;
        b.texBottomX = 10; b.texBottomY = 1;
        b.texSideX = 10; b.texSideY = 1;
        s_blocks[(size_t)BlockType::FoliageLeaves] = b;
    }

    // Items setup
    auto regItem = [](ItemType type, const std::string& name, const std::string& desc, bool tool, bool placeable, BlockType block, uint8_t ix, uint8_t iy) {
        ItemInfo it;
        it.name = name;
        it.description = desc;
        it.isTool = tool;
        it.isPlaceable = placeable;
        it.placeBlock = block;
        it.iconX = ix;
        it.iconY = iy;
        it.maxStack = tool ? 1 : 64;
        s_items[type] = it;
    };

    regItem(ItemType::RayGun, "Ray Gun", "Energy weapon and plasma bolt projector [Left Click]", true, false, BlockType::Air, 0, 2);
    regItem(ItemType::MiningDrill, "Mining Laser Drill", "High-frequency plasma excavator for instant matter breakdown [Hold: Left Click]", true, false, BlockType::Air, 8, 3);
    regItem(ItemType::Flashlight, "Flashlight", "High-power directional suit floodlight [Toggle: F]", true, false, BlockType::Air, 1, 2);
    regItem(ItemType::Jetpack, "Thruster Jetpack", "Personal low-g propulsion unit [Hold: Space]", true, false, BlockType::Air, 2, 2);
    regItem(ItemType::Grenade, "Plasma Frag Grenade", "Throwable high-explosive ordnance with 2.5s fuse [Left/Right Click]", false, false, BlockType::Air, 9, 3);

    regItem(ItemType::TitaniumOreItem, "Titanium Ore", "Raw crystalline titanium harvested from Rama's ribs", false, false, BlockType::Air, 3, 2);
    regItem(ItemType::TitaniumIngot, "Titanium Ingot", "Refined structural alloy for base expansion", false, false, BlockType::Air, 4, 2);
    regItem(ItemType::CobaltCrystalItem, "Cobalt Crystal", "Resonant blue energy crystals", false, false, BlockType::Air, 5, 2);
    regItem(ItemType::CarboniteChunk, "Carbonite Chunk", "Dense carbon composite material", false, false, BlockType::Air, 6, 2);
    regItem(ItemType::SilicaDust, "Silica Dust", "Refined sand suitable for optical glass", false, false, BlockType::Air, 7, 2);
    regItem(ItemType::SiliconWafer, "Silicon Wafer", "Etched micro-substrate for electronics", false, false, BlockType::Air, 8, 2);

    regItem(ItemType::AlienCircuit, "Alien Circuit", "Salvaged microprocessor board from ancient consoles", false, false, BlockType::Air, 9, 2);
    regItem(ItemType::ServoMotor, "Servo Motor", "Salvaged precision micro-actuator from machines", false, false, BlockType::Air, 10, 2);
    regItem(ItemType::PowerMatrix, "Power Matrix", "Alien fusion power conduit module", false, false, BlockType::Air, 11, 2);
    regItem(ItemType::PlasteelSheet, "Plasteel Sheet", "Ultra-light durable habitat structural plate", false, false, BlockType::Air, 12, 2);
    regItem(ItemType::OpticLens, "Optic Lens", "High-intensity focusing crystal", false, false, BlockType::Air, 13, 2);
    regItem(ItemType::FusionBattery, "Fusion Battery", "Compact high-density energy storage cell", false, false, BlockType::Air, 14, 2);

    regItem(ItemType::BiotShell, "Biot Shell", "Chitinous alloy carapace from nocturnal robot beetles", false, false, BlockType::Air, 0, 3);
    regItem(ItemType::MicroActuator, "Micro Actuator", "High-speed mechanical limb motor from Biots", false, false, BlockType::Air, 1, 3);
    regItem(ItemType::RobotCore, "Robot Core", "Autonomous AI logic sub-processor from Biots", false, false, BlockType::Air, 2, 3);
    regItem(ItemType::EnergyCell, "Energy Cell", "Charged micro-plasma battery from Biots", false, false, BlockType::Air, 3, 3);

    regItem(ItemType::BlockItem_RamaHull, "Hull Alloy Block", "Dark alloy block from Rama hull", false, true, BlockType::RamaHullAlloy, 0, 0);
    regItem(ItemType::BlockItem_AlienRuin, "Alien Ruin Stone", "Ancient megalithic building block", false, true, BlockType::AlienRuinWall, 1, 0);
    regItem(ItemType::BlockItem_DarkMonolith, "Dark Monolith", "Impenetrable obsidian alien obelisk", false, true, BlockType::DarkMonolith, 2, 0);
    regItem(ItemType::BlockItem_AncientMachinery, "Ancient Machine", "Dismantled alien computer terminal", false, true, BlockType::AncientMachinery, 3, 0);
    regItem(ItemType::BlockItem_BrokenReactor, "Reactor Core", "Glowing alien energy station", false, true, BlockType::BrokenReactor, 4, 0);
    regItem(ItemType::BlockItem_TitaniumOre, "Titanium Ore Block", "Unrefined titanium deposit", false, true, BlockType::TitaniumOre, 5, 0);
    regItem(ItemType::BlockItem_CobaltCrystal, "Cobalt Crystal Cluster", "Glowing blue crystal cluster", false, true, BlockType::CobaltCrystal, 6, 0);
    regItem(ItemType::BlockItem_Carbonite, "Carbonite Block", "Dense structural mineral", false, true, BlockType::CarboniteBlock, 7, 0);
    regItem(ItemType::BlockItem_SilicaSand, "Silica Ground", "Loose glass silicon soil", false, true, BlockType::SilicaSand, 8, 0);
    regItem(ItemType::BlockItem_Moss, "Luminescent Moss", "Glowing cyan alien flora", false, true, BlockType::BioluminescentMoss, 10, 0);
    regItem(ItemType::BlockItem_BaseWall, "Habitat Wall", "Pressurized human habitat wall", false, true, BlockType::BaseHabitatWall, 12, 0);
    regItem(ItemType::BlockItem_ReinforcedGlass, "Reinforced Glass", "Transparent pressurized dome glass", false, true, BlockType::ReinforcedGlass, 13, 0);
    regItem(ItemType::BlockItem_AirlockDoor, "Airlock Door", "Automated airtight habitat door", false, true, BlockType::AirlockDoor, 14, 0);
    regItem(ItemType::BlockItem_Torch, "Plasma Torch", "Placeable glowing plasma beacon", false, true, BlockType::Torch, 15, 0);
    regItem(ItemType::BlockItem_Turret, "Biot Defense Turret", "Auto-targeting turret against night biots", false, true, BlockType::BiotDefenseTurret, 0, 1);
    regItem(ItemType::BlockItem_Fabricator, "Fabricator Bench", "Station for advanced Rama construction", false, true, BlockType::FabricatorBench, 1, 1);
    regItem(ItemType::BlockItem_Grass, "Grass Block", "Natural lush vegetation block", false, true, BlockType::GrassBlock, 3, 1);
    regItem(ItemType::BlockItem_Dirt, "Dirt Block", "Rich fertile soil block", false, true, BlockType::Dirt, 4, 1);
    regItem(ItemType::BlockItem_Stone, "Stone Block", "Natural solid stone", false, true, BlockType::NaturalStone, 6, 1);
    regItem(ItemType::BlockItem_Sand, "Sand Block", "Golden beach sand", false, true, BlockType::Sand, 7, 1);
    regItem(ItemType::BlockItem_Wood, "Wood Log", "Natural tree wood log", false, true, BlockType::WoodLog, 9, 1);
    regItem(ItemType::BlockItem_Leaves, "Leaves Block", "Foliage tree canopy leaves", false, true, BlockType::FoliageLeaves, 10, 1);

    regItem(ItemType::HighCapJetpack, "High-Cap Thruster", "Infinite range propulsion thruster", true, false, BlockType::Air, 4, 3);
    regItem(ItemType::OverclockedRayGun, "Overclocked Ray Gun", "High-intensity plasma cannon", true, false, BlockType::Air, 5, 3);
    regItem(ItemType::OxygenRecycler, "Oxygen Recycler", "Continuous life-support habitat module", false, false, BlockType::Air, 6, 3);
    regItem(ItemType::RadarScanner, "Ruin Scanner", "HUD radar tracking alien ruins and ore veins", false, false, BlockType::Air, 7, 3);
}

const BlockInfo& BlockRegistry::get(BlockType type) {
    if (s_blocks.empty()) init();
    size_t idx = (size_t)type;
    if (idx >= s_blocks.size()) return s_blocks[0];
    return s_blocks[idx];
}

const ItemInfo& BlockRegistry::getItem(ItemType type) {
    if (s_items.empty()) init();
    auto it = s_items.find(type);
    if (it != s_items.end()) return it->second;
    static ItemInfo s_empty{"Unknown", "", BlockType::Air, false, false, 64, 0, 0};
    return s_empty;
}

BlockType BlockRegistry::itemToBlock(ItemType item) {
    const ItemInfo& info = getItem(item);
    return info.isPlaceable ? info.placeBlock : BlockType::Air;
}

ItemType BlockRegistry::blockToItem(BlockType block) {
    const BlockInfo& info = get(block);
    return info.dropItem;
}
