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
- **Starting Equipment**:
  - **Ray Gun**: Left click to fire plasma bolts or hold to mine blocks.
  - **Flashlight**: Press `[F]` to toggle directional spotlight.
  - **Jetpack**: Hold `[SPACE]` for low-gravity flight with thruster exhaust.
  - **10x Plasma Torches**: Right click to place warm light sources in dark ruins.

---

## Controls

| Key / Action | Function |
|---|---|
| `W`, `A`, `S`, `D` | Move along Rama's hull |
| `SPACE` | Thruster Jetpack (Hold for flight) |
| `LEFT CLICK` | Fire Ray Gun / Hold to Mine Block |
| `RIGHT CLICK` | Place Block or Torch |
| `1` - `9` / Mouse Wheel | Select Hotbar Slot |
| `F` | Toggle Dynamic Flashlight |
| `E` or `TAB` | Suit Cargo & Inventory |
| `C` | Tech Fabricator / Base Crafting |
| `K` | Toggle Cylindrical World Curvature |
| `ESC` | Release Cursor / Close Menus |

---

## Building and Running

### Native macOS Build:
```bash
make
./bin/ramacraft
```

### WebGL 2 / Browser Build (with Emscripten):
```bash
./build_web.sh
# Open web/index.html via any local web server (e.g. python3 -m http.server 8080)
```
