#ifndef LOOT_MANAGER_H
#define LOOT_MANAGER_H

#include "Loot.h"
#include "Player.h"
#include "WeaponManager.h"
#include "Enemy.h"
#include <array>

namespace LastSurvivor {

class LootManager {
public:
    static constexpr int MAX_LOOT_ITEMS = 30;

    LootManager();

    void Reset();
    void Update(float dt, const Vector3& playerPos, Player& player, WeaponManager& weapons);

    void SpawnDrop(const Vector3& pos, EnemyType enemyType);

    const std::array<LootItem, MAX_LOOT_ITEMS>& GetItems() const { return m_Items; }

private:
    std::array<LootItem, MAX_LOOT_ITEMS> m_Items;
};

} // namespace LastSurvivor

#endif // LOOT_MANAGER_H
