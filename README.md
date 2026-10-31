#  Snake Army 2.0 

> **Trò chơi hành động chiến thuật 2D đỉnh cao** – Tuyển mộ nghĩa binh, bứt phá tuyệt kỹ, xây dựng đoàn quân và phân định thắng bại trong trận đại chiến giáp lá cà.  
> Được xây dựng hoàn toàn bằng **C++17** và **SFML 2.5.1**, tối ưu chuẩn **OOP & DSA** đạt 60 FPS mượt mà.

---

## 1. Giới thiệu tổng quan

**Snake Army 2.0** là trò chơi 2D Top-down arcade chiến thuật độc đáo kết hợp giữa cơ chế **Rắn săn mồi (Snake)** và **Đại chiến tự động (Auto-Battler Tug-of-War)**. 

Khác với game rắn cổ điển chỉ ăn mồi để dài ra, mỗi mắt xích trong **Snake Army 2.0** là một **chiến binh có sinh mệnh, chủng loại và chiến lực riêng**. Người chơi hóa thân thành dũng tướng sa trường, vừa phải khéo léo điều khiển đội hình lượn lách qua đá tảng, rừng cây, vừa kích hoạt tuyệt kỹ độc quyền để cướp quân đối phương, né bom pháo kích, né lửa rồng và hộ tống binh lực về căn cứ tích trữ.

Khi hết thời gian thu thập (90 giây), trận đấu bước sang **Phase 2 – Clash**: Hai quân đoàn hợp nhất toàn bộ quân lực, dàn trận diễu binh rồi lao vào nhau xung phong giáp lá cà theo quy tắc ngũ hành tương khắc trên nền nhạc trống trận hào hùng.

---

## 2. Các tính năng và Update mới nhất (Version 2.0)

###  2.1. Hệ thống 3 Hệ Tướng & Chọn Tướng Độc Quyền (Mới)
Người chơi được tự do lựa chọn 1 trong 3 hệ Tướng với phong cách chiến đấu, chỉ số và tuyệt kỹ riêng biệt:

| Hệ Tướng | Sprite Sheet (Xanh / Đỏ) | Chỉ số | Tuyệt Kỹ Độc Quyền | Mô Tả Kỹ Năng & Lợi Thế Chiến Thuật |
| :--- | :--- | :---: | :--- | :--- |
| ** Tiên Phong**<br>*(Vanguard)* | `player1_sheet.png`<br>`bot_sheet.png` | Tốc độ: ★★★★★<br>Chiến lực: ★★★☆☆ | **Đoạt Mệnh Trảm**<br>*(Dash Slash)* | Dồn nội lực lướt xuyên thấu với vận tốc cực đại $750\text{ px/s}$. Cắt cụt đuôi đoàn lính đối phương từ xa, cướp toàn bộ lính trên đường bay và cuốn lính tự do vào hàng ngũ. |
| **Quân Sư**<br>*(Tactician)* | `hero_mage_sheet_blue.png`<br>`hero_mage_sheet_red.png` | Tốc độ: ★★★★☆<br>Chiến lực: ★★★★☆ | **Bát Quái Hấp Tinh**<br>*(Bát Quái Vortex)* | Triệu hồi pháp trận Thái Cực Bát Quái xoay tròn bán kính $260\text{ px}$. Hút sạch toàn bộ lính tự do xung quanh vào đội hình trong chớp mắt mà không cần chạy tới chạm từng con. |
| ** Hộ Quốc**<br>*(Titan)* | `hero_titan_sheet_blue.png`<br>`hero_titan_sheet_red.png` | Tốc độ: ★★★☆☆<br>Chiến lực: ★★★★★ | **Kim Cang Hống**<br>*(Titan Roar)* | Vận khí gầm vang làm rung chuyển đất trời sa trường. Làm choáng (Stun) toàn bộ kẻ địch trong bán kính $280\text{ px}$ trong $2.0\text{s}$, bẻ gãy mọi ý đồ áp sát cướp lính. |

