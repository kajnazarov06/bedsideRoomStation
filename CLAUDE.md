# Bedside Room Station — project rules

## Build

- Board: ESP32 Dev Module (`esp32:esp32:esp32`), esp32 core 3.x.
- Arduino CLI (bundled with the IDE):
  `"/Applications/Arduino IDE.app/Contents/Resources/app/lib/backend/resources/arduino-cli"`
- Compile check after every code change:
  `"$CLI" compile --fqbn esp32:esp32:esp32 .`

## VS Code IntelliSense (include errors)

VS Code resolves includes from `build/intellisense/compile_commands.json`, not from
the real build. If that file is stale, the editor shows "cannot open source file"
errors even though the sketch compiles.

**Rule:** whenever you add, rename or delete a `.cpp`/`.h`/`.ino` file, or install a
new library, regenerate the database before finishing:

```bash
python3 .vscode/gen_intellisense.py
```

Then confirm the new files appear in it, e.g.
`grep -o '"file": "[^"]*<NewFile>.cpp"' build/intellisense/compile_commands.json`.

## Code structure

- `src/core/` interfaces, `src/drivers/` hardware, `src/storage/` NVS, `src/app/`
  logic, `src/ui/` LCD views, `src/net/` WiFi / HTTP API / mDNS, `src/config/` settings.
- Each module is a class; hardware modules implement `IComponent` (`begin()`, `name()`).
- Include project headers with relative paths (`"../drivers/RtcClock.h"`); the `.ino`
  uses `"src/..."`.
- User-tunable values go in `src/config/Settings.h`; internal timings stay in an
  anonymous namespace next to the code that uses them.
- Secrets live in `src/config/Secrets.h` (gitignored); keep `Secrets.example.h` in sync.
- When adding a library dependency, also add it to the library table in `README.md`.

## Naming

- Name variables and parameters with whole words that say what they hold:
  `response`, `reading`, `settings`, `dateTime`, `component` — never `d`, `r`, `a`,
  `t`, `c`, and no cryptic abbreviations like `ts` or `caps`.
- Only exception: `i` / `j` as plain loop counters.
