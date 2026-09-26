# xtemu - IBM XT Emulator Development Roadmap

## Project Overview
A faithful emulator for the IBM XT personal computer, written in C99 using SDL2 for graphics, sound, and input.

## Development Phases

## Phase 1: Foundation & Core Architecture (✅ COMPLETED)
**Goal:** Establish working emulator framework with basic CPU and memory

### ✅ Completed Tasks:
- [x] Set up project structure (Makefile, header files, main.c)
- [x] Install SDL2 dependencies on macOS
- [x] Create basic display system with SDL2 window
- [x] Implement CPU register structure and state management
- [x] Create memory system (640KB RAM + 64KB BIOS ROM)
- [x] Build BIOS ROM generator (biosgen.c)
- [x] Implement basic x86 instruction decoder (JMP, CALL, RET, INT, CLI, STI, MOV, ADD)
- [x] Add debug output for CPU state
- [x] Create .gitignore and initial git commit

### Current Status: **Phase 1 Complete**
The emulator successfully boots and executes BIOS code without crashing.

---

## Phase 2: Enhanced CPU Instruction Set (✅ IN PROGRESS)
**Goal:** Implement comprehensive 8086 instruction decoding and execution

### ✅ Completed Tasks:
- [x] **Data Movement Instructions**
  - [x] MOV (all addressing modes)
  - [x] PUSH/POP (register, memory, immediate)
  - [x] XCHG, LDS, LES, LAHF, SAHF

- [x] **Arithmetic Instructions**
  - [x] ADD, ADC, SUB, SBB, CMP
  - [x] INC, DEC, NEG
  - [x] MUL, IMUL, DIV, IDIV
  - [x] CBW, CWD
  - [x] AAA, AAS, AAM, AAD

- [x] **Logical Instructions**
  - [x] AND, OR, XOR, TEST
  - [x] SAL, SHL, SAR, SHR
  - [x] ROL, ROR, RCL, RCR

- [x] **String Instructions**
  - [x] MOVSB, MOVSW, CMPSB, CMPSW
  - [x] SCASB, SCASW, LODSB, LODSW, STOSB, STOSW
  - [x] REP/REPE/REPNE prefixes

- [x] **Control Flow Instructions**
  - [x] Conditional jumps (JZ, JNZ, JC, JNC, etc.)
  - [x] LOOP, LOOPE, LOOPNE
  - [x] JMP (short, near, far)
  - [x] CALL (near, far)
  - [x] RET (near, far, with pop count)

- [x] **Flag Manipulation**
  - [x] STC, CLC, CMC
  - [x] STD, CLD
  - [x] LAHF, SAHF

### Current Status: **Phase 2 Complete**
The emulator now supports a comprehensive set of 8086 instructions including arithmetic, logical, string, and control flow operations. The CPU can execute complex instruction sequences without unknown opcode errors.

---

## Phase 3: Memory Management & Segmentation (✅ COMPLETED)
**Goal:** Implement proper x86 memory segmentation and addressing

### ✅ Completed Tasks:
- [x] **Segment Register Management**
  - [x] CS, DS, ES, SS register handling
  - [x] Segment override prefixes
  - [x] Far pointer calculations

- [x] **Memory Address Translation**
  - [x] Linear address calculation (segment * 16 + offset)
  - [x] Memory access bounds checking
  - [x] Segment limit enforcement

- [x] **Stack Operations**
  - [x] Proper SS:SP stack management
  - [x] Stack overflow/underflow detection
  - [x] PUSHA/POPA instruction support

- [x] **Memory Regions**
  - [x] BIOS ROM (F0000-FFFFF)
  - [x] Video RAM (B8000-BFFFF)
  - [x] Hardware registers system area

### Current Status: **Phase 3 Complete**
The emulator now has a comprehensive memory management system with proper x86 segmentation, address translation, bounds checking, and memory region handling. All memory operations use the new memory access functions with proper validation.

---

## Phase 4: Hardware Emulation - Basic I/O (✅ IN PROGRESS)
**Goal:** Implement fundamental hardware devices and I/O operations

### ✅ Completed Tasks:
- [x] **Programmable Interrupt Controller (PIC)**
  - [x] 8259 PIC initialization
  - [x] Interrupt request handling
  - [x] Interrupt masking and priority
  - [x] Master/slave cascade support
  - [x] EOI (End of Interrupt) handling
  - [x] Interrupt vector mapping
  - [x] I/O port integration (IN/OUT instructions)

