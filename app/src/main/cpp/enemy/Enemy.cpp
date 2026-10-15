#include "Enemy.h"
#include "AudioManager.h"
#include <cmath>
#include <algorithm>

namespace LastSurvivor {

constexpr float RAD2DEG = 180.0f / 3.14159265358979323846f;

Enemy::Enemy() {
    m_IsActive = false;
    m_State = AIState::IDLE;
}

void Enemy::Spawn(EnemyType type, const Vector3& pos, float difficultyMultiplier) {
    m_IsActive = true;
    m_Type = type;
    m_Position = pos;
    m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
    m_Yaw = 0.0f;
    m_State = AIState::SEARCH;
    m_StateTimer = 0.0f;
    m_AttackTimer = 0.0f;
    m_DidAttack = false;
    m_DidShootProj = false;

    switch (type) {
        case EnemyType::BASIC:
            m_MaxHp = 80.0f * difficultyMultiplier;
            m_MoveSpeed = 3.2f + (difficultyMultiplier * 0.2f);
            m_AttackDamage = 14.0f * difficultyMultiplier;
            m_AttackRange = 1.6f;
            m_AttackCooldown = 1.1f;
            m_BoundingRadius = 0.55f;
            m_Height = 1.8f;
            break;

        case EnemyType::RANGED:
            m_MaxHp = 60.0f * difficultyMultiplier;
            m_MoveSpeed = 2.8f;
            m_AttackDamage = 12.0f * difficultyMultiplier;
            m_AttackRange = 16.0f;
            m_AttackCooldown = 1.8f;
            m_RangedPreferredDist = 11.0f;
            m_BoundingRadius = 0.5f;
            m_Height = 1.7f;
            break;

        case EnemyType::TANK:
            m_MaxHp = 240.0f * difficultyMultiplier;
            m_MoveSpeed = 2.0f;
            m_AttackDamage = 32.0f * difficultyMultiplier;
            m_AttackRange = 2.2f;
            m_AttackCooldown = 1.6f;
            m_BoundingRadius = 0.9f;
            m_Height = 2.4f;
            break;

        case EnemyType::FAST:
            m_MaxHp = 45.0f * difficultyMultiplier;
            m_MoveSpeed = 5.4f + (difficultyMultiplier * 0.3f);
            m_AttackDamage = 10.0f * difficultyMultiplier;
            m_AttackRange = 1.4f;
            m_AttackCooldown = 0.8f;
            m_BoundingRadius = 0.45f;
            m_Height = 1.5f;
            break;
    }

    m_Hp = m_MaxHp;
}

void Enemy::Update(float dt, const Vector3& playerPos, bool playerDead) {
    if (!m_IsActive) return;

    if (m_AttackTimer > 0.0f) {
        m_AttackTimer -= dt;
    }

    if (m_State == AIState::DEAD) {
        m_StateTimer += dt;
        if (m_StateTimer >= 1.2f) {
            m_IsActive = false; // Despawn after death animation time
        }
        return;
    }

    if (m_State == AIState::DAMAGE) {
        m_StateTimer -= dt;
        if (m_StateTimer <= 0.0f) {
            m_State = AIState::CHASE;
        }
        return;
    }

    if (playerDead) {
        m_State = AIState::IDLE;
        return;
    }

    float distToPlayer = Vector3::Distance(m_Position, playerPos);
    UpdateStateMachine(dt, playerPos, distToPlayer);

    // Apply movement
    m_Position.x += m_Velocity.x * dt;
    m_Position.z += m_Velocity.z * dt;

    // Bounds limit
    const float MAP_BOUND = 34.0f;
    m_Position.x = std::clamp(m_Position.x, -MAP_BOUND, MAP_BOUND);
    m_Position.z = std::clamp(m_Position.z, -MAP_BOUND, MAP_BOUND);
}

void Enemy::UpdateStateMachine(float dt, const Vector3& playerPos, float distToPlayer) {
    Vector3 toPlayer = playerPos - m_Position;
    toPlayer.y = 0.0f;
    float dist = toPlayer.Length();

    // Orient yaw towards player
    if (dist > 0.01f) {
        m_Yaw = std::atan2(-toPlayer.x, -toPlayer.z) * RAD2DEG;
    }

    switch (m_State) {
        case AIState::IDLE:
            m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
            if (dist < m_DetectRadius) {
                m_State = AIState::CHASE;
            }
            break;

        case AIState::SEARCH:
            // Roam or acquire target
            if (dist < m_DetectRadius) {
                m_State = AIState::CHASE;
            }
            break;

        case AIState::CHASE: {
            if (m_Type == EnemyType::RANGED) {
                if (dist <= m_RangedPreferredDist && dist >= 6.0f) {
                    m_State = AIState::ATTACK;
                    m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
                } else if (dist < 6.0f) {
                    // Back away from player
                    Vector3 away = (m_Position - playerPos).Normalized();
                    m_Velocity = away * (m_MoveSpeed * 0.8f);
                } else {
                    Vector3 dir = toPlayer.Normalized();
                    m_Velocity = dir * m_MoveSpeed;
                }
            } else {
                if (dist <= m_AttackRange) {
                    m_State = AIState::ATTACK;
                    m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
                } else {
                    Vector3 dir = toPlayer.Normalized();
                    m_Velocity = dir * m_MoveSpeed;
                }
            }
            break;
        }

        case AIState::ATTACK: {
            m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
            if (m_Type == EnemyType::RANGED) {
                if (dist > m_AttackRange || dist < 4.0f) {
                    m_State = AIState::CHASE;
                } else if (m_AttackTimer <= 0.0f) {
                    // Shoot projectile
                    m_AttackTimer = m_AttackCooldown;
                    m_DidShootProj = true;
                    m_ProjOrigin = Vector3(m_Position.x, m_Position.y + 1.2f, m_Position.z);
                    Vector3 aimTarget = Vector3(playerPos.x, playerPos.y + 1.2f, playerPos.z);
                    m_ProjDir = (aimTarget - m_ProjOrigin).Normalized();
                    AudioManager::Instance().PostEvent(AudioEvent::ENEMY_ATTACK);
                }
            } else {
                if (dist > m_AttackRange * 1.3f) {
                    m_State = AIState::CHASE;
                } else if (m_AttackTimer <= 0.0f) {
                    m_AttackTimer = m_AttackCooldown;
                    m_DidAttack = true;
                    AudioManager::Instance().PostEvent(AudioEvent::ENEMY_ATTACK);
                }
            }
            break;
        }

        default:
            break;
    }
}

bool Enemy::TakeDamage(float damage, bool isCrit) {
    if (!m_IsActive || m_State == AIState::DEAD) return false;

    m_Hp -= damage;
    if (m_Hp <= 0.0f) {
        m_Hp = 0.0f;
        Die();
        return true; // Killed
    }

    m_State = AIState::DAMAGE;
    m_StateTimer = 0.15f; // Brief flinch
    AudioManager::Instance().PostEvent(AudioEvent::ENEMY_HURT);
    return false;
}

void Enemy::Die() {
    m_State = AIState::DEAD;
    m_StateTimer = 0.0f;
    m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
    AudioManager::Instance().PostEvent(AudioEvent::ENEMY_DEATH);
}

bool Enemy::DidShootProjectile(Vector3& outOrigin, Vector3& outDir) {
    if (m_DidShootProj) {
        outOrigin = m_ProjOrigin;
        outDir = m_ProjDir;
        m_DidShootProj = false;
        return true;
    }
    return false;
}

} // namespace LastSurvivor
