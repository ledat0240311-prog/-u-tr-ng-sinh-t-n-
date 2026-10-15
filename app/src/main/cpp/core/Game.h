#ifndef GAME_H
#define GAME_H

#include "Player.h"
#include "WeaponManager.h"
#include "EnemyAI.h"
#include "WaveManager.h"
#include "LootManager.h"
#include "Renderer3D.h"
#include "HUD.h"
#include <vector>

namespace LastSurvivor {

struct MapObstacle {
    Vector3 position;
    Vector3 size;
    float yaw;
    float r, g, b;
};

class Game {
public:
    static Game& Instance();

    bool Init(int width, int height);
    void Resize(int width, int height);
    void Update(float dt);
    void Render();

    // Input handlers
    void HandleMove(float inputX, float inputY, bool sprint);
    void HandleLook(float deltaYaw, float deltaPitch);
    void HandleFire(bool pressed);
    void HandleAim(bool aiming);
    void HandleReload();
    void HandleJump();
    void HandleCrouch();
    void HandleSwitchWeapon(int slot);
    bool ApplyUpgrade(int upgradeType);
    void Restart();

    void GetHUDData(HUDData& outData);

private:
    Game();
    ~Game() = default;

    void InitMap();
    void PerformShooting();
    void CheckEnemyCollisionsWithPlayer(float dt);
    void CheckProjectilesWithPlayer();

    Player m_Player;
    WeaponManager m_Weapons;
    EnemyManager m_Enemies;
    WaveManager m_Waves;
    LootManager m_Loot;
    Renderer3D m_Renderer;

    std::vector<MapObstacle> m_MapObstacles;

    // Camera state
    Vector3 m_CameraPos{0.0f, 2.0f, 4.0f};
    Vector3 m_CameraTarget{0.0f, 1.5f, 0.0f};
    float m_CurrentFov{65.0f};
    float m_TargetFov{65.0f};

    bool m_IsFiring{false};
    float m_HitmarkerTimer{0.0f};
    float m_MuzzleFlashTimer{0.0f};
    float m_DamageVignette{0.0f};

    int m_Width{1280};
    int m_Height{720};
    bool m_IsInitialized{false};
};

} // namespace LastSurvivor

#endif // GAME_H
