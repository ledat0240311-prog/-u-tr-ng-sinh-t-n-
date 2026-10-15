#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <vector>
#include <mutex>

namespace LastSurvivor {

enum class AudioEvent {
    GUNSHOT_PISTOL = 1,
    GUNSHOT_SMG = 2,
    GUNSHOT_RIFLE = 3,
    GUNSHOT_SHOTGUN = 4,
    GUNSHOT_SNIPER = 5,
    RELOAD = 6,
    EMPTY_CLICK = 7,
    PLAYER_HURT = 8,
    PLAYER_FOOTSTEP = 9,
    PLAYER_DEATH = 10,
    ENEMY_ATTACK = 11,
    ENEMY_HURT = 12,
    ENEMY_DEATH = 13,
    LOOT_PICKUP = 14,
    WAVE_START = 15,
    WAVE_CLEARED = 16,
    UPGRADE_PURCHASED = 17
};

class AudioManager {
public:
    static AudioManager& Instance();

    void PostEvent(AudioEvent event);
    int DrainEvents(int* outBuffer, int maxCapacity);

private:
    AudioManager() = default;
    std::mutex m_Mutex;
    std::vector<int> m_PendingEvents;
};

} // namespace LastSurvivor

#endif // AUDIO_MANAGER_H
