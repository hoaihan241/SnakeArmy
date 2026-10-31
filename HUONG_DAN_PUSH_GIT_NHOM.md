# 📘 HƯỚNG DẪN QUY TRÌNH PUSH CODE LÊN GITHUB CHO NHÓM 5 THÀNH VIÊN
**Dự Án: Đồ Án Lập Trình Hướng Đối Tượng - Snake Army 2.0 (UTC2)**  
**Repository:** `https://github.com/hoaihan241/SnakeArmy.git`  
**Nhánh tích hợp chính:** `develop`

---

## 📌 PHẦN I: CÔNG VIỆC CHUẨN BỊ (TRƯỞNG NHÓM / NGƯỜI GIỮ CODE GỐC)

### 1. Cấp quyền đóng góp (Collaborators) trên GitHub:
Để 4 bạn còn lại có quyền push code lên repository:
1. Trưởng nhóm vào GitHub repository: `https://github.com/hoaihan241/SnakeArmy`
2. Chọn tab **Settings** $\rightarrow$ chọn mục **Collaborators** ở thanh bên trái.
3. Bấm **Add people** $\rightarrow$ Nhập username hoặc email GitHub của 4 bạn trong nhóm $\rightarrow$ Bấm gửi lời mời.
4. **4 bạn vào email (hoặc thông báo trên GitHub) bấm "Accept invitation"** để kích hoạt quyền.

### 2. Gửi mã nguồn gốc mới nhất cho nhóm:
1. Nén toàn bộ thư mục `snakearmy` thành file **`snakearmy.zip`** (không cần nén các thư mục ẩn `.git`, `.vscode`, `bin`, `obj`).
2. Gửi file `snakearmy.zip` lên nhóm Zalo / Google Drive để 4 bạn tải về máy cá nhân và giải nén ra một thư mục riêng (để lấy file copy).

---

## 📌 PHẦN II: HƯỚNG DẪN THỰC HIỆN TRÊN MÁY TỪNG THÀNH VIÊN

Mỗi bạn ngồi trên máy tính của riêng mình, mở **Terminal / Git Bash / PowerShell** và thực hiện đúng 3 bước:

### ⚡ BƯỚC CHUNG: Tải mã nguồn khung về máy
```bash
# 1. Clone dự án về máy cá nhân
git clone https://github.com/hoaihan241/SnakeArmy.git

# 2. Di chuyển vào thư mục dự án
cd SnakeArmy

# 3. Chuyển sang nhánh develop
git checkout develop
```

---

### 👤 THÀNH VIÊN 1: Kiến trúc Cốt Lõi, Game Loop & Lưu Trữ CSV
* **Công việc phụ trách:** Quản lý vòng lặp 60 FPS, Game Engine, Quản lý tài nguyên `ResourceManager`, Cài đặt `ConfigManager`, Lưu trữ `match_history.csv` và script `build.bat`.
* **Tên nhánh:** `feature/core-engine`

#### Các bước thực hiện:
1. **Tạo nhánh:**
   ```bash
   git checkout -b feature/core-engine
   ```
2. **Copy file:** Mở thư mục code giải nén từ file zip, copy các file sau dán đè vào thư mục `SnakeArmy`:
   * `src/Core/GameEngine.h`, `src/Core/GameEngine.cpp`
   * `src/Core/ResourceManager.h`, `src/Core/ResourceManager.cpp`
   * `src/Core/ConfigManager.h`, `src/Core/ConfigManager.cpp`
   * `src/main.cpp`
   * `build.bat`, `.gitignore`, `match_history.csv`
3. **Commit và Push:**
   ```bash
   git add src/Core/GameEngine.* src/Core/ResourceManager.* src/Core/ConfigManager.* src/main.cpp build.bat .gitignore match_history.csv
   git commit -m "feat: implement GameEngine 60 FPS, Resource/Config managers and CSV persistence"
   git push -u origin feature/core-engine
   ```

---

