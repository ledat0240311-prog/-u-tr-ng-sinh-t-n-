#ifndef WEAPON_H
#define WEAPON_H

#include "Math3D.h"
#include "AudioManager.h"
#include <string>

namespace LastSurvivor {

enum class WeaponType {
    PISTOL = 0,
    SMG = 1,
    ASSAULT_RIFLE = 2,
    SHOTGUN = 3,
    SNIPER_RIFLE = 4
};

class Weapon {
public:
    Weapon(WeaponType type, const std::string& name, float damage, float fireRate,
           int magSize, int reserveAmmo, float reloadTime, float range,
           float recoil, float accuracy, float critChance, AudioEvent soundEvent);
    virtual ~Weapon() = default;

    virtual void Update(float dt);
    virtual bool CanShoot() const;
    virtual bool Shoot();
    virtual bool Reload();
    virtual void AddReserveAmmo(int amount);

    void UpgradeDamage(float multiplier);
    void UpgradeReloadSpeed(float multiplier);
    void UpgradeMagSize(int bonus);
    void UpgradeCritChance(float bonus);

    WeaponType GetType() const { return m_Type; }
    const std::string& GetName() const { return m_Name; }
    float GetDamage() const { return m_Damage; }
    float GetFireRate() const { return m_FireRate; }
    int GetCurrentAmmo() const { return m_CurrentAmmo; }
    int GetMagSize() const { return m_MagSize; }
    int GetReserveAmmo() const { return m_ReserveAmmo; }
    float GetReloadTime() const { return m_ReloadTime; }
    float GetRange() const { return m_Range; }
    float GetRecoil() const { return m_Recoil; }
    float GetAccuracy() const { return m_Accuracy; }
    float GetCritChance() const { return m_CritChance; }
    bool IsReloading() const { return m_IsReloading; }
    float GetReloadProgress() const;
    AudioEvent GetSoundEvent() const { return m_SoundEvent; }

protected:
    WeaponType m_Type;
    std::string m_Name;
    float m_Damage;
    float m_FireRate;       // shots per second
    int m_MagSize;
    int m_CurrentAmmo;
    int m_ReserveAmmo;
    float m_ReloadTime;
    float m_Range;
    float m_Recoil;
    float m_Accuracy;
    float m_CritChance;
    AudioEvent m_SoundEvent;

    float m_ShotCooldown{0.0f};
    float m_ReloadTimer{0.0f};
    bool m_IsReloading{false};
};

class Pistol : public Weapon {
public:
    Pistol();
};

class SMG : public Weapon {
public:
    SMG();
};

class AssaultRifle : public Weapon {
public:
    AssaultRifle();
};

class Shotgun : public Weapon {
public:
    Shotgun();
    int GetPelletCount() const { return 8; }
};

class SniperRifle : public Weapon {
public:
    SniperRifle();
};

} // namespace LastSurvivor

#endif // WEAPON_H