- **Màn hình Chọn Tướng trực quan (Hero Selection Screen)**:
  - **PvE**: Người chơi chọn 1 trong 3 tướng phe Xanh, Bot AI ngẫu nhiên chọn 1 trong 3 tướng phe Đỏ.
  - **PvP**: P1 chọn tướng Xanh (phím `A`/`D`/`Space`), P2 chọn tướng Đỏ (phím `←`/`→`/`Enter`).
  - Card tướng hiển thị hoạt ảnh Sprite Animation Idle mượt mà, khung viền hoa văn hoàng gia, mô tả tuyệt kỹ và thông số chi tiết.
- **HUD & Avatar động**: Khung nguyệt quế hiển thị chính xác chân dung vị tướng đã chọn kèm huy hiệu tuyệt kỹ và thanh tiến trình hồi chiêu (Cooldown Bar).

---

###  2.2. Trí tuệ nhân tạo Bot AI 2.0 & Chống kẹt vật cản thông minh (Mới)
Bot AI đã được nâng cấp toàn diện, sở hữu tư duy chiến thuật sắc sảo:
- **Máy trạng thái FSM 8 Trạng Thái**: `SCOUTING`, `STEALING`, `FLEEING`, `BANKING`, `EVADING_BOMB`, `AMBUSH`, `HUNTING`, `ITEM_RUSH`.
- **Độ khó thích ứng (Adaptive Difficulty)**:
  - *Early game (0–30s)*: Thận trọng thu gom lính, về căn cứ cất lính khi đạt 6 con.
  - *Mid game (30–60s)*: Tăng tốc độ phản xạ, chủ động săn lùng đối thủ nếu có ưu thế quân số.
  - *Late game (60–90s)*: Cực kỳ hung hãn (Full Aggro), radar quét toàn bộ bản đồ $1200\text{ px}$, phản xạ ra quyết định $0.1\text{s}$.
- **Tư duy Giao chiến Chủ động & Tung chiêu chuẩn xác (Aggressive Combat AI)**:
  - **Tiên Phong**: Chủ động nhắm thẳng chiêu **Đoạt Mệnh Trảm** vào người chơi (`dist <= 290px`) để lướt chém đứt đuôi cướp quân bất ngờ, không bao giờ lãng phí chiêu thức để tháo chạy.
  - **Hộ Quốc**: Khi người chơi áp sát ($< 250\text{px}$), bot lập tức vận khí gầm **Kim Cang Hống** làm người chơi choáng (Stun) $2.0\text{s}$ rồi lao vào cướp lính.
  - **Quân Sư**: Tự động kích hoạt **Bát Quái Hấp Tinh** hút sạch lính tự do xung quanh khi phát hiện có từ 2 lính gần bên.
  - **Dũng cảm & Quyết đoán**: Bot không còn chạy trốn vô cớ khi gặp người chơi. Bot chỉ tạm thời rút lui (`FLEEING`) khi người chơi **bật Khiên bất tử** hoặc **quân địch quá đông (áp đảo từ 5 lính trở lên)** khi bot đang chờ hồi chiêu. Nếu người chơi không có khiên, bot sẵn sàng lao vào giao chiến đọ kiếm cướp quân sòng phẳng!
- **Thuật toán Né vật cản & Chống kẹt đá/cây đột phá**:
  - **Context Steering & Whisker Clearance**: Bắn tia dò phía trước. Nếu phát hiện vật cản hoặc khe hẹp không thể lọt qua ($< 60\text{ px}$), bot tự động quét các góc lệch ($\pm 25^\circ, \pm 50^\circ, \pm 75^\circ...$) để tìm lối đi rộng rãi nhất lượn vòng ra ngoài, không bao giờ bị kẹt cứng giữa 2 hòn đá hay thân cây.
  - **True Displacement Tracking**: Đo độ dịch chuyển thực tế sau va chạm; nếu bị kẹt $< 8\text{ px}$ trong $0.35\text{s}$, lập tức kích hoạt cờ thoát hiểm.
  - **Raycast 16 hướng khẩn cấp (`bestEscapeDir`)**: Quét $360^\circ$ tìm hướng thoáng nhất để bứt tốc thoát hiểm ra ngoài.