### 👤 THÀNH VIÊN 2: Cấu Trúc Dữ Liệu Lính & Nạp Chồng Toán Tử
* **Công việc phụ trách:** Cài đặt cấu trúc Node `Troop`, danh sách liên kết đơn `TroopLinkedList` đạt độ phức tạp $O(1)$, thuật toán Follow-the-Leader và toàn bộ Operator Overloading (`+=`, `>`, `<`, `==`, `<<`).
* **Tên nhánh:** `feature/troop-linkedlist`

#### Các bước thực hiện:
1. **Tạo nhánh:**
   ```bash
   git checkout -b feature/troop-linkedlist
   ```
2. **Copy file:** Mở thư mục giải nén, copy toàn bộ thư mục `src/DataStructures/` dán đè vào `SnakeArmy/src/`:
   * `src/DataStructures/Troop.h`, `src/DataStructures/Troop.cpp`
   * `src/DataStructures/TroopLinkedList.h`, `src/DataStructures/TroopLinkedList.cpp`
3. **Commit và Push:**
   ```bash
   git add src/DataStructures/
   git commit -m "feat: implement TroopLinkedList O(1) follow-the-leader and operator overloading"
   git push -u origin feature/troop-linkedlist
   ```

---

### 👤 THÀNH VIÊN 3: Trí Tuệ Nhân Tạo Bot AI 2.0 & Địa Hình Sa Trường
* **Công việc phụ trách:** Trí tuệ nhân tạo `BotPlayer` (FSM 8 trạng thái, Whisker Steering né vật cản, thoát kẹt Raycast 16 hướng, cướp lính), `Obstacle`, Bụi rậm ngụy trang, Căn cứ Doanh trại `BaseZone`.
* **Tên nhánh:** `feature/bot-ai-obstacles`

#### Các bước thực hiện:
1. **Tạo nhánh:**
   ```bash
   git checkout -b feature/bot-ai-obstacles
   ```
2. **Copy file:**
   * Trong `src/Entities/`: copy `BotPlayer.*`, `Obstacle.*`, `BaseZone.*`
   * Trong `assets/sprites/`: copy `obstacle.png`, `tree1.png`, `tree2.png`, `bush.png`, `ruins_pillar.png`, `ruins_wall.png`, `base_blue.png`, `base_red.png`, `background_tile.png`
3. **Commit và Push:**
   ```bash
   git add src/Entities/BotPlayer.* src/Entities/Obstacle.* src/Entities/BaseZone.*
   git add assets/sprites/obstacle.png assets/sprites/tree*.png assets/sprites/bush.png assets/sprites/ruins_* assets/sprites/base_* assets/sprites/background_tile.png
   git commit -m "feat: implement aggressive Bot AI 2.0, Whisker Steering, and battleground obstacles"
   git push -u origin feature/bot-ai-obstacles
   ```

---

### 👤 THÀNH VIÊN 4: Hệ Thống 3 Hệ Tướng & Cây Kế Thừa Bùa Lợi
* **Công việc phụ trách:** Lớp trừu tượng `GameObject`, lớp cơ sở `Character`, điều khiển `HumanPlayer`, 3 Hệ Tướng (Tiên Phong, Quân Sư, Hộ Quốc) và cây đa hình `Item` (`ShieldItem`, `SpeedItem`, `MagnetItem`).
* **Tên nhánh:** `feature/heroes-items`

#### Các bước thực hiện:
1. **Tạo nhánh:**
   ```bash
   git checkout -b feature/heroes-items
   ```
2. **Copy file:**
   * Trong `src/Entities/`: copy `GameObject.*`, `Character.*`, `HumanPlayer.*`
   * Toàn bộ thư mục `src/Items/` (`Item.*`, `ShieldItem.*`, `SpeedItem.*`, `MagnetItem.*`)
   * Trong `assets/sprites/`: copy `player1_sheet.png`, `player2_sheet.png`, `bot_sheet.png`, `hero_mage_sheet.png`, `hero_mage_sheet_red.png`, `hero_titan_sheet.png`, `hero_titan_sheet_red.png`, `item_shield_sheet.png`, `item_speed_sheet.png`, `item_magnet_sheet.png`, `shield_aura.png`
