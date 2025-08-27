#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <EEPROM.h>

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define LEFT_BUTTON 12
#define RIGHT_BUTTON 14
#define RESTART_BUTTON 13
#define BUZZER_PIN 15
#define MUTE_BUTTON 27

int playerX = 120;
int playerY = 310;
int playerWidth = 30;
int playerHeight = 10;
int playerSpeed = 5;
int lives = 3;
int score = 0;
int highScore = 0;
bool gameOver = false;
bool muted = false;

#define EEPROM_ADDR 0
#define MAX_OBJECTS 3
#define OBJECT_SIZE 10

int objectX[MAX_OBJECTS];
int objectY[MAX_OBJECTS];
int objectType[MAX_OBJECTS];
int activeObjects = 1;

unsigned long lastToneTime = 0;
int toneIndex = 0;
// int bgTune[] = {262, 294, 330, 392, 330, 294, 262, 0};
// int bgDuration = 150;
int bgTune[] = {
  659, 784, 880, 988, 1047, 988, 880, 784, 659, 140,
  392, 440, 494, 523, 587, 659, 698, 784,
  180, 880, 784, 698, 659, 587, 659, 698, 784,
  880, 784, 698, 659, 587, 659, 523, 494,
  659, 698, 659, 523, 587, 494, 140,
  523, 659, 784, 698, 659, 523,
  587, 494, 659, 698, 784, 880,
  523, 587, 659, 698, 784, 880, 988,
  880, 784, 698, 659, 587, 523,
  659, 523, 659, 698, 880, 0
};
int bgDuration = 140;

void beep(int freq, int duration) {
  if (muted) return;
  tone(BUZZER_PIN, freq, duration);
  delay(duration);
  noTone(BUZZER_PIN);
}

void playBGMusic() {
  if (muted || millis() - lastToneTime < bgDuration) return;
  tone(BUZZER_PIN, bgTune[toneIndex], bgDuration);
  lastToneTime = millis();
  toneIndex = (toneIndex + 1) % (sizeof(bgTune) / sizeof(int));
}

void stopMusic() {
  noTone(BUZZER_PIN);
}

void drawHeart(int x, int y, uint16_t color) {
  tft.fillCircle(x + 3, y + 3, 3, color);
  tft.fillCircle(x + 9, y + 3, 3, color);
  tft.fillTriangle(x, y + 4, x + 12, y + 4, x + 6, y + 12, color);
}

void showMainMenu() {
  tft.setRotation(0);
  EEPROM.get(EEPROM_ADDR, highScore);
  if (highScore > 9999 || highScore < 0) highScore = 0;
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(3);
  tft.setCursor(20, 80);
  tft.println("BOLL CATCH ");

  tft.setTextSize(2);
  tft.setCursor(30, 140);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Press any button");

  tft.setCursor(50, 170);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("High Score: ");
  tft.print(highScore);

  while (digitalRead(LEFT_BUTTON) == HIGH && digitalRead(RIGHT_BUTTON) == HIGH && digitalRead(RESTART_BUTTON) == HIGH) {
    playBGMusic();
    delay(10);
  }

  delay(300);
  startGame();
}

void startGame() {
  score = 0;
  lives = 3;
  gameOver = false;
  playerWidth = 30;
  playerX = 120;
  toneIndex = 0;
  tft.fillScreen(ST77XX_BLACK);
  spawnInitialObjects();
}

