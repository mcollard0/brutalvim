#
# BrutalVim Architecture Evaluation 
# A README
#
## Overview
BrutalVim is a fork of Neovim (currently at upstream Neovim codebase state) with the goal of adding difficulty modes inspired by Laslo Szeremi's idea. The project aims to introduce four command-line arguments that modify editor behavior: `--hard`, `--harder`, `--hardest`, and `--easy`.
## Current State
### Documentation
The `architecture.md` file exists but is minimal (22 lines). It defines the four modes:
* **`--hard`**: Disables cursor keys, leaves `zz` and `:q` enabled
* **`--harder`**: Disables `zz` and `:q`
* **`--hardest`**: Randomizes all keybindings with logical replacements, disables copy/paste/yank/undo/visual mode, potentially disables echo in normal mode command line
* **`--easy`**: Multiple quit shortcuts (hold ESC 5 seconds, triple Ctrl+X/C/Q), Windows-style shortcuts (Ctrl+X/C/V for cut/copy/paste, Ctrl+Z for undo), cursor keys enabled, numpad navigation
### Implementation Status
**No implementation exists yet.** Grep searches for the flag strings (`--hard`, `--harder`, `--hardest`, `--easy`) across `src/` returned no matches. This is purely a design document at this stage.
### Codebase Structure
BrutalVim inherits the full Neovim codebase (~255,000 lines of C code in `src/nvim/` alone):
**Key Files for Implementation:**
* `src/nvim/main.c` - Command-line argument parsing (lines 1052-1504 in `command_line_scan()`)
* `src/nvim/getchar.c` - Character input and keybinding handling
* `src/nvim/mapping.c` - Key mapping system
* `src/nvim/tui/input.c` - Terminal UI input processing
* `src/nvim/normal.c` - Normal mode command processing
**Project Structure:**
* `src/nvim/` - Core application (organized into subsystems: api/, eval/, event/, lua/, os/, tui/)
* `runtime/` - Plugins and documentation
* `test/` - Test suite (exists, but no brutal mode tests yet)
* No `backup/` directory exists yet (per user rules, this should be created)
## Architecture Analysis
### Strengths
1. **Clear Command-Line Interface Design** - The flag-based approach (`--hard`, etc.) follows Neovim conventions and integrates naturally with existing CLI parsing in `main.c`
2. **Solid Foundation** - Built on Neovim's mature, well-architected codebase with comprehensive input/mapping systems
3. **Progressive Difficulty** - Four distinct modes provide a learning curve from easy to expert
### Challenges
1. **Complexity of --hardest Mode** - Randomizing keybindings while maintaining "logical replacements" is non-trivial. Need to define:
    * What constitutes a "logical replacement"?
    * How to ensure quit/escape mappings don't conflict?
    * Persistence of randomization (per-session? global seed?)
2. **Deep Integration Required** - Modifications touch multiple subsystems:
    * Command parsing (main.c)
    * Input handling (getchar.c, tui/input.c)
    * Mapping system (mapping.c)
    * Normal mode commands (normal.c)
3. **Testing Complexity** - Validating randomized keybindings and disabled features requires comprehensive test coverage
4. **Missing Specifications**:
    * How do modes interact with user config/plugins?
    * Should modes be mutually exclusive?
    * What happens with `--hard --easy` simultaneously?
    * Behavior in visual/insert/command modes?
### Recommendations
1. **Start with --hard Mode** - Simplest implementation (just disable cursor key inputs)
2. **Create Global Mode State** - Add `enum brutal_mode { BRUTAL_NONE, BRUTAL_EASY, BRUTAL_HARD, BRUTAL_HARDER, BRUTAL_HARDEST }` to track active mode
3. **Implement in Phases**:
    * Phase 1: CLI parsing + mode flag storage
    * Phase 2: Input filtering for --hard (disable cursor keys)
    * Phase 3: Command filtering for --harder (disable :q, zz)
    * Phase 4: --easy mode shortcuts
    * Phase 5: --hardest randomization (most complex)
4. **Define Randomization Algorithm** - For --hardest:
    * Use seeded PRNG for reproducibility
    * Create mapping categories (motion, editing, quitting, etc.)
    * Ensure one-to-one mapping (no key becomes unmapped)
    * Store seed in config for consistency
5. **Architecture Documentation Updates**:
    * Expand architecture.md with implementation details
    * Document which files will be modified
    * Add state machine diagram for mode interactions
    * Specify test strategy
6. **Create Backup Strategy** - Per user rules, establish `backup/` directory with dated copies before major changes
## Gap Analysis
### Missing from architecture.md
* File modification plan
* State management approach
* Randomization algorithm specification
* Mode conflict resolution
* Plugin/config interaction behavior
* Testing approach
* Performance considerations
### Missing from Codebase
* `backup/` directory
* `warp.md` (user rule mentions checking this)
* Implementation plan document
* Any brutal mode code
* Tests for brutal modes
## Implementation Complexity Estimate
* **--hard**: Low (2-3 hours) - Simple input filtering
* **--harder**: Low-Medium (4-6 hours) - Command interception
* **--easy**: Medium (8-12 hours) - New keybindings, timer logic for hold-ESC
* **--hardest**: High (20-30 hours) - Randomization system, extensive testing

This is informally known at my company as bastardvim. 

## Next Steps
1. Expand architecture.md with missing specifications
2. Create backup/ directory structure
3. Create implementation plan with file-by-file changes
4. Implement --hard mode first (proof of concept)
5. Add test coverage for --hard
6. Iterate through remaining modes
