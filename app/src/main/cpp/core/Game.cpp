#include "Game.h"
#include "Time.h"
#include "AudioManager.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace LastSurvivor {

constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

Game& Game::Instance() {
    static Game instance;
    return instance;
}

Game::Game() {
    InitMap();
}

void Game::InitMap() {
    m_MapObstacles.clear();

    // Shipping containers (red, blue, green, rust)
    m_MapObstacles.push_back({Vector3(-10.0f, 1.3f, -8.0f), Vector3(2.5f, 2.6f, 6.0f), 15.0f, 0.65f, 0.20f, 0.18f});
    m_MapObstacles.push_back({Vector3(-10.0f, 3.9f, -8.0f), Vector3(2.5f, 2.6f, 6.0f), 15.0f, 0.20f, 0.40f, 0.65f});
    m_MapObstacles.push_back({Vector3(12.0f, 1.3f, -12.0f), Vector3(6.0f, 2.6f, 2.5f), 0.0f, 0.22f, 0.48f, 0.30f});
    m_MapObstacles.push_back({Vector3(14.0f, 1.3f, 10.0f), Vector3(2.5f, 2.6f, 6.0f), -30.0f, 0.70f, 0.35f, 0.15f});

    // Abandoned vehicles (cars / truck chassis)
    m_MapObstacles.push_back({Vector3(0.0f, 0.75f, 12.0f), Vector3(2.2f, 1.5f, 4.2f), 45.0f, 0.32f, 0.34f, 0.38f});
    m_MapObstacles.push_back({Vector3(-16.0f, 0.75f, 6.0f), Vector3(2.0f, 1.4f, 4.0f), -15.0f, 0.50f, 0.18f, 0.18f});

    // Concrete barricades and crates
    m_MapObstacles.push_back({Vector3(-4.0f, 0.6f, -3.0f), Vector3(3.0f, 1.2f, 0.8f), 0.0f, 0.45f, 0.45f, 0.48f});
    m_MapObstacles.push_back({Vector3(5.0f, 0.6f, 4.0f), Vector3(3.0f, 1.2f, 0.8f), 90.0f, 0.45f, 0.45f, 0.48f});
    m_MapObstacles.push_back({Vector3(6.0f, 0.5f, -6.0f), Vector3(1.2f, 1.0f, 1.2f), 10.0f, 0.55f, 0.40f, 0.25f});
    m_MapObstacles.push_back({Vector3(7.2f, 0.5f, -6.2f), Vector3(1.2f, 1.0f, 1.2f), 25.0f, 0.55f, 0.40f, 0.25f});
    m_MapObstacles.push_back({Vector3(-8.0f, 0.5f, 15.0f), Vector3(1.4f, 1.0f, 1.4f), 0.0f, 0.55f, 0.40f, 0.25f});

    // Warehouse structural pillars / walls around boundary
    for (float x = -32.0f; x <= 32.0f; x += 16.0f) {
        m_MapObstacles.push_back({Vector3(x, 2.5f, -34.0f), Vector3(2.0f, 5.0f, 2.0f), 0.0f, 0.30f, 0.32f, 0.35f});
        m_MapObstacles.push_back({Vector3(x, 2.5f,  34.0f), Vector3(2.0f, 5.0f, 2.0f), 0.0f, 0.30f, 0.32f, 0.35f});
    }
    for (float z = -24.0f; z <= 24.0f; z += 16.0f) {
        m_MapObstacles.push_back({Vector3(-34.0f, 2.5f, z), Vector3(2.0f, 5.0f, 2.0f), 0.0f, 0.30f, 0.32f, 0.35f});
        m_MapObstacles.push_back({Vector3( 34.0f, 2.5f, z), Vector3(2.0f, 5.0f, 2.0f), 0.0f, 0.30f, 0.32f, 0.35f});
    }
}

bool Game::Init(int width, int height) {
    m_Width = width;
    m_Height = height;
    Time::Init();

    if (!m_Renderer.Init()) {
        return false;
    }
    m_Renderer.Resize(width, height);

    Restart();
    m_IsInitialized = true;
    return true;
}