void spawnInitialObjects() {
  for (int i = 0; i < MAX_OBJECTS; i++) {
    objectY[i] = random(-100 * i, 0);
    bool valid = false;
    while (!valid) {
      objectX[i] = random(0, 240 - OBJECT_SIZE);
      valid = true;
      for (int j = 0; j < i; j++) {
        if (abs(objectX[i] - objectX[j]) < 30) {
          valid = false;
          break;
        }
      }
    }
    //     int randVal = random(100);
    // if (randVal < 60) objectType[i] = 0;              // Normal white ball
    // else if (randVal < 70) objectType[i] = 1;         // Extra life
    // else if (randVal < 80) objectType[i] = 2;         // +5 points
    // else if (randVal < 90) objectType[i] = 3;         // -1 life
    // else if (randVal < 95) objectType[i] = 4;         // Bigger player
    // else if (randVal < 98) objectType[i] = 5;         // Smaller player
    // else objectType[i] = 6;                           // Bomb (Game over)

    int randVal = random(100);
    if (randVal < 70) objectType[i] = 0;       // White normal
    else if (randVal < 73) objectType[i] = 1;  // Extra life (less)
    else if (randVal < 76) objectType[i] = 2;  // +5 points (less)
    else if (randVal < 86) objectType[i] = 3;  // -1 life
    else if (randVal < 88) objectType[i] = 4;  // Bigger player (rare)
    else if (randVal < 90) objectType[i] = 5;  // Smaller player (rare)
    else objectType[i] = 6;                    // Red bomb
  }
}

void drawPlayer() {
  tft.fillRect(playerX, playerY, playerWidth, playerHeight, ST77XX_GREEN);
}

void drawObject(int i) {
  uint16_t color;
  switch (objectType[i]) {
    case 0: color = ST77XX_WHITE; break;
    case 1: color = ST77XX_MAGENTA; break;
    case 2: color = ST77XX_YELLOW; break;
    case 3: color = ST77XX_ORANGE; break;
    case 4: color = ST77XX_BLUE; break;
    case 5: color = ST77XX_CYAN; break;
    case 6: color = ST77XX_RED; break;
  }
  tft.fillCircle(objectX[i] + OBJECT_SIZE / 2, objectY[i] + OBJECT_SIZE / 2, OBJECT_SIZE / 2, color);
}

