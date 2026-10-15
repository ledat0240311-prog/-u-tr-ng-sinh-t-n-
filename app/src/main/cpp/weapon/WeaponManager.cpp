#include "WeaponManager.h"

namespace LastSurvivor {

WeaponManager::WeaponManager() {
    Reset();
}

void WeaponManager::Reset() {
    m_Weapons.clear();
    m_Weapons.push_back(std::make_unique<Pistol>());
    m_Weapons.push_back(std::make_unique<SMG>());
    m_Weapons.push_back(std::make_unique<AssaultRifle>());
    m_Weapons.push_back(std::make_unique<Shotgun>());
    m_Weapons.push_back(std::make_unique<SniperRifle>());
    m_ActiveSlot = 0;
}

void WeaponManager::Update(float dt) {
    if (m_ActiveSlot >= 0 && m_ActiveSlot < static_cast<int>(m_Weapons.size())) {
        m_Weapons[m_ActiveSlot]->Update(dt);
    }
}

bool WeaponManager::Shoot() {
    Weapon* active = GetActiveWeapon();
    if (active) {
        return active->Shoot();
    }
    return false;
}

bool WeaponManager::Reload() {
    Weapon* active = GetActiveWeapon();
    if (active) {
        return active->Reload();
    }
    return false;
}

bool WeaponManager::SwitchWeapon(int slot) {
    if (slot >= 0 && slot < static_cast<int>(m_Weapons.size()) && slot != m_ActiveSlot) {
        m_ActiveSlot = slot;
        return true;
    }
    return false;
}

void WeaponManager::NextWeapon() {
    if (!m_Weapons.empty()) {
        m_ActiveSlot = (m_ActiveSlot + 1) % m_Weapons.size();
    }
}

void WeaponManager::PreviousWeapon() {
    if (!m_Weapons.empty()) {
        m_ActiveSlot = (m_ActiveSlot - 1 + m_Weapons.size()) % m_Weapons.size();
    }
}

Weapon* WeaponManager::GetActiveWeapon() const {
    if (m_ActiveSlot >= 0 && m_ActiveSlot < static_cast<int>(m_Weapons.size())) {
        return m_Weapons[m_ActiveSlot].get();
    }
    return nullptr;
}

Weapon* WeaponManager::GetWeapon(int slot) const {
    if (slot >= 0 && slot < static_cast<int>(m_Weapons.size())) {
        return m_Weapons[slot].get();
    }
    return nullptr;
}

void WeaponManager::AddAmmo(WeaponType type, int amount) {
    for (auto& w : m_Weapons) {
        if (w->GetType() == type) {
            w->AddReserveAmmo(amount);
            break;
        }
    }
}

void WeaponManager::UpgradeAllDamage(float multiplier) {
    for (auto& w : m_Weapons) {
        w->UpgradeDamage(multiplier);
    }
}

void WeaponManager::UpgradeAllReloadSpeed(float multiplier) {
    for (auto& w : m_Weapons) {
        w->UpgradeReloadSpeed(multiplier);
    }
}

void WeaponManager::UpgradeAllMagSize(int bonus) {
    for (auto& w : m_Weapons) {
        w->UpgradeMagSize(bonus);
    }
}

void WeaponManager::UpgradeAllCritChance(float bonus) {
    for (auto& w : m_Weapons) {
        w->UpgradeCritChance(bonus);
    }
}

} // namespace LastSurvivor
