# RamaCraft: Minecraft meets "Rendezvous with Rama"

A 3D voxel survival, exploration, and base-building game set inside the colossal cylindrical alien starship **Rama**.

[![Release](https://img.shields.io/github/v/release/Augustalex/ramacraft?color=blue&label=Latest%20Release)](https://github.com/Augustalex/ramacraft/releases/latest)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-brightgreen)](#-quick-install--play)

---

## ⚡ Quick Install & Play

### 🐧 Linux (Kubuntu, Ubuntu, Debian):
Paste either command in your terminal to automatically download, install dependencies, and launch:

**Using `wget`:**
```bash
wget -qO- https://raw.githubusercontent.com/Augustalex/ramacraft/main/install.sh | bash
```

**Using `curl`:**
```bash
curl -sSL https://raw.githubusercontent.com/Augustalex/ramacraft/main/install.sh | bash
```

### 🪟 Windows & 🍎 macOS:
Download directly from the [**Releases Page**](https://github.com/Augustalex/ramacraft/releases/latest):
- **Windows**: Download [`ramacraft-windows-x64.zip`](https://github.com/Augustalex/ramacraft/releases/latest/download/ramacraft-windows-x64.zip), extract, and double-click `ramacraft.exe`.
- **macOS**: Download [`ramacraft-macos.tar.gz`](https://github.com/Augustalex/ramacraft/releases/latest/download/ramacraft-macos.tar.gz), extract, and run `./ramacraft`.
- **Linux (`.deb`)**: Download [`ramacraft-ubuntu-debian-amd64.deb`](https://github.com/Augustalex/ramacraft/releases/latest/download/ramacraft-ubuntu-debian-amd64.deb) and double-click to install.

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
  - **Ray Gun**: Combat energy pistol for hunting biots and multiplayer PvP shootouts.
  - **Plasma Frag Grenades**: Bouncy high-explosive ordnance with a 2.4s timed fuse; creates craters, deals blast damage, and launches players!
  - **Flashlight**: Press `[F]` to toggle directional spotlight.
  - **Jetpack**: Press `[TAB]` to toggle full 3D flight thrusters.
  - **Plasma Torches & Building Materials**: Place blocks and torches to establish pressurized bases.

---

## Controls

| Key / Action | Function |
|---|---|
| `TAB` | **Toggle Jetpack Flight Mode ON / OFF** |
| `W`, `A`, `S`, `D` | Walk on hull (or in-flight 3D steering with Jetpack ON) |
| `SPACE` | Jump 1.5 blocks (Jetpack OFF) / **Thrust towards look direction** (Jetpack ON) |
| `SHIFT` | Sprint on ground (Jetpack OFF) / **Thrust view-up** (Jetpack ON) |
| `CTRL` | Crouch (Jetpack OFF) / **Thrust view-down** (Jetpack ON) |
| `LEFT CLICK` | **Fire Ray Gun / Throw Grenade (if selected)** / Hold to Mine |
| `RIGHT CLICK` | **Throw Grenade (if selected)** / Place Block or Torch |
| `1` - `9` / Mouse Wheel | Select Hotbar Slot (Slot 1: Ray Gun, Slot 2: Grenades x8) |
| `F` | Toggle Dynamic Flashlight |
| `E` | Suit Cargo & Inventory |
| `C` | Tech Fabricator / Base Crafting |
| `M` | WiFi LAN Multiplayer Menu |
| `K` | Toggle Cylindrical World Curvature |
| `ESC` | Release Cursor / Close Menus |

---

## Download & Installation

### Linux 1-Line Quick Install (Kubuntu, Ubuntu, Debian):
Run either of these commands in your terminal to automatically download, install dependencies, and launch RamaCraft:

**Using `wget`:**
```bash
wget -qO- https://raw.githubusercontent.com/Augustalex/ramacraft/main/install.sh | bash
```

**Using `curl`:**
```bash
curl -sSL https://raw.githubusercontent.com/Augustalex/ramacraft/main/install.sh | bash
```

---

### Manual Downloads (Windows, macOS & Linux)
1. Go to the [**Releases Page**](https://github.com/Augustalex/ramacraft/releases).
2. Download the package for your OS:
   - **Kubuntu / Ubuntu / Debian (`.deb`)**: Download **`ramacraft-ubuntu-debian-amd64.deb`** and double-click to install (or run `sudo apt install ./ramacraft-ubuntu-debian-amd64.deb`). Launch directly from your application menu!
   - **Linux Standalone / Portable (`.tar.gz`)**: Download **`ramacraft-linux-x64.tar.gz`**, extract, and run `./run.sh` (or `./ramacraft`).
   - **Windows (`.zip`)**: Download **`ramacraft-windows-x64.zip`**, extract, and double-click **`ramacraft.exe`**.
   - **macOS (`.tar.gz`)**: Download **`ramacraft-macos.tar.gz`**, extract, and run `./ramacraft`.

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