void drawScoreAndLives() {
  tft.fillRect(0, 0, 240, 20, ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("S:");
  tft.print(score);

  for (int i = 0; i < lives; i++) {
    drawHeart(100 + i * 16, 2, ST77XX_RED);
  }
}

void resetObject(int i) {
  objectY[i] = random(-100, 0);
  bool valid = false;
  while (!valid) {
    objectX[i] = random(0, 240 - OBJECT_SIZE);
    valid = true;
    for (int j = 0; j < MAX_OBJECTS; j++) {
      if (j != i && abs(objectX[i] - objectX[j]) < 30) {
        valid = false;
        break;
      }
    }
  }


  int randVal = random(100);
  if (randVal < 60) objectType[i] = 0;       // Normal white ball
  else if (randVal < 70) objectType[i] = 1;  // Extra life
  else if (randVal < 80) objectType[i] = 2;  // +5 points
  else if (randVal < 90) objectType[i] = 3;  // -1 life
  else if (randVal < 95) objectType[i] = 4;  // Bigger player
  else if (randVal < 98) objectType[i] = 5;  // Smaller player
  else objectType[i] = 6;
}

void explosionEffect() {
  int centerX = playerX + playerWidth / 2;
  int centerY = playerY;
  for (int r = 4; r <= 50; r += 2) {
    tft.fillCircle(centerX, centerY, r, ST77XX_RED);
    if (!muted) tone(BUZZER_PIN, 200, 100);
    delay(500);
    tft.fillCircle(centerX, centerY, r, ST77XX_BLACK);
    delay(300);
  }
}

// void applyObjectEffect(int type) {
//   switch (type) {
//     case 0: score++; beep(1000, 100); break;
//     case 1: if (lives < 5) lives++; beep(1500, 100); break;
//     case 2: score += 5; beep(1200, 100); break;
//     case 3: lives--; beep(300, 200); if (lives <= 0) gameOver = true; break;
//     case 4: playerWidth = min(playerWidth + 10, 50); beep(1600, 100); break;
//     case 5: playerWidth = max(playerWidth - 10, 15); beep(400, 100); break;
//     case 6: explosionEffect(); gameOver = true; break;
//   }
// }
void applyObjectEffect(int type) {
  switch (type) {
    case 0:
      score++;
      beep(1000, 100);
      break;

    case 1:
      if (lives < 5) lives++;
      // 🎵 Happy sound for gaining a life
      beep(1800, 80);
      delay(50);
      beep(2200, 80);
      break;

    case 2:
      score += 2;
      beep(1200, 100);
      break;

    case 3:
      lives--;
      beep(300, 200);
      if (lives <= 0) gameOver = true;
      break;

    case 4:
      playerWidth = min(playerWidth + 10, 50);
      beep(1600, 100);
      break;

    case 5:
      playerWidth = max(playerWidth - 10, 15);
      beep(400, 100);
      break;

    case 6:  // 💣 Red bomb (game over) with sad tune
      explosionEffect();
      if (!muted) {
        int sadTune[] = { 440, 392, 349, 330, 294, 262, 0 };
        for (int i = 0; i < 7; i++) {
          tone(BUZZER_PIN, sadTune[i], 200);
          delay(200);
        }
      }
      gameOver = true;
      break;
  }
}



void loop() {
  if (gameOver) {
    playSadTune();  // 🎵 Play sad music at game over
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(3);
    tft.setCursor(60, 100);
    tft.println("GAME OVER");

    tft.setTextSize(2);
    tft.setCursor(60, 150);
    tft.setTextColor(ST77XX_WHITE);
    tft.print("Score: ");
    tft.print(score);

    if (score > highScore) {
      highScore = score;
      EEPROM.put(EEPROM_ADDR, highScore);
    }

    tft.setCursor(60, 180);
    tft.print("High: ");
    tft.print(highScore);
    stopMusic();

    while (digitalRead(RESTART_BUTTON) == HIGH) delay(10);
    delay(300);
    showMainMenu();
    return;
  }

  tft.fillRect(playerX, playerY, playerWidth, playerHeight, ST77XX_BLACK);

  if (digitalRead(LEFT_BUTTON) == LOW && playerX > 0) playerX -= playerSpeed;
  if (digitalRead(RIGHT_BUTTON) == LOW && playerX < (240 - playerWidth)) playerX += playerSpeed;

  if (digitalRead(MUTE_BUTTON) == LOW) {
    muted = !muted;
    delay(300);
  }

  // if (score > 10) {
  //   activeObjects = 2;
  //   playerSpeed = 6;
  // }
  // if (score > 20) activeObjects = 3;
  if (score > 30) activeObjects = 3;
  else if (score > 20) activeObjects = 3;
  else if (score > 10) activeObjects = 2;
  else activeObjects = 1;

  for (int i = 0; i < activeObjects; i++) {
    tft.fillCircle(objectX[i] + OBJECT_SIZE / 2, objectY[i] + OBJECT_SIZE / 2, OBJECT_SIZE / 2, ST77XX_BLACK);
    objectY[i] += 3;
    ;

    if (objectY[i] > 320) {
      if (objectType[i] == 0 || objectType[i] == 2 || objectType[i] == 1) {
        lives--;
        if (lives <= 0) gameOver = true;
      }
      resetObject(i);
      continue;
    }

    if (objectY[i] + OBJECT_SIZE >= playerY && objectY[i] <= playerY + playerHeight && objectX[i] + OBJECT_SIZE >= playerX && objectX[i] <= playerX + playerWidth) {
      applyObjectEffect(objectType[i]);
      resetObject(i);
    }

    drawObject(i);
  }

  drawPlayer();
  drawScoreAndLives();
  playBGMusic();
  delay(30);
}

void setup() {
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(RESTART_BUTTON, INPUT_PULLUP);
  pinMode(MUTE_BUTTON, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  EEPROM.begin(10);
  tft.init(240, 320);
  tft.setRotation(0);
  showMainMenu();
}
void playSadTune() {
  if (!muted) {
    int sadTune[] = { 440, 392, 349, 330, 294, 262, 0 };
    for (int i = 0; i < 7; i++) {
      tone(BUZZER_PIN, sadTune[i], 200);
      delay(200);
    }
  }
}
