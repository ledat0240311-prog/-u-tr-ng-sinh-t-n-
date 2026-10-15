#ifndef WEAPON_MANAGER_H
#define WEAPON_MANAGER_H

#include "Weapon.h"
#include <vector>
#include <memory>

namespace LastSurvivor {

class WeaponManager {
public:
    WeaponManager();

    void Update(float dt);
    bool Shoot();
    bool Reload();
    bool SwitchWeapon(int slot);
    void NextWeapon();
    void PreviousWeapon();

    Weapon* GetActiveWeapon() const;
    int GetActiveSlot() const { return m_ActiveSlot; }
    int GetWeaponCount() const { return static_cast<int>(m_Weapons.size()); }
    Weapon* GetWeapon(int slot) const;

    void AddAmmo(WeaponType type, int amount);
    void UpgradeAllDamage(float multiplier);
    void UpgradeAllReloadSpeed(float multiplier);
    void UpgradeAllMagSize(int bonus);
    void UpgradeAllCritChance(float bonus);

    void Reset();

private:
    std::vector<std::unique_ptr<Weapon>> m_Weapons;
    int m_ActiveSlot{0};
};

} // namespace LastSurvivor

#endif // WEAPON_MANAGER_H
