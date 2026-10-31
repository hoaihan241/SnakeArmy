#pragma once
#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <stdexcept>

// ================================================================
//  ResourceManager – SINGLETON
//  Quản lý tập trung Texture & Font, mỗi file chỉ load 1 lần.
//  Ngăn memory leak và tối ưu tốc độ khung hình.
// ================================================================
class ResourceManager {
private:
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font>    fonts;
    std::string basePath; // Đường dẫn gốc tới thư mục project

    ResourceManager() {}

public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    // Ngăn copy (Singleton phải là unique)
    ResourceManager(const ResourceManager&)            = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // Tìm và set basePath tự động từ vị trí exe
    void detectBasePath();

    // Preload toàn bộ assets khi khởi động – gọi 1 lần duy nhất
    void preloadAll();

    // Getter – throw nếu chưa preload
    sf::Texture& getTexture(const std::string& name);
    sf::Font&    getFont(const std::string& name);

    // Lấy đường dẫn đầy đủ
    std::string getFullPath(const std::string& relativePath) const;
};