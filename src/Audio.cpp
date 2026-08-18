#include "Audio.hpp"
#include <iostream>
#include <random>
#include <algorithm>
#include <cmath>

AudioSystem& AudioSystem::instance() {
    static AudioSystem s_audio;
    return s_audio;
}

bool AudioSystem::init() {
    SDL_AudioSpec wanted, have;
    SDL_zero(wanted);
    wanted.freq = 44100;
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2;
    wanted.samples = 1024;
    wanted.callback = audioCallback;
    wanted.userdata = this;

    m_deviceID = SDL_OpenAudioDevice(nullptr, 0, &wanted, &have, 0);
    if (m_deviceID == 0) {
        std::cerr << "SDL_OpenAudioDevice failed: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_PauseAudioDevice(m_deviceID, 0); // Start audio playback
    return true;
}

void AudioSystem::cleanup() {
    if (m_deviceID != 0) {
        SDL_CloseAudioDevice(m_deviceID);
        m_deviceID = 0;
    }
}

void AudioSystem::playSound(SoundEffect effect) {
    std::lock_guard<std::mutex> lock(m_mutex);
    Voice v;
    v.effect = effect;
    v.time = 0.0f;
    v.phase = 0.0f;

    switch (effect) {
        case SoundEffect::LaserFire:
            v.duration = 0.22f;
            v.freq = 880.0f;
            v.volume = 0.45f;
            break;
        case SoundEffect::BiotScreech:
            v.duration = 0.4f;
            v.freq = 1200.0f;
            v.volume = 0.35f;
            break;
        case SoundEffect::BiotDamage:
            v.duration = 0.18f;
            v.freq = 300.0f;
            v.volume = 0.4f;
            break;
        case SoundEffect::BlockBreak:
            v.duration = 0.12f;
            v.freq = 140.0f;
            v.volume = 0.3f;
            break;
        case SoundEffect::BlockPlace:
            v.duration = 0.08f;
            v.freq = 550.0f;
            v.volume = 0.25f;
            break;
        case SoundEffect::PlayerHurt:
            v.duration = 0.3f;
            v.freq = 220.0f;
            v.volume = 0.5f;
            break;
        case SoundEffect::AlarmSiren:
            v.duration = 1.2f;
            v.freq = 440.0f;
            v.volume = 0.4f;
            break;
        case SoundEffect::CraftItem:
            v.duration = 0.25f;
            v.freq = 660.0f;
            v.volume = 0.35f;
            break;
        case SoundEffect::JetpackBurst:
            v.duration = 0.15f;
            v.freq = 100.0f;
            v.volume = 0.25f;
            break;
        case SoundEffect::Explosion:
            v.duration = 1.4f;
            v.freq = 65.0f;
            v.volume = 0.85f;
            break;
        case SoundEffect::GrenadeBounce:
            v.duration = 0.14f;
            v.freq = 1400.0f;
            v.volume = 0.35f;
            break;
    }

    m_voices.push_back(v);
}

void AudioSystem::setJetpackHum(bool active) {
    m_jetpackActive = active;
}

void AudioSystem::setMiningBeamHum(bool active) {
    m_miningBeamActive = active;
}

void AudioSystem::audioCallback(void* userdata, Uint8* stream, int len) {
    AudioSystem* sys = static_cast<AudioSystem*>(userdata);
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int numSamples = len / (sizeof(int16_t) * 2); // stereo

    std::lock_guard<std::mutex> lock(sys->m_mutex);

    float dt = 1.0f / 44100.0f;

    for (int i = 0; i < numSamples; ++i) {
        float mixL = 0.0f;
        float mixR = 0.0f;

        // 1. Ambient Rama Deep Drone (40Hz / 80Hz subtle hum)
        sys->m_ambientPhase += 55.0f * dt * 2.0f * 3.14159f;
        if (sys->m_ambientPhase > 3.14159f * 2.0f) sys->m_ambientPhase -= 3.14159f * 2.0f;
        float ambient = std::sin(sys->m_ambientPhase) * 0.04f + std::sin(sys->m_ambientPhase * 0.5f) * 0.03f;
        mixL += ambient;
        mixR += ambient;

        // 2. Jetpack Thruster Noise
        if (sys->m_jetpackActive) {
            float noise = ((float)(rand() % 2000) / 1000.0f - 1.0f) * 0.14f;
            sys->m_jetpackPhase += 120.0f * dt * 2.0f * 3.14159f;
            float lowRumble = std::sin(sys->m_jetpackPhase) * 0.12f;
            mixL += (noise + lowRumble);
            mixR += (noise + lowRumble);
        }

        // 3. Mining Beam Electrical Buzz
        if (sys->m_miningBeamActive) {
            sys->m_miningBeamPhase += 320.0f * dt * 2.0f * 3.14159f;
            float saw = (std::fmod(sys->m_miningBeamPhase, 3.14159f * 2.0f) / 3.14159f - 1.0f) * 0.12f;
            float pulse = std::sin(sys->m_miningBeamPhase * 2.5f) * 0.08f;
            mixL += saw + pulse;
            mixR += saw + pulse;
        }

        // 4. One-shot sound voices
        for (auto& v : sys->m_voices) {
            float sample = 0.0f;
            float progress = v.time / v.duration;

            switch (v.effect) {
                case SoundEffect::LaserFire: {
                    // Fast pitch envelope drop: 1200Hz down to 200Hz
                    float curFreq = v.freq * (1.0f - progress * 0.8f);
                    v.phase += curFreq * dt * 2.0f * 3.14159f;
                    sample = (std::sin(v.phase) > 0.0f ? 1.0f : -1.0f) * 0.5f + std::sin(v.phase * 2.0f) * 0.5f;
                    sample *= (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::BiotScreech: {
                    // Robot beetle alien metallic screech / frequency modulation
                    float mod = std::sin(v.time * 60.0f) * 400.0f;
                    v.phase += (v.freq + mod) * dt * 2.0f * 3.14159f;
                    float noise = ((float)(rand() % 1000) / 1000.0f - 0.5f) * 0.4f;
                    sample = (std::sin(v.phase) + noise) * (1.0f - progress * 0.5f) * v.volume;
                    break;
                }
                case SoundEffect::BiotDamage: {
                    float noise = ((float)(rand() % 1000) / 1000.0f - 0.5f);
                    v.phase += v.freq * dt * 2.0f * 3.14159f;
                    sample = (std::sin(v.phase) * 0.4f + noise * 0.6f) * (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::BlockBreak: {
                    float noise = ((float)(rand() % 1000) / 1000.0f - 0.5f);
                    sample = noise * (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::BlockPlace: {
                    v.phase += v.freq * dt * 2.0f * 3.14159f;
                    sample = std::sin(v.phase) * (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::PlayerHurt: {
                    v.phase += (v.freq + std::sin(v.time * 30.0f) * 50.0f) * dt * 2.0f * 3.14159f;
                    sample = std::sin(v.phase) * (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::AlarmSiren: {
                    // 2-tone alarm siren
                    float tone = (std::fmod(v.time, 0.4f) < 0.2f) ? 587.33f : 440.0f;
                    v.phase += tone * dt * 2.0f * 3.14159f;
                    sample = std::sin(v.phase) * v.volume * (1.0f - progress * 0.2f);
                    break;
                }
                case SoundEffect::CraftItem: {
                    // Arpeggio chime
                    float chimeFreq = (progress < 0.33f) ? 440.0f : ((progress < 0.66f) ? 554.37f : 659.25f);
                    v.phase += chimeFreq * dt * 2.0f * 3.14159f;
                    sample = std::sin(v.phase) * (1.0f - progress * 0.8f) * v.volume;
                    break;
                }
                case SoundEffect::JetpackBurst: {
                    float noise = ((float)(rand() % 1000) / 1000.0f - 0.5f);
                    sample = noise * (1.0f - progress) * v.volume;
                    break;
                }
                case SoundEffect::Explosion: {
                    float subBass = std::sin(v.phase) * (1.0f - progress);
                    v.phase += (v.freq * (1.0f - progress * 0.7f)) * dt * 2.0f * 3.14159f;
                    float noise = ((float)(rand() % 2000) / 1000.0f - 1.0f);
                    float expDecay = std::exp(-progress * 4.5f);
                    sample = (subBass * 0.5f + noise * expDecay * 0.8f) * v.volume;
                    break;
                }
                case SoundEffect::GrenadeBounce: {
                    v.phase += v.freq * dt * 2.0f * 3.14159f;
                    sample = (std::sin(v.phase) + std::sin(v.phase * 2.41f) * 0.4f) * (1.0f - progress) * v.volume;
                    break;
                }
            }

            mixL += sample;
            mixR += sample;
            v.time += dt;
        }

        // Clamp to 16-bit PCM
        mixL = std::max(-1.0f, std::min(1.0f, mixL));
        mixR = std::max(-1.0f, std::min(1.0f, mixR));

        buffer[i * 2 + 0] = static_cast<int16_t>(mixL * 32760.0f);
        buffer[i * 2 + 1] = static_cast<int16_t>(mixR * 32760.0f);
    }

    // Remove finished voices once per buffer callback (outside sample loop)
    sys->m_voices.erase(
        std::remove_if(sys->m_voices.begin(), sys->m_voices.end(),
                       [](const Voice& v) { return v.time >= v.duration; }),
        sys->m_voices.end()
    );
}
