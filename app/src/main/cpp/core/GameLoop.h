#ifndef GAMELOOP_H
#define GAMELOOP_H

namespace LastSurvivor {

class GameLoop {
public:
    static GameLoop& Instance();

    void Init(int width, int height);
    void Step();

private:
    GameLoop() = default;

    float m_Accumulator{0.0f};
    bool m_IsRunning{true};
};

} // namespace LastSurvivor

#endif // GAMELOOP_H