---

###  2.3. Tinh chỉnh Đấu Trường Giáp Lá Cà (Phase 2 Clash)
- **Tối ưu hóa giao diện Clash**: Loại bỏ các khung viền hộp trắng chữ thừa, màn hình giao tranh thoáng đãng, tập trung vào hoạt cảnh đọ kiếm giáp lá cà kịch tính.
- **Thanh giằng co Tug-of-War động**: Cặp song kiếm tóe lửa `icon_clash_swords.png` đập nhịp theo thời gian thực tại chiến tuyến giao tranh giữa 2 phe.
- **Hệ thống tương khắc binh chủng**: Kỵ Binh khắc Kiếm Sĩ, Kiếm Sĩ khắc Hộ Vệ, Hộ Vệ khắc Kỵ Binh (+2 sát thương mỗi nhịp va chạm).
- **Xử lý toàn diện trường hợp biên (Edge Cases)**: Nếu một trong hai bên không nhặt được con lính nào trong 90 giây, hai Tướng vẫn dàn trận và xung phong oai vệ, sau đó phe có quân sẽ lập tức quét sạch đối phương để giành chiến thắng. Nếu cả 2 bên cùng không có lính hoặc lính 2 phe đồng quy vu tận cùng lúc, trận đấu được xử Hòa (Lưỡng Bại Câu Thương), triệt tiêu hoàn toàn nguy cơ đơ/treo game.

---

###  2.4. Bộ 8 Sprite Giao Diện UI/UX Hoàng Gia
1. `badge_victory.png`: Huy hiệu Đại Thắng Hoàng Kim khi giành thắng lợi.
2. `badge_defeat.png`: Huy hiệu Chiến Bại Sa Trường khi thất thủ trước đối phương.
3. `badge_draw.png`: Huy hiệu Bất Phân Thắng Bại khi hai bên cân bằng điểm lực chiến.
4. `hud_frame_blue.png`: Khung nguyệt quế hoàng gia lồng mặt tướng P1.
5. `hud_frame_red.png`: Khung nguyệt quế hoàng gia lồng mặt tướng P2 / Bot.
6. `icon_base_vault.png`: Biểu tượng Doanh trại biểu thị quân số dự trữ trong kho an toàn.
7. `icon_clash_swords.png`: Cặp song kiếm tóe lửa làm marker chiến tuyến kéo co.
8. `banner_ribbon.png`: Dải lụa rồng cuộn thông báo sự kiện nổi (Hỏa Long, Hòm tiếp tế VIP).

---

###  2.5. Hệ thống Lồng tiếng Voice Acting & Âm thanh sống động
- Lồng tiếng Voice Acting độc quyền theo từng khoảnh khắc (Vào trận, Tiếp tế VIP rơi, Đại chiến Clash, Thắng trận, Thua trận).
- Hệ thống nhạc nền BGM tách biệt 5 khu vực: Menu chính, Săn lùng binh mã 90s, Đại chiến Clash, Khải hoàn ca Victory, Nhạc bi tráng Defeat.
- Hiệu ứng âm thanh SFX chuẩn từng va chạm: chém kiếm, cướp lính, tiếng bước chân bụi cỏ, bom nổ, tiếng rồng gầm thét.

---

###  2.6. Kỹ thuật OOP Nâng Cao: Nạp Chồng Toán Tử & Lưu Trữ Bền Vững (Mới)
- **Nạp chồng toán tử (Operator Overloading)**:
  - `operator+=`: Thêm lính hoặc gộp 2 đoàn quân (`army += newTroop`, `army += otherArmy`).
  - `operator>` / `operator<` / `operator==`: So sánh quy mô và tổng lực chiến giữa 2 đoàn quân.
  - `operator<<`: Xuất thông tin chi tiết từng chiến binh và toàn bộ đoàn quân ra luồng `std::ostream`.