void Game::Resize(int width, int height) {
    m_Width = width;
    m_Height = height;
    m_Renderer.Resize(width, height);
}

void Game::Restart() {
    m_Player.Reset();
    m_Weapons.Reset();
    m_Enemies.Reset();
    m_Waves.Reset();
    m_Loot.Reset();

    m_IsFiring = false;
    m_HitmarkerTimer = 0.0f;
    m_MuzzleFlashTimer = 0.0f;
    m_DamageVignette = 0.0f;

    m_CameraPos = Vector3(0.0f, 2.0f, 4.0f);
    m_CameraTarget = Vector3(0.0f, 1.5f, 0.0f);
    m_CurrentFov = 65.0f;
    m_TargetFov = 65.0f;
}

void Game::HandleMove(float inputX, float inputY, bool sprint) {
    m_Player.Move(inputX, inputY, sprint);
}

void Game::HandleLook(float deltaYaw, float deltaPitch) {
    // Dampen rotation sensitivity when aiming
    float sensitivity = m_Player.IsAiming() ? 0.55f : 1.0f;
    m_Player.RotateView(deltaYaw * sensitivity, deltaPitch * sensitivity);
}

void Game::HandleFire(bool pressed) {
    m_IsFiring = pressed;
}

void Game::HandleAim(bool aiming) {
    m_Player.SetAiming(aiming);
    m_TargetFov = aiming ? 42.0f : 65.0f;
}

void Game::HandleReload() {
    m_Weapons.Reload();
}

void Game::HandleJump() {
    m_Player.Jump();
}

void Game::HandleCrouch() {
    m_Player.SetCrouch(!m_Player.IsCrouching());
}

void Game::HandleSwitchWeapon(int slot) {
    m_Weapons.SwitchWeapon(slot);
}

bool Game::ApplyUpgrade(int upgradeType) {
    int cost = 100;
    if (upgradeType == 1) cost = 120; // Damage
    else if (upgradeType == 2) cost = 80;  // Max HP
    else if (upgradeType == 3) cost = 80;  // Armor
    else if (upgradeType == 4) cost = 100; // Speed
    else if (upgradeType == 5) cost = 90;  // Reload speed
    else if (upgradeType == 6) cost = 110; // Mag size
    else if (upgradeType == 7) cost = 130; // Crit chance

    if (!m_Waves.SpendCoins(cost)) {
        return false;
    }

    switch (upgradeType) {
        case 1: m_Weapons.UpgradeAllDamage(1.20f); break;
        case 2: m_Player.UpgradeMaxHp(25.0f); break;
        case 3: m_Player.UpgradeMaxArmor(25.0f); break;
        case 4: m_Player.UpgradeSpeed(1.12f); break;
        case 5: m_Weapons.UpgradeAllReloadSpeed(0.80f); break;
        case 6: m_Weapons.UpgradeAllMagSize(5); break;
        case 7: m_Weapons.UpgradeAllCritChance(0.08f); break;
    }

    AudioManager::Instance().PostEvent(AudioEvent::UPGRADE_PURCHASED);
    return true;
}

