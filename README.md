<img align=center width="1066" height="836" alt="image" src="https://github.com/user-attachments/assets/d6d7aae6-2d31-491b-8d6b-76a5678d683d" />

#
# BrutalVim Architecture Evaluation 
#
# A README
#

## Overview
BrutalVim is a fork of Neovim (currently at upstream Neovim codebase state) with the goal of adding difficulty modes inspired by Laslo Szeremi's idea. The project aims to introduce four command-line arguments that modify editor behavior: `--hard`, `--harder`, `--hardest`, and `--easy`.

# BrutalVim Codebase Analysis

## Executive Summary

**BrutalVim** is a fork of Neovim that implements progressive difficulty modes to help users master Vim motions and break bad editing habits. The project modifies the core Neovim editor behavior through command-line arguments (`--easy`, `--hard`, `--harder`, `--hardest`) to create varying levels of challenge or assistance.

**Project Origin:** Based on an idea by Laslo Szeremi  
**Base:** Neovim (forked from upstream)  
**Primary Language:** C (257,849 lines in `src/nvim/`)  
**Secondary Language:** Lua (code generation scripts)  
**Total Files:** 493 C source/header files in `src/`

---

## Project Function

BrutalVim serves as an **educational training tool** for Vim users by:

1. **Removing crutches** (arrow keys, easy quit commands) to force muscle memory development
2. **Adding assistance** (Windows-style shortcuts, multiple quit methods) for beginners
3. **Creating chaos** (randomized keybindings) to test true Vim mastery
4. **Progressive learning** through four distinct difficulty levels

The editor functions identically to standard Neovim when no brutal mode flag is specified.

---

## Architecture Overview

### High-Level Design

```
┌─────────────────────────────────────────────────────────────┐
│                      Neovim Core                            │
│  (257,849 lines of C code - inherited from upstream)        │
└─────────────────────────────────────────────────────────────┘
                              │
                    ┌─────────┴──────────┐
                    │  BrutalVim Layer   │
                    │  (350 lines C code)│
                    └─────────┬──────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        │                     │                     │
    ┌───▼────┐          ┌─────▼─────┐        ┌─────▼──────┐
    │ main.c │          │ getchar.c │        │ ex_docmd.c │
    │ CLI    │          │ Input     │        │ Commands   │
    │ Parsing│          │ Filtering │        │ Blocking   │
    └────────┘          └───────────┘        └────────────┘
                              │
                        ┌─────▼──────┐
                        │  normal.c  │
                        │  Command   │
                        │  Blocking  │
                        └────────────┘
```

### Core Components

**1. Brutal Mode State Management** (`brutal.h`, `brutal.c`)
- Centralized mode logic
- Global state tracking
- Keybinding randomization algorithms
- Easter egg detection system

**2. Command-Line Interface** (`main.c`)
- Parses `--easy`, `--hard`, `--harder`, `--hardest` flags
- Initializes brutal mode system on startup
- Displays ASCII banner

**3. Input Processing** (`getchar.c`)
- Filters blocked keys (arrow keys, navigation keys)
- Applies keybinding remapping for HARDEST mode
- Windows-style key mapping for EASY mode
- Easter egg character tracking

**4. Command Execution** (`ex_docmd.c`)
- Blocks `:q`, `:quit`, `:qall` commands in HARDER/HARDEST modes
- Allows forced quit in EASY mode

**5. Normal Mode Commands** (`normal.c`)
- Blocks `zz` (center screen) command in HARDER/HARDEST modes

---

## Languages and Technologies

### Primary Language: C

**Usage:** 99% of codebase (core editor functionality)
- **Standard:** C11
- **Style:** K&R with Neovim conventions
- **Key Features Used:**
  - Enums for state management (`BrutalMode`)
  - Function pointers for callbacks
  - Static functions for internal helpers
  - Fisher-Yates shuffle algorithm for randomization

**Brutal-Specific Code:**
- `src/nvim/brutal.h` (55 lines) - Header definitions
- `src/nvim/brutal.c` (541 lines) - Implementation
- Modified: `main.c`, `getchar.c`, `ex_docmd.c`, `normal.c` (~87 lines added)

### Secondary Language: Lua

