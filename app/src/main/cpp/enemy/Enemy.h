#ifndef ENEMY_H
#define ENEMY_H

#include "Math3D.h"

namespace LastSurvivor {

enum class EnemyType {
    BASIC = 0,
    RANGED = 1,
    TANK = 2,
    FAST = 3
};

enum class AIState {
    IDLE = 0,
    SEARCH = 1,
    CHASE = 2,
    ATTACK = 3,
    DAMAGE = 4,
    DEAD = 5
};

class Enemy {
public:
    Enemy();

    void Spawn(EnemyType type, const Vector3& pos, float difficultyMultiplier = 1.0f);
    void Update(float dt, const Vector3& playerPos, bool playerDead);

    bool TakeDamage(float damage, bool isCrit);
    void Die();

    // Getters
    bool IsActive() const { return m_IsActive; }
    EnemyType GetType() const { return m_Type; }
    AIState GetState() const { return m_State; }
    const Vector3& GetPosition() const { return m_Position; }
    float GetYaw() const { return m_Yaw; }
    float GetHp() const { return m_Hp; }
    float GetMaxHp() const { return m_MaxHp; }
    float GetAttackDamage() const { return m_AttackDamage; }
    bool DidAttackThisFrame() { bool a = m_DidAttack; m_DidAttack = false; return a; }
    bool DidShootProjectile(Vector3& outOrigin, Vector3& outDir);

    float GetBoundingRadius() const { return m_BoundingRadius; }
    float GetHeight() const { return m_Height; }

private:
    void UpdateStateMachine(float dt, const Vector3& playerPos, float distToPlayer);

    bool m_IsActive{false};
    EnemyType m_Type{EnemyType::BASIC};
    AIState m_State{AIState::IDLE};

    Vector3 m_Position{0.0f, 0.0f, 0.0f};
    Vector3 m_Velocity{0.0f, 0.0f, 0.0f};
    float m_Yaw{0.0f};

    float m_Hp{100.0f};
    float m_MaxHp{100.0f};
    float m_MoveSpeed{3.0f};
    float m_AttackDamage{15.0f};
    float m_AttackRange{1.8f};
    float m_AttackCooldown{1.2f};
    float m_AttackTimer{0.0f};

    float m_DetectRadius{28.0f};
    float m_RangedPreferredDist{10.0f};

    float m_BoundingRadius{0.6f};
    float m_Height{1.8f};

    float m_StateTimer{0.0f};
    bool m_DidAttack{false};
    bool m_DidShootProj{false};
    Vector3 m_ProjOrigin;
    Vector3 m_ProjDir;
};

} // namespace LastSurvivor

#endif // ENEMY_H
