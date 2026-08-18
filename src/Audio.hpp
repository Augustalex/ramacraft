#pragma once

#include <SDL.h>
#include <vector>
#include <mutex>
#include <cmath>

enum class SoundEffect {
    LaserFire,
    BiotScreech,
    BiotDamage,
    BlockBreak,
    BlockPlace,
    PlayerHurt,
    AlarmSiren,
    CraftItem,
    JetpackBurst
};

class AudioSystem {
public:
    static AudioSystem& instance();

    bool init();
    void cleanup();

    void playSound(SoundEffect effect);
    void setJetpackHum(bool active);
    void setMiningBeamHum(bool active);

private:
    AudioSystem() = default;
    static void audioCallback(void* userdata, Uint8* stream, int len);

    SDL_AudioDeviceID m_deviceID = 0;
    std::mutex m_mutex;

    struct Voice {
        SoundEffect effect;
        float phase = 0.0f;
        float phaseInc = 0.0f;
        float duration = 0.0f;
        float time = 0.0f;
        float volume = 0.5f;
        float freq = 440.0f;
    };

    std::vector<Voice> m_voices;
    bool m_jetpackActive = false;
    float m_jetpackPhase = 0.0f;
    bool m_miningBeamActive = false;
    float m_miningBeamPhase = 0.0f;
    float m_ambientPhase = 0.0f;
};
