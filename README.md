# RamaCraft: Minecraft meets "Rendezvous with Rama"

A 3D voxel survival, exploration, and base-building game set inside the colossal cylindrical alien starship **Rama**.

---

## Features

- **Cylindrical Rama Megastructure**: Experience the world curving overhead in a giant cylinder. Look up to see the opposite side of the starship and the central axis. (Toggle curvature anytime with `[K]`).
- **Linear Sun Strips & Day/Night Cycle**: Rama's 6 interior sun troughs ignite and dim in cyclical shifts, plunging the ancient hull into darkness accompanied by warning sirens.
- **Biomes & Landscapes**:
  - **Ancient Alien Cities & Monoliths**: Towering obsidian obelisks, labyrinthine ruins, and data terminals.
  - **The Cylindrical Sea**: A circular band of water encircling Rama with seabed crystals and underwater physics.
  - **Industrial Machinery Hubs**: Broken alien reactors with glowing orange plasma cores, computer racks, and steam vents.
  - **Subterranean Caves**: Rich in Titanium veins, Cobalt crystals, Carbonite composite, and Silica.
- **Dismantling Machinery & Mining**: Break down consoles and reactors to salvage **Alien Circuits**, **Servo Motors**, **Power Matrices**, and **Plasteel Sheets**.
- **Nocturnal Robot Beetles (Biots)**: Maintenance biots with glowing red optical eyes emerge from vents at night. Attack them with your Ray Gun to harvest **Robot Cores**, **Micro Actuators**, **Biot Shells**, and **Energy Cells**.
- **Base Expansion & Tech Crafting (`[C]` Key)**:
  - Habitat Base Walls, Pressurized Dome Glass, Airlock Doors
  - Biot Defense Auto-Turrets
  - High-Capacity Thruster Jetpack Upgrade
  - Overclocked Twin-Beam Ray Gun (with instant mining drill)
  - Ruin Scanner Radar & Oxygen Recyclers
- **WiFi LAN Multiplayer (`[M]` Key)**:
  - Play cooperatively over your local WiFi network.
  - Automatic LAN beacon discovery finds active games on your network with 1-click connect.
  - Full real-time synchronization of 3D astronaut movements, jetpack flights, mining, building, and laser gun combat.
- **Starting Equipment**:
  - **Ray Gun**: Left click to fire plasma bolts or hold to mine blocks.
  - **Flashlight**: Press `[F]` to toggle directional spotlight.
  - **Jetpack**: Hold `[SPACE]` for low-gravity flight with thruster exhaust.
  - **10x Plasma Torches**: Right click to place warm light sources in dark ruins.

---

## Controls

| Key / Action | Function |
|---|---|
| `W`, `A`, `S`, `D` / Arrows | Move along Rama's hull & in-flight steering |
| `SPACE` | Jetpack Thrust **towards look direction** (where looking) |
| `SHIFT` | Jetpack Thrust **upwards relative to view** (or Sprint on ground) |
| `CTRL` | Jetpack Thrust **downwards relative to view** |
| `LEFT CLICK` | Fire Ray Gun / Hold to Mine Block |
| `RIGHT CLICK` | Place Block or Torch |
| `1` - `9` / Mouse Wheel | Select Hotbar Slot |
| `F` | Toggle Dynamic Flashlight |
| `E` or `TAB` | Suit Cargo & Inventory |
| `C` | Tech Fabricator / Base Crafting |
| `M` | WiFi LAN Multiplayer Menu |
| `K` | Toggle Cylindrical World Curvature |
| `ESC` | Release Cursor / Close Menus |

---

## Download & Installation

### Option 1: Download Pre-Compiled Game (Easiest for Windows & macOS)
1. Go to the [**Releases Page**](https://github.com/Augustalex/ramacraft/releases).
2. Download **`ramacraft-windows-x64.zip`** (for Windows) or **`ramacraft-macos.tar.gz`** (for Mac).
3. Extract the ZIP folder.
4. Double-click **`ramacraft.exe`** on Windows (or `./ramacraft` on Mac) to start playing immediately!

---

## Building from Source

### Windows (Using CMake & Visual Studio / MinGW):
1. Install [CMake](https://cmake.org/download/) and [SDL2 Development Libraries](https://github.com/libsdl-org/SDL/releases).
2. Open terminal in the project directory:
   ```bash
   cmake -B build
   cmake --build build --config Release
   ```
3. Run `build/Release/ramacraft.exe`.

### macOS (Native):
1. Install SDL2 via Homebrew:
   ```bash
   brew install sdl2
   ```
2. Build and run:
   ```bash
   make
   ./bin/ramacraft
   ```

### WebGL 2 / Browser Build (with Emscripten):
```bash
./build_web.sh
# Open web/index.html via any local web server (e.g. python3 -m http.server 8080)
```

---

## WiFi LAN Multiplayer Launch

- **Host a game directly on port 7777**:
  ```bash
  ./bin/ramacraft --host 7777
  ```
- **Join a game on your local WiFi**:
  ```bash
  ./bin/ramacraft --join <HOST_IP> 7777
  ```
- *(Or simply start the game and press `[M]` to host or join discovered LAN games visually!)*
