#include "ConfigManager.h"
#include <algorithm>

ConfigManager::ConfigManager()
    : currentLang(Language::VI),
      musicEnabled(true),
      sfxEnabled(true),
      musicVolume(80),
      sfxVolume(90)
{
    // ================================================================
    //  Bilingual Dictionary (VI / EN) — Tiếng Việt có dấu đầy đủ
    // ================================================================
    // Menu Buttons
    dictionary["btn_pve"] = { u8"1. Người Đấu Bot (PvE)", "1. Player vs Bot (PvE)" };
    dictionary["btn_pvp"] = { u8"2. Hai Người Chơi (PvP)", "2. Two Players (PvP)" };
    dictionary["btn_story"] = { u8"3. Cốt Truyện & Luật", "3. Story & Rules" };
    dictionary["btn_settings"] = { u8"4. Cài Đặt", "4. Settings" };
    dictionary["btn_exit"] = { u8"5. Thoát Game", "5. Exit Game" };

    // Common / Dialog
    dictionary["btn_close"] = { u8"[ESC] Đóng", "[ESC] Close" };
    dictionary["tab_story"] = { u8"CỐT TRUYỆN", "STORY LORE" };
    dictionary["tab_rules"] = { u8"LUẬT CHƠI", "GAME RULES" };

    // Settings Modal
    dictionary["settings_title"] = { u8"CÀI ĐẶT HỆ THỐNG", "SYSTEM SETTINGS" };
    dictionary["settings_music"] = { u8"Âm nhạc (BGM)", "Background Music" };
    dictionary["settings_sfx"] = { u8"Hiệu ứng (SFX)", "Sound Effects" };
    dictionary["settings_lang"] = { u8"Ngôn ngữ", "Language" };
    dictionary["settings_lang_val"] = { u8"Tiếng Việt", "English" };
    dictionary["settings_on"] = { u8"BẬT", "ON" };
    dictionary["settings_off"] = { u8"TẮT", "OFF" };
    dictionary["settings_hint"] = { 
        u8"[1] Nhạc | [2] Hiệu ứng | [3/4] Âm lượng +/- | [5] Ngôn ngữ | [ESC] Đóng",
        "[1] Music | [2] SFX | [3/4] Volume +/- | [5] Language | [ESC] Back" 
    };

    // Pause Modal
    dictionary["pause_title"] = { u8"TẠM DỪNG TRÒ CHƠI", "GAME PAUSED" };
    dictionary["pause_resume"] = { u8"Tiếp Tục Trận Đấu", "Resume Battle" };
    dictionary["pause_main_menu"] = { u8"Thoát Ra Menu Chính", "Exit to Main Menu" };
    dictionary["pause_hint"] = { 
        u8"[ESC / Space] Tiếp tục  |  [M] Thoát Menu  |  [1-5] Cài đặt nhanh",
        "[ESC / Space] Resume  |  [M] Main Menu  |  [1-5] Quick Settings" 
    };

    // Story Modal Lore
    dictionary["story_title"] = { u8"TRUYỀN THUYẾT SNAKE ARMY", "LEGEND OF SNAKE ARMY" };
    dictionary["story_pve_title"] = { u8"1. KHỞI NGHĨA ĐẤU SĨ (PvE - Chiến dịch Đơn):", "1. GLADIATOR UPRISING (PvE - Single Campaign):" };
    dictionary["story_pve_body"] = {
        u8"Đế chế La Mã chìm trong loạn lạc khói lửa. Bạn hóa thân thành vị Đại Tướng\n"
        u8"Xanh đứng lên tập hợp các đấu sĩ tự do, tù binh và nghĩa binh dưới ngọn cờ.\n"
        u8"Đối đầu với bạn là Quân Đoàn Hoàng Gia được thống lĩnh bởi Trí Tuệ Tự Động\n"
        u8"(Bot AI) tàn bạo, luôn tìm cách bành trướng lãnh địa và dập tắt ngọn lửa khởi nghĩa!",
        
        "The Roman Empire is engulfed in chaos and flames. You command the\n"
        "valiant Blue General, rallying displaced warriors, prisoners, and gladiators.\n"
        "Facing you is the fierce Imperial Legion led by aggressive Bot AI,\n"
        "relentlessly expanding territory to crush your uprising!"
    };

    dictionary["story_pvp_title"] = { u8"2. NỘI CHIẾN CAESAR & POMPEY (PvP - Song Đấu):", "2. CIVIL WAR OF LEGIONS (PvP - Dual Clash):" };
    dictionary["story_pvp_body"] = {
        u8"Hai danh tướng vĩ đại nhất La Mã: Quân Xanh (Player 1) và Quân Đỏ (Player 2)\n"
        u8"trực tiếp đối đầu nghênh chiến trên đại sa trường! Ai sẽ là bậc Đế Vương thực thụ\n"
        u8"thống lĩnh toàn bộ quân đội La Mã, mở ra kỷ nguyên khải hoàn huy hoàng mới?",

        "Two legendary Roman commanders: Blue Legion (Player 1) and Red Legion\n"
        "(Player 2) face off in a supreme clash! Who will become the rightful\n"
        "Emperor to rule Rome into a glorious new epoch?"
    };

    // Rules Modal
    dictionary["rules_title"] = { u8"LUẬT CHƠI SA TRƯỜNG LA MÃ", "ROMAN ARENA RULES" };
    dictionary["rules_p1_title"] = { u8"PHASE 1: THU THẬP LỰC LƯỢNG (90 Giây)", "PHASE 1: TROOP GATHERING (90s)" };
    dictionary["rules_p1_body"] = {
        u8"- Di chuyển tướng tiếp cận lính tự do trên bản đồ để kết nạp vào hàng ngũ.\n"
        u8"- 3 Binh chủng: KIẾM SĨ (Cận chiến) | HỘ VỆ (Giảm sát thương) | KỴ BINH (Tốc độ cao).\n"
        u8"- VÙNG CĂN CỨ (BASE): Đưa lính về căn cứ để 'Lưu kho bảo toàn điểm số' an toàn.\n"
        u8"- VẬT PHẨM (ITEMS): Nhặt Giày Tốc Độ, Khiên Bất Tử, Nam Châm Thu Phục Lính.\n"
        u8"- BOM OANH TẠC: Nhanh chân né tránh vòng đỏ! Vụ nổ xé đôi quân đoàn và đốt cháy đất!",

        "- Move your General close to wandering troops to recruit them into your army.\n"
        "- 3 Troop Classes: SWORDSMAN (Melee) | GUARDIAN (Damage Reduction) | CAVALRY (Mobility).\n"
        "- BASE ZONE: Escort troops to base to safely store and secure your score points.\n"
        "- ITEMS: Pick up Speed Boots, Aegis Shield, and Magnet to recruit instantly.\n"
        "- AIRSTRIKE HAZARDS: Avoid red blast zones! Explosions sever troops & scorch ground!"
    };

    dictionary["rules_p2_title"] = { u8"PHASE 2: ĐẠI CHIẾN QUYẾT ĐẤU (Arena Clash)", "PHASE 2: DECISIVE CLASH (Arena Clash)" };
    dictionary["rules_p2_body"] = {
        u8"- Toàn bộ quân số (lính theo sau + lính trong kho) quy đổi thành chiến lực ra trận.\n"
        u8"- Hai quân đoàn xếp đội hình chiến thuật, đồng loạt bứt tốc lao vào giao phong.\n"
        u8"- Tương khắc thực chiến: Kỵ Binh khắc Kiếm Sĩ, Kiếm Sĩ khắc Hộ Vệ, Hộ Vệ khắc Kỵ Binh.\n"
        u8"- THANH CÂN NÃO (Tug-of-War): Quân bên nào chiếm ưu thế sẽ đẩy lùi chiến tuyến đối phương.\n"
        u8"- Tiêu diệt toàn bộ quân địch hoặc đẩy lùi hoàn toàn chiến tuyến để GIÀNH CHIẾN THẮNG!",

        "- Total army count (following troops + banked troops) enters the Grand Arena.\n"
        "- Armies form tactical formations and charge into the arena frontline.\n"
        "- Tactical Counters: Cavalry counters Swordsman, Swordsman counters Guardian,\n"
        "  Guardian counters Cavalry in real-time combat!\n"
        "- TUG-OF-WAR BAR: The dominant army pushes the frontline toward the enemy base.\n"
        "- Annihilate all enemy forces or dominate the frontline to CLAIM VICTORY!"
    };
}

