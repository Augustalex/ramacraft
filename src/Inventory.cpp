#include "Inventory.hpp"
#include <algorithm>

Inventory::Inventory() {
    m_slots.resize(INVENTORY_SIZE);
}

void Inventory::initStartingGear() {
    for (auto& s : m_slots) s.clear();

    // Hotbar Setup with dedicated Combat Ray Gun, Grenades, Mining Drill and building gear:
    m_slots[0] = {ItemType::RayGun, 1};                  // 1: Combat Ray Gun
    m_slots[1] = {ItemType::Grenade, 8};                 // 2: Plasma Frag Grenades (x8)
    m_slots[2] = {ItemType::MiningDrill, 1};             // 3: Plasma Mining Laser Drill
    m_slots[3] = {ItemType::Flashlight, 1};              // 4: Directional Flashlight
    m_slots[4] = {ItemType::BlockItem_Torch, 20};        // 5: Plasma Torches
    m_slots[5] = {ItemType::BlockItem_BaseWall, 32};     // 6: Habitat Wall blocks
    m_slots[6] = {ItemType::BlockItem_ReinforcedGlass, 16};// 7: Dome Glass
    m_slots[7] = {ItemType::BlockItem_Grass, 32};        // 8: Natural Grass blocks
    m_slots[8] = {ItemType::BlockItem_Wood, 16};         // 9: Wood Logs

    m_selectedHotbar = 0;
}

ItemStack& Inventory::getSlot(int index) {
    if (index >= 0 && index < (int)m_slots.size()) {
        return m_slots[index];
    }
    static ItemStack s_empty;
    return s_empty;
}

const ItemStack& Inventory::getSlot(int index) const {
    if (index >= 0 && index < (int)m_slots.size()) {
        return m_slots[index];
    }
    static ItemStack s_empty;
    return s_empty;
}

bool Inventory::addItem(ItemType type, int count) {
    if (type == ItemType::None || count <= 0) return false;

    const ItemInfo& info = BlockRegistry::getItem(type);
    int remaining = count;

    // 1. Try to merge into existing non-full stacks
    for (auto& slot : m_slots) {
        if (slot.type == type && slot.count < info.maxStack) {
            int canAdd = info.maxStack - slot.count;
            int add = std::min(remaining, canAdd);
            slot.count += add;
            remaining -= add;
            if (remaining <= 0) return true;
        }
    }

    // 2. Try to put into empty slots
    for (auto& slot : m_slots) {
        if (slot.isEmpty()) {
            slot.type = type;
            int add = std::min(remaining, info.maxStack);
            slot.count = add;
            remaining -= add;
            if (remaining <= 0) return true;
        }
    }

    return remaining == 0;
}

bool Inventory::removeItem(ItemType type, int count) {
    if (type == ItemType::None || count <= 0) return false;
    if (!hasItem(type, count)) return false;

    int remaining = count;
    for (auto& slot : m_slots) {
        if (slot.type == type) {
            if (slot.count <= remaining) {
                remaining -= slot.count;
                slot.clear();
            } else {
                slot.count -= remaining;
                remaining = 0;
            }
            if (remaining <= 0) break;
        }
    }
    return true;
}

int Inventory::countItem(ItemType type) const {
    int total = 0;
    for (const auto& slot : m_slots) {
        if (slot.type == type) {
            total += slot.count;
        }
    }
    return total;
}

bool Inventory::hasItem(ItemType type, int count) const {
    return countItem(type) >= count;
}

void Inventory::swapSlots(int a, int b) {
    if (a >= 0 && a < INVENTORY_SIZE && b >= 0 && b < INVENTORY_SIZE) {
        std::swap(m_slots[a], m_slots[b]);
    }
}
