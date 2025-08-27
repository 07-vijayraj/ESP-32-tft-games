#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <SD.h>


#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     2

#define BUZZER_PIN 15 
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define BTN_LEFT   12
#define BTN_RIGHT  13
#define BTN_JUMP   14
#define BTN_RESET  27

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

enum GameState { MENU, PLAYING, GAME_OVER, LEVEL_COMPLETE };
GameState gameState = MENU;

int marioX = 40, marioY = 240;
int velocityY = 0;
bool isJumping = false;
bool hasPowerup = false;
bool menuSoundPlayed = false;
int score = 0;

const int melody[] = { 262, 294, 330, 349, 392, 440, 494, 523 }; // C major scale
const int noteDurations[] = { 200, 200, 200, 200, 200, 200, 200, 400 };
const int melodyLength = sizeof(melody) / sizeof(melody[0]);

int currentNote = 0;
unsigned long lastNoteTime = 0;
bool isMenuMusicPlaying = false;

struct Enemy {
  int x;
  bool alive;
};
#define MAX_GOOMBAS 5
Enemy goombas[MAX_GOOMBAS];
int goombaCount = 3;

#define MAX_POWERUPS 3
int mushroomX[MAX_POWERUPS];
bool mushroomVisible[MAX_POWERUPS];

const int POWERUP_WIDTH = 16;
const int GROUND_Y = 256;
const int GRAVITY = 2;
const int JUMP_POWER = -20;
const int MARIO_WIDTH = 24;
const int MARIO_HEIGHT = 32;
const int ENEMY_WIDTH = 16;
const int ENEMY_HEIGHT = 16;
const int GROUND_HEIGHT = 16;
const int TILE_WIDTH = 16;

#define LEVEL_COUNT 3
const char* levelData[LEVEL_COUNT] = {
  "11111001101111110011001111011011111110011111101111011",
  "111101111011100111001111011011110110111101101111011111",
  "1111110111011100111001110101011100111010110110101101011"
};
int currentLevel = 0;
uint8_t levelMap[60];
const int LEVEL_LENGTH = 60;
int cameraX = 0;

#define MAX_PLATFORMS 3
int platformX[MAX_PLATFORMS] = {100, 180, 260};
int platformY[MAX_PLATFORMS] = {GROUND_Y - 60, GROUND_Y - 90, GROUND_Y - 45};
bool platformDirRight[MAX_PLATFORMS] = {true, false, true};
int platformWidth = 40;

const uint16_t marioColor = ST77XX_RED;
const uint16_t groundColor = ST77XX_GREEN;
const uint16_t enemyColor = ST77XX_YELLOW;
const uint16_t powerupColor = ST77XX_MAGENTA;
const uint16_t platformColor = ST77XX_CYAN;

void playTone(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
}
void playMenuMusic() {
  unsigned long now = millis();
  if (now - lastNoteTime >= noteDurations[currentNote]) {
    tone(BUZZER_PIN, melody[currentNote], noteDurations[currentNote]);
    lastNoteTime = now;
    currentNote = (currentNote + 1) % melodyLength;
  }
}

void drawScore() {
  tft.setCursor(5, 5);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.print("Score: ");
  tft.print(score);
}

void drawMario() {
  int drawX = marioX - cameraX;
  tft.fillRect(drawX + 6, marioY, 12, 12, ST77XX_YELLOW);
  tft.fillRect(drawX + 8, marioY + 12, 8, 12, hasPowerup ? ST77XX_BLUE : marioColor);
  tft.drawPixel(drawX + 5, marioY + 14, ST77XX_RED);
  tft.drawPixel(drawX + 17, marioY + 14, ST77XX_RED);
  tft.fillRect(drawX + 7, marioY + 24, 4, 6, ST77XX_WHITE);
  tft.fillRect(drawX + 11, marioY + 24, 4, 6, ST77XX_WHITE);
}

void drawEnemies() {
  for (int i = 0; i < goombaCount; i++) {
    if (goombas[i].alive) {
      int drawX = goombas[i].x - cameraX;
      tft.fillRect(drawX, GROUND_Y - ENEMY_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT, enemyColor);
    }
  }
}

