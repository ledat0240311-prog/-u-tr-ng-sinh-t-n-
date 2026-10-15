#include "EnemyAI.h"

namespace LastSurvivor {

EnemyManager::EnemyManager() {
    Reset();
}

void EnemyManager::Reset() {
    for (auto& e : m_Enemies) {
        e = Enemy();
    }
    for (auto& p : m_Projectiles) {
        p.active = false;
    }
}

Enemy* EnemyManager::SpawnEnemy(EnemyType type, const Vector3& pos, float difficultyMultiplier) {
    for (auto& e : m_Enemies) {
        if (!e.IsActive()) {
            e.Spawn(type, pos, difficultyMultiplier);
            return &e;
        }
    }
    return nullptr;
}

void EnemyManager::SpawnProjectile(const Vector3& origin, const Vector3& dir, float speed, float damage) {
    for (auto& p : m_Projectiles) {
        if (!p.active) {
            p.active = true;
            p.position = origin;
            p.velocity = dir * speed;
            p.damage = damage;
            p.lifeTimer = 4.0f;
            return;
        }
    }
}

void EnemyManager::Update(float dt, const Vector3& playerPos, bool playerDead) {
    // Update enemies
    for (auto& e : m_Enemies) {
        if (e.IsActive()) {
            e.Update(dt, playerPos, playerDead);
            Vector3 origin, dir;
            if (e.DidShootProjectile(origin, dir)) {
                SpawnProjectile(origin, dir, 18.0f, e.GetAttackDamage());
            }
        }
    }

    // Update projectiles
    for (auto& p : m_Projectiles) {
        if (p.active) {
            p.position += p.velocity * dt;
            p.lifeTimer -= dt;
            if (p.lifeTimer <= 0.0f || p.position.y < 0.1f) {
                p.active = false;
            }
        }
    }
}

Enemy* EnemyManager::GetEnemy(int index) {
    if (index >= 0 && index < MAX_ENEMIES) {
        return &m_Enemies[index];
    }
    return nullptr;
}

const Enemy* EnemyManager::GetEnemy(int index) const {
    if (index >= 0 && index < MAX_ENEMIES) {
        return &m_Enemies[index];
    }
    return nullptr;
}

int EnemyManager::GetActiveEnemyCount() const {
    int count = 0;
    for (const auto& e : m_Enemies) {
        if (e.IsActive() && e.GetState() != AIState::DEAD) {
            count++;
        }
    }
    return count;
}

} // namespace LastSurvivor
