#include "src/config/Pins.h"
#include "src/drivers/LcdDisplay.h"
#include "src/drivers/DhtSensor.h"
#include "src/drivers/RtcClock.h"
#include "src/drivers/Buzzer.h"
#include "src/drivers/Mp3Player.h"
#include "src/ui/DashboardView.h"

constexpr unsigned long REFRESH_INTERVAL_MS = 2000;  // DHT11 needs >= 1 s between reads
constexpr unsigned long MESSAGE_TIME_MS     = 1500;

LcdDisplay lcd(Pins::LCD_SDA, Pins::LCD_SCL, 0x27);  // change to 0x3F if the screen stays blank
DhtSensor  dht(Pins::DHT_DATA);
RtcClock   rtc(Pins::RTC_DAT, Pins::RTC_CLK, Pins::RTC_RST);
Buzzer     buzzer(Pins::BUZZER);
Mp3Player  mp3(Serial2, Pins::DFPLAYER_RX, Pins::DFPLAYER_TX);

DashboardView dashboard(lcd, rtc, dht);

IComponent* components[] = { &dht, &rtc, &buzzer, &mp3 };

void showMessage(const char* top, const char* bottom) {
  lcd.printLine(0, top);
  lcd.printLine(1, bottom);
  delay(MESSAGE_TIME_MS);
}

void initComponents() {
  lcd.begin();
  showMessage("Starting...", "");

  for (IComponent* c : components) {
    if (!c->begin()) showMessage(c->name(), "FAILED");
  }
}

void testBuzzer() {
  static const Note melody[] = { {262, 200}, {330, 200}, {392, 200}, {523, 300} };
  lcd.printLine(0, "Buzzer test");
  lcd.printLine(1, "");
  buzzer.playMelody(melody, sizeof(melody) / sizeof(melody[0]));
}

void testMp3() {
  mp3.setVolume(20);
  mp3.play(1);
  showMessage("DFPlayer", "Playing track 1");
}

void setup() {
  initComponents();
  testBuzzer();
  testMp3();
  lcd.clear();
}

void loop() {
  static unsigned long lastRefresh = 0;
  static bool firstRun = true;
  if (!firstRun && millis() - lastRefresh < REFRESH_INTERVAL_MS) return;
  firstRun = false;
  lastRefresh = millis();

  dashboard.render();
}