**Usage:** Code generation scripts (not runtime)
- Located in `src/gen/`
- Generate API bindings, documentation, metadata
- Not used for brutal mode implementation

### Build System

**CMake** - Automatic glob-based inclusion
- No manual CMakeLists.txt modifications required for brutal.c
- Automatic header generation via `INCLUDE_GENERATED_DECLARATIONS`

---

## Mode Specifications

### 1. EASY Mode (`--easy`)

**Philosophy:** Remove intimidation, provide escape hatches

**Features:**
- Arrow keys enabled (standard behavior)
- Easter egg: Type "fuck you let me out" (case-insensitive, spaces optional) to force quit
- Windows-style shortcuts:
  - `Ctrl+Z` → Undo
  - `Ctrl+C` → Copy (visual mode)
  - `Ctrl+X` → Cut (visual mode)
  - `Ctrl+V` → Paste
- Shift\+Arrow for text selection
- Multiple quit methods (force quit with `:q!` allowed)

**Technical Implementation:**
- Key remapping in `brutal_apply_easy_mode_mappings()`
- Easter egg detection via substring matching
- Character buffer tracking (31-char rolling window)
- Timer-based Ctrl\+X/C/Q triple-press detection (8-second window)
- ESC triple-press detection (10-second window)

**Complexity:** `O(1)` for key checks, `O(n)` for easter egg string matching

---

### 2. HARD Mode (`--hard`)

**Philosophy:** Force hjkl muscle memory, break arrow key dependence

**Restrictions:**
- Arrow keys blocked (UP, DOWN, LEFT, RIGHT)
- PageUp, PageDown, Home, End disabled
- Must use `h/j/k/l` for navigation

**Technical Implementation:**
- `brutal_should_block_key()` returns true for cursor keys
- Blocked keys return `K_IGNORE` (no action)
- Applied in `getchar.c` at input processing layer

**Complexity:** `O(1)` - simple conditional checks

---

### 3. HARDER Mode (`--harder`)

**Philosophy:** Prevent muscle memory exits during flow

**Restrictions:**
- All HARD mode restrictions
- `:q`, `:quit`, `:qall` commands blocked
- `zz` (center screen) command blocked
- Only forced quits potentially work (if not in headless/embedded mode)

**Technical Implementation:**
- `brutal_should_block_quit()` checks mode state
- Quit blocking in `ex_quit()` and `ex_quitall()`
- `zz` blocking in `nv_zet()` function
- Error message: "Quit commands are disabled in this brutal mode."

**Complexity:** `O(1)` - boolean flag checks

---

### 4. HARDEST Mode (`--hardest`)

**Philosophy:** Complete chaos, muscle memory reset

**Restrictions:**
- All HARDER mode restrictions
- **ALL keybindings randomized** within logical groups
- Copy/paste/yank/undo operations remapped
- Visual mode keys remapped