- **Lưu trữ dữ liệu bền vững qua File CSV (`match_history.csv`)**:
  - Tự động ghi nhận lịch sử mọi trận đấu sau khi kết thúc: Thời gian, Chế độ (PvE/PvP), Tướng 2 phe, Số lượng từng loại lính (Kiếm Sĩ, Hộ Vệ, Kỵ Binh), Tổng lực chiến và Kết quả thắng/thua. Dữ liệu được lưu trữ vĩnh viễn trên ổ đĩa phục vụ thống kê.

---

## 3. Cấu trúc thư mục dự án

```
SnakeArmy/
├── assets/
│   ├── fonts/                    # Font chữ Unicode Việt hóa
│   ├── sfx/                      # Hiệu ứng âm thanh va chạm, chém, cướp lính
│   ├── bgm/                      # Nhạc nền các giai đoạn trận đấu
│   ├── voice/                    # File lồng tiếng Voice Acting
│   └── sprites/                  # Toàn bộ Sprite Sheet & Hình ảnh
│       ├── player1_sheet.png          # Tướng Tiên Phong Xanh
│       ├── bot_sheet.png              # Tướng Tiên Phong Đỏ
│       ├── hero_mage_sheet_blue.png   # Tướng Quân Sư Xanh
│       ├── hero_mage_sheet_red.png    # Tướng Quân Sư Đỏ
│       ├── hero_titan_sheet_blue.png  # Tướng Hộ Quốc Xanh
│       ├── hero_titan_sheet_red.png   # Tướng Hộ Quốc Đỏ
│       ├── troop_swordsman_sheet.png  # Kiếm Sĩ (Power 1)
│       ├── troop_guardian_sheet.png   # Hộ Vệ (Power 2)
│       ├── troop_cavalry_sheet.png    # Kỵ Binh (Power 3)
│       ├── dragon_sheet.png           # Rồng lửa bay (192×192)
│       ├── dragon_fire_sheet.png      # Vệt lửa rồng khạc
│       ├── bomb.png & explosion.png   # Bom pháo kích & Hiệu ứng nổ
│       ├── item_shield_sheet.png      # Bùa Khiên Hộ Thể
│       ├── item_speed_sheet.png       # Bùa Tăng Tốc
│       ├── item_magnet_sheet.png      # Bùa Nam Châm hút quân
│       ├── base_blue.png / base_red   # Căn cứ Doanh trại 2 phe
│       ├── clash_arena_bg.png         # Đấu trường Clash giáp lá cà
│       ├── badge_victory / defeat     # Huy hiệu kết quả trận đấu
│       ├── hud_frame_blue / red       # Khung avatar nguyệt quế
│       └── icon_clash_swords.png      # Song kiếm kéo co
│
├── src/
│   ├── main.cpp                  # Điểm khởi chạy chương trình (Entry point)
│   ├── Core/                     # Lõi game engine
│   │   ├── GameEngine.h/.cpp     # Vòng lặp chính, Quản lý State, Clash, Viewport
│   │   ├── ResourceManager.h/.cpp# Singleton: Quản lý cache Texture, Font
│   │   ├── AudioManager.h/.cpp   # Singleton: Phát nhạc nền, SFX, Voice Acting
│   │   └── ConfigManager.h/.cpp  # Quản lý cài đặt, Đa ngôn ngữ (VI / EN)
│   ├── Entities/                 # Cây kế thừa thực thể trò chơi
│   │   ├── GameObject.h/.cpp     # Lớp cha thuần ảo cơ sở
│   │   ├── Character.h/.cpp      # Lớp cha Tướng: Đoàn quân, Buff, Tuyệt kỹ
│   │   ├── HumanPlayer.h/.cpp    # Tướng người chơi (nhận input bàn phím)
│   │   ├── BotPlayer.h/.cpp      # Tướng máy AI FSM 8 trạng thái, né vật cản
│   │   ├── Obstacle.h/.cpp       # Đá, Cây, Bụi rậm ngụy trang, Cột tàn tích
│   │   └── BaseZone.h/.cpp       # Căn cứ Doanh trại gửi lính an toàn
│   ├── DataStructures/           # Cấu trúc dữ liệu tự định nghĩa
│   │   ├── Troop.h/.cpp          # Node mắt xích chiến binh
│   │   └── TroopLinkedList.h/.cpp# Danh sách liên kết: Cướp lính O(1)
│   ├── Items/                    # Bùa lợi phù trợ
│   │   ├── Item.h/.cpp           # Lớp cha trừu tượng bùa lợi
│   │   ├── ShieldItem.h/.cpp     # Bùa Bất tử chống cướp quân
│   │   ├── SpeedItem.h/.cpp      # Bùa Tăng 45% tốc độ di chuyển
│   │   └── MagnetItem.h/.cpp     # Bùa Hút lính tự do xung quanh
│   └── Events/                   # Sự kiện sa trường ngẫu nhiên
│       ├── BombHazard.h/.cpp     # Pháo kích bom rải thảm & vùng đất cháy
│       ├── DragonEvent.h/.cpp    # Hỏa Long sa trường khạc biển lửa
│       └── VipDropEvent.h/.cpp   # Hòm tiếp tế Kỵ Binh VIP rơi từ trời
│
├── bin/
│   └── main.exe                  # File thực thi game
└── README.md
```

