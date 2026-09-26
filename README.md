# xtemu - IBM XT Emulator

A faithful emulator for the IBM XT personal computer, written in C99 using SDL2 for graphics, sound, and input.

## Features

- Intel 8088 CPU emulation (4.77MHz base, 7.16MHz turbo mode)
- 640KB RAM support
- IBM Enhanced Graphics Adapter emulation
- PC-Speaker audio
- MZFlop floppy controller (two 720KB drives)
- MZDisk XTA controller (up to 40MB raw images)
- Hotswap floppy support
- Turbo mode for faster execution

## Build Requirements

- C99-compatible compiler
- SDL2 development libraries
- SDL2_mixer development libraries

## Installation

### Install dependencies

**macOS (Homebrew):**
```bash
brew install sdl2 sdl2_mixer
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install libsdl2-dev libsdl2-mixer-dev
```

**Linux (Fedora/CentOS):**
```bash
sudo dnf install SDL2-devel SDL2_mixer-devel
```

### Build the emulator

```bash
make
```

### Run the emulator

```bash
./xtemu
```

## Usage

```bash
xtemu \
-f0 <floppy> \     # Load floppy disk in drive 0
-f1 <floppy> \     # Load floppy disk in drive 1
-hd <disk> \      # Load hard disk image
-q               # Quiet mode (disable PC speaker)
-t               # Turbo mode (faster execution)
```

## Development

### Debug mode

```bash
make debug
./xtemu
```

### Clean build artifacts

```bash
make clean
```

### Install to system

```bash
make install
```

## Project Structure

- `main.c` - Main emulator logic and CPU emulation
- `xtemu.h` - Header file with data structures and function prototypes
- `hardware.c` - Hardware emulation (PIC, Keyboard, Timer, DMA)
- `Makefile` - Build configuration
- `ROADMAP.md` - Development roadmap and progress
- `IDEA.md` - Project details and specifications

## Goals

The goal is to create a faithful enough implementation to boot any XT-supported operating system, with the first milestone being to run Xenix.

## Hardware Emulation

- Intel 8088 @4.77MHz (7.16MHz turbo mode)
- 640KB RAM
- Onboard PC-Speaker
- IBM Enhanced Graphics Adapter
- MZFlop floppy controller (two 720KB drives)
- MZDisk XTA controller (accepts up to 40MB raw image)
- PIC, Keyboard Controller, System Timer, DMA Controller

## License

MIT License