**Randomization Algorithm:**
- **Fisher-Yates shuffle** for permutation generation
- Time-based seeding: `srand(time(NULL))`
- **Logical grouping** maintains usability:
  - Motion keys (h/j/k/l/w/b/e/0/\$/etc.) shuffle within motion group
  - Edit keys (i/a/o/c/d/y/p/x/u/etc.) shuffle within edit group
  - Visual keys (v/V/Ctrl\+V) shuffle together
  - Search keys (//?/n/N/\*/\#) shuffle together
  - Mark keys (m/'/\`) shuffle together

**Technical Implementation:**
- `brutal_init_keymap_hardest()` generates 256-element lookup table
- `brutal_remap_key()` applies mapping: `brutal_keymap[c]`
- Identity mapping for non-randomized keys
- Session-consistent (same seed per launch)

**Complexity:** 
- Initialization: `O(n)` where n = number of keys in group
- Lookup: `O(1)` - array access

---

## Key Data Structures

### BrutalMode Enum

```c
typedef enum {
  BRUTAL_NONE = 0,     // Normal Neovim mode
  BRUTAL_EASY = 1,     // Easy mode with extra shortcuts
  BRUTAL_HARD = 2,     // Hard mode: no cursor keys
  BRUTAL_HARDER = 3,   // Harder mode: no cursor keys, no :q, no zz
  BRUTAL_HARDEST = 4,  // Hardest mode: randomized keybindings
} BrutalMode;
```

### Global State Variables

```c
BrutalMode brutal_mode;                        // Current active mode
uint8_t brutal_keymap[256];                    // HARDEST mode key remapping table
uint64_t brutal_esc_hold_start;                // ESC hold timer (unused)
int brutal_ctrl_quit_count;                    // Ctrl+X/C/Q press counter
uint64_t brutal_ctrl_quit_times[5];            // Timestamp buffer for quit detection
uint64_t brutal_esc_press_times[5];            // Timestamp buffer for ESC detection
int brutal_esc_press_count;                    // ESC press counter
char brutal_easter_egg_buffer[32];             // Rolling character buffer
int brutal_easter_egg_pos;                     // Current buffer position
```

---

## Testing Strategy

### Current Status: **Manual Testing Only**

**No automated tests exist** - requires comprehensive test suite implementation.

### Recommended Test Coverage

1. **Unit Tests** (C - check/cmocka framework):
   - `brutal_should_block_key()` for all key codes
   - `brutal_remap_key()` for identity preservation
   - `brutal_should_block_quit()` for all mode combinations
   - Fisher-Yates shuffle correctness
   - Easter egg detection edge cases

2. **Integration Tests**:
   - CLI argument parsing (`--hard`, etc.)
   - Mode initialization sequence
   - Key filtering in various editor states (insert/normal/visual)
   - Quit blocking in ex commands
   - Banner display on startup

3. **Regression Tests**:
   - Normal mode (no brutal flags) unaffected
   - Headless mode bypasses quit blocking
   - Embedded mode bypasses quit blocking

---

## Enhancement Ideas

### High Priority

1. **Persistent Randomization Seeds**
   - Save HARDEST mode seed to `~/.config/nvim/brutal_seed`
   - Allow reproducible sessions for training
   - Complexity: Low (file I/O)

2. **Visual Feedback for Blocked Keys**
   - Display hint messages when blocked keys pressed
   - "Use h/j/k/l" reminder in HARD mode
   - Complexity: Low (message API calls)

3. **Configuration File Support**
   - JSON/TOML config for mode customization
   - Adjustable time windows, key groups
   - Complexity: Medium (parsing library integration)

4. **Progress Tracking & Statistics**
   - Arrow key attempt counter (HARD mode)
   - Session duration tracking
   - Mistake rate calculation
   - Complexity: Medium (persistence \+ UI)

### Medium Priority

5. **Mode Switching Without Restart**
   - `:BrutalMode [easy|hard|harder|hardest]` command
   - Dynamic keybinding regeneration
   - Complexity: Medium (runtime state management)

6. **Help Command for HARDEST Mode**
   - `:BrutalHelp` to show current key mappings
   - Cheat sheet generation
   - Complexity: Low (formatting \+ display)

7. **Achievement System**
   - Badges for consistent usage
   - Streak tracking
   - Complexity: High (gamification design)

8. **Tutorial Mode**
   - Interactive lessons per difficulty
   - Guided challenges
   - Complexity: Very High (content creation)

### Low Priority / Advanced

9. **Multiplayer Stats Comparison**
   - Compare progress with friends
   - Leaderboards
   - Complexity: Very High (backend infrastructure)

10. **Timed Challenges**
    - Speed-run mode with scoring
    - Vim golf integration
    - Complexity: High (timing \+ scoring logic)

11. **Custom Difficulty Profiles**
    - User-defined key blocking rules
    - Plugin API for mode extensions
    - Complexity: Very High (plugin architecture)

---

## Known Issues & Limitations

### Implementation Gaps

1. **EASY Mode Windows Shortcuts Not Fully Functional**
   - `Ctrl+C/V/X` remapping incomplete
   - Shift\+Arrow text selection needs testing
   - Status: Partial implementation

2. **No Undo/Redo in Mode Switching**
   - Must restart editor to change modes
   - Session state lost on mode change

3. **Easter Egg Debug Logging**
   - Writes to `/tmp/brutal_debug.log`
   - Should be conditional on debug flag
   - Potential security concern (log injection)

### Design Constraints

4. **Randomization Not User-Controllable**
   - Time-based seed only
   - No deterministic mode for testing
   - Users can't "re-roll" mappings

5. **Quit Blocking Too Aggressive**
   - No escape hatch in HARDEST mode

