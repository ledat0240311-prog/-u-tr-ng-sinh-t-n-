#ifndef WAVE_MANAGER_H
#define WAVE_MANAGER_H

#include "EnemyAI.h"
#include <vector>

namespace LastSurvivor {

enum class WaveState {
    PREPARING = 0,
    IN_PROGRESS = 1,
    WAVE_CLEARED = 2,
    GAME_OVER = 3
};

class WaveManager {
public:
    WaveManager();

    void Reset();
    void Update(float dt, EnemyManager& enemyMgr, const Vector3& playerPos);

    void StartNextWave();
    void OnEnemyKilled(EnemyType type);

    int GetCurrentWave() const { return m_CurrentWave; }
    int GetTotalEnemiesInWave() const { return m_TotalEnemiesInWave; }
    int GetEnemiesRemaining() const { return m_EnemiesRemaining; }
    int GetTotalKills() const { return m_TotalKills; }
    int GetScore() const { return m_Score; }
    int GetCoins() const { return m_Coins; }
    void AddCoins(int amount) { m_Coins += amount; }
    bool SpendCoins(int amount);

    WaveState GetState() const { return m_State; }
    float GetStateTimer() const { return m_StateTimer; }

private:
    void CalculateWaveComposition();
    Vector3 CalculateSpawnPosition(const Vector3& playerPos);

    int m_CurrentWave{0};
    int m_TotalEnemiesInWave{0};
    int m_EnemiesSpawned{0};
    int m_EnemiesRemaining{0};
    int m_TotalKills{0};
    int m_Score{0};
    int m_Coins{0};

    WaveState m_State{WaveState::PREPARING};
    float m_StateTimer{2.0f}; // Preparation countdown
    float m_SpawnCooldown{0.0f};
    float m_SpawnInterval{1.4f};
    float m_DifficultyMultiplier{1.0f};

    std::vector<EnemyType> m_SpawnQueue;
};

} // namespace LastSurvivor

#endif // WAVE_MANAGER_H
