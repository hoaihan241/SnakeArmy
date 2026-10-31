#pragma once
#include <string>
#include <unordered_map>
#include <SFML/System/String.hpp>

// ================================================================
//  ConfigManager – Singleton quản lý Cài đặt & Đa ngôn ngữ (VI/EN)
//  Hỗ trợ Tiếng Việt có dấu UTF-8 chuẩn xác qua SFML String
// ================================================================
enum class Language { VI, EN };

class ConfigManager {
private:
    ConfigManager();
    Language currentLang;
    bool musicEnabled;
    bool sfxEnabled;
    int musicVolume; // 0 - 100
    int sfxVolume;   // 0 - 100

    std::unordered_map<std::string, std::pair<std::string, std::string>> dictionary;

public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }

    // Tiện ích chuyển đổi std::string UTF-8 sang sf::String hiển thị Tiếng Việt có dấu
    static sf::String toUtf8(const std::string& str) {
        return sf::String::fromUtf8(str.begin(), str.end());
    }

    Language getLanguage() const { return currentLang; }
    void setLanguage(Language lang) { currentLang = lang; }
    void toggleLanguage() { currentLang = (currentLang == Language::VI) ? Language::EN : Language::VI; }

    bool isMusicEnabled() const { return musicEnabled; }
    void setMusicEnabled(bool val);
    void toggleMusic();

    bool isSfxEnabled() const { return sfxEnabled; }
    void setSfxEnabled(bool val);
    void toggleSfx();

    int getMusicVolume() const { return musicVolume; }
    void setMusicVolume(int vol);
    void adjustMusicVolume(int delta);

    int getSfxVolume() const { return sfxVolume; }
    void setSfxVolume(int vol);
    void adjustSfxVolume(int delta);

    std::string getText(const std::string& key) const;

    sf::String getSfText(const std::string& key) const {
        std::string raw = getText(key);
        return sf::String::fromUtf8(raw.begin(), raw.end());
    }
};