3. **Commit và Push:**
   ```bash
   git add src/Entities/GameObject.* src/Entities/Character.* src/Entities/HumanPlayer.*
   git add src/Items/
   git add assets/sprites/player*.png assets/sprites/hero_* assets/sprites/item_* assets/sprites/shield_aura.png assets/sprites/bot_sheet.png
   git commit -m "feat: implement 3 Hero classes, special skills, and polymorphic Item hierarchy"
   git push -u origin feature/heroes-items
   ```

---

### 👤 THÀNH VIÊN 5: Sự Kiện Sa Trường, Âm Thanh & Báo Cáo Đồ Án
* **Công việc phụ trách:** Sự kiện ngẫu nhiên (`BombHazard`, `DragonEvent`, `VipDropEvent`), Singleton `AudioManager` (BGM, SFX, Voice Acting), Giao diện UI hoàng gia và tài liệu báo cáo `README.md`, `BAO_CAO_OOP_SNAKE_ARMY.md`.
* **Tên nhánh:** `feature/events-audio-docs`

#### Các bước thực hiện:
1. **Tạo nhánh:**
   ```bash
   git checkout -b feature/events-audio-docs
   ```
2. **Copy file:**
   * Toàn bộ thư mục `src/Events/`
   * `src/Core/AudioManager.h`, `src/Core/AudioManager.cpp`
   * Toàn bộ thư mục âm thanh `assets/audio/`
   * Các sprite UI còn lại trong `assets/sprites/` (`ui_*.png`, `badge_*.png`, `banner_ribbon.png`, `icon_*.png`, `hud_*.png`, `clash_arena_bg.png`, `bomb.png`, `dragon_*.png`, `dragon_fire_sheet.png`, `explosion_sheet.png`, `warning_zone.png`, `troop_*.png`)
   * 2 file tài liệu: `README.md` và `BAO_CAO_OOP_SNAKE_ARMY.md`
3. **Commit và Push:**
   ```bash
   git add src/Events/
   git add src/Core/AudioManager.*
   git add assets/audio/
   git add assets/sprites/
   git add README.md BAO_CAO_OOP_SNAKE_ARMY.md
   git commit -m "feat: add battlefield events, voice acting AudioManager, and comprehensive OOP report"
   git push -u origin feature/events-audio-docs
   ```

---

## 📌 PHẦN III: TẠO PULL REQUEST (PR) GỘP VÀO NHÁNH DEVELOP

Sau khi cả 5 bạn đã hoàn thành lệnh `git push` lên GitHub:

1. Mở trình duyệt vào trang repository: `https://github.com/hoaihan241/SnakeArmy`
2. GitHub sẽ tự động hiển thị thanh màu vàng với thông báo:  
   `feature/... had recent pushes ... Compare & pull request`
3. Lần lượt bấm vào **Compare & pull request** cho từng nhánh:
   * **Base branch (nhánh đích):** chọn `develop`
   * **Compare branch (nhánh nguồn):** chọn nhánh `feature/...` của bạn đó
   * Tiêu đề PR: Đặt theo đúng nội dung công việc (ví dụ: `[TV2] Cài đặt TroopLinkedList và Operator Overloading`)
   * Bấm nút xanh **Create pull request**.
4. Người tạo (hoặc Trưởng nhóm) bấm nút **Merge pull request** $\rightarrow$ **Confirm merge**.
5. Lặp lại cho đủ cả 5 nhánh.

---

## 🏆 KẾT QUẢ ĐẠT ĐƯỢC SAU KHI HOÀN THÀNH (ĐẠT CHUẨN YC02)
* Khi giáo viên kiểm tra mục **Insights $\rightarrow$ Contributors**: GitHub sẽ hiển thị đủ biểu đồ đóng góp của **5 tài khoản sinh viên độc lập**.
* Khi giáo viên kiểm tra **Network Graph**: Biểu đồ hiển thị 5 nhánh tính năng riêng biệt được phân chia và merge vào nhánh `develop` cực kỳ chuyên nghiệp và chuẩn mực công nghệ phần mềm.

