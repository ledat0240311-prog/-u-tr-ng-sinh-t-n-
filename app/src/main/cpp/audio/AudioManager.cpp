#include "AudioManager.h"

namespace LastSurvivor {

AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}

void AudioManager::PostEvent(AudioEvent event) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_PendingEvents.size() < 64) {
        m_PendingEvents.push_back(static_cast<int>(event));
    }
}

int AudioManager::DrainEvents(int* outBuffer, int maxCapacity) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    int count = static_cast<int>(m_PendingEvents.size());
    if (count > maxCapacity) {
        count = maxCapacity;
    }
    for (int i = 0; i < count; ++i) {
        outBuffer[i] = m_PendingEvents[i];
    }
    m_PendingEvents.erase(m_PendingEvents.begin(), m_PendingEvents.begin() + count);
    return count;
}

} // namespace LastSurvivor
