#include "src/config/Pins.h"
#include "src/drivers/LcdDisplay.h"
#include "src/drivers/DhtSensor.h"
#include "src/drivers/RtcClock.h"
#include "src/drivers/Buzzer.h"
#include "src/drivers/Mp3Player.h"
#include "src/drivers/Button.h"
#include "src/storage/AlarmStore.h"
#include "src/ui/DashboardView.h"
#include "src/app/AlarmController.h"

constexpr unsigned long REFRESH_INTERVAL_MS = 2000;  // DHT11 needs >= 1 s between reads
constexpr unsigned long MESSAGE_TIME_MS     = 1500;

LcdDisplay lcd(Pins::LCD_SDA, Pins::LCD_SCL, 0x27);  // change to 0x3F if the screen stays blank
DhtSensor  dht(Pins::DHT_DATA);
RtcClock   rtc(Pins::RTC_DAT, Pins::RTC_CLK, Pins::RTC_RST);
Buzzer     buzzer(Pins::BUZZER);
Mp3Player  mp3(Serial2, Pins::DFPLAYER_RX, Pins::DFPLAYER_TX);
Button     setButton(Pins::BUTTON_SET);  // hold time: 1.5 s
AlarmStore alarmStore;

DashboardView   dashboard(lcd, rtc, dht);
AlarmController alarmController(lcd, rtc, mp3, buzzer, alarmStore);

IComponent* components[] = { &dht, &rtc, &buzzer, &mp3, &setButton, &alarmStore };

void showMessage(const char* top, const char* bottom) {
  lcd.printLine(0, top);
  lcd.printLine(1, bottom);
  delay(MESSAGE_TIME_MS);
}

void initComponents() {
  lcd.begin();
  showMessage("Starting...", "");

  for (IComponent* c : components) {
    bool ok = c->begin();
    Serial.printf("[INIT] %-8s %s\n", c->name(), ok ? "OK" : "FAILED");
    if (!ok) showMessage(c->name(), "FAILED");
  }
}

// Test commands typed in the Serial Monitor:
//   p = play track 1   r = ring alarm now   s = stop   i = DFPlayer info   t = RTC time
void handleSerialCommand() {
  if (!Serial.available()) return;
  switch (Serial.read()) {
    case 'p': mp3.setVolume(20); mp3.play(1); break;
    case 'r': alarmController.ringNow();      break;
    case 's': mp3.stop();                     break;
    case 'i': mp3.printInfo();                break;
    case 't': {
      RtcDateTime t = rtc.now();
      Serial.printf("[RTC] %04u-%02u-%02u %02u:%02u:%02u%s\n", t.Year(), t.Month(), t.Day(),
                    t.Hour(), t.Minute(), t.Second(), t.IsValid() ? "" : " (invalid)");
      break;
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n=== Bedside Room Station ===");
  Serial.println("Commands: p=play, r=ring now, s=stop, i=mp3 info, t=time");
  initComponents();
  alarmController.begin();
  lcd.clear();
}

void loop() {
  // Button and alarm are serviced on every pass so presses are never missed.
  alarmController.handle(setButton.poll());
  alarmController.update();
  mp3.poll();
  handleSerialCommand();

  static unsigned long lastRefresh = 0;
  static bool needRefresh = true;

  if (alarmController.ownsDisplay()) {
    needRefresh = true;  // redraw the dashboard as soon as the alarm screen closes
    return;
  }

  if (needRefresh || millis() - lastRefresh >= REFRESH_INTERVAL_MS) {
    needRefresh = false;
    lastRefresh = millis();
    dashboard.render();
  }
}
