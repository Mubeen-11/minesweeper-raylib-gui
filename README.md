# Minesweeper Game

A modern, visually enhanced Minesweeper implementation featuring a dark-themed GUI, recursive board revealing, persistent save/load functionality, and hardware-accelerated rendering.

## Features

- **Interactive GUI**: Modern dark-themed layout built completely with hardware-accelerated shapes and text rendering.
- **State Management**: Fully functional text input interface for user profile initialization.
- **Recursive Revealing**: Automated cascade uncovering for blank ($0$) safe cells.
- **Persistence Layer**: Save and load your game data dynamically into custom encrypted `.txt` files tied directly to your unique username.

## Technical Prerequisites

- **Operating System**: Windows 10 or 11
- **IDE**: Microsoft Visual Studio (2019 or 2022 recommended)
- **Graphics API Requirements**: OpenGL 3.3 compatible environment (or OpenGL 1.1/2.1 fallback library binaries for legacy Intel HD architectures)

## Setup & Visual Studio Configuration Guide

To build and compile this project, Raylib must be linked statically within your Visual Studio environment. Follow these exact configuration steps:

### 1. Download Raylib

Download the pre-compiled binaries for Windows (`raylib-x.x_win64_msvc16.zip`) from the [official Raylib GitHub Releases](https://github.com/raysan5/raylib/releases). Extract the folder somewhere memorable on your system (e.g., `C:\raylib`).

### 2. Configure Project Properties

Open your project in Visual Studio, right-click on your Project Name in the **Solution Explorer**, select **Properties**, and apply the following settings:

**Include Directories:** Go to **Configuration Properties** → **C/C++** → **General**. Edit **Additional Include Directories** and append the path to the Raylib `include` folder: `C:\raylib\include`

**Library Directories:** Go to **Configuration Properties** → **Linker** → **General**. Edit **Additional Library Directories** and append the path to the Raylib static library folder: `C:\raylib\lib`

**Linker Dependencies:** Go to **Configuration Properties** → **Linker** → **Input**. Edit **Additional Dependencies** and explicitly add these four specific system libraries:

raylib.lib
winmm.lib
gdi32.lib
opengl32.lib


**Advanced Configuration Switch (For Legacy Intel Graphics Fallbacks):** Set the project build architecture configuration dropdown at the top of your IDE window strictly to **x64** matching your compiled `.lib` targets.

## How to Use & Play

**Compilation:** Press `Ctrl + Shift + B` to build your binary inside Visual Studio, then press `F5` to execute.

**Profile Creation:** When the window spawns, type your desired player nickname inside the text field box. Press `ENTER` to lock in your identity profile context.

**Gameplay Interaction:** If a matching `<username>.txt` file is found locally, the engine automatically reconstructs your previous match data layout. If you are a new player, a pristine `9 × 9` matrix generates immediately. **Left Click** on hidden cells to reveal them. Avoid the `6` hidden explosive landmines!

**Interface Actions:** Click **SAVE GAME** to dump your current board array state directly to the disk tracking map file. Click **LOAD GAME** to rollback layout shifts. Click **NEW GAME** to restart board generation logic.

---

Enjoy the game!