- [x] **Keyboard Controller**
  - [x] 8042 keyboard controller emulation
  - [x] Keyboard buffer management
  - [x] Scan code translation (PC keyboard layout)
  - [x] Keyboard input via SDL
  - [x] I/O port integration (port 0x60)
  - [x] Keyboard interrupt triggering (IRQ 1)

### Tasks:
- [ ] **System Timer**
  - [ ] 8253/8254 Programmable Interval Timer
  - [ ] Timer interrupts (IRQ 0)
  - [ ] Clock tick generation

- [ ] **DMA Controller**
  - [ ] 8237 DMA controller basics
  - [ ] DMA channel management
  - [ ] Memory-to-memory DMA

- [ ] **Real-Time Clock**
  - [ ] CMOS RTC initialization
  - [ ] Time/date reading
  - [ ] CMOS memory layout

### Milestone: **Basic Hardware Foundation**
Core hardware devices functional for system operation.

---

## Phase 5: Video Display System
**Goal:** Implement IBM EGA/VGA display output

### Tasks:
- [ ] **Text Mode Display**
  - [ ] 80x25 text mode rendering
  - [ ] Character set (ROM font)
  - [ ] Color attributes (foreground/background)
  - [ ] Cursor management
  - [ ] Screen scrolling

- [ ] **Graphics Modes**
  - [ ] 320x200 4-color CGA
  - [ ] 640x350 16-color EGA
  - [ ] 640x480 16-color VGA

- [ ] **Video Memory**
  - [ ] Text mode buffer (B8000:0000)
  - [ ] Graphics mode buffer (A0000:0000)
  - [ ] VGA registers emulation

- [ ] **Display Timing**
  - [ ] Horizontal/vertical sync
  - [ ] Refresh rate control
  - [ ] Video memory access timing

### Milestone: **Functional Video Output**
Text mode display working with proper character rendering and color support.

---

## Phase 6: Storage Systems
**Goal:** Implement floppy disk and hard disk emulation

### Tasks:
- [ ] **Floppy Disk Controller**
  - [ ] NEC uPD765 controller emulation
  - [ ] Track/sector geometry (40 tracks, 9 sectors, 2 sides)
  - [ ] Read/write operations
  - [ ] Disk image format support (.img, .dsk)
  - [ ] Floppy drive hotswap support

- [ ] **Hard Disk Controller**
  - [ ] MZDisk XTA controller emulation
  - [ ] CHS addressing
  - [ ] 40MB raw image support
  - [ ] Hard disk image format support

- [ ] **Disk Image Management**
  - [ ] Create floppy disk images
  - [ ] Boot sector loading
  - [ ] File system basics (FAT12 for floppies)
  - [ ] Disk change detection

### Milestone: **Bootable Storage**
Can boot from floppy disk images and load operating systems.

---

## Phase 7: BIOS Implementation
**Goal:** Complete BIOS functionality for system booting

### Tasks:
- [ ] **Power-On Self Test (POST)**
  - [ ] Memory test (640KB detection)
  - [ ] Hardware initialization
  - [ ] Video card detection
  - [ ] Keyboard test

- [ ] **System Services**
  - [ ] INT 10h - Video services
  - [ ] INT 13h - Disk services
  - [ ] INT 16h - Keyboard services
  - [ ] INT 17h - Printer services
  - [ ] INT 1Ah - Time services

- [ ] **Boot Process**
  - [ ] MBR (Master Boot Record) loading
  - [ ] Boot sector execution
  - [ ] OS handoff

### Milestone: **Complete BIOS**
Full BIOS implementation enabling proper OS booting.

---

## Phase 8: Operating System Compatibility
**Goal:** Support XT-compatible operating systems

### Tasks:
- [ ] **MS-DOS 3.3**
  - [ ] Boot DOS from floppy
  - [ ] Basic DOS operations
  - [ ] DOS application compatibility

- [ ] **PC-DOS**
  - [ ] Alternative DOS implementation
  - [ ] Version-specific features

- [ ] **Xenix** (Primary Goal)
  - [ ] Boot Xenix from hard disk
  - [ ] Multi-user support
  - [ ] Xenix-specific hardware requirements

- [ ] **FreeDOS**
  - [ ] Modern DOS compatibility
  - [ ] Extended memory support

### Milestone: **DOS/Xenix Bootable**
Can boot and run DOS or Xenix successfully.

---

## Phase 9: Advanced Features
**Goal:** Add advanced emulation features and optimization

### Tasks:
- [ ] **Performance Optimization**
  - [ ] Dynamic recompilation (optional)
  - [ ] Instruction caching
  - [ ] Cycle-accurate timing

