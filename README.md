# 🎮 ESP32 TFT Gaming Projects (Ball Catcher & Super ESP Mario)

This repository showcases two games built on the **ESP32 Devkit** with an **ST7789 TFT display** — without using an SD card.  
The projects demonstrate how embedded systems can handle **real-time graphics, button inputs, and sound effects** to deliver fun and interactive standalone games.  

---

## 🕹️ Games Included

### 1️⃣ Ball Catcher Game
- Arcade-style game where the player controls a paddle to **catch falling balls**.
- Features:
  - Score tracking  
  - Increasing difficulty (faster ball drops)  
  - Background color changes after milestones  
  - Restart option with button control  
  - Buzzer-based sound effects (catch, miss, game over)  
- Concepts used: **collision detection, random object generation, and game state management**.

---

### 2️⃣ Super ESP Mario
- A **platformer game** inspired by Super Mario, adapted for ESP32 hardware.
- Features:
  - Player (blue character) moves with **left, right, and jump buttons**  
  - Moving platforms for dynamic challenges  
  - Random red enemies for difficulty  
  - Multi-colored power-ups (some placed in air for jumps)  
  - Buzzer sound effects for jumps, collisions, and item collection  
- Concepts used: **sprite rendering, platform physics, enemy AI, and dynamic object handling**.

---

## ⚡ Technical Highlights
- **Real-time 2D rendering** on ST7789 TFT via SPI.  
- **GPIO button controls** for left, right, jump, and restart.  
- **Buzzer integration** for classic arcade-style sound effects.  
- Optimized **game loop & memory management** for smooth gameplay.  

---

## 📚 Learning & Impact
- Showcases how ESP32 can be used beyond IoT, entering **entertainment electronics & retro-style gaming**.  
- Practical exposure to:
  - Graphics programming  
  - Event-driven design  
  - Interrupt handling  
  - Low-level hardware interfacing  
- Reflects creativity in **blending electronics with game design**.  

---

## 🛠️ Hardware Used
- ESP32 Devkit (WROOM-B / Node32)  
- ST7789 TFT Display (SPI)  
- Push buttons (GPIO)  
- Passive buzzer  
- Rechargeable Li-ion battery pack  

---

## 🔌 Wiring Diagram (ESP32 + ST7789 + Buttons + Buzzer)

### 📺 ST7789 TFT Display

- TFT Display to ESP 32 Connections

| TFT_Display            | ESP32                                                                |  
| ----------------- | ------------------------------------------------------------------ |
  |   VCC | 3.3V |
| GND | GND |
|DIN |   Pin 23 |
|CLK| Pin 18  |
|   CS |  Pin 5|
| DC |  Pin 2|
|RST |Pin 4 |
|BL| 3.3V |


### 🔊 Buzzer

- Buzzer to ESP 32 Connections
  
| buzzer            | ESP32                                                                |  
| ----------------- | ------------------------------------------------------------------ |
  |   VCC | Pin 15 |
| GND | GND |



### 🎮 Buttons


- Buttoms to ESP 32 Connections (**Ball Catcher Game**)
  
| Buttoms           | ESP32                                                                |  
| ----------------- | ------------------------------------------------------------------ |
  |  Left  | Pin 12 |
| Right | Pin 14 |
|Reset  | Pin 13    |

- Buttoms to ESP 32 Connections (**Super ESP Mario Game**)
  
| Buttoms           | ESP32                                                                |  
| ----------------- | ------------------------------------------------------------------ |
  |  Left  | Pin 12 |
| Right | Pin 13 |
| Jump  | Pin 14 |
| Reset  | Pin 27    |


## 🌐 Connect with Me

[![GitHub](https://img.shields.io/badge/GitHub-000000?style=for-the-badge&logo=github&logoColor=white)](https://github.com/07-vijayraj)

[![LinkedIn](https://img.shields.io/badge/LinkedIn-0077B5?style=for-the-badge&logo=linkedin&logoColor=white)](https://linkedin.com/in/vijay-raj-a6aa95163)

[![YouTube](https://img.shields.io/badge/YouTube-FF0000?style=for-the-badge&logo=youtube&logoColor=white)](https://youtu.be/y6UMaMx-1bU)

[![Portfolio](https://img.shields.io/badge/Portfolio-FF6F00?style=for-the-badge&logo=firefox&logoColor=white)](https://your-portfolio-link.com)



## 🙌 Author

**Aman Raj**  
_B.Tech in Electronics and Communication Engineering (ECE)_  
_Passionate about Embedded Systems, IoT, and Game Development on Microcontrollers_  
