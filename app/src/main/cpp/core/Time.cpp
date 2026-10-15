#include "Time.h"

namespace LastSurvivor {

std::chrono::steady_clock::time_point Time::s_LastFrameTime;
std::chrono::steady_clock::time_point Time::s_StartTime;
float Time::s_DeltaTime = 0.016f;
float Time::s_TotalTime = 0.0f;
float Time::s_FixedDeltaTime = 0.02f;
uint64_t Time::s_FrameCount = 0;

void Time::Init() {
    s_StartTime = std::chrono::steady_clock::now();
    s_LastFrameTime = s_StartTime;
    s_DeltaTime = 0.016f;
    s_TotalTime = 0.0f;
    s_FrameCount = 0;
}

void Time::Update() {
    auto now = std::chrono::steady_clock::now();
    std::chrono::duration<float> elapsed = now - s_LastFrameTime;
    s_DeltaTime = elapsed.count();
    
    // Clamp delta time to 100ms max to prevent physics explosions upon lag
    if (s_DeltaTime > 0.1f) {
        s_DeltaTime = 0.1f;
    }
    if (s_DeltaTime < 0.0001f) {
        s_DeltaTime = 0.0001f;
    }

    std::chrono::duration<float> total = now - s_StartTime;
    s_TotalTime = total.count();
    s_LastFrameTime = now;
    s_FrameCount++;
}

} // namespace LastSurvivor
