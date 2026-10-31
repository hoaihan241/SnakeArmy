#include "AudioManager.h"
#include "ResourceManager.h"
#include "ConfigManager.h"
#include <iostream>

AudioManager::AudioManager()
    : nextSoundIdx(0),
      wasVoicePlaying(false)
{
    soundPool.resize(SOUND_POOL_SIZE);
}

std::string AudioManager::normalizeName(const std::string& name) {
    if (name.size() >= 4 && name.substr(name.size() - 4) == ".ogg") {
        return name;
    }
    return name + ".ogg";
}

void AudioManager::init() {
    std::cout << "[AudioManager] Initializing sound and voice buffers..." << std::endl;

    // 1. Danh sách toàn bộ SFX
    const std::string sfxFiles[] = {
        "sfx_bank.ogg",
        "sfx_bomb_siren.ogg",
        "sfx_bush.ogg",
        "sfx_clash_hit.ogg",
        "sfx_click.ogg",
        "sfx_dragon_fire.ogg",
        "sfx_dragon_roar.ogg",
        "sfx_explosion.ogg",
        "sfx_hover.ogg",
        "sfx_item_magnet.ogg",
        "sfx_item_shield.ogg",
        "sfx_item_speed.ogg",
        "sfx_pause.ogg",
        "sfx_recruit.ogg",
        "sfx_steal.ogg",
        "sfx_sword_slash.ogg",
        "sfx_troop_death.ogg"
    };

    for (const auto& file : sfxFiles) {
        sf::SoundBuffer buffer;
        std::string fullPath = ResourceManager::getInstance().getFullPath("assets/audio/sfx/" + file);
        if (buffer.loadFromFile(fullPath)) {
            sfxBuffers[file] = buffer;
            std::cout << "[AudioManager] Loaded SFX: " << file << std::endl;
        } else {
            std::cerr << "[AudioManager] FAILED to load SFX: " << fullPath << std::endl;
        }
    }

    // 2. Danh sách toàn bộ Lồng tiếng Voice Lines
    const std::string voiceFiles[] = {
        "pve_intro.ogg",
        "pve_drop.ogg",
        "pve_clash.ogg",
        "pve_win.ogg",
        "pve_lose.ogg",
        "pve_draw.ogg",
        "pvp_intro.ogg",
        "pvp_drop.ogg",
        "pvp_clash.ogg",
        "pvp_win_p1.ogg",
        "pvp_win_p2.ogg",
        "pvp_draw.ogg",
        "vo_dragon.ogg"
    };

    for (const auto& file : voiceFiles) {
        sf::SoundBuffer buffer;
        std::string fullPath = ResourceManager::getInstance().getFullPath("assets/audio/voices/" + file);
        if (buffer.loadFromFile(fullPath)) {
            voiceBuffers[file] = buffer;
            std::cout << "[AudioManager] Loaded Voice: " << file << std::endl;
        } else {
            std::cerr << "[AudioManager] FAILED to load Voice: " << fullPath << std::endl;
        }
    }

    std::cout << "[AudioManager] Initialized successfully. Total SFX: " 
              << sfxBuffers.size() << ", Total Voices: " << voiceBuffers.size() << std::endl;
}

// ================================================================
//  BGM Controls
// ================================================================
void AudioManager::playBGM(const std::string& name, bool loop) {
    std::string key = normalizeName(name);

    // Nếu bài nhạc này đang phát rồi thì không ngắt
    if (currentBgmName == key && currentBgm.getStatus() == sf::Music::Playing) {
        return;
    }

    std::string fullPath = ResourceManager::getInstance().getFullPath("assets/audio/bgm/" + key);
    currentBgm.stop();

    if (currentBgm.openFromFile(fullPath)) {
        currentBgmName = key;
        currentBgm.setLoop(loop);
        applyMusicVolume();
        currentBgm.play();
        std::cout << "[AudioManager] Playing BGM: " << key << (loop ? " (Loop)" : "") << std::endl;
    } else {
        std::cerr << "[AudioManager] FAILED to open BGM stream: " << fullPath << std::endl;
        currentBgmName = "";
    }
}

void AudioManager::stopBGM() {
    currentBgm.stop();
    currentBgmName = "";
}

void AudioManager::pauseBGM() {
    if (currentBgm.getStatus() == sf::Music::Playing) {
        currentBgm.pause();
    }
}

void AudioManager::resumeBGM() {
    if (currentBgm.getStatus() == sf::Music::Paused) {
        applyMusicVolume();
        currentBgm.play();
    }
}

