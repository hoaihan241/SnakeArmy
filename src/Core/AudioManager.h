
#pragma once
#include <SFML/Audio.hpp>
#include <string>
#include <unordered_map>
#include <vector>

// ================================================================
//  AudioManager – SINGLETON PATTERN
//  Quản lý toàn diện hệ thống âm thanh trò chơi Snake Army:
//  1. Nhạc nền (BGM): Streaming qua sf::Music (tiết kiệm RAM, loop mượt mà)
//  2. Hiệu ứng (SFX): Preload sf::SoundBuffer + Sound Pool (đa kênh, không đè tiếng)
//  3. Lồng tiếng (Voice Lines): Kênh thoại chuyên biệt
//  Tương thích trực tiếp với ConfigManager (âm lượng & bật/tắt thời gian thực)
// ================================================================
class AudioManager {
private:
    AudioManager();
    ~AudioManager() = default;

    // Ngăn chặn copy / gán (Singleton)
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    // Nhạc nền (BGM)
    sf::Music currentBgm;
    std::string currentBgmName;

    // Buffer lưu trữ SFX & Voice
    std::unordered_map<std::string, sf::SoundBuffer> sfxBuffers;
    std::unordered_map<std::string, sf::SoundBuffer> voiceBuffers;

    // Sound Pool cho SFX (hỗ trợ nhiều âm thanh phát đồng thời)
    static constexpr size_t SOUND_POOL_SIZE = 32;
    std::vector<sf::Sound> soundPool;
    size_t nextSoundIdx;

    // Kênh thoại nhân vật riêng biệt
    sf::Sound voiceSound;
    std::string currentVoiceName;
    bool wasVoicePlaying;

    // Helper chuẩn hóa tên file (.ogg)
    static std::string normalizeName(const std::string& name);

public:
    static AudioManager& getInstance() {
        static AudioManager instance;
        return instance;
    }

    // Nạp toàn bộ SFX và Voice vào RAM (gọi 1 lần khi khởi động game)
    void init();

    // Cập nhật hệ thống âm thanh (Audio Ducking thời gian thực)
    void update(float dt);

    // === BGM (Nhạc nền) ===
    void playBGM(const std::string& name, bool loop = true);
    void stopBGM();
    void pauseBGM();
    void resumeBGM();
    void applyMusicVolume();
    std::string getCurrentBgmName() const { return currentBgmName; }

    // === SFX (Hiệu ứng âm thanh) ===
    void playSound(const std::string& name, float pitch = 1.0f);
    void stopAllSounds();
    void applySfxVolume();

    // === Voice (Lồng tiếng nhân vật) ===
    void playVoice(const std::string& name);
    void stopVoice();
    bool isVoicePlaying() const;
    std::string getCurrentVoiceName() const { return currentVoiceName; }
};