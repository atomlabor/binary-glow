# Binary Glow v1.0.0

A futuristic, high-tech watchface for the Pebble smartwatch ecosystem. **Binary Glow Ultra** combines a classic "Matrix" aesthetic with a modern HUD (Heads-Up Display) feel, featuring dynamic animations and system monitoring at a glance.

Proudly made by [Atomlabor](https://atomlabor.de)

---

## 🚀 Key Features

### 1. Dynamic HUD Geometry
The center of the watchface is occupied by ten concentric rings that represent the passage of time. These rings pulse and react to the system state, creating a living "core" on your wrist.

### 2. Multi-Platform Support
One codebase to rule them all. Optimized for:
* **Emery & Gabbro:** Full resolution, vibrant colors, and high-density text.
* **Basalt & Chalk:** Balanced layouts for the original color series and the Pebble Round.
* **Aplite & Diorite:** Retro-tech monochrome support using **2x2 Bayer Dithering** to simulate grayscale depth on 1-bit displays.

### 3. Integrated Battery HUD
A precision-aligned battery bar is situated perfectly beneath the time.
* **Color-Coded:** The bar remains Matrix-Yellow/Green during normal operation.
* **Critical Alert:** The bar turns **Bright Red** once the battery drops below 20% to warn of impending power loss.

### 4. Seamless Binary Border
The watchface is framed by a continuous stream of `I`s and `0`s. 
* **Smart Layout:** On rectangular watches, the binary code forms a gapless frame that perfectly fills the screen edges.
* **Chalk Optimization:** On the Pebble Round, the binaries follow the circular curvature of the glass.

### 5. Kinetic Glitch Effect
Tap your watch or flick your wrist to trigger a 5-second **Glitch Protocol**. The system core will visually destabilize with rapid color shifts and shifting ring geometry before recalibrating back to normal.

### 6. Quick View Compatibility
Full support for Pebble "Quick View" (Timeline overlays). When a notification arrives, the entire HUD automatically shifts upwards to remain visible and perfectly centered in the unobstructed area.

---

## 🎨 Color Schemes
The watchface automatically transitions through **6 distinct color palettes** based on the time of day, ensuring the look stays fresh and current.

---

## 🛠 Installation & Build

Ensure you have the Pebble SDK 4.0 or higher installed.

1. Clone the repository into your Pebble workspace.
2. Ensure your `menu_icon.png` is in `resources/images/`.
3. Build the project:
   ```bash
   pebble build