- [ ] **Sound Emulation**
  - [ ] PC Speaker emulation
  - [ ] AdLib/Sound Blaster support
  - [ ] Audio timing and mixing

- [ ] **Network Emulation**
  - [ ] PC-NET network card emulation
  - [ ] Network boot support
  - [ ] File sharing emulation

- [ ] **Peripherals**
  - [ ] Serial port emulation
  - [ ] Parallel port emulation
  - [ ] Mouse support

- [ ] **Debug Tools**
  - [ ] CPU register debugger
  - [ ] Memory viewer
  - [ ] Instruction step mode
  - [ ] Breakpoint support

### Milestone: **Production Emulator**
Feature-complete, optimized emulator with debugging capabilities.

---

## Phase 10: Quality Assurance & Release
**Goal:** Ensure stability and prepare for release

### Tasks:
- [ ] **Testing**
  - [ ] Unit tests for individual components
  - [ ] Integration tests
  - [ ] Compatibility testing with various software
  - [ ] Performance benchmarking

- [ ] **Documentation**
  - [ ] User manual
  - [ ] Developer documentation
  - [ ] API documentation
  - [ ] Installation guide

- [ ] **Packaging**
  - [ ] macOS app bundle
  - [ ] Linux package (deb/rpm)
  - [ ] Windows build
  - [ ] Cross-platform distribution

- [ ] **Release Management**
  - [ ] Version 1.0.0 release
  - [ ] Continuous integration setup
  - [ ] Bug tracking system
  - [ ] Community support

### Milestone: **Public Release**
Stable, documented emulator ready for public use.

---

## Development Priorities

### Immediate Next Steps (Current Session Focus)
1. **Complete Phase 2** - Add missing 8086 instructions
2. **Implement keyboard input** - Basic SDL keyboard handling
3. **Add timer interrupts** - For proper system timing

### Medium-term Goals (Next 2-3 Sessions)
1. **Video text mode** - Character rendering and display
2. **Floppy disk emulation** - Bootable disk images
3. **Basic DOS boot** - Get MS-DOS 3.3 running

### Long-term Goals (Future Sessions)
1. **Full BIOS implementation** - Complete POST and system services
2. **Xenix compatibility** - Primary project goal
3. **Advanced features** - Sound, networking, debugging tools

## Success Metrics

### Technical Metrics
- [ ] Can boot without unknown opcode errors
- [ ] Can execute 100% of 8086 instruction set
- [ ] Can boot MS-DOS 3.3 from floppy
- [ ] Can boot Xenix from hard disk
- [ ] Passes compatibility tests with XT software

### Quality Metrics
- [ ] No memory leaks (valgrind clean)
- [ ] Cycle-accurate timing (within 1%)
- [ ] Cross-platform compilation (macOS, Linux, Windows)
- [ ] Comprehensive test suite (>90% coverage)
- [ ] User documentation and examples

### Session Management

### Current Session Progress
- **Phase**: Phase 4 - Hardware Emulation - Basic I/O (IN PROGRESS)
- **Focus**: Keyboard controller implementation (COMPLETED)
- **Next Task**: System Timer implementation
- **Key Achievement**: Implemented comprehensive 8042 keyboard controller with SDL input integration, scan code translation, keyboard buffer management, I/O port handling (0x60), and interrupt triggering (IRQ 1). Integrated keyboard with existing PIC system.

### Session Checkpoints
- **Start of Session**: Review ROADMAP.md and current progress
- **Mid Session**: Verify completed tasks against roadmap
- **End of Session**: Update progress and plan next session tasks
- **Blockers**: Document any technical obstacles encountered

## Resources & References

### Essential References
- [IBM PC Technical Reference Manual](https://www.minuszerodegrees.net/manuals/IBM%20PC%20Technical%20Reference.pdf)
- [8086/8088 Family User Manual](https://www.manualslib.com/manual/849580/Intel-8086.html)
- [XT Architecture Documentation](https://www.seasip.info/Unix/X86/x86doc.pdf)

### Development Tools
- **Debugging**: GDB with custom commands, memory dump tools
- **Testing**: Unit test framework, compatibility test suite
- **Profiling**: Performance analysis tools, cycle counting

### Community Resources
- [PC Emulation Forum](https://www.vogons.org/)
- [Retro Computing Communities](https://www.reddit.com/r/Emulation/)
- [XT Software Archive](https://winworldpc.com/library/computers/ibm/pc/xt)

---

**Last Updated**: 2026-09-25
**Current Phase**: Phase 3 - Memory Management & Segmentation
**Next Focus**: Implement proper x86 memory segmentation and addressing