#include "AudioManager.h"
#include <algorithm>

void ConfigManager::setMusicEnabled(bool val) {
    musicEnabled = val;
    AudioManager::getInstance().applyMusicVolume();
}

void ConfigManager::toggleMusic() {
    setMusicEnabled(!musicEnabled);
}

void ConfigManager::setSfxEnabled(bool val) {
    sfxEnabled = val;
    AudioManager::getInstance().applySfxVolume();
}

void ConfigManager::toggleSfx() {
    setSfxEnabled(!sfxEnabled);
}

void ConfigManager::setMusicVolume(int vol) {
    musicVolume = std::clamp(vol, 0, 100);
    AudioManager::getInstance().applyMusicVolume();
}

void ConfigManager::adjustMusicVolume(int delta) {
    setMusicVolume(musicVolume + delta);
}

void ConfigManager::setSfxVolume(int vol) {
    sfxVolume = std::clamp(vol, 0, 100);
    AudioManager::getInstance().applySfxVolume();
}

void ConfigManager::adjustSfxVolume(int delta) {
    setSfxVolume(sfxVolume + delta);
}

std::string ConfigManager::getText(const std::string& key) const {
    auto it = dictionary.find(key);
    if (it != dictionary.end()) {
        return (currentLang == Language::VI) ? it->second.first : it->second.second;
    }
    return key;
}