# BrutalVim

**Custom Vim configuration focused on minimalism and efficiency. It's BRUTAL!**

A fork of Neovim with progressive difficulty modes that force users to master Vim navigation and break bad habits through increasingly challenging constraints. Based on an idea by Laslo Szeremi.

Informally known at my company as "bastardvim."

## Overview

BrutalVim extends Neovim with four difficulty modes that modify editor behavior through command-line arguments. Each mode provides a different challenge level, from beginner-friendly to absolute chaos.

## Features

### 🟢 EASY Mode (`--easy`)
Perfect for beginners transitioning from other editors. Makes Vim feel like a modern editor while gently introducing modal concepts.

**Navigation & Selection (Like Windows/Mac/VSCode):**
- **Arrow keys**: Character-wise movement in all directions
- **Ctrl+Arrow**: Word-wise jumping (left/right by word)
- **Shift+Arrow**: Character-wise text selection (starts visual mode automatically)
- **Ctrl+Shift+Arrow**: Word-wise text selection (bonus combo!)

**Windows-Style Editing Shortcuts:**
- **Ctrl+Z**: Undo (maps to `u`)
- **Ctrl+C**: Copy selected text (works in visual mode)
- **Ctrl+X**: Cut selected text (works in visual mode)
- **Ctrl+V**: Paste (use `"*p` or `"+p` for system clipboard)

**Easy Exit Options (No More :q! Confusion):**
- Hold `ESC` for 5 seconds → Quit with save prompt
- Press `ESC` three times within 5 seconds → Quit prompt
- Triple-press `Ctrl+X`, `Ctrl+C`, or `Ctrl+Q` → Quit prompt
- **Easter egg**: Type "fuck you let me out" anywhere → Instant quit!

**Quick Start Workflow:**
```bash
nvim --easy myfile.txt
# 1. Press 'i' to insert text
# 2. Type your content
# 3. Press ESC to return to normal mode
# 4. Use Shift+Arrow to select text
# 5. Press Ctrl+C to copy, Ctrl+X to cut
# 6. Hold ESC for 5 seconds to quit
```

### 🟡 HARD Mode (`--hard`)
Forces proper Vim navigation habits.
- **Disabled**: Arrow keys, PageUp/Down, Home/End
- **Requires**: hjkl navigation, word motions (w/b/e)
- Tracks arrow key attempts and shows stats on quit

### 🟠 HARDER Mode (`--harder`)
All HARD restrictions plus intentional quitting.
- **Additionally disabled**: `:q`, `:quit`, `:qall`, `zz` (center screen)
- **Must use**: `:wq!` or `:x!` to exit
- Forces deliberate actions and prevents muscle memory exits

### 🔴 HARDEST Mode (`--hardest`)
Complete keyboard chaos with randomized keybindings.
- **All keybindings randomized** within logical groups (motion, edit, visual, search)
- **Disabled**: All HARDER mode restrictions apply
- **Fisher-Yates shuffle**: Ensures one-to-one mapping consistency
- **Session-based**: Different random mappings each launch
- Keys stay within their logical groups (e.g., motion keys remain motion keys)

## Installation

### From Source

```bash
git clone https://github.com/mcollard0/brutalvim.git
cd brutalvim
make CMAKE_BUILD_TYPE=RelWithDebInfo
sudo make install
```

## Usage

Launch with one of the difficulty flags:

```bash
nvim --easy myfile.txt      # Beginner mode
nvim --hard myfile.txt      # No arrow keys
nvim --harder myfile.txt    # No easy quit
nvim --hardest myfile.txt   # Randomized chaos
```

## Implementation Status

### ✅ Fully Implemented
- **HARD Mode**: Complete with input filtering in `getchar.c`
- **HARDER Mode**: Quit blocking in `ex_docmd.c`, `zz` blocking in `normal.c`
- **HARDEST Mode**: Full randomization system with Fisher-Yates shuffle
- **Startup Banners**: ASCII art display for all modes showing restrictions

### 🚧 Partially Implemented
- **EASY Mode**: Core features implemented, Windows shortcuts need additional hooks

### 📁 Architecture
```
src/nvim/
├── brutal.h          # Mode enum, global state, function declarations
├── brutal.c          # All brutal mode logic (~350 lines)
├── main.c            # CLI argument parsing integration
├── getchar.c         # Key filtering and remapping
├── ex_docmd.c        # Quit command blocking
└── normal.c          # zz command blocking
```

