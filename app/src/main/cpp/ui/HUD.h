#ifndef HUD_H
#define HUD_H

#include "Player.h"
#include "WeaponManager.h"
#include "WaveManager.h"
#include <string>

namespace LastSurvivor {

struct HUDData {
    float hp{100.0f};
    float maxHp{100.0f};
    float armor{50.0f};
    float maxArmor{50.0f};
    float stamina{100.0f};
    float maxStamina{100.0f};

    int currentAmmo{30};
    int reserveAmmo{120};
    int weaponSlot{0};
    char weaponName[32]{"M4A1 Survivor"};

    int waveNumber{1};
    int enemiesRemaining{5};
    int totalKills{0};
    int score{0};
    int coins{100};

    int waveState{0}; // 0: preparing, 1: in_progress, 2: cleared, 3: game_over
    float stateTimer{0.0f};

    float reloadProgress{0.0f};
    int isAiming{0};
    int isReloading{0};
    int hitmarkerActive{0};
    float damageVignette{0.0f};
};

class HUD {
public:
    static void PackHUDData(const Player& player, const WeaponManager& weapons,
                            const WaveManager& waves, float hitmarkerTimer,
                            float damageVignette, HUDData& outData);
};

} // namespace LastSurvivor

#endif // HUD_H