---

##  4. Chi tiết Gameplay 2 Giai Đoạn

```
┌────────────────────────────────────┐      ┌────────────────────────────────┐      ┌────────────────────────────────┐
│      PHASE 1: GATHERING            │ 90s  │        PHASE 2: CLASH          │ Auto │       PHASE 3: RESULT          │
│ - Chọn tướng                       │      │ - Hợp nhất quân dắt theo & kho │      │ - Huy hiệu Hoàng Gia hiển thị  │
│ - Gom lính tự do, né bom, né rồng  ├─────►│ - Diễu binh dàn trận (2.4s)    ├─────►│ - Bảng thống kê chi tiết 2 phe │
│ - Cướp lính & Dùng Tuyệt Kỹ        │      │ - Xung phong đọ kiếm giáp lá cà│      │ - Nút Tái Chiến / Menu Chính   │
│ - Gửi quân an toàn vào Căn Cứ      │      │ - Kéo co Tug-of-War theo nhịp  │      │ - Lồng tiếng khải hoàn         │
└────────────────────────────────────┘      └────────────────────────────────┘      └────────────────────────────────┘
```

### 4.1. Binh chủng lính sa trường
| Binh chủng | Sprite Sheet | Lực chiến (Power) | Tỷ lệ xuất hiện | Điểm đặc trưng |
| :--- | :--- | :---: | :---: | :--- |
| **Kiếm Sĩ** *(Swordsman)* | `troop_swordsman_sheet.png` | **1** | Phổ biến (60%) | Chém nhanh, khắc chế Hộ Vệ |
| **Hộ Vệ** *(Guardian)* | `troop_guardian_sheet.png` | **2** | Trung bình (30%) | Giáp dày cản lướt, khắc chế Kỵ Binh |
| **Kỵ Binh** *(Cavalry)* | `troop_cavalry_sheet.png` | **3** | Hiếm (10%) | Xung kích cực mạnh, khắc chế Kiếm Sĩ |
| **VIP Hoàng Gia** *(Royal)* | Rơi từ hòm tiếp tế | **5** | Sự kiện 45s | Quân tinh nhuệ toàn diện chỉ số vượt trội |

