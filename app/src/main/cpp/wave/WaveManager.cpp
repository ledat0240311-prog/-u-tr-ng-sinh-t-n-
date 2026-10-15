#include "WaveManager.h"
#include "AudioManager.h"
#include <cstdlib>
#include <cmath>

namespace LastSurvivor {

WaveManager::WaveManager() {
    Reset();
}

void WaveManager::Reset() {
    m_CurrentWave = 0;
    m_TotalEnemiesInWave = 0;
    m_EnemiesSpawned = 0;
    m_EnemiesRemaining = 0;
    m_TotalKills = 0;
    m_Score = 0;
    m_Coins = 100;
    m_DifficultyMultiplier = 1.0f;
    m_State = WaveState::PREPARING;
    m_StateTimer = 2.0f;
    m_SpawnCooldown = 0.0f;
    m_SpawnQueue.clear();
}

void WaveManager::StartNextWave() {
    m_CurrentWave++;
    m_DifficultyMultiplier = 1.0f + (m_CurrentWave - 1) * 0.15f;
    m_SpawnInterval = std::max(0.4f, 1.5f - (m_CurrentWave * 0.08f));
    CalculateWaveComposition();

    m_EnemiesSpawned = 0;
    m_EnemiesRemaining = static_cast<int>(m_SpawnQueue.size());
    m_TotalEnemiesInWave = m_EnemiesRemaining;
    m_State = WaveState::IN_PROGRESS;
    m_StateTimer = 0.0f;
    m_SpawnCooldown = 0.5f;

    AudioManager::Instance().PostEvent(AudioEvent::WAVE_START);
}

void WaveManager::CalculateWaveComposition() {
    m_SpawnQueue.clear();

    // Wave 1: 5 enemies, Wave 2: 8, Wave 3: 12, etc.
    int count = 5 + (m_CurrentWave - 1) * 3 + (m_CurrentWave > 4 ? (m_CurrentWave - 4) * 2 : 0);
    if (count > 35) count = 35; // Maximum cap per wave

    for (int i = 0; i < count; ++i) {
        if (m_CurrentWave == 1) {
            m_SpawnQueue.push_back(EnemyType::BASIC);
        } else if (m_CurrentWave == 2) {
            if (i % 3 == 0) m_SpawnQueue.push_back(EnemyType::FAST);
            else m_SpawnQueue.push_back(EnemyType::BASIC);
        } else if (m_CurrentWave == 3) {
            if (i % 4 == 0) m_SpawnQueue.push_back(EnemyType::RANGED);
            else if (i % 3 == 0) m_SpawnQueue.push_back(EnemyType::FAST);
            else m_SpawnQueue.push_back(EnemyType::BASIC);
        } else {
            // Wave 4+ introducing Tanks and mixed swarms
            int r = rand() % 10;
            if (r < 4) m_SpawnQueue.push_back(EnemyType::BASIC);
            else if (r < 7) m_SpawnQueue.push_back(EnemyType::FAST);
            else if (r < 9) m_SpawnQueue.push_back(EnemyType::RANGED);
            else m_SpawnQueue.push_back(EnemyType::TANK);
        }
    }
}

Vector3 WaveManager::CalculateSpawnPosition(const Vector3& playerPos) {
    // Spawn around the player at a random angle between 16m and 26m
    float angle = (rand() % 360) * (3.14159265f / 180.0f);
    float dist = 16.0f + (rand() % 10);

    float sx = playerPos.x + std::cos(angle) * dist;
    float sz = playerPos.z + std::sin(angle) * dist;

    // Clamp inside arena bounds
    const float ARENA_LIMIT = 32.0f;
    sx = std::clamp(sx, -ARENA_LIMIT, ARENA_LIMIT);
    sz = std::clamp(sz, -ARENA_LIMIT, ARENA_LIMIT);

    return Vector3(sx, 0.0f, sz);
}

void WaveManager::Update(float dt, EnemyManager& enemyMgr, const Vector3& playerPos) {
    if (m_State == WaveState::PREPARING) {
        m_StateTimer -= dt;
        if (m_StateTimer <= 0.0f) {
            StartNextWave();
        }
        return;
    }

    if (m_State == WaveState::IN_PROGRESS) {
        // Spawn pending enemies from queue
        if (m_EnemiesSpawned < static_cast<int>(m_SpawnQueue.size())) {
            m_SpawnCooldown -= dt;
            if (m_SpawnCooldown <= 0.0f && enemyMgr.GetActiveEnemyCount() < 24) {
                EnemyType type = m_SpawnQueue[m_EnemiesSpawned];
                Vector3 spawnPos = CalculateSpawnPosition(playerPos);
                if (enemyMgr.SpawnEnemy(type, spawnPos, m_DifficultyMultiplier)) {
                    m_EnemiesSpawned++;
                    m_SpawnCooldown = m_SpawnInterval;
                }
            }
        }

        // Check if wave is completed
        if (m_EnemiesSpawned >= static_cast<int>(m_SpawnQueue.size()) && enemyMgr.GetActiveEnemyCount() == 0) {
            m_State = WaveState::WAVE_CLEARED;
            m_StateTimer = 4.0f; // Intermission window
            int waveBonus = m_CurrentWave * 50;
            m_Score += waveBonus;
            m_Coins += 50 + m_CurrentWave * 20;
            AudioManager::Instance().PostEvent(AudioEvent::WAVE_CLEARED);
        }
        return;
    }

    if (m_State == WaveState::WAVE_CLEARED) {
        m_StateTimer -= dt;
        if (m_StateTimer <= 0.0f) {
            m_State = WaveState::PREPARING;
            m_StateTimer = 1.5f;
        }
    }
}

void WaveManager::OnEnemyKilled(EnemyType type) {
    m_TotalKills++;
    if (m_EnemiesRemaining > 0) {
        m_EnemiesRemaining--;
    }

    int baseScore = 50;
    int coins = 10;
    switch (type) {
        case EnemyType::BASIC: baseScore = 50; coins = 10; break;
        case EnemyType::FAST: baseScore = 75; coins = 15; break;
        case EnemyType::RANGED: baseScore = 100; coins = 20; break;
        case EnemyType::TANK: baseScore = 200; coins = 40; break;
    }

    m_Score += static_cast<int>(baseScore * m_DifficultyMultiplier);
    m_Coins += coins;
}

bool WaveManager::SpendCoins(int amount) {
    if (m_Coins >= amount) {
        m_Coins -= amount;
        return true;
    }
    return false;
}

} // namespace LastSurvivor