void Game::PerformShooting() {
    Weapon* active = m_Weapons.GetActiveWeapon();
    if (!active || !active->CanShoot()) return;

    if (!m_Weapons.Shoot()) return;

    m_MuzzleFlashTimer = 0.08f;

    // Apply recoil to view
    float recoilKick = active->GetRecoil();
    if (m_Player.IsAiming()) recoilKick *= 0.5f;
    m_Player.RotateView(((rand() % 100) - 50) * 0.01f * recoilKick, recoilKick * 0.8f);

    Vector3 rayOrigin = m_Player.GetEyePosition();
    Vector3 rayDir = m_Player.GetForwardVector();

    // Check hit against active enemies
    int pelletCount = (active->GetType() == WeaponType::SHOTGUN) ? 8 : 1;

    for (int p = 0; p < pelletCount; ++p) {
        Vector3 shotDir = rayDir;
        if (pelletCount > 1 || active->GetAccuracy() < 0.95f) {
            float spread = (1.0f - active->GetAccuracy()) * 0.12f;
            if (m_Player.IsAiming()) spread *= 0.4f;
            float rx = ((rand() % 200) - 100) * 0.01f * spread;
            float ry = ((rand() % 200) - 100) * 0.01f * spread;
            shotDir.x += rx;
            shotDir.y += ry;
            shotDir = shotDir.Normalized();
        }

        Enemy* closestHitEnemy = nullptr;
        float closestDist = active->GetRange();
        bool isHeadshot = false;

        auto& enemies = m_Enemies.GetAllEnemies();
        for (auto& enemy : enemies) {
            if (!enemy.IsActive() || enemy.GetState() == AIState::DEAD) continue;

            Vector3 ePos = enemy.GetPosition();
            float radius = enemy.GetBoundingRadius();
            float height = enemy.GetHeight();

            // Cylinder / Ray intersection approximation
            Vector3 toEnemy = ePos - rayOrigin;
            float projDist = Vector3::Dot(toEnemy, shotDir);

            if (projDist > 0.0f && projDist < closestDist) {
                Vector3 closestPt = rayOrigin + shotDir * projDist;
                float dx = closestPt.x - ePos.x;
                float dz = closestPt.z - ePos.z;
                float horizDistSq = dx * dx + dz * dz;

                if (horizDistSq <= (radius * radius) && closestPt.y >= ePos.y && closestPt.y <= (ePos.y + height)) {
                    closestDist = projDist;
                    closestHitEnemy = &enemy;
                    isHeadshot = (closestPt.y >= (ePos.y + height * 0.75f));
                }
            }
        }

        if (closestHitEnemy) {
            float damage = active->GetDamage();
            if (isHeadshot) damage *= 1.8f;

            bool isCrit = ((rand() % 100) < (active->GetCritChance() * 100.0f));
            if (isCrit) damage *= 1.5f;

            bool killed = closestHitEnemy->TakeDamage(damage, isCrit);
            m_HitmarkerTimer = 0.2f;

            if (killed) {
                m_Waves.OnEnemyKilled(closestHitEnemy->GetType());
                m_Loot.SpawnDrop(closestHitEnemy->GetPosition(), closestHitEnemy->GetType());
            }
        }
    }
}

void Game::CheckEnemyCollisionsWithPlayer(float dt) {
    if (m_Player.IsDead()) return;

    Vector3 pPos = m_Player.GetPosition();
    auto& enemies = m_Enemies.GetAllEnemies();

    for (auto& enemy : enemies) {
        if (!enemy.IsActive() || enemy.GetState() == AIState::DEAD) continue;

        if (enemy.DidAttackThisFrame()) {
            float dist = Vector3::Distance(enemy.GetPosition(), pPos);
            if (dist <= 2.2f) {
                m_Player.TakeDamage(enemy.GetAttackDamage());
                m_DamageVignette = 1.0f;
            }
        }
    }
}

void Game::CheckProjectilesWithPlayer() {
    if (m_Player.IsDead()) return;

    Vector3 pEye = m_Player.GetEyePosition();
    auto& projs = m_Enemies.GetProjectiles();

    for (auto& p : projs) {
        if (p.active) {
            float distSq = Vector3::DistanceSq(p.position, pEye);
            if (distSq < 1.2f) {
                m_Player.TakeDamage(p.damage);
                m_DamageVignette = 1.0f;
                p.active = false;
            }
        }
    }
}

