#include "Player.h"
#include "AudioManager.h"
#include <algorithm>
#include <cmath>

namespace LastSurvivor {

constexpr float DEG2RAD = 3.14159265358979323846f / 180.0f;

Player::Player() {
    Reset();
}

void Player::Reset() {
    m_Position = Vector3(0.0f, 0.0f, 0.0f);
    m_Velocity = Vector3(0.0f, 0.0f, 0.0f);
    m_Yaw = 0.0f;
    m_Pitch = 0.0f;

    m_Hp = m_MaxHp;
    m_Armor = m_MaxArmor;
    m_Stamina = m_MaxStamina;

    m_IsAiming = false;
    m_IsSprinting = false;
    m_IsCrouching = false;
    m_IsGrounded = true;
    m_IsDead = false;

    m_RegenTimer = 0.0f;
    m_HitTimer = 0.0f;
    m_FootstepTimer = 0.0f;
}

Vector3 Player::GetForwardVector() const {
    float yawRad = m_Yaw * DEG2RAD;
    float pitchRad = m_Pitch * DEG2RAD;

    float fx = -std::sin(yawRad) * std::cos(pitchRad);
    float fy = std::sin(pitchRad);
    float fz = -std::cos(yawRad) * std::cos(pitchRad);

    return Vector3(fx, fy, fz).Normalized();
}

Vector3 Player::GetRightVector() const {
    float yawRad = m_Yaw * DEG2RAD;
    float rx = std::cos(yawRad);
    float ry = 0.0f;
    float rz = -std::sin(yawRad);
    return Vector3(rx, ry, rz).Normalized();
}

Vector3 Player::GetEyePosition() const {
    float height = m_IsCrouching ? 0.9f : 1.7f;
    return Vector3(m_Position.x, m_Position.y + height, m_Position.z);
}

void Player::Move(float inputX, float inputY, bool sprint) {
    if (m_IsDead) return;

    m_IsSprinting = sprint && (m_Stamina > 5.0f) && !m_IsCrouching && !m_IsAiming;

    float speed = m_BaseSpeed * m_SpeedMultiplier;
    if (m_IsSprinting) {
        speed *= m_SprintMultiplier;
    } else if (m_IsCrouching) {
        speed *= m_CrouchMultiplier;
    } else if (m_IsAiming) {
        speed *= 0.6f;
    }

    float yawRad = m_Yaw * DEG2RAD;
    float fwdX = -std::sin(yawRad);
    float fwdZ = -std::cos(yawRad);
    float rightX = std::cos(yawRad);
    float rightZ = -std::sin(yawRad);

    // inputX = right/left, inputY = forward/backward
    float moveX = (rightX * inputX + fwdX * inputY);
    float moveZ = (rightZ * inputX + fwdZ * inputY);

    float len = std::sqrt(moveX * moveX + moveZ * moveZ);
    if (len > 0.01f) {
        moveX = (moveX / len) * speed;
        moveZ = (moveZ / len) * speed;
    } else {
        moveX = 0.0f;
        moveZ = 0.0f;
    }

    m_Velocity.x = moveX;
    m_Velocity.z = moveZ;
}

void Player::RotateView(float deltaYaw, float deltaPitch) {
    if (m_IsDead) return;

    m_Yaw += deltaYaw;
    while (m_Yaw < 0.0f) m_Yaw += 360.0f;
    while (m_Yaw >= 360.0f) m_Yaw -= 360.0f;

    m_Pitch += deltaPitch;
    if (m_Pitch > 80.0f) m_Pitch = 80.0f;
    if (m_Pitch < -80.0f) m_Pitch = -80.0f;
}

void Player::Jump() {
    if (m_IsDead || !m_IsGrounded) return;
    m_Velocity.y = 5.2f;
    m_IsGrounded = false;
}

void Player::SetCrouch(bool crouch) {
    if (m_IsDead) return;
    m_IsCrouching = crouch;
}

void Player::SetAiming(bool aiming) {
    if (m_IsDead) return;
    m_IsAiming = aiming;
}

void Player::Update(float dt) {
    if (m_IsDead) return;

    // Apply gravity
    if (!m_IsGrounded) {
        m_Velocity.y -= 14.0f * dt;
    }

    // Integrate position
    m_Position.x += m_Velocity.x * dt;
    m_Position.y += m_Velocity.y * dt;
    m_Position.z += m_Velocity.z * dt;

    // Floor collision
    if (m_Position.y <= 0.0f) {
        m_Position.y = 0.0f;
        m_Velocity.y = 0.0f;
        m_IsGrounded = true;
    }

    // Map boundaries (-35 to +35 meters)
    const float MAP_BOUND = 34.0f;
    m_Position.x = std::clamp(m_Position.x, -MAP_BOUND, MAP_BOUND);
    m_Position.z = std::clamp(m_Position.z, -MAP_BOUND, MAP_BOUND);

    // Stamina drain / recovery
    float horizSpeedSq = m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z;
    if (m_IsSprinting && horizSpeedSq > 0.5f) {
        m_Stamina -= 22.0f * dt;
        if (m_Stamina < 0.0f) {
            m_Stamina = 0.0f;
            m_IsSprinting = false;
        }
    } else {
        m_Stamina += 15.0f * dt;
        if (m_Stamina > m_MaxStamina) {
            m_Stamina = m_MaxStamina;
        }
    }

    // Footsteps sound
    if (m_IsGrounded && horizSpeedSq > 0.5f) {
        float stepInterval = m_IsSprinting ? 0.3f : 0.5f;
        m_FootstepTimer += dt;
        if (m_FootstepTimer >= stepInterval) {
            m_FootstepTimer = 0.0f;
            AudioManager::Instance().PostEvent(AudioEvent::PLAYER_FOOTSTEP);
        }
    } else {
        m_FootstepTimer = 0.0f;
    }

    // Passive slow regen when not hit for 6 seconds
    if (m_HitTimer > 0.0f) {
        m_HitTimer -= dt;
    } else {
        if (m_Hp < m_MaxHp) {
            m_Hp += 3.0f * dt;
            if (m_Hp > m_MaxHp) m_Hp = m_MaxHp;
        }
    }
}

bool Player::TakeDamage(float amount) {
    if (m_IsDead) return false;

    m_HitTimer = 1.0f;

    // Armor absorbs 65% of damage first
    if (m_Armor > 0.0f) {
        float absorbed = amount * 0.65f;
        float direct = amount - absorbed;

        if (m_Armor >= absorbed) {
            m_Armor -= absorbed;
            m_Hp -= direct;
        } else {
            float overflow = absorbed - m_Armor;
            m_Armor = 0.0f;
            m_Hp -= (direct + overflow);
        }
    } else {
        m_Hp -= amount;
    }

    if (m_Hp <= 0.0f) {
        m_Hp = 0.0f;
        m_IsDead = true;
        AudioManager::Instance().PostEvent(AudioEvent::PLAYER_DEATH);
        return true;
    }

    AudioManager::Instance().PostEvent(AudioEvent::PLAYER_HURT);
    return false;
}

void Player::Heal(float amount) {
    if (m_IsDead) return;
    m_Hp = std::min(m_MaxHp, m_Hp + amount);
}

void Player::AddArmor(float amount) {
    if (m_IsDead) return;
    m_Armor = std::min(m_MaxArmor, m_Armor + amount);
}

void Player::UpgradeMaxHp(float amount) {
    m_MaxHp += amount;
    m_Hp += amount;
}

void Player::UpgradeMaxArmor(float amount) {
    m_MaxArmor += amount;
    m_Armor += amount;
}

void Player::UpgradeSpeed(float multiplier) {
    m_SpeedMultiplier *= multiplier;
}

} // namespace LastSurvivor
