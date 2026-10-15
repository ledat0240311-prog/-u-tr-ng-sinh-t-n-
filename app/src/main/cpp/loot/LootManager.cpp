#include "LootManager.h"
#include "AudioManager.h"
#include <cstdlib>

namespace LastSurvivor {

LootManager::LootManager() {
    Reset();
}

void LootManager::Reset() {
    for (auto& item : m_Items) {
        item.active = false;
    }
}

void LootManager::SpawnDrop(const Vector3& pos, EnemyType enemyType) {
    // Determine drop rate
    int roll = rand() % 100;
    if (roll > 65) return; // 65% drop chance

    LootType type = LootType::AMMO;
    int value = 30;

    int typeRoll = rand() % 100;
    if (typeRoll < 40) {
        type = LootType::AMMO;
        value = 25 + (rand() % 20);
    } else if (typeRoll < 70) {
        type = LootType::HEALTH;
        value = 25;
    } else if (typeRoll < 85) {
        type = LootType::ARMOR;
        value = 25;
    } else {
        type = LootType::COINS;
        value = 30 + (rand() % 30);
    }

    for (auto& item : m_Items) {
        if (!item.active) {
            item.active = true;
            item.type = type;
            item.position = Vector3(pos.x, 0.4f, pos.z);
            item.value = value;
            item.rotationYaw = 0.0f;
            item.lifeTimer = 25.0f;
            return;
        }
    }
}

void LootManager::Update(float dt, const Vector3& playerPos, Player& player, WeaponManager& weapons) {
    const float PICKUP_RADIUS_SQ = 2.0f * 2.0f;

    for (auto& item : m_Items) {
        if (item.active) {
            item.rotationYaw += 90.0f * dt;
            item.lifeTimer -= dt;
            if (item.lifeTimer <= 0.0f) {
                item.active = false;
                continue;
            }

            float distSq = Vector3::DistanceSq(item.position, playerPos);
            if (distSq <= PICKUP_RADIUS_SQ) {
                // Collect item
                switch (item.type) {
                    case LootType::AMMO: {
                        Weapon* w = weapons.GetActiveWeapon();
                        if (w) {
                            w->AddReserveAmmo(item.value);
                        }
                        break;
                    }
                    case LootType::HEALTH:
                        player.Heal(static_cast<float>(item.value));
                        break;
                    case LootType::ARMOR:
                        player.AddArmor(static_cast<float>(item.value));
                        break;
                    case LootType::COINS:
                        // Coins handled in game logic
                        break;
                }

                AudioManager::Instance().PostEvent(AudioEvent::LOOT_PICKUP);
                item.active = false;
            }
        }
    }
}

} // namespace LastSurvivor