## Key Technical Details

### Mode Tracking
- Global enum `BrutalMode` in `brutal.h` (NONE, EASY, HARD, HARDER, HARDEST)
- Initialized during `main.c` startup via CLI parsing
- Persists for entire session

### Key Filtering (HARD/HARDER/HARDEST)
- `brutal_should_block_key()` - O(1) checks for arrow/cursor keys
- Integrated at lowest input level in `getchar.c`
- Returns `K_IGNORE` for blocked keys

### Randomization (HARDEST)
- `brutal_init_keymap_hardest()` - Fisher-Yates shuffle algorithm
- Keys grouped by function: motion (19), edit (21), visual (3), search (6), marks (3)
- Seeded with `time(NULL)` for session uniqueness
- `brutal_remap_key()` - O(1) array lookup for remapping

### Quit Blocking (HARDER/HARDEST)
- `brutal_should_block_quit()` - Boolean check
- Intercepted in `ex_quit()` and `ex_quitall()` functions
- Forces `:wq!` or `:x!` usage

## Performance

- **Key remapping**: O(1) array lookup
- **Key blocking**: Simple conditional checks
- **Startup overhead**: Minimal (banner display + mode init)
- **Negligible impact** on editor operations

## Documentation

- `architecture.md` - Detailed implementation specs (734 lines)
- `BRUTAL_MODES.md` - User guide with examples and tips (225 lines)
- `BUILD.md` - Build instructions
- `CONTRIBUTING.md` - Development guidelines

## Philosophy

- **EASY**: Remove intimidation, familiarize with Vim concepts
- **HARD**: Force muscle memory for hjkl navigation
- **HARDER**: Build discipline through deliberate actions
- **HARDEST**: Embrace chaos, truly understand modal editing

## Testing

Manual testing required:
- `--hard`: Verify arrow keys blocked, hjkl functional
- `--harder`: Verify quit commands blocked, zz disabled
- `--hardest`: Verify randomization, consistency within session
- `--easy`: Verify ESC-hold quit, shortcuts functional
- Normal mode: Verify no regression without flags

## Future Enhancements

1. Complete EASY mode Windows-style shortcuts with proper insert/visual mode hooks
2. Persistent randomization seeds for HARDEST mode reproducibility
3. Visual feedback for blocked key presses
4. `:BrutalMode` command to query current mode
5. In-session mode switching capability
6. Automated test suite for all modes
7. Integration with Neovim help system (`:help brutal`)

## Code Statistics

- **New files**: 2 (brutal.h, brutal.c)
- **Modified files**: 4 (main.c, getchar.c, ex_docmd.c, normal.c)
- **Lines added**: ~437 total
  - brutal.c: ~350 lines
  - Integration code: ~87 lines
- **Build system changes**: 0 (automatic via CMake glob)

## Architecture Evaluation

### Strengths
- **Minimal invasiveness**: Only 4 files modified with focused changes
- **Centralized logic**: All mode code in brutal.c/h
- **Clean separation**: Mode functions provide clear API boundaries
- **Efficient implementation**: Optimal algorithms at lowest input levels
- **Extensible design**: Easy to add new modes or modify behavior

### Current Limitations
- EASY mode incomplete (Windows shortcuts need deeper integration)
- No mode persistence between sessions
- No runtime mode switching
- Limited automated testing coverage
- Silent failure for blocked operations (except quit commands)

## Credits

- **Original Concept**: Laslo Szeremi
- **Implementation**: Fork of Neovim with custom extensions
- **Base Project**: [Neovim](https://github.com/neovim/neovim)

## License

Same as Neovim (Apache 2.0 License). See [LICENSE.txt](LICENSE.txt).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for development guidelines.

## Support

For issues specific to BrutalVim modes, please check:
1. `BRUTAL_MODES.md` - Comprehensive usage guide
2. `architecture.md` - Implementation details
3. GitHub Issues - Report bugs or request features

---

**Remember**: The best way to learn Vim is to embrace the brutality! 💪

Actual User Reviews: 

*"It's not a bug, it's a feature that makes you better."*

"It's Brutal"

"Bastard Mode Activated!" 

"F*ck you let me out!"
