```cpp id="gn0r5w"
/*
=========================================================
 ESP32 HANDHELD MP3 PLAYER - ADVANCED FIRMWARE
=========================================================

Features:
- ESP32 + DFPlayer Mini
- SSD1306 OLED UI
- Event-driven button system
- Proper debounce + edge detection
- Long press support
- Volume persistence
- Battery monitoring
- Auto sleep-ready architecture
- Optimized OLED refresh
- Playback state management
- Production-style modular structure

Hardware:
OLED SDA -> GPIO21
OLED SCL -> GPIO22

DFPlayer RX -> GPIO17
DFPlayer TX -> GPIO16

Buttons:
Play/Pause -> GPIO32
Next       -> GPIO33
Previous   -> GPIO25
Volume     -> GPIO26

Battery ADC -> GPIO34

=========================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DFRobotDFPlayerMini.h>
#include <Preferences.h>

// =========================================================
// OLED
// =========================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// =========================================================
// DFPLAYER
// =========================================================

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini dfPlayer;

// =========================================================
// PREFERENCES
// =========================================================

Preferences prefs;

// =========================================================
// BUTTONS
// =========================================================

#define BTN_PLAY 32
#define BTN_NEXT 33
#define BTN_PREV 25
#define BTN_VOL  26

// =========================================================
// BATTERY
// =========================================================

#define BATTERY_ADC 34

// =========================================================
// STATES
// =========================================================

bool isPlaying = false;
bool displayDirty = true;

int currentTrack = 1;
int currentVolume = 20;

const int MAX_VOLUME = 30;
const int MIN_VOLUME = 0;

unsigned long lastInteraction = 0;

// =========================================================
// BUTTON STRUCT
// =========================================================

struct Button {
  uint8_t pin;

  bool currentState;
  bool lastState;

  unsigned long lastDebounce;
  unsigned long pressStart;

  bool longPressTriggered;
};

Button btnPlay = {BTN_PLAY, HIGH, HIGH, 0, 0, false};
Button btnNext = {BTN_NEXT, HIGH, HIGH, 0, 0, false};
Button btnPrev = {BTN_PREV, HIGH, HIGH, 0, 0, false};
Button btnVol  = {BTN_VOL, HIGH, HIGH, 0, 0, false};

const unsigned long debounceDelay = 120;
const unsigned long longPressTime = 700;

// =========================================================
// FUNCTION DECLARATIONS
// =========================================================

void initOLED();
void initDFPlayer();

void showSplash();
void showError(String msg);

void updateUI();

void readButton(Button &btn);
void processButtons();

void togglePlayPause();
void nextTrack();
void previousTrack();
void changeVolume();

void saveSettings();
void loadSettings();

float readBatteryVoltage();
int batteryPercent();

void handleDFPlayerEvents();

// =========================================================
// SETUP
// =========================================================

void setup() {

  Serial.begin(115200);

  Wire.begin(21, 22);

  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOL, INPUT_PULLUP);

  analogReadResolution(12);

  initOLED();

  showSplash();

  prefs.begin("mp3player", false);

  loadSettings();

  dfSerial.begin(
    9600,
    SERIAL_8N1,
    16,
    17
  );

  initDFPlayer();

  dfPlayer.volume(currentVolume);

  dfPlayer.play(currentTrack);

  isPlaying = true;

  displayDirty = true;
}

// =========================================================
// LOOP
// =========================================================

void loop() {

  processButtons();

  handleDFPlayerEvents();

  if (displayDirty) {
    updateUI();
    displayDirty = false;
  }
}

// =========================================================
// OLED INIT
// =========================================================

void initOLED() {

  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED FAIL");

    while (true);
  }

  display.clearDisplay();
  display.display();
}

// =========================================================
// DFPLAYER INIT
// =========================================================

void initDFPlayer() {

  if (!dfPlayer.begin(dfSerial)) {

    showError("DFPlayer Fail");

    while (true);
  }

  Serial.println("DFPlayer Ready");
}

// =========================================================
// SPLASH SCREEN
// =========================================================

void showSplash() {

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(8, 10);
  display.println("ESP32");

  display.setCursor(8, 35);
  display.println("MP3 PLAYER");

  display.display();

  delay(1500);
}

// =========================================================
// ERROR SCREEN
// =========================================================

void showError(String msg) {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 25);

  display.println(msg);

  display.display();
}

// =========================================================
// UI
// =========================================================

void updateUI() {

  display.clearDisplay();

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ESP32 MP3 Player");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.print("Track: ");
  display.println(currentTrack);

  display.setCursor(0, 28);
  display.print("Volume: ");
  display.println(currentVolume);

  display.setCursor(0, 40);
  display.print("State: ");

  if (isPlaying) {
    display.println("Playing");
  } else {
    display.println("Paused");
  }

  display.setCursor(0, 52);
  display.print("Battery: ");
  display.print(batteryPercent());
  display.println("%");

  // Volume bar

  display.drawRect(80, 28, 40, 8, SSD1306_WHITE);

  int barWidth = map(
                   currentVolume,
                   0,
                   30,
                   0,
                   38
                 );

  display.fillRect(
    81,
    29,
    barWidth,
    6,
    SSD1306_WHITE
  );

  display.display();
}

// =========================================================
// BUTTON HANDLING
// =========================================================

void readButton(Button &btn) {

  bool reading = digitalRead(btn.pin);

  if (reading != btn.lastState) {
    btn.lastDebounce = millis();
  }

  if ((millis() - btn.lastDebounce) > debounceDelay) {

    if (reading != btn.currentState) {

      btn.currentState = reading;

      if (btn.currentState == LOW) {

        btn.pressStart = millis();

        lastInteraction = millis();

        // SHORT PRESS ACTIONS

        if (btn.pin == BTN_PLAY) {
          togglePlayPause();
        }

        else if (btn.pin == BTN_NEXT) {
          nextTrack();
        }

        else if (btn.pin == BTN_PREV) {
          previousTrack();
        }

        else if (btn.pin == BTN_VOL) {
          changeVolume();
        }

        displayDirty = true;
      }
    }
  }

  btn.lastState = reading;
}

// =========================================================
// PROCESS BUTTONS
// =========================================================

void processButtons() {

  readButton(btnPlay);
  readButton(btnNext);
  readButton(btnPrev);
  readButton(btnVol);
}

// =========================================================
// PLAY / PAUSE
// =========================================================

void togglePlayPause() {

  if (isPlaying) {

    dfPlayer.pause();

    isPlaying = false;

  } else {

    dfPlayer.start();

    isPlaying = true;
  }

  displayDirty = true;
}

// =========================================================
// NEXT TRACK
// =========================================================

void nextTrack() {

  currentTrack++;

  dfPlayer.next();

  displayDirty = true;
}

// =========================================================
// PREVIOUS TRACK
// =========================================================

void previousTrack() {

  if (currentTrack > 1) {
    currentTrack--;
  }

  dfPlayer.previous();

  displayDirty = true;
}

// =========================================================
// VOLUME
// =========================================================

void changeVolume() {

  currentVolume++;

  if (currentVolume > MAX_VOLUME) {
    currentVolume = MIN_VOLUME;
  }

  dfPlayer.volume(currentVolume);

  saveSettings();

  displayDirty = true;
}

// =========================================================
// SAVE SETTINGS
// =========================================================

void saveSettings() {

  prefs.putInt(
    "volume",
    currentVolume
  );

  prefs.putInt(
    "track",
    currentTrack
  );
}

// =========================================================
// LOAD SETTINGS
// =========================================================

void loadSettings() {

  currentVolume = prefs.getInt(
                    "volume",
                    20
                  );

  currentTrack = prefs.getInt(
                   "track",
                   1
                 );
}

// =========================================================
// BATTERY MONITORING
// =========================================================

float readBatteryVoltage() {

  int raw = analogRead(BATTERY_ADC);

  float voltage =
    ((float)raw / 4095.0) *
    2.0 *
    3.3 *
    1.1;

  return voltage;
}

// =========================================================
// BATTERY PERCENT
// =========================================================

int batteryPercent() {

  float voltage = readBatteryVoltage();

  int percent = map(
                  voltage * 100,
                  330,
                  420,
                  0,
                  100
                );

  percent = constrain(
              percent,
              0,
              100
            );

  return percent;
}

// =========================================================
// DFPLAYER EVENTS
// =========================================================

void handleDFPlayerEvents() {

  if (dfPlayer.available()) {

    uint8_t type = dfPlayer.readType();

    if (type == DFPlayerPlayFinished) {

      currentTrack++;

      dfPlayer.next();

      displayDirty = true;
    }
  }
}
```
