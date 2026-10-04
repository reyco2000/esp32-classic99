# Classic99 ESP32 — a TI-99/4A on the TTGO VGA32

A standalone Texas Instruments TI-99/4A home computer running on a TTGO VGA32
v1.4 board (ESP32). Plug in a VGA monitor, a PS/2 keyboard and an SD card with
the TI ROMs, and the board boots straight to the TI title screen.

It is a port of [Classic99](https://github.com/tursilion/classic99), the
Windows TI-99/4A emulator by Mike Brent ("Tursi"). The CPU core, memory map,
video, sound and disk behaviour come from Classic99; the display, keyboard,
audio and storage are driven through [FabGL](https://github.com/fdivitto/FabGL).

All the code of the port lives in this `esp32/` directory. Everything outside
it is the unmodified Classic99 source, kept as the reference.

**Version 0.1.0**

## Features

- TMS9900 CPU (Classic99's core), console ROM and GROM, 32K memory expansion
- TMS9918A video: all four modes and sprites, at 60 frames per second
- TMS9919 sound on the board's audio jack
- Cartridges: ROM/GROM files (`C`/`D`/`G`) and bank-switched ROMs (378 and 379 types)
- TI disk controller with three drives (DSK1 to DSK3), using disk images on the SD card
- PS/2 keyboard with six layouts; joystick 1 on the arrow keys and Tab
- "Supervisor" on-screen menu (F12) to change cartridges, disks and settings
- Settings saved on the SD card and restored at power-on

Not emulated: speech synthesiser, F18A / 80 columns, AMS/SAMS memory, cassette,
RS232, TIPI, joystick 2, the debugger.

### Tested

- TI BASIC, TI Extended BASIC, RXB 2026, Editor/Assembler (menus), Parsec,
  TI Invaders, Super Space Acer (256K, 378 type), TI Workshop (379 type)
- Loading and saving BASIC programs on 90K and 180K disk images, on all three drives
- Extended BASIC auto-loading `DSK1.LOAD`

Not yet tested: 360K disk images, assembling under Editor/Assembler,
multicolor and text-bitmap video modes.

## Hardware

- **TTGO VGA32 v1.4** (ESP32-PICO-D4 with 4 MB PSRAM). The PSRAM is required.
- VGA monitor (the picture is 320x240 at 60 Hz)
- PS/2 keyboard
- SD card, FAT formatted
- Optional: speaker or amplifier on the audio jack

## Building and flashing

1. Install [PlatformIO](https://platformio.org/) (the command-line `pio` is enough).
2. Install **FabGL 1.0.9** in `~/Arduino/libraries/FabGL`. `platformio.ini`
   looks for it there (`lib_extra_dirs`).
3. Connect the board by USB, then from this directory:

```bash
pio run                                         # build
pio run -t upload --upload-port /dev/ttyACM0    # build and flash
```

PlatformIO downloads the ESP32 toolchain (`espressif32@6.10.0`) on the first
build. The serial port must be free while flashing.

Notes for anyone changing the code:

- `src/cpu9900.cpp` is **generated** from Classic99's `console/cpu9900.cpp`.
  Do not edit it; change `tools/port_cpu.py` and run `python3 tools/port_cpu.py`.
- The firmware version is `CLASSIC99_ESP32_VERSION` in `src/version.h`. It is
  shown on the About screen and in the serial boot banner with the build date.
- Keep `-mfix-esp32-psram-cache-issue` in `platformio.ini`; the board's chip
  revision needs it.

## SD card

No ROMs, cartridges or disk images are included. Copy your own to the card:

```
/ti99/rom/994AROM.BIN     console ROM (8K)
/ti99/rom/994AGROM.BIN    console GROM (24K)
/ti99/rom/DISK.BIN        TI disk controller DSR (8K), needed for disk access
/ti99/carts/              cartridge images
/ti99/disks/              disk images (*.dsk)
/ti99/config.txt          written by the Supervisor menu
```

The card is never formatted by the firmware.

**Cartridges** use V9T9-style names. Files are grouped into one cartridge by
the part of the name before the type suffix, so the names must match:

| Suffix | Contents |
|---|---|
| `xxxC` | ROM at >6000 (bank 0) |
| `xxxD` | second ROM bank |
| `xxxG` | GROM at >6000 |
| `xxx8` | bank-switched ROM, 378 type |
| `xxx9` | bank-switched ROM, 379 type (inverted) |
| none | treated as 378 type |

Example: `PARSECC.BIN` and `PARSECG.BIN` appear as one cartridge, `PARSEC`.

**Disks** are plain sector dumps of 90K, 180K or 360K. The standard TI
`DISK.BIN` is required: the port intercepts its sector routine at fixed
addresses, so other controller ROMs will not work. If there is no
`config.txt`, `DSK1.dsk` is mounted on drive 1 when present.

## Keyboard

Letters, digits and punctuation are typed as printed on the PC keyboard; the
port translates them to the TI key combinations.

| PC key | TI-99/4A |
|---|---|
| Alt | FCTN (left Alt only on non-US layouts) |
| Ctrl | CTRL |
| Caps Lock | ALPHA LOCK |
| F1 to F9 | FCTN+1 to FCTN+9 |
| F10 | FCTN+= (QUIT) |
| Esc | FCTN+9 (BACK) |
| Backspace | FCTN+S (cursor left) |
| Delete / Insert | FCTN+1 (DEL) / FCTN+2 (INS) |
| Arrow keys | FCTN+E/S/D/X, or joystick 1 while a program reads the joystick |
| Tab | joystick 1 fire |
| F12 | Supervisor menu |

Layouts: US English, Spanish (Spain), Spanish (Latam), French, Italian and
Portuguese (Brazil ABNT2). On the US layout both Alt keys are FCTN. On the
others the left Alt is FCTN and the right Alt is AltGr, as printed on the
keyboard. Accent keys are not dead keys: `^`, `` ` `` and `~` type immediately.

## Supervisor menu (F12)

Every screen uses the colours of the TI title screen; the main screen is a
grid of icon tiles. Arrow keys move, Enter selects, Esc goes back one level,
F12 closes the menu. The emulation is paused while the menu is open.

- **Cartridge**: the inserted cartridge is shown at the top (grey when there
  is none); pick one from the list, or `Remove cartridge`. The console resets.
- **Disks** (Disk Manager): the three drives are shown as buttons at the top
  and the disk images in `/ti99/disks` are listed below. Tab, or Up from the
  first file, moves between the two.
  - Enter on a disk image asks which drive to mount it on, and asks for
    confirmation before replacing a disk or mounting the same image twice.
  - Enter on a drive that holds a disk unmounts it, after confirmation.
- **Setup**
  - **Keyboard**: the layout; it applies immediately.
  - **Debug log**: ON/OFF. When ON the firmware prints diagnostics on the
    serial port (boot messages, emulation statistics every 5 s, menu activity).
    OFF by default.
- **Reset**: resets the console, after confirmation.
- **About**: credits, firmware version, build date and PSRAM free/total.
- **Resume**: back to the TI-99/4A.

The settings are saved to `/ti99/config.txt` when the menu closes.

## Serial console

At 115200 baud the firmware accepts commands, mainly for testing:

| Command | Action |
|---|---|
| `ls` | list cartridges |
| `cart <n>` | insert cartridge number `n` and reset |
| `eject` | remove the cartridge |
| `reset` | reset the console |
| `disk <1-3> [file]` | mount a disk image, or unmount if no file is given |
| `screen` | print the text on the TI screen |
| `debug on\|off` | diagnostics for this session (the Setup menu saves the setting) |
| `type <text>` | type text on the TI (`\|` = Enter, `~` = FCTN+9) |
| `key f12\|up\|down\|left\|right\|tab\|enter\|esc` | send a menu key |

With the debug log on, every 5 seconds it prints
`emu: <fps>, <instructions/s>, load <%>, PC=>xxxx`.
`tools/sercmd.py` scripts these commands; note that opening the serial port
resets the board.

## Source layout

| File | Role |
|---|---|
| `platformio.ini`, `partitions.csv` | build configuration, 3 MB application partition |
| `tools/port_cpu.py` | generates `src/cpu9900.cpp` from Classic99's CPU source |
| `tools/sercmd.py` | serial test helper |
| `src/main.cpp` | start-up, memory allocation, keyboard task, serial console |
| `src/emu.cpp` | emulation task: instruction loop and 60 Hz pacing |
| `src/cpu9900.cpp/.h` | TMS9900 CPU (generated from Classic99) |
| `src/bus.cpp` | memory map, GROM, 9901/CRU, cartridge banking |
| `src/vdp9918.cpp` | TMS9918A video and scanline renderer |
| `src/sound9919.cpp` | TMS9919 sound |
| `src/keyboard_ti.cpp` | PC keys to the TI key matrix and joystick |
| `src/kbd_layouts.cpp` | keyboard layouts |
| `src/cart.cpp` | cartridge scanning and loading |
| `src/ticc.cpp` | TI disk controller and disk images |
| `src/menu.cpp` | Supervisor menu and `config.txt` |
| `src/version.h` | firmware version |

The emulation runs alone on one ESP32 core; keyboard, menu and serial console
run on the other. Console ROM and GROM, video RAM and the frame buffer are in
internal RAM; cartridges, the 32K expansion and the disk controller ROM are in
PSRAM.

## Credits

- **Mike Brent ("Tursi")** — [Classic99](https://github.com/tursilion/classic99),
  the emulator this port is derived from. The CPU core and the behaviour of
  the video, sound, keyboard and disk emulation are his work.
- **Fabrizio Di Vittorio** — [FabGL](https://github.com/fdivitto/FabGL), the
  VGA, PS/2, sound and SD library for the ESP32.
- **Reinaldo Torres (reyco2000)** — ESP32 / TTGO VGA32 port, co-developed with
  Claude Code.
- Texas Instruments — the TI-99/4A.

## Licence

This port is a derived work of Classic99 and remains under Classic99's
licence (see the Classic99 sources and documentation in the parent
directory). That licence asks that the author be contacted before derived
works or ports are distributed. The ported files keep their
"Derived from Classic99 (C) Mike Brent" headers.

FabGL is used under its own licence and is not included here.

The TI ROMs, cartridges and disk images are copyrighted by their owners and
are not part of this project.
