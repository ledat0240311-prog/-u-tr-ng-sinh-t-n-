#include "GameLoop.h"
#include "Game.h"
#include "Time.h"

namespace LastSurvivor {

GameLoop& GameLoop::Instance() {
    static GameLoop instance;
    return instance;
}

void GameLoop::Init(int width, int height) {
    m_Accumulator = 0.0f;
    Game::Instance().Init(width, height);
}

void GameLoop::Step() {
    float dt = Time::DeltaTime();
    float fixedDt = Time::FixedDeltaTime();

    m_Accumulator += dt;
    if (m_Accumulator > 0.2f) {
        m_Accumulator = 0.2f; // clamp to prevent death spiral during severe hitches
    }

    // Run fixed physics updates
    while (m_Accumulator >= fixedDt) {
        Game::Instance().Update(fixedDt);
        m_Accumulator -= fixedDt;
    }

    // Render 3D frame
    Game::Instance().Render();
}

} // namespace LastSurvivor
