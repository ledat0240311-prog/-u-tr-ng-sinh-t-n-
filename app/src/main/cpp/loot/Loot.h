#ifndef LOOT_H
#define LOOT_H

#include "Math3D.h"

namespace LastSurvivor {

enum class LootType {
    AMMO = 0,
    HEALTH = 1,
    ARMOR = 2,
    COINS = 3
};

struct LootItem {
    bool active{false};
    LootType type{LootType::AMMO};
    Vector3 position;
    int value{0};
    float rotationYaw{0.0f};
    float lifeTimer{25.0f}; // auto-despawn after 25s
};

} // namespace LastSurvivor

#endif // LOOT_H
