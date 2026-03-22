# Will of the Prescript

A handheld prescript device for the M5StickC Plus2. Inspired by the mission-card aesthetic of Library of Ruina and Limbus Company — you press a button, you receive a directive, you carry it out (or don't).

Each prescript scrambles into existence with a reveal animation and a sound effect. Once you've read it, clear it and you'll be sent back to the waiting screen. Press again when you're ready for the next one. A status screen tracks your battery and how many prescripts you've cleared across all sessions. Leave it alone long enough and it puts itself to sleep.

---

## Hardware

**Target: M5StickC Plus2 only.**

| | |
|---|---|
| Chip | ESP32-PICO-V3-02 |
| Display | 240 × 135 colour LCD, landscape |
| Audio | Built-in speaker via M5Unified |
| BtnA | Large front button |
| BtnB | Small side button |
| BtnPWR | Top power button |

GPIO 4 (PIN_HOLD) is driven HIGH on boot to keep the device powered through the self-latch circuit. It goes LOW when the device shuts down.

---

## Libraries

**M5Unified** — install from the Arduino IDE Library Manager. Handles display, speaker, buttons, and power management.

**LittleFS** — ships with the ESP32 Arduino core, nothing extra to install. Used to load audio and image assets from flash.

---

## Setting Up the Filesystem

Assets live in a `data` folder next to the `.ino` file and are flashed to the device using the [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload) plugin.

### Required files

**`/index_message_1.wav`** — the reveal sound, played when a prescript appears.
Must be uncompressed PCM WAV (not MP3, not ADPCM). At boot the firmware reads the file's sample rate, channel count, and bit depth to calculate its exact duration, then syncs the letter-reveal animation to end at the same moment the sound does.

**`/index_message_2.wav`** — the clear sound, played when you clear a prescript.
Also must be uncompressed PCM WAV.

**`/index_new.png`** — the logo shown on the waiting screen, centred above the "- Click to Receive -" text.

### Converting audio to PCM WAV

```
ffmpeg -i input.mp3 -ar 44100 -ac 1 -f wav output.wav
```

If the WAV is the wrong format, the sound won't play. The animation will still run using the built-in timing fallback.

---

## Configuration

These constants near the top of `prescript_device.ino` are the ones you'd actually want to change:

| Constant | What it does |
|---|---|
| `USER_NAME` | Name shown on the status screen, e.g. `"Proxy Denis"` |
| `BRIGHTNESS` | Screen brightness, 0–255. The default (10) is intentionally dim |
| `INACTIVITY_MS` | How long to wait with no input before sleeping, in ms. Default is 120000 (2 min) |
| `COL_BG` | Background colour as RGB565. Default `0x19A6` is the dark teal `#1A3535` |
| `SLEEP_TEXT` | Text displayed during the shutdown animation. Default is `"2.718281"` |

---

## Controls

**BtnA (front button)**
- Waiting screen → generates and reveals the next prescript
- Reading a prescript → clears it
- Status screen → returns to the waiting screen
- During any animation → ignored, the animation plays through in full

**BtnB (side button)**
- From any non-animating state → opens the status screen
- From the status screen → returns to the waiting screen
- During any animation → ignored

**BtnPWR (top power button)**
- Short press → nothing visible happens (the AXP2101 hardware briefly cuts the backlight; the firmware restores it on release)
- Hold 3 seconds → triggers the shutdown sequence from any state

---

## How the States Work

The main loop: **BOOT → ANIMATING → READING → CLEARING → CLEARING_HOLD → BOOT → ...**

BOOT is the waiting screen — it's where you start and where you land after every cleared prescript. Nothing auto-generates; you have to ask for each one.

| State | What's happening |
|---|---|
| `BOOT` | Waiting screen. Logo + "- Click to Receive -". Press BtnA to go. 2 min idle → sleep. |
| `ANIMATING` | Scramble-reveal animation plays in sync with the generation sound. |
| `READING` | Prescript is on screen, waiting for you. Press BtnA to clear. |
| `CLEARING` | "_CLEAR._" animates in with the clear sound. |
| `CLEARING_HOLD` | "_CLEAR._" held briefly, then returns to BOOT. |
| `STATUS` | Status screen animates in. |
| `STATUS_HOLD` | Status held on screen. Times out or any button → BOOT. |
| `SLEEPING` | Shutdown animation plays, then power cuts. No coming back from here. |

BtnB opens the status screen from any non-animating state. During ANIMATING, CLEARING, CLEARING_HOLD, and MESSAGE, all buttons are locked out.

---

## How Prescripts Are Generated

`generatePrescript()` picks one of 16 structural templates at random and fills the slots from word arrays. The templates cover things like:

- A hand-picked standalone prescript from the `singles[]` list
- `[time] [action] [target].` with an optional postscript
- `[action capitalised] [target].` with an optional postscript
- Two-part prescripts joined by a transition phrase
- Location patterns: `Go to [place]. [Action] [target].`
- Person-finding: `Find the [adjective] [type] in [place]. [Verb] them.`
- Appearance patterns involving clothing and materials
- Object patterns: `Bring [object]. [Action] [target].`
- Games, topics, activities
- Compound structures from the `nc_act1 / nc_markers / nc_act2` arrays

Once the text is generated, `wrapIntoLines()` breaks it into lines targeting around 7 rows, which keeps the font size consistent regardless of text length.

Content sourced from `nyos.dev/prescript.js` and `prescript.neocities.org/prescripts.js`.

---

## Things Worth Knowing

**The AXP2101 power button fires on speaker activity.** Whenever the speaker changes its power draw, the AXP2101 PMIC generates a spurious power-button interrupt. This is why BtnPWR isn't used for navigation — phantom events would fire on every sound. The firmware handles this by only calling `M5.Speaker.stop()` when the speaker is actually playing, and by restoring display brightness any time a BtnPWR release is detected.

**Animation timing falls back gracefully.** If `index_message_1.wav` is missing or isn't valid PCM, the animation runs on compiled-in defaults: 600 ms spin phase, 3900 ms reveal, 4500 ms total. The sound just won't play.

---

## Developer Notes

**`totalClear` is persisted across sessions.** The counter is saved to NVS (non-volatile storage) via the `Preferences` library when the device shuts down through `powerOff()`. On boot it is restored, so the status screen shows a lifetime total rather than a per-session count.

**Global string arrays are declared `const char* const`.** All word-pool arrays (`singles`, `times`, `actions`, etc.) use `const char* const`, which tells the compiler to place the pointer tables in `.rodata` (read-only flash) rather than `.data` (writable DRAM). This is correct C++ for immutable string tables and also ensures heap corruption cannot silently overwrite the pointers.

**Speaker initialisation allocates ~12 KB of DRAM on first use.** The M5Unified speaker task (I2S DMA ring buffers + FreeRTOS task stack) is allocated the first time a sound is played in a session, not at boot. This is normal and expected; the heap stays stable for the remainder of the session.

**The AXP2101 interrupt caveat.** Any change in speaker power draw causes the AXP2101 PMIC to assert a power-button interrupt. This is why `M5.Speaker.stop()` is guarded by `isPlaying()` — calling stop on a silent speaker still triggers the interrupt, which the firmware misreads as a BtnPWR press and would restore backlight brightness unnecessarily (and could interfere with hold-to-sleep detection).
