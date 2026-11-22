#include "src/config/Pins.h"
#include "src/config/Settings.h"
#include "src/config/Secrets.h"
#include "src/net/WifiConnection.h"
#include "src/net/DeviceAuth.h"
#include "src/net/api/ApiServer.h"
#include "src/net/Discovery.h"
#include "src/drivers/LcdDisplay.h"
#include "src/drivers/DhtSensor.h"
#include "src/drivers/RtcClock.h"
#include "src/drivers/Buzzer.h"
#include "src/drivers/Mp3Player.h"
#include "src/drivers/Button.h"
#include "src/storage/AlarmStore.h"
#include "src/ui/DashboardView.h"
#include "src/app/AlarmController.h"

LcdDisplay lcd(Pins::LCD_SDA, Pins::LCD_SCL, 0x27);  // change to 0x3F if the screen stays blank
DhtSensor  dht(Pins::DHT_DATA);
RtcClock   rtc(Pins::RTC_DAT, Pins::RTC_CLK, Pins::RTC_RST);
Buzzer     buzzer(Pins::BUZZER);
Mp3Player  mp3(Serial2, Pins::DFPLAYER_RX, Pins::DFPLAYER_TX);
Button     setButton(Pins::BUTTON_SET);  // hold time: 1.5 s
AlarmStore alarmStore;
WifiConnection wifi(Secrets::WIFI_SSID, Secrets::WIFI_PASSWORD);

DashboardView   dashboard(lcd, rtc, dht);
AlarmController alarmController(lcd, rtc, mp3, buzzer, alarmStore);

DeviceAuth deviceAuth;
ApiServer  api(deviceAuth, rtc, dht, alarmController, Settings::API_PORT);
Discovery  discovery(Settings::API_PORT);

IComponent* components[] = { &dht, &rtc, &buzzer, &mp3, &setButton, &alarmStore, &deviceAuth };
IComponent* netComponents[] = { &api, &discovery };  // started after WiFi

void showMessage(const char* top, const char* bottom) {
  lcd.printLine(0, top);
  lcd.printLine(1, bottom);
  delay(Settings::MESSAGE_TIME_MS);
}

void initComponents() {
  lcd.begin();
  showMessage("Starting...", "");

  for (IComponent* component : components) {
    bool ok = component->begin();
    Serial.printf("[INIT] %-8s %s\n", component->name(), ok ? "OK" : "FAILED");
    if (!ok) showMessage(component->name(), "FAILED");
  }
}

void initWifi() {
  showMessage("Connecting WiFi", Secrets::WIFI_SSID);
  bool ok = wifi.begin();
  Serial.printf("[INIT] %-8s %s\n", wifi.name(), ok ? "OK" : "FAILED");
  if (ok) showMessage(Secrets::WIFI_SSID, wifi.localIP().toString().c_str());
  else    showMessage("WiFi", "FAILED");

  // Started even if WiFi failed: auto-reconnect brings the network up later.
  for (IComponent* component : netComponents) {
    bool started = component->begin();
    Serial.printf("[INIT] %-8s %s\n", component->name(), started ? "OK" : "FAILED");
  }
}

// Test commands typed in the Serial Monitor:
//   p = play track 1   r = ring alarm now   s = stop   i = DFPlayer info   t = RTC time
void handleSerialCommand() {
  if (!Serial.available()) return;
  switch (Serial.read()) {
    case 'p': mp3.setVolume(Settings::ALARM_VOLUME); mp3.play(Settings::ALARM_TRACK); break;
    case 'r': alarmController.ringNow();      break;
    case 's': mp3.stop();                     break;
    case 'i': mp3.printInfo();                break;
    case 't': {
      RtcDateTime dateTime = rtc.now();
      Serial.printf("[RTC] %04u-%02u-%02u %02u:%02u:%02u%s\n", dateTime.Year(), dateTime.Month(), dateTime.Day(),
                    dateTime.Hour(), dateTime.Minute(), dateTime.Second(), dateTime.IsValid() ? "" : " (invalid)");
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
  initWifi();
  alarmController.begin();
  lcd.clear();
}

void loop() {
  // Button and alarm are serviced on every pass so presses are never missed.
  alarmController.handle(setButton.poll());
  alarmController.update();
  mp3.poll();
  api.poll();
  handleSerialCommand();

  static unsigned long lastRefresh = 0;
  static bool needRefresh = true;

  if (alarmController.ownsDisplay()) {
    needRefresh = true;  // redraw the dashboard as soon as the alarm screen closes
    return;
  }

  if (needRefresh || millis() - lastRefresh >= Settings::REFRESH_INTERVAL_MS) {
    needRefresh = false;
    lastRefresh = millis();
    dashboard.render();
  }
}