void drawPowerup() {
  for (int i = 0; i < MAX_POWERUPS; i++) {
    if (mushroomVisible[i]) {
      int drawX = mushroomX[i] - cameraX;
      tft.fillRect(drawX, GROUND_Y - 80, POWERUP_WIDTH, POWERUP_WIDTH, powerupColor);
    }
  }
}

void drawGround() {
  for (int i = 0; i < LEVEL_LENGTH; i++) {
    if (levelMap[i] == 1) {
      int tileX = i * TILE_WIDTH - cameraX;
      tft.fillRect(tileX, GROUND_Y, TILE_WIDTH, GROUND_HEIGHT, groundColor);
    }
  }
}

void drawPlatform() {
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    int drawX = platformX[i] - cameraX;
    tft.fillRect(drawX, platformY[i], platformWidth, 8, platformColor);
  }
}

void updatePlatform() {
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (platformDirRight[i]) platformX[i] += 2;
    else platformX[i] -= 2;
    if (platformX[i] < 40 + i * 30) platformDirRight[i] = true;
    if (platformX[i] > 200 + i * 20) platformDirRight[i] = false;
  }
}

void loadLevel(int level) {
  const char* data = levelData[level];
  for (int i = 0; i < LEVEL_LENGTH; i++) levelMap[i] = data[i] - '0';
  randomSeed(millis());
  goombaCount = random(1, MAX_GOOMBAS + 1);
  for (int i = 0; i < goombaCount; i++) {
    goombas[i].x = random(120, LEVEL_LENGTH * TILE_WIDTH - 40);
    goombas[i].alive = true;
  }
  for (int i = 0; i < MAX_POWERUPS; i++) {
    mushroomX[i] = random(100 + i * 60, LEVEL_LENGTH * TILE_WIDTH - 40);
    mushroomVisible[i] = true;
  }
}

void resetGame() {
  marioX = 40; marioY = 240;
  velocityY = 0;
  isJumping = false;
  hasPowerup = false;
  cameraX = 0;
  loadLevel(currentLevel);
  tft.fillScreen(ST77XX_BLACK);
}

void fullGameReset() {
  score = 0;
  currentLevel = 0;
  resetGame();
}

void nextLevel() {
  currentLevel++;
  if (currentLevel >= LEVEL_COUNT) currentLevel = 0;
  resetGame();
  gameState = PLAYING;
}

void playMenuSound() {
  tone(BUZZER_PIN, 523, 200);
  delay(200);
  tone(BUZZER_PIN, 659, 200);
  delay(200);
  noTone(BUZZER_PIN);
}


void setup() {
  Serial.begin(115200);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_JUMP, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  tft.init(240, 320);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  fullGameReset();
  playMenuSound();
}

