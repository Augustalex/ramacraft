#pragma once

#include <vector>
#include <string>
#include "Block.hpp"

struct ItemStack {
    ItemType type = ItemType::None;
    int count = 0;

    bool isEmpty() const {
        return type == ItemType::None || count <= 0;
    }

    void clear() {
        type = ItemType::None;
        count = 0;
    }
};

class Inventory {
public:
    static constexpr int HOTBAR_SIZE = 9;
    static constexpr int INVENTORY_SIZE = 36;

    Inventory();

    void initStartingGear();

    ItemStack& getSlot(int index);
    const ItemStack& getSlot(int index) const;

    int getSelectedHotbarIndex() const { return m_selectedHotbar; }
    void setSelectedHotbarIndex(int idx) {
        if (idx >= 0 && idx < HOTBAR_SIZE) m_selectedHotbar = idx;
    }

    ItemStack& getSelectedItem() {
        return m_slots[m_selectedHotbar];
    }
    const ItemStack& getSelectedItem() const {
        return m_slots[m_selectedHotbar];
    }

    bool addItem(ItemType type, int count = 1);
    bool removeItem(ItemType type, int count = 1);
    int countItem(ItemType type) const;
    bool hasItem(ItemType type, int count = 1) const;

    void swapSlots(int a, int b);

private:
    std::vector<ItemStack> m_slots;
    int m_selectedHotbar = 0;
};