void AudioManager::applyMusicVolume() {
    auto& cfg = ConfigManager::getInstance();
    float vol = cfg.isMusicEnabled() ? static_cast<float>(cfg.getMusicVolume()) : 0.f;
    // Audio Ducking: Tự động hạ âm lượng nhạc nền xuống 35% khi có giọng nói Tướng phát biểu
    if (isVoicePlaying()) {
        vol *= 0.35f;
    }
    currentBgm.setVolume(vol);
}

void AudioManager::update(float dt) {
    (void)dt;
    // Khi câu thoại kết thúc, tự động khôi phục âm lượng BGM trở lại bình thường
    if (wasVoicePlaying && !isVoicePlaying()) {
        wasVoicePlaying = false;
        applyMusicVolume();
    }
}

// ================================================================
//  SFX Controls
// ================================================================
void AudioManager::playSound(const std::string& name, float pitch) {
    auto& cfg = ConfigManager::getInstance();
    if (!cfg.isSfxEnabled()) return;

    std::string key = normalizeName(name);
    auto it = sfxBuffers.find(key);
    if (it == sfxBuffers.end()) {
        std::cerr << "[AudioManager] SFX not found: " << key << std::endl;
        return;
    }

    // Tìm một slot khả dụng trong Sound Pool
    size_t chosenIdx = nextSoundIdx;
    for (size_t i = 0; i < soundPool.size(); ++i) {
        size_t idx = (nextSoundIdx + i) % soundPool.size();
        if (soundPool[idx].getStatus() == sf::Sound::Stopped) {
            chosenIdx = idx;
            break;
        }
    }
    nextSoundIdx = (chosenIdx + 1) % soundPool.size();

    // Cân bằng âm thanh: SFX mặc định hạ xuống 45% để êm tai, có chiều sâu, không chát chúa
    float sfxVol = static_cast<float>(cfg.getSfxVolume()) * 0.45f;
    // Audio Ducking: Nếu Tướng đang cất giọng thoại thì hạ SFX xuống 35% để câu thoại nổi bật nhất
    if (isVoicePlaying()) {
        sfxVol *= 0.35f;
    }

    soundPool[chosenIdx].setBuffer(it->second);
    soundPool[chosenIdx].setVolume(sfxVol);
    soundPool[chosenIdx].setPitch(pitch);
    soundPool[chosenIdx].play();
}

void AudioManager::stopAllSounds() {
    for (auto& s : soundPool) {
        s.stop();
    }
}

void AudioManager::applySfxVolume() {
    auto& cfg = ConfigManager::getInstance();
    float sfxVol = cfg.isSfxEnabled() ? static_cast<float>(cfg.getSfxVolume()) * 0.45f : 0.f;
    if (isVoicePlaying()) {
        sfxVol *= 0.35f;
    }

    for (auto& s : soundPool) {
        if (s.getStatus() == sf::Sound::Playing) {
            s.setVolume(sfxVol);
        }
    }
    if (voiceSound.getStatus() == sf::Sound::Playing) {
        voiceSound.setVolume(cfg.isSfxEnabled() ? 100.f : 0.f);
    }
}

// ================================================================
//  Voice Controls
// ================================================================
void AudioManager::playVoice(const std::string& name) {
    auto& cfg = ConfigManager::getInstance();
    if (!cfg.isSfxEnabled()) return;

    std::string key = normalizeName(name);
    auto it = voiceBuffers.find(key);
    if (it == voiceBuffers.end()) {
        std::cerr << "[AudioManager] Voice not found: " << key << std::endl;
        return;
    }

    voiceSound.stop();
    voiceSound.setBuffer(it->second);
    // Voice luôn được phát ở mức âm lượng tối đa 100% để to, rõ ràng, hào sảng
    voiceSound.setVolume(100.f);
    voiceSound.play();
    currentVoiceName = key;
    wasVoicePlaying = true;

    // Tự động kích hoạt Ducking cho BGM
    applyMusicVolume();

    std::cout << "[AudioManager] Playing Voice (Max Volume 100% + Audio Ducking): " << key << std::endl;
}

void AudioManager::stopVoice() {
    voiceSound.stop();
    currentVoiceName = "";
    if (wasVoicePlaying) {
        wasVoicePlaying = false;
        applyMusicVolume(); // Khôi phục lại nhạc nền
    }
}

bool AudioManager::isVoicePlaying() const {
    return voiceSound.getStatus() == sf::Sound::Playing;
}