void Game::Update(float dt) {
    Time::Update();

    // Weapon automatic fire or cooldown update
    m_Weapons.Update(dt);
    if (m_IsFiring && !m_Player.IsDead()) {
        PerformShooting();
    }

    m_Player.Update(dt);
    m_Enemies.Update(dt, m_Player.GetPosition(), m_Player.IsDead());
    m_Waves.Update(dt, m_Enemies, m_Player.GetPosition());
    m_Loot.Update(dt, m_Player.GetPosition(), m_Player, m_Weapons);

    CheckEnemyCollisionsWithPlayer(dt);
    CheckProjectilesWithPlayer();

    // Update timers
    if (m_HitmarkerTimer > 0.0f) m_HitmarkerTimer -= dt;
    if (m_MuzzleFlashTimer > 0.0f) m_MuzzleFlashTimer -= dt;
    if (m_DamageVignette > 0.0f) m_DamageVignette -= dt * 2.0f;

    // Smooth FOV zoom during ADS
    m_CurrentFov += (m_TargetFov - m_CurrentFov) * (12.0f * dt);

    // Third-person camera positioning
    float yawRad = m_Player.GetYaw() * DEG2RAD;
    float pitchRad = m_Player.GetPitch() * DEG2RAD;

    // Over-the-shoulder offset
    float camDist = m_Player.IsAiming() ? 1.6f : 2.8f;
    float shoulderOffset = 0.55f;

    float fwdX = -std::sin(yawRad) * std::cos(pitchRad);
    float fwdY = std::sin(pitchRad);
    float fwdZ = -std::cos(yawRad) * std::cos(pitchRad);

    float rightX = std::cos(yawRad);
    float rightZ = -std::sin(yawRad);

    Vector3 eye = m_Player.GetEyePosition();
    Vector3 targetCamPos = eye - Vector3(fwdX, fwdY, fwdZ) * camDist + Vector3(rightX, 0.0f, rightZ) * shoulderOffset;
    targetCamPos.y = std::max(0.4f, targetCamPos.y);

    // Smooth camera damping
    m_CameraPos += (targetCamPos - m_CameraPos) * (18.0f * dt);
    m_CameraTarget = eye + Vector3(fwdX, fwdY, fwdZ) * 20.0f;
}

