## xtemu
Emulator for the IBM XT personal computer. Uses SDL2 for graphics/sound/input.
The goal is a faithful enough implementation to boot any XT-supported operating
system, but the first gold star on this project is running Xenix.

### Project Details
Language: C99
Host: macOS on arm64
Target: Any POSIX-compatible OS
Compiler/Debugger: clang/lldb
Build System: make
Version Control: git

- Emulator should run at native speed.
- Emulator should support ejecting/hotswapping floppies. Some OSes need this.
- Required imaginary "MZ-" hardware (like the floppy and disk controllers)
  should "just work" in the BIOS and in all OSes.
- Implement the ISA bus so that virtual hardware can be easily added later.

I imagine the usage syntax being something like:
```
xtemu \
-f0 <floppy> \
-f1 <floppy> \
-hd <disk image> \
-q \ # to shut up PC-Speaker beeping
-t # WARP SPEED MR. SULU (turbo mode)
```

### (Emulated) Hardware
- Intel 8088 @4.77MHz (7.16MHz turbo mode)
- 640KB RAM
- Onboard PC-Speaker
- IBM Enhanced Graphics Adapter
- MZFlop floppy controller (two 720KB drives)
- MZDisk XTA controller (accepts up to 40MB raw image)
