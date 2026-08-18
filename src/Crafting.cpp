#include "Crafting.hpp"
#include "Audio.hpp"

CraftingSystem& CraftingSystem::instance() {
    static CraftingSystem s_crafting;
    return s_crafting;
}

void CraftingSystem::initRecipes() {
    m_recipes.clear();

    // 1. Plasma Torches
    {
        CraftingRecipe r;
        r.name = "Plasma Torches (x4)";
        r.description = "Compact self-powering ruin illumination beacons";
        r.outputItem = ItemType::BlockItem_Torch;
        r.outputCount = 4;
        r.ingredients = {
            {ItemType::SilicaDust, 1},
            {ItemType::CobaltCrystalItem, 1}
        };
        m_recipes.push_back(r);
    }

    // Plasma Frag Grenades
    {
        CraftingRecipe r;
        r.name = "Plasma Frag Grenades (x4)";
        r.description = "Explosive ordnance: 2.5s timed detonation crater charge";
        r.outputItem = ItemType::Grenade;
        r.outputCount = 4;
        r.ingredients = {
            {ItemType::CarboniteChunk, 2},
            {ItemType::SilicaDust, 1}
        };
        m_recipes.push_back(r);
    }

    // 2. Plasteel Sheet (Material Refining)
    {
        CraftingRecipe r;
        r.name = "Plasteel Sheet (x2)";
        r.description = "Refined ultra-hard structural alloy plate";
        r.outputItem = ItemType::PlasteelSheet;
        r.outputCount = 2;
        r.ingredients = {
            {ItemType::TitaniumOreItem, 2},
            {ItemType::CarboniteChunk, 2}
        };
        m_recipes.push_back(r);
    }

    // 3. Silicon Wafer (Microchip Substrate)
    {
        CraftingRecipe r;
        r.name = "Silicon Wafer (x2)";
        r.description = "Purified crystalline silicon substrate";
        r.outputItem = ItemType::SiliconWafer;
        r.outputCount = 2;
        r.ingredients = {
            {ItemType::SilicaDust, 3}
        };
        m_recipes.push_back(r);
    }

    // 4. Habitat Base Wall (x4)
    {
        CraftingRecipe r;
        r.name = "Habitat Base Wall (x4)";
        r.description = "Pressurized insulated habitat module wall";
        r.outputItem = ItemType::BlockItem_BaseWall;
        r.outputCount = 4;
        r.isBaseBuilding = true;
        r.ingredients = {
            {ItemType::PlasteelSheet, 1},
            {ItemType::TitaniumOreItem, 2}
        };
        m_recipes.push_back(r);
    }

    // 5. Reinforced Visor Glass (x4)
    {
        CraftingRecipe r;
        r.name = "Reinforced Glass (x4)";
        r.description = "High-pressure transparent dome glass";
        r.outputItem = ItemType::BlockItem_ReinforcedGlass;
        r.outputCount = 4;
        r.isBaseBuilding = true;
        r.ingredients = {
            {ItemType::SilicaDust, 2},
            {ItemType::CarboniteChunk, 1}
        };
        m_recipes.push_back(r);
    }

    // 6. Airlock Door
    {
        CraftingRecipe r;
        r.name = "Airlock Door";
        r.description = "Airtight modular doorway for Rama habitat base";
        r.outputItem = ItemType::BlockItem_AirlockDoor;
        r.outputCount = 1;
        r.isBaseBuilding = true;
        r.ingredients = {
            {ItemType::PlasteelSheet, 2},
            {ItemType::ServoMotor, 1}
        };
        m_recipes.push_back(r);
    }

    // 7. Fabricator Station
    {
        CraftingRecipe r;
        r.name = "Fabricator Station";
        r.description = "Workbench for high-tier Rama engineering";
        r.outputItem = ItemType::BlockItem_Fabricator;
        r.outputCount = 1;
        r.isBaseBuilding = true;
        r.ingredients = {
            {ItemType::PlasteelSheet, 2},
            {ItemType::AlienCircuit, 1},
            {ItemType::TitaniumOreItem, 2}
        };
        m_recipes.push_back(r);
    }

    // 8. Biot Defense Turret
    {
        CraftingRecipe r;
        r.name = "Biot Defense Turret";
        r.description = "Automated plasma sentry targeting nocturnal robot beetles";
        r.outputItem = ItemType::BlockItem_Turret;
        r.outputCount = 1;
        r.isBaseBuilding = true;
        r.ingredients = {
            {ItemType::AlienCircuit, 1},
            {ItemType::MicroActuator, 2},
            {ItemType::EnergyCell, 1},
            {ItemType::TitaniumOreItem, 3}
        };
        m_recipes.push_back(r);
    }

    // 9. High-Capacity Thruster Jetpack
    {
        CraftingRecipe r;
        r.name = "High-Cap Thruster Upgrade";
        r.description = "Enhances jetpack with 2.5x thrust and extended fuel tank";
        r.outputItem = ItemType::HighCapJetpack;
        r.outputCount = 1;
        r.ingredients = {
            {ItemType::MicroActuator, 3},
            {ItemType::BiotShell, 2},
            {ItemType::PowerMatrix, 1}
        };
        m_recipes.push_back(r);
    }

    // 10. Overclocked Ray Gun
    {
        CraftingRecipe r;
        r.name = "Overclocked Twin-Ray Gun";
        r.description = "High-yield plasma beam weapon with instant mining drill";
        r.outputItem = ItemType::OverclockedRayGun;
        r.outputCount = 1;
        r.ingredients = {
            {ItemType::RobotCore, 2},
            {ItemType::CobaltCrystalItem, 4},
            {ItemType::PowerMatrix, 1}
        };
        m_recipes.push_back(r);
    }

    // 11. Oxygen Recycler
    {
        CraftingRecipe r;
        r.name = "Oxygen Recycler";
        r.description = "Life support unit providing continuous oxygen in Rama";
        r.outputItem = ItemType::OxygenRecycler;
        r.outputCount = 1;
        r.ingredients = {
            {ItemType::RobotCore, 1},
            {ItemType::BiotShell, 3},
            {ItemType::SiliconWafer, 2}
        };
        m_recipes.push_back(r);
    }

    // 12. Ruin Scanner Radar
    {
        CraftingRecipe r;
        r.name = "Ruin Scanner Radar";
        r.description = "Geological scanner revealing mineral nodes and ancient structures";
        r.outputItem = ItemType::RadarScanner;
        r.outputCount = 1;
        r.ingredients = {
            {ItemType::CobaltCrystalItem, 2},
            {ItemType::AlienCircuit, 2},
            {ItemType::SiliconWafer, 2}
        };
        m_recipes.push_back(r);
    }
}

bool CraftingSystem::canCraft(const CraftingRecipe& recipe, const Inventory& inv) const {
    for (const auto& ing : recipe.ingredients) {
        if (!inv.hasItem(ing.item, ing.count)) {
            return false;
        }
    }
    return true;
}

bool CraftingSystem::craft(const CraftingRecipe& recipe, Inventory& inv) {
    if (!canCraft(recipe, inv)) return false;

    // Deduct ingredients
    for (const auto& ing : recipe.ingredients) {
        inv.removeItem(ing.item, ing.count);
    }

    // Add crafted output
    inv.addItem(recipe.outputItem, recipe.outputCount);

    AudioSystem::instance().playSound(SoundEffect::CraftItem);
    return true;
}
