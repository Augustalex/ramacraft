#pragma once

#include <vector>
#include <string>
#include "Block.hpp"
#include "Inventory.hpp"

struct CraftingIngredient {
    ItemType item = ItemType::None;
    int count = 1;
};

struct CraftingRecipe {
    std::string name;
    std::string description;
    ItemType outputItem = ItemType::None;
    int outputCount = 1;
    std::vector<CraftingIngredient> ingredients;
    bool isBaseBuilding = false;
};

class CraftingSystem {
public:
    static CraftingSystem& instance();

    void initRecipes();
    const std::vector<CraftingRecipe>& getRecipes() const { return m_recipes; }

    bool canCraft(const CraftingRecipe& recipe, const Inventory& inv) const;
    bool craft(const CraftingRecipe& recipe, Inventory& inv);

private:
    CraftingSystem() = default;
    std::vector<CraftingRecipe> m_recipes;
};
