#include "HUD.h"
#include <cstring>
#include <algorithm>

namespace LastSurvivor {

void HUD::PackHUDData(const Player& player, const WeaponManager& weapons,
                      const WaveManager& waves, float hitmarkerTimer,
                      float damageVignette, HUDData& outData) {
    outData.hp = player.GetHp();
    outData.maxHp = player.GetMaxHp();
    outData.armor = player.GetArmor();
    outData.maxArmor = player.GetMaxArmor();
    outData.stamina = player.GetStamina();
    outData.maxStamina = player.GetMaxStamina();

    Weapon* active = weapons.GetActiveWeapon();
    if (active) {
        outData.currentAmmo = active->GetCurrentAmmo();
        outData.reserveAmmo = active->GetReserveAmmo();
        outData.weaponSlot = weapons.GetActiveSlot();
        strncpy(outData.weaponName, active->GetName().c_str(), sizeof(outData.weaponName) - 1);
        outData.weaponName[sizeof(outData.weaponName) - 1] = '\0';
        outData.isReloading = active->IsReloading() ? 1 : 0;
        outData.reloadProgress = active->GetReloadProgress();
    } else {
        outData.currentAmmo = 0;
        outData.reserveAmmo = 0;
        outData.weaponSlot = 0;
        strncpy(outData.weaponName, "None", sizeof(outData.weaponName));
        outData.isReloading = 0;
        outData.reloadProgress = 0.0f;
    }

    outData.waveNumber = waves.GetCurrentWave();
    outData.enemiesRemaining = waves.GetEnemiesRemaining();
    outData.totalKills = waves.GetTotalKills();
    outData.score = waves.GetScore();
    outData.coins = waves.GetCoins();

    if (player.IsDead()) {
        outData.waveState = 3; // Game Over
    } else {
        outData.waveState = static_cast<int>(waves.GetState());
    }
    outData.stateTimer = waves.GetStateTimer();

    outData.isAiming = player.IsAiming() ? 1 : 0;
    outData.hitmarkerActive = (hitmarkerTimer > 0.0f) ? 1 : 0;
    outData.damageVignette = std::clamp(damageVignette, 0.0f, 1.0f);
}

} // namespace LastSurvivor