void Game::Render() {
    float aspect = (m_Height > 0) ? (static_cast<float>(m_Width) / static_cast<float>(m_Height)) : 1.777f;
    Mat4 proj = Mat4::Perspective(m_CurrentFov, aspect, 0.1f, 150.0f);
    Mat4 view = Mat4::LookAt(m_CameraPos, m_CameraTarget, Vector3(0.0f, 1.0f, 0.0f));

    m_Renderer.BeginFrame(view, proj);

    // Draw Ground
    m_Renderer.DrawGround(40.0f);

    // Draw Obstacles (Containers, Walls, Cars, Crates)
    for (const auto& obs : m_MapObstacles) {
        m_Renderer.DrawCube(obs.position, obs.size, obs.yaw, obs.r, obs.g, obs.b);
    }

    // Draw Player Body (Third-Person tactical model)
    if (!m_Player.IsDead()) {
        Vector3 pPos = m_Player.GetPosition();
        float yaw = m_Player.GetYaw();

        // Legs/Pants (Dark tactical slate)
        m_Renderer.DrawCube(Vector3(pPos.x, 0.45f, pPos.z), Vector3(0.5f, 0.9f, 0.35f), yaw, 0.18f, 0.20f, 0.24f);
        // Torso / Tactical Vest (Military Green / Coyote)
        m_Renderer.DrawCube(Vector3(pPos.x, 1.15f, pPos.z), Vector3(0.6f, 0.7f, 0.4f), yaw, 0.28f, 0.36f, 0.24f);
        // Head / Combat Helmet (Graphite with orange visor stripe)
        m_Renderer.DrawCube(Vector3(pPos.x, 1.65f, pPos.z), Vector3(0.35f, 0.35f, 0.35f), yaw, 0.22f, 0.24f, 0.28f);

        // Weapon in hands
        Vector3 right = m_Player.GetRightVector();
        Vector3 fwd = m_Player.GetForwardVector();
        Vector3 gunPos = pPos + Vector3(0.0f, 1.2f, 0.0f) + right * 0.3f + fwd * 0.4f;
        m_Renderer.DrawCube(gunPos, Vector3(0.12f, 0.15f, 0.6f), yaw, 0.15f, 0.15f, 0.15f);

        // Muzzle flash
        if (m_MuzzleFlashTimer > 0.0f) {
            Vector3 flashPos = gunPos + fwd * 0.45f;
            m_Renderer.DrawCube(flashPos, Vector3(0.25f, 0.25f, 0.25f), yaw, 1.0f, 0.85f, 0.2f);
        }
    }

    // Draw Enemies
    auto& enemies = m_Enemies.GetAllEnemies();
    for (const auto& enemy : enemies) {
        if (!enemy.IsActive()) continue;

        Vector3 ePos = enemy.GetPosition();
        float yaw = enemy.GetYaw();

        float r = 0.8f, g = 0.2f, b = 0.2f; // Red for basic
        Vector3 bodyScale(0.6f, 1.8f, 0.5f);

        switch (enemy.GetType()) {
            case EnemyType::BASIC:
                r = 0.85f; g = 0.25f; b = 0.25f;
                bodyScale = Vector3(0.65f, 1.8f, 0.55f);
                break;
            case EnemyType::FAST:
                r = 0.95f; g = 0.55f; b = 0.15f; // Agile orange
                bodyScale = Vector3(0.45f, 1.5f, 0.45f);
                break;
            case EnemyType::RANGED:
                r = 0.75f; g = 0.20f; b = 0.85f; // Purple shooter
                bodyScale = Vector3(0.55f, 1.7f, 0.50f);
                break;
            case EnemyType::TANK:
                r = 0.35f; g = 0.10f; b = 0.10f; // Dark crimson brute
                bodyScale = Vector3(1.1f, 2.4f, 1.0f);
                break;
        }

        if (enemy.GetState() == AIState::DAMAGE) {
            r = 1.0f; g = 1.0f; b = 1.0f; // White flash on damage
        } else if (enemy.GetState() == AIState::DEAD) {
            // Fallen enemy on ground
            m_Renderer.DrawCube(Vector3(ePos.x, 0.2f, ePos.z), Vector3(bodyScale.z, 0.3f, bodyScale.y), yaw, 0.25f, 0.25f, 0.25f);
            continue;
        }

        m_Renderer.DrawCube(Vector3(ePos.x, bodyScale.y * 0.5f, ePos.z), bodyScale, yaw, r, g, b);
    }

    // Draw Enemy Projectiles
    auto& projs = m_Enemies.GetProjectiles();
    for (const auto& p : projs) {
        if (p.active) {
            m_Renderer.DrawCube(p.position, Vector3(0.2f, 0.2f, 0.2f), 0.0f, 0.9f, 0.1f, 0.9f);
        }
    }

    // Draw Loot Items
    const auto& lootItems = m_Loot.GetItems();
    for (const auto& item : lootItems) {
        if (item.active) {
            float r = 1.0f, g = 1.0f, b = 1.0f;
            Vector3 size(0.45f, 0.45f, 0.45f);

            switch (item.type) {
                case LootType::AMMO:
                    r = 0.2f; g = 0.8f; b = 0.3f; // Green ammo
                    break;
                case LootType::HEALTH:
                    r = 0.9f; g = 0.15f; b = 0.15f; // Red medkit
                    break;
                case LootType::ARMOR:
                    r = 0.2f; g = 0.6f; b = 0.95f; // Blue armor
                    break;
                case LootType::COINS:
                    r = 0.95f; g = 0.85f; b = 0.2f; // Gold coins
                    break;
            }

            m_Renderer.DrawCube(item.position, size, item.rotationYaw, r, g, b);
        }
    }

    m_Renderer.EndFrame();
}

void Game::GetHUDData(HUDData& outData) {
    HUD::PackHUDData(m_Player, m_Weapons, m_Waves, m_HitmarkerTimer, m_DamageVignette, outData);
}

} // namespace LastSurvivor
