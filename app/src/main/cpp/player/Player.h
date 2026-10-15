#ifndef PLAYER_H
#define PLAYER_H

#include "Math3D.h"

namespace LastSurvivor {

class Player {
public:
    Player();

    void Reset();
    void Update(float dt);

    void Move(float inputX, float inputY, bool sprint);
    void RotateView(float deltaYaw, float deltaPitch);
    void Jump();
    void SetCrouch(bool crouch);
    void SetAiming(bool aiming);

    bool TakeDamage(float amount);
    void Heal(float amount);
    void AddArmor(float amount);
    void UpgradeMaxHp(float amount);
    void UpgradeMaxArmor(float amount);
    void UpgradeSpeed(float multiplier);

    // Getters
    const Vector3& GetPosition() const { return m_Position; }
    Vector3 GetEyePosition() const;
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;

    float GetHp() const { return m_Hp; }
    float GetMaxHp() const { return m_MaxHp; }
    float GetArmor() const { return m_Armor; }
    float GetMaxArmor() const { return m_MaxArmor; }
    float GetStamina() const { return m_Stamina; }
    float GetMaxStamina() const { return m_MaxStamina; }
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }

    bool IsAiming() const { return m_IsAiming; }
    bool IsSprinting() const { return m_IsSprinting; }
    bool IsCrouching() const { return m_IsCrouching; }
    bool IsDead() const { return m_IsDead; }
    bool IsGrounded() const { return m_IsGrounded; }

    float GetHitTimer() const { return m_HitTimer; }

private:
    Vector3 m_Position{0.0f, 0.0f, 0.0f};
    Vector3 m_Velocity{0.0f, 0.0f, 0.0f};

    float m_Yaw{0.0f};     // degrees
    float m_Pitch{0.0f};   // degrees

    float m_Hp{100.0f};
    float m_MaxHp{100.0f};
    float m_Armor{50.0f};
    float m_MaxArmor{50.0f};
    float m_Stamina{100.0f};
    float m_MaxStamina{100.0f};

    float m_BaseSpeed{4.5f};
    float m_SpeedMultiplier{1.0f};
    float m_SprintMultiplier{1.6f};
    float m_CrouchMultiplier{0.5f};

    bool m_IsAiming{false};
    bool m_IsSprinting{false};
    bool m_IsCrouching{false};
    bool m_IsGrounded{true};
    bool m_IsDead{false};

    float m_RegenTimer{0.0f};
    float m_HitTimer{0.0f};
    float m_FootstepTimer{0.0f};
};

} // namespace LastSurvivor

#endif // PLAYER_H
