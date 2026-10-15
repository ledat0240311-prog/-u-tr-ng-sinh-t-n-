#ifndef ENEMY_AI_H
#define ENEMY_AI_H

#include "Enemy.h"
#include <vector>
#include <array>

namespace LastSurvivor {

struct EnemyProjectile {
    bool active{false};
    Vector3 position;
    Vector3 velocity;
    float damage{10.0f};
    float lifeTimer{0.0f};
};

class EnemyManager {
public:
    static constexpr int MAX_ENEMIES = 40;
    static constexpr int MAX_PROJECTILES = 30;

    EnemyManager();

    void Reset();
    void Update(float dt, const Vector3& playerPos, bool playerDead);

    Enemy* SpawnEnemy(EnemyType type, const Vector3& pos, float difficultyMultiplier = 1.0f);
    void SpawnProjectile(const Vector3& origin, const Vector3& dir, float speed, float damage);

    Enemy* GetEnemy(int index);
    const Enemy* GetEnemy(int index) const;
    int GetActiveEnemyCount() const;

    std::array<Enemy, MAX_ENEMIES>& GetAllEnemies() { return m_Enemies; }
    std::array<EnemyProjectile, MAX_PROJECTILES>& GetProjectiles() { return m_Projectiles; }

private:
    std::array<Enemy, MAX_ENEMIES> m_Enemies;
    std::array<EnemyProjectile, MAX_PROJECTILES> m_Projectiles;
};

} // namespace LastSurvivor

#endif // ENEMY_AI_H
