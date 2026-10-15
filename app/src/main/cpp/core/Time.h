#ifndef TIME_H
#define TIME_H

#include <chrono>
#include <cstdint>

namespace LastSurvivor {

class Time {
public:
    static void Init();
    static void Update();

    static float DeltaTime() { return s_DeltaTime; }
    static float TotalTime() { return s_TotalTime; }
    static float FixedDeltaTime() { return s_FixedDeltaTime; }
    static uint64_t FrameCount() { return s_FrameCount; }

private:
    static std::chrono::steady_clock::time_point s_LastFrameTime;
    static std::chrono::steady_clock::time_point s_StartTime;
    static float s_DeltaTime;
    static float s_TotalTime;
    static float s_FixedDeltaTime;
    static uint64_t s_FrameCount;
};

} // namespace LastSurvivor

#endif // TIME_H