void loop() {
  static int lastTile = -1;

  // ---------------- MENU STATE ----------------
  if (gameState == MENU) {
    if (!isMenuMusicPlaying) {
      currentNote = 0;
      lastNoteTime = millis();
      isMenuMusicPlaying = true;
    }
    playMenuMusic();

    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(10, 100);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(3);
    tft.println("Super ");
    tft.setCursor(110, 105);
    tft.setTextColor(ST77XX_BLUE);
    tft.setTextSize(2);
    tft.println("ESP-Mario");
    tft.setCursor(10, 160);
    tft.setTextColor(ST77XX_WHITE);
    tft.println("Press Jump or RESET");
    tft.setCursor(10, 185);
    tft.println("to Start");

    if (digitalRead(BTN_JUMP) == LOW || digitalRead(BTN_RESET) == LOW) {
      noTone(BUZZER_PIN);
      isMenuMusicPlaying = false;
      gameState = PLAYING;
      delay(300);
    }
    return; // stay in menu until button pressed
  }

  // ---------------- GAME OVER STATE ----------------
  if (gameState == GAME_OVER) {
    menuSoundPlayed = false;
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(50, 100);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(3);
    tft.println("Game Over");
    tft.setCursor(20, 160);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(2);
    tft.println("Press RESET to Return");

    if (digitalRead(BTN_RESET) == LOW) {
      fullGameReset();
      gameState = MENU;
      delay(300);
    }
    return;
  }

  // ---------------- LEVEL COMPLETE STATE ----------------
  if (gameState == LEVEL_COMPLETE) {
    static unsigned long levelClearStart = 0;
    if (levelClearStart == 0) {
      levelClearStart = millis();
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(20, 100);
      tft.setTextColor(ST77XX_MAGENTA);
      tft.setTextSize(3);
      tft.println("Level Clear!");
      tft.setCursor(20, 140);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.println("Get ready for next level!");
    }
    if (millis() - levelClearStart > 2000) {
      levelClearStart = 0;
      nextLevel();
    }
    return;
  }

  // ---------------- PLAYING STATE ----------------
  tft.fillScreen(ST77XX_BLACK);

  // Mario movement
  if (digitalRead(BTN_LEFT) == LOW && marioX > 0) marioX -= 3;
  if (digitalRead(BTN_RIGHT) == LOW && marioX < LEVEL_LENGTH * TILE_WIDTH - MARIO_WIDTH) marioX += 3;
  if (!isJumping && digitalRead(BTN_JUMP) == LOW) {
    velocityY = JUMP_POWER;
    isJumping = true;
    playTone(800, 100);
  }

  // Gravity & jump
  velocityY += GRAVITY;
  marioY += velocityY;

  updatePlatform();

  // Collision with ground
  int tileIndex = (marioX + MARIO_WIDTH / 2) / TILE_WIDTH;
  if (tileIndex < LEVEL_LENGTH && levelMap[tileIndex] == 1 && marioY + MARIO_HEIGHT >= GROUND_Y) {
    marioY = GROUND_Y - MARIO_HEIGHT;
    velocityY = 0;
    isJumping = false;
  }

  // Collision with platforms
  for (int i = 0; i < MAX_PLATFORMS; i++) {
    if (marioY + MARIO_HEIGHT >= platformY[i] && marioY + MARIO_HEIGHT <= platformY[i] + 6 &&
        marioX + MARIO_WIDTH > platformX[i] && marioX < platformX[i] + platformWidth) {
      marioY = platformY[i] - MARIO_HEIGHT;
      velocityY = 0;
      isJumping = false;
    }
  }

  // Score on air jump
  if (tileIndex != lastTile && levelMap[tileIndex] == 0 && isJumping) {
    score++;
    lastTile = tileIndex;
  }

  // Power-up collection
  for (int i = 0; i < MAX_POWERUPS; i++) {
    if (mushroomVisible[i] && abs(marioX - mushroomX[i]) < 16) {
      mushroomVisible[i] = false;
      hasPowerup = true;
      score += 5;
      playTone(1200, 150);
    }
  }

  // Enemy collisions
  for (int i = 0; i < goombaCount; i++) {
    if (goombas[i].alive && abs(marioX - goombas[i].x) < 16 && abs(marioY - (GROUND_Y - ENEMY_HEIGHT)) < 16) {
      if (hasPowerup) {
        goombas[i].alive = false;
        hasPowerup = false;
        score += 10;
        playTone(600, 80);
      } else {
        gameState = GAME_OVER;
        playTone(400, 300);
        delay(300);
        tone(BUZZER_PIN, 262, 200);
        delay(200);
        tone(BUZZER_PIN, 196, 200);
        delay(200);
        tone(BUZZER_PIN, 164, 200);
        delay(200);
        tone(BUZZER_PIN, 131, 400);
        delay(400);
        noTone(BUZZER_PIN);
      }
    }
  }

  // Fall off screen
  if (marioY > SCREEN_HEIGHT) {
    gameState = GAME_OVER;
    playTone(300, 300);
  }

  // Level finish
  if (marioX > LEVEL_LENGTH * TILE_WIDTH - 32) {
    score += 20; 
    gameState = LEVEL_COMPLETE;
    playTone(1000, 300);
  }

  // Camera follow
  cameraX = marioX - SCREEN_WIDTH / 2;
  if (cameraX < 0) cameraX = 0;
  if (cameraX > LEVEL_LENGTH * TILE_WIDTH - SCREEN_WIDTH) cameraX = LEVEL_LENGTH * TILE_WIDTH - SCREEN_WIDTH;

  // Draw all objects
  drawGround();
  drawMario();
  drawPowerup();
  drawEnemies();
  drawPlatform();
  drawScore();

  delay(30);
}
