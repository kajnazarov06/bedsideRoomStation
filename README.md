# Bedside Room Station

A bedside clock and room monitor built with an **ESP32** (or an Arduino UNO) and common hobby modules.
It shows the time, date, room temperature and humidity, wakes you with your own MP3 song and a sunrise light, and works as a night light.

```
┌──────────────────┐
│ 22:46:05  30.09  │
│ Wed  24°C  45%   │
└──────────────────┘
```

## Features

- **Clock:** time, date and weekday from a battery-backed real-time clock (keeps time without power)
- **Room climate:** temperature and humidity from a DHT11 sensor
- **Alarms:** up to 5 alarms, each with its own weekdays (every day, Mon–Fri, Sat–Sun or any single day); plays an MP3 from a microSD card, with snooze
- **Sunrise light:** the LED ring slowly brightens 15 minutes before the alarm
- **Night light:** turns on automatically when the room gets dark; clap to switch it on or off
- **Room report:** tells you when the room is too dry, too humid or too warm
- **Settings survive power cuts:** alarm times, days, volume and on/off are saved in flash memory

## Hardware

| Part | Notes |
|---|---|
| ESP32 DevKit V1 (ESP32-WROOM-32) | main board; an Arduino UNO also works (see [UNO pins](#arduino-uno-pins)) |
| ESP32 screw terminal / expansion board | optional, makes wiring easier |
| LCD 1602 with I2C backpack | address usually `0x27` (sometimes `0x3F`) |
| DS1302 RTC module + CR2032 coin cell | keeps the time |
| DHT11 temperature and humidity module | 3-pin board version |
| DFPlayer Mini + microSD card | MP3 player |
| Speaker, 8 Ω, 2–3 W (about 40 mm) | a real speaker, **not** a buzzer |
| WS2812 RGB LED ring, 8 LEDs | sunrise and night light |
| Light sensor module (photoresistor) | analog output |
| Sound sensor module (microphone) | digital output, for clap control |
| Potentiometer (10 kΩ) | night-light brightness |
| Passive buzzer | button beeps |
| 3 push buttons (12 mm, momentary) | SET, SNOOZE, LIGHT |
| Resistors: 2 × 1 kΩ, 1 × 330 Ω | DFPlayer serial lines, LED ring data line |
| 5V USB power supply (1 A or more) | |

## Wiring (ESP32)

> ⚠️ ESP32 pins work at **3.3 V**. Never connect a 5 V signal to a GPIO.
> Power the sensors and the RTC from **3V3**; power the LCD, DFPlayer and LED ring from **5V (VIN)**.
> On an expansion board with **S / V / G** pin rows, check the V-row jumper: with it on **3V**, the V pins give 3.3 V.

| Part | Part pin | ESP32 pin |
|---|---|---|
| LCD (I2C) | SDA / SCL | GPIO 21 / GPIO 22 |
| | VCC | 5V (VIN) |
| RTC DS1302 | DAT / CLK / RST | GPIO 25 / 26 / 27 |
| | VCC | **3V3** |
| DHT11 | S (data) | GPIO 4 |
| | middle pin (+) | **3V3** |
| DFPlayer Mini | TX (pin 3) | GPIO 16 (RX2), through 1 kΩ |
| | RX (pin 2) | GPIO 17 (TX2), through 1 kΩ |
| | VCC (pin 1) | 5V (VIN) |
| | SPK1 / SPK2 (pins 6, 8) | speaker |
| LED ring WS2812 | DIN | GPIO 13, through 330 Ω |
| | VCC | 5V (VIN) |
| Light sensor | AO | GPIO 34 |
| Sound sensor | DO | GPIO 35 |
| Potentiometer | middle leg | GPIO 32 (outer legs to 3V3 and GND) |
| Buzzer | + | GPIO 33 |
| Button SET | one leg | GPIO 18 |
| Button SNOOZE | one leg | GPIO 19 |
| Button LIGHT | one leg | GPIO 23 |

All GND pins go to a common GND. Each button connects its GPIO to GND (internal pull-ups are used, so no resistors are needed). Use **diagonally opposite legs** of each 4-leg button.

<details>
<summary id="arduino-uno-pins">Arduino UNO pins</summary>

| Part | Part pin | UNO pin |
|---|---|---|
| LCD (I2C) | SDA / SCL | A4 / A5 |
| RTC DS1302 | DAT / CLK / RST | 4 / 5 / 2 |
| DHT11 | S | 7 |
| DFPlayer Mini | TX / RX (through 1 kΩ) | 10 / 11 (SoftwareSerial) |
| LED ring | DIN (through 330 Ω) | 6 |
| Light sensor | AO | A0 |
| Sound sensor | DO | 3 |
| Potentiometer | middle leg | A1 |
| Buzzer | + | 8 |
| Buttons SET / SNOOZE / LIGHT | one leg each | 9 / 12 / A2 |

On the UNO, everything is powered from 5V.
</details>

## Controls

**Press** = short tap. **Hold** = keep pressed for 1.5 seconds.

| Control | On the clock screen | While setting the alarm | While the alarm rings or snoozes |
|---|---|---|---|
| **SET** | Press: show alarm 1; press again for alarms 2–5. Hold: start setting the alarm on screen | Press: next value (hour → minute → days → volume → on/off), then save | Press: stop the alarm until tomorrow |
| **SNOOZE** | Press: room report | Press: value − 1 (hold: fast) | Press: snooze 9 minutes. Hold while snoozing: cancel |
| **LIGHT** | Press: night light on/off. Hold: back to automatic | Press: value + 1 (hold: fast) | Press: stop the alarm |
| **Knob** | night-light brightness | not used | not used |
| **Clap** | night light on/off | ignored | ignored |

- In a dark room the screen light turns off after 15 s; the first press of any button only wakes the screen.
- Setting the alarm cancels itself after 20 s without a press.
- The alarm stops by itself after 10 minutes.

## Installation

### 1. Software

1. Install the [Arduino IDE 2](https://www.arduino.cc/en/software).
2. Add ESP32 support: **Tools → Board → Boards Manager**, search **esp32**, install **esp32 by Espressif Systems** (version **3.x**).
   If it isn't listed, add this URL under **File → Preferences → Additional boards manager URLs**:
   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`
3. Install these libraries with **Tools → Manage Libraries**:

| Library | Author |
|---|---|
| LiquidCrystal I2C | Frank de Brabander |
| Rtc by Makuna | Michael C. Miller |
| DHT sensor library | Adafruit (also install *Adafruit Unified Sensor* when asked) |
| DFRobotDFPlayerMini | DFRobot |
| Adafruit NeoPixel | Adafruit |
| ArduinoJson (7.x) | Benoit Blanchon |

### 2. microSD card

1. Format the card as **FAT32** (32 GB or smaller).
2. Copy your alarm song to the main folder as **`0001.mp3`**.
   It must be a real MP3 (44.1 kHz, 128 kbps is safe). To convert any audio file:

   ```bash
   ffmpeg -i yoursong.m4a -ar 44100 -ac 2 -b:a 128k -map_metadata -1 -id3v2_version 0 0001.mp3
   ```

### 3. Upload

1. Open `bedside_station/bedside_station.ino`.
2. Check the pin numbers at the top of the sketch match your wiring.
3. Select **Tools → Board → esp32 → ESP32 Dev Module** and your port.
4. Click **Upload**. If it stops at `Connecting.....`, hold the **BOOT** button on the ESP32 until the upload starts.

The clock is set automatically from your computer's time during the first upload.

## Updating an existing station

1. Connect the station to your computer with a USB cable. You don't need to open the box or change any wiring.
2. Pull the latest version of this repository.
3. Compare the pin numbers at the top of the new sketch with your station's wiring, and change them if your station is wired differently.
4. Upload as in [Installation → Upload](#3-upload).

Your alarm settings and the clock's time are kept: the time lives in the RTC (with its coin cell), and the settings in the ESP32's flash.
Keep **Tools → Erase All Flash Before Sketch Upload** set to **Disabled**, or the saved alarm settings are erased.

## Checking a station

Use these checks after building, after an update, or when something seems wrong.

1. **Screen:** after power-on, the LCD shows the time and date on line 1, and weekday, temperature and humidity on line 2. `--` for temperature only appears for the first 2 seconds.
2. **Clock:** compare the time with your phone. Unplug the station for a minute, plug it back in: the time should still be right.
3. **Buttons:** press SET once: the alarm time appears for 3 seconds. Press LIGHT: the night light toggles.
4. **Sound:** set the alarm 2 minutes ahead and wait. The ring brightens and the song plays.
5. **Sensors:** cover the light sensor: the night light turns on (in automatic mode). Clap: it toggles.

To test one part on its own, upload the matching sketch from the `tests/` folder and open the **Serial Monitor at 9600 baud**.

## Troubleshooting

| Problem | Fix |
|---|---|
| LCD lit but empty | turn the blue contrast screw on the LCD's back |
| LCD completely dark | the backlight jumper on the I2C board must be in place |
| LCD shows nothing at all | run an I2C scanner; change `0x27` to the address it finds (often `0x3F`) |
| `RTC error` or impossible times | check DAT, CLK, RST wiring and the coin cell; power the RTC from 3V3 on ESP32 |
| Temperature shows `--` | DHT11 middle pin is VCC (+); check it is connected |
| `DFPlayer not found` | swap the TX/RX wires; try `player.begin(Serial2, false, true)` |
| No sound from the speaker | use a real 8 Ω speaker on DFPlayer pins 6 and 8 (not a buzzer); check the MP3 format |
| A button always reads "pressed" | wire the button using diagonally opposite legs |
| `SoftwareSerial.h: No such file` | you're compiling UNO code for ESP32: use `Serial2` instead |
| `'tone' was not declared` | update the esp32 board package to 3.x |
| Upload stops at `Connecting...` | hold the BOOT button during upload |

## Repository structure

```
bedside-station/
├── README.md
├── bedside_station/
│   └── bedside_station.ino      main sketch
├── tests/                       one small sketch per part
│   ├── lcd_test/
│   ├── rtc_test/
│   ├── dht11_test/
│   ├── dfplayer_test/
│   ├── buttons_test/
│   └── buzzer_test/
└── docs/
    └── wiring.png
```

## Roadmap

- [ ] Exact time from the internet (NTP) over WiFi, with the RTC as backup
- [ ] Set the alarm from a phone through a small web page
- [ ] Connect to Home Assistant together with the other room projects

## License

MIT, see [LICENSE](LICENSE).
