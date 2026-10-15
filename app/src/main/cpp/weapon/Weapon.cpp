#include "Weapon.h"
#include <algorithm>

namespace LastSurvivor {

Weapon::Weapon(WeaponType type, const std::string& name, float damage, float fireRate,
               int magSize, int reserveAmmo, float reloadTime, float range,
               float recoil, float accuracy, float critChance, AudioEvent soundEvent)
    : m_Type(type),
      m_Name(name),
      m_Damage(damage),
      m_FireRate(fireRate),
      m_MagSize(magSize),
      m_CurrentAmmo(magSize),
      m_ReserveAmmo(reserveAmmo),
      m_ReloadTime(reloadTime),
      m_Range(range),
      m_Recoil(recoil),
      m_Accuracy(accuracy),
      m_CritChance(critChance),
      m_SoundEvent(soundEvent) {
}

void Weapon::Update(float dt) {
    if (m_ShotCooldown > 0.0f) {
        m_ShotCooldown -= dt;
    }

    if (m_IsReloading) {
        m_ReloadTimer += dt;
        if (m_ReloadTimer >= m_ReloadTime) {
            int needed = m_MagSize - m_CurrentAmmo;
            int load = std::min(needed, m_ReserveAmmo);
            m_CurrentAmmo += load;
            m_ReserveAmmo -= load;
            m_IsReloading = false;
            m_ReloadTimer = 0.0f;
        }
    }
}

bool Weapon::CanShoot() const {
    return !m_IsReloading && m_ShotCooldown <= 0.0f && m_CurrentAmmo > 0;
}

bool Weapon::Shoot() {
    if (m_IsReloading) return false;

    if (m_CurrentAmmo <= 0) {
        AudioManager::Instance().PostEvent(AudioEvent::EMPTY_CLICK);
        Reload();
        return false;
    }

    if (m_ShotCooldown > 0.0f) return false;

    m_CurrentAmmo--;
    m_ShotCooldown = (m_FireRate > 0.0f) ? (1.0f / m_FireRate) : 0.2f;
    AudioManager::Instance().PostEvent(m_SoundEvent);

    return true;
}

bool Weapon::Reload() {
    if (m_IsReloading) return false;
    if (m_CurrentAmmo >= m_MagSize) return false;
    if (m_ReserveAmmo <= 0) return false;

    m_IsReloading = true;
    m_ReloadTimer = 0.0f;
    AudioManager::Instance().PostEvent(AudioEvent::RELOAD);
    return true;
}

void Weapon::AddReserveAmmo(int amount) {
    m_ReserveAmmo += amount;
}

float Weapon::GetReloadProgress() const {
    if (!m_IsReloading || m_ReloadTime <= 0.0f) return 0.0f;
    return std::min(1.0f, m_ReloadTimer / m_ReloadTime);
}

void Weapon::UpgradeDamage(float multiplier) {
    m_Damage *= multiplier;
}

void Weapon::UpgradeReloadSpeed(float multiplier) {
    m_ReloadTime = std::max(0.4f, m_ReloadTime * multiplier);
}

void Weapon::UpgradeMagSize(int bonus) {
    m_MagSize += bonus;
    m_CurrentAmmo += bonus;
}

void Weapon::UpgradeCritChance(float bonus) {
    m_CritChance = std::min(0.8f, m_CritChance + bonus);
}

// Concrete weapons
Pistol::Pistol()
    : Weapon(WeaponType::PISTOL, "9mm Tactical", 28.0f, 4.0f, 15, 90, 1.2f, 40.0f, 0.8f, 0.92f, 0.15f, AudioEvent::GUNSHOT_PISTOL) {}

SMG::SMG()
    : Weapon(WeaponType::SMG, "Vector Sub", 18.0f, 12.0f, 30, 180, 1.5f, 30.0f, 1.2f, 0.82f, 0.10f, AudioEvent::GUNSHOT_SMG) {}

AssaultRifle::AssaultRifle()
    : Weapon(WeaponType::ASSAULT_RIFLE, "M4A1 Survivor", 35.0f, 8.0f, 30, 150, 1.8f, 60.0f, 1.4f, 0.88f, 0.18f, AudioEvent::GUNSHOT_RIFLE) {}

Shotgun::Shotgun()
    : Weapon(WeaponType::SHOTGUN, "SPAS-12 Combat", 16.0f, 1.5f, 8, 48, 2.4f, 20.0f, 2.8f, 0.70f, 0.20f, AudioEvent::GUNSHOT_SHOTGUN) {}

SniperRifle::SniperRifle()
    : Weapon(WeaponType::SNIPER_RIFLE, "AWM Marksman", 140.0f, 0.9f, 5, 25, 2.6f, 100.0f, 3.5f, 0.98f, 0.35f, AudioEvent::GUNSHOT_SNIPER) {}

} // namespace LastSurvivor