### 4.2. Căn cứ Doanh trại (Base Zone)
- Căn cứ Phe Xanh đặt tại $X = 150$, Căn cứ Phe Đỏ đặt tại $X = 2650$.
- Khi Tướng dẫn lính bước vào căn cứ phe mình, toàn bộ lính đang dắt theo sẽ được cất giữ an toàn vào kho ([Banked Troops](file:///d:/snakearmy/src/Core/GameEngine.cpp)). Lính trong kho **không thể bị đối thủ cướp** hay bị lửa thiêu, được bảo toàn $100\%$ cho trận đại chiến Phase 2.

### 4.3. Bụi rậm ngụy trang (Bush Stealth)
- Tướng dẫn quân đi vào bụi rậm sẽ được chuyển sang trạng thái ngụy trang (trong suốt $45\%$).
- Tướng ẩn trong bụi rậm sẽ tàng hình trước tầm quét của Bot AI và đối thủ ở khoảng cách xa.

### 4.4. Màn hình Kết Quả & Thống Kê Chi Tiết Đối Xứng (Result Screen)
Khi trận đại chiến Phase 2 kết thúc, màn hình vinh danh hoàng gia sẽ hiển thị bảng đối chiếu quân lực chi tiết giữa hai phe:
- **Huy hiệu kết quả**: Đại Thắng Hoàng Kim (`badge_victory.png`), Chiến Bại Sa Trường (`badge_defeat.png`) hoặc Bất Phân Thắng Bại (`badge_draw.png`).
- **Khung thẻ bài đại diện**: Avatar kèm Khung Nguyệt Quế Hoàng Gia theo đúng hệ tướng đã chọn.
- **Thống kê chi tiết từng phe**:
  - **Tổng số lính**: Tổng quân số toàn đội xuất trận tham gia đại chiến.
  - **Số lượng lính từng loại**: Phân loại chi tiết số lượng Kiếm Sĩ (Lực 1), Hộ Vệ (Lực 2), Kỵ Binh (Lực 3).
  - **TỔNG LỰC CHIẾN**: Tổng điểm sức mạnh chiến đấu của toàn đội (`Total Battle Power`).
- **Phím tắt tương tác**: Phím `Enter` / Nút `[TÁI CHIẾN]` để chơi lại ván mới; Phím `ESC` / Nút `[MENU CHÍNH]` để quay về màn hình chính.

---

##  5. Hướng dẫn điều khiển

### 5.1. Trong trận đấu (Gathering Phase)
| Thao tác | Người chơi 1 (Phe Xanh) | Người chơi 2 (Phe Đỏ - PvP) |
| :--- | :--- | :--- |
| **Di chuyển** | `W`, `A`, `S`, `D` | Các phím mũi tên `↑`, `←`, `↓`, `→` |
| **Tuyệt Kỹ Tướng** | Phím `Space` | `Right Shift` hoặc `Numpad 0` |
| **Tạm dừng trận đấu** | Phím `ESC` hoặc `P` | Phím `ESC` hoặc `P` |

### 5.2. Trong màn hình chọn Tướng
| Thao tác | Người chơi 1 (Phe Xanh) | Người chơi 2 (Phe Đỏ - PvP) |
| :--- | :--- | :--- |
| **Đổi Tướng** | Phím `A` (Trái) / `D` (Phải) | Phím `←` (Trái) / `→` (Phải) |
| **Khóa Tướng / Xác nhận** | Phím `Space` | Phím `Enter` |

---

## 💻 6. Hướng dẫn cài đặt, biên dịch & Vận hành

### 6.1. Chạy nhanh trò chơi (Quick Start)
Nếu máy bạn đã có sẵn môi trường runtime SFML hoặc các file DLL cần thiết:
1. Mở thư mục gốc của dự án `SnakeArmy`.
2. Chạy trực tiếp file thực thi đã biên dịch sẵn:
   - **Cách 1 (Giao diện):** Nhấp đúp chuột vào file `bin\main.exe`.
   - **Cách 2 (PowerShell / CMD):**
     ```powershell
     .\bin\main.exe
     ```

> [!IMPORTANT]
> **Lưu ý về Thư mục làm việc (Working Directory):**  
> Luôn khởi chạy game từ thư mục gốc `SnakeArmy` (nơi chứa thư mục `assets/`). Nếu mở thẳng file bên trong thư mục `bin/` bằng một số trình terminal mà working directory bị chuyển vào `bin/`, game sẽ không tìm thấy thư mục `assets/` và báo lỗi không tải được hình ảnh/âm thanh.

---

### 6.2. Yêu cầu môi trường phát triển (Prerequisites)
Để tự chỉnh sửa mã nguồn và biên dịch game, máy tính cần cài đặt:
- **Hệ điều hành:** Windows 10 hoặc Windows 11 (64-bit).
- **Trình biên dịch C++:** MinGW-w64 GCC (hỗ trợ chuẩn **C++17** trở lên).
  - Kiểm tra trong terminal: `g++ --version`
- **Thư viện đồ họa SFML 2.5.1:**
  - **Cách cài qua MSYS2 (Khuyên dùng):**
    ```bash
    pacman -S mingw-w64-ucrt-x86_64-sfml
    # hoặc nếu dùng MinGW64 truyền thống:
    pacman -S mingw-w64-x86_64-sfml
    ```
  - **Cách cài thủ công:** Tải bản SFML 2.5.1 dành cho MinGW từ [sfml-dev.org](https://www.sfml-dev.org/download/sfml/2.5.1/) và giải nén vào thư mục `C:\sfml` (sao cho có `C:\sfml\include` và `C:\sfml\lib`).

---

### 6.3. Cách 1: Biên dịch tự động 1-Click bằng Script (`build.bat`)
Trong thư mục gốc dự án đã có sẵn file script tự động [`build.bat`](file:///d:/snakearmy/build.bat):
1. Nhấp đúp chuột vào file `build.bat` (hoặc gõ `.\build.bat` trong CMD/PowerShell).
2. Script sẽ tự động:
   - Tạo thư mục `bin/` nếu chưa có.
   - Quét và biên dịch toàn bộ các file `.cpp` trong các thư mục con `src/`.
   - Xuất file thực thi `bin\main.exe`.
   - Tự động khởi chạy game ngay sau khi biên dịch thành công!

---

### 6.4. Cách 2: Biên dịch thủ công từ dòng lệnh (PowerShell / CMD)

Mở **PowerShell** hoặc **Command Prompt** tại thư mục gốc của dự án `SnakeArmy` và thực thi câu lệnh sau:

```powershell
g++ -std=c++17 src\*.cpp src\Core\*.cpp src\Entities\*.cpp src\DataStructures\*.cpp src\Items\*.cpp src\Events\*.cpp -o bin\main.exe -I C:\sfml\include -L C:\sfml\lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -mwindows
```

####  Giải thích chi tiết các cờ biên dịch:
| Cờ biên dịch / Tham số | Chức năng giải thích |
| :--- | :--- |
| `-std=c++17` | Kích hoạt chuẩn ngôn ngữ C++17 (hỗ trợ structured binding, clamp, std::min/max...). |
| `src\*.cpp src\Core\*.cpp ...` | Liên kết tất cả mã nguồn C++ của từng module phân tầng trong dự án. |
| `-o bin\main.exe` | Đặt tên và đường dẫn file thực thi đầu ra là `main.exe` nằm trong thư mục `bin`. |
| `-I C:\sfml\include` | Thêm đường dẫn chứa các file header `.h` / `.hpp` của thư viện SFML. |
| `-L C:\sfml\lib` | Thêm đường dẫn chứa các file thư viện liên kết `.a` / `.lib` của SFML. |
| `-lsfml-graphics` | Liên kết module đồ họa SFML (Sprites, RenderWindow, Views, Colors). |
| `-lsfml-window` | Liên kết module cửa sổ SFML (Events, Keyboard, Mouse, Input handling). |
| `-lsfml-system` | Liên kết module hệ thống SFML (Clock, Time, Vector2f). |
| `-lsfml-audio` | Liên kết module âm thanh SFML (Sound, SoundBuffer, Music streams). |
| `-mwindows` | *(Tùy chọn)* Ẩn cửa sổ dòng lệnh console màu đen, chỉ hiện cửa sổ game đồ họa. Nếu muốn xem log console khi debug, bạn có thể bỏ cờ này. |

Sau khi biên dịch xong, khởi chạy game bằng lệnh:
```powershell
.\bin\main.exe
```

---

### 6.5. Cách 3: Cấu hình Build trực tiếp trong Visual Studio Code
Nếu bạn sử dụng VS Code, bạn có thể tạo file `.vscode/tasks.json` để biên dịch nhanh bằng phím tắt `Ctrl + Shift + B`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "type": "shell",
            "label": "Build Snake Army 2.0",
            "command": "g++",
            "args": [
                "-std=c++17",
                "src/*.cpp",
                "src/Core/*.cpp",
                "src/Entities/*.cpp",
                "src/DataStructures/*.cpp",
                "src/Items/*.cpp",
                "src/Events/*.cpp",
                "-o",
                "bin/main.exe",
                "-I", "C:/sfml/include",
                "-L", "C:/sfml/lib",
                "-lsfml-graphics",
                "-lsfml-window",
                "-lsfml-system",
                "-lsfml-audio",
                "-mwindows"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": ["$gcc"]
        }
    ]
}
```

---

### 6.6. Xử lý các sự cố thường gặp (Troubleshooting)

1. **Lỗi: `The code execution cannot proceed because sfml-graphics-2.dll was not found`**:
   - **Nguyên nhân:** Windows không tìm thấy các file thư viện động `.dll` của SFML trong PATH hệ thống.
   - **Cách khắc phục:** Copy toàn bộ các file DLL từ thư mục `bin` của SFML (hoặc `C:\msys64\ucrt64\bin\`) vào cùng thư mục `bin/` chứa file `main.exe`:
     - `sfml-graphics-2.dll` (hoặc `libsfml-graphics-2.dll`)
     - `sfml-window-2.dll` (hoặc `libsfml-window-2.dll`)
     - `sfml-system-2.dll` (hoặc `libsfml-system-2.dll`)
     - `sfml-audio-2.dll` (hoặc `libsfml-audio-2.dll`)
     - `openal32.dll` (cần cho module audio)

2. **Lỗi: `Failed to load font "assets/fonts/..."` hoặc màn hình trắng**:
   - **Nguyên nhân:** Chạy game sai working directory nên đường dẫn tương đối `assets/...` bị lệch.
   - **Cách khắc phục:** Đảm bảo cửa sổ dòng lệnh đang đứng ở thư mục gốc của dự án `SnakeArmy` trước khi gọi `.\bin\main.exe`.

3. **Lỗi: `fatal error: SFML/Graphics.hpp: No such file or directory`**:
   - **Nguyên nhân:** Đường dẫn `-I C:\sfml\include` không chính xác hoặc bạn chưa cài đặt SFML.
   - **Cách khắc phục:** Kiểm tra lại chính xác thư mục chứa thư viện SFML trên máy và sửa lại đường dẫn cờ `-I` và `-L`.

---

##  7. Phân công vai trò nhóm phát triển

| STT | Thành viên | Nhiệm vụ chính phụ trách |
| :---: | :--- | :--- |
| **1** | **Nguyễn Hoài Hận** | `GameEngine`, Quản lý vòng lặp Game Loop 60 FPS, Hệ thống màn hình chọn tướng, Xử lý va chạm, Viewport Dynamic Split-screen |
| **2** | **Lê Thị Ngọc Tuyết** | Xây dựng danh sách liên kết đơn `TroopLinkedList`, Thuật toán Follow-the-Leader, Cắt và cướp lính $O(1)$, Xử lý đứt xích bom |
| **3** | **Ngô Thị Ngọc Ánh** | Phát triển `BotPlayer` FSM 8 trạng thái, Thuật toán Context Steering né chướng ngại vật & chống kẹt đá/cây, Thiết kế địa hình Sa trường |
| **4** | **Vũ Bảo An** | Xây dựng hệ thống 3 Hệ Tướng (Tiên Phong, Quân Sư, Hộ Quốc), Bộ kỹ năng Tuyệt Kỹ, Hệ thống Buff vật phẩm (Khiên, Tốc, Nam Châm) |
| **5** | **Nguyễn Đức Đại Dương** | Thiết kế sự kiện sa trường (`DragonEvent`, `BombHazard`, `VipDropEvent`), Tích hợp bộ 8 Sprite UI/UX mới, Hệ thống Âm thanh & Voice Acting |

---
* Snake Army 2.0 – Bản quyền thuộc về Nhóm Phát Triển Dự Án Game C++ / SFML.*
