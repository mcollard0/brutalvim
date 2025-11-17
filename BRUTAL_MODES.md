# BrutalVim Modes Guide

BrutalVim extends Neovim with four difficulty modes that modify editor behavior. Choose your challenge level!

## Starting BrutalVim

Launch with one of these flags:

```bash
nvim --easy      # Beginner-friendly mode with extra shortcuts
nvim --hard      # No arrow keys - learn hjkl!
nvim --harder    # No arrow keys, no easy quit
nvim --hardest   # Complete chaos - all keys randomized!
```

## Mode Details

### EASY Mode (`--easy`)

Perfect for beginners or those transitioning from other editors.

**Windows-Style Shortcuts:**
- `Ctrl+Z` → Undo (same as `u` in normal mode)
- `Ctrl+C` → Copy (works in visual mode)
- `Ctrl+X` → Cut (works in visual mode)
- `Ctrl+V` → Paste (use `"*p` or `"+p` for system clipboard)

**Easy Quit Options:**
- Hold `ESC` for 5 seconds → Quit
- Press `Ctrl+X`, `Ctrl+C`, or `Ctrl+Q` three times in a row → Quit prompt
- Press `ESC` once in normal mode → Quit prompt

**Quick Start Example:**
1. Start: `nvim --easy myfile.txt`
2. Press `i` to enter insert mode
3. Type your text
4. Press `ESC` to return to normal mode
5. Press `Ctrl+Z` to undo if needed
6. Hold `ESC` for 5 seconds to quit

**Copying and Pasting:**
```
1. Press 'v' to enter visual mode
2. Move cursor to select text
3. Press Ctrl+C to copy
4. Move cursor to destination
5. Press "*p to paste from system clipboard
```

---

### HARD Mode (`--hard`)

Forces you to learn proper Vim navigation.

**Disabled:**
- Arrow keys ( UP, DOWN, LEFT, RIGHT )
- PageUp, PageDown
- Home, End

**Use Instead:**
- `h` → Move left
- `j` → Move down
- `k` → Move up
- `l` → Move right
- `w` → Next word
- `b` → Previous word
- `0` → Start of line
- `$` → End of line

**Quick Navigation Tips:**
- `gg` → Top of file
- `G` → Bottom of file
- `3j` → Move down 3 lines
- `5w` → Move forward 5 words

---

### HARDER Mode (`--harder`)

All HARD restrictions plus you can't easily quit.

**Disabled:**
- All arrow keys (like HARD mode)
- `:q`, `:quit`, `:qall` commands
- `zz` command (center screen)

**How to Exit:**
Since quit is blocked, you'll need to use alternative methods:
- Save and force quit: `:wq!` or `:x!`
- Force quit without saving: `:q!` (may be blocked)
- Kill the terminal/process externally

**Purpose:**
Forces you to stay focused and think before quitting. Great for building discipline!

---

### HARDEST Mode (`--hardest`)

Complete keyboard chaos! All keybindings are randomized within logical groups.

**What's Randomized:**
- Motion keys (h,j,k,l,w,b,e, etc.)
- Edit commands (i,a,o,c,d,y,p, etc.)
- Visual mode keys (v, V, Ctrl+V)
- Search keys (/, ?, n, N, *, #)
- Marks (m, ', `)
- Numbers and punctuation

**What's NOT Randomized:**
- ESC key
- Colon (:) for command mode
- Some special keys

**Restrictions:**
- All HARDER mode restrictions apply
- No arrow keys
- Quit commands blocked
- `zz` blocked

**Strategy:**
- Keys are grouped logically - motion keys stay as motion keys
- Use `:help` to figure out what still works
- Trial and error is your friend
- Each session has different mappings ( time-based randomization )

**Example Session:**
```
You start nvim --hardest myfile.txt
The banner shows: "All keybindings randomized within logical groups"

Try pressing 'j' → Might move you right instead of down!
Try pressing 'i' → Might actually be 'o' ( open line below )!
Try pressing 'd' → Might be 'y' ( yank/copy )!

Good luck! Figure it out yourself.
```

---

## Tips and Tricks

### For EASY Mode Users:
- Use visual mode ( v ) before Ctrl+C/X for better results
- System clipboard paste: `"*p` or `"+p`
- The ESC-hold-to-quit is great for quick exits

### For HARD/HARDER Mode Users:
- Practice h/j/k/l until it becomes muscle memory
- Learn word motions: `w`, `b`, `e`, `ge`
- Learn line motions: `0`, `^`, `$`
- Use counts: `3j`, `5w`, `10k`

### For HARDEST Mode Users:
- Start with small files
- Keep `:help` open in another window
- Try to identify patterns in the randomization
- Remember that keys within groups stay within groups
- Accept that you'll be confused - that's the point!

### Universal Tips:
- `:help` is your friend in all modes
- Practice in a test file first
- Each mode shows a startup banner with its rules
- You can always Ctrl+Z ( outside vim ) to suspend and check the manual

---

## Comparison Table

| Feature | EASY | HARD | HARDER | HARDEST |
|---------|------|------|--------|---------|
| Arrow keys | ✓ Enabled | ✗ Disabled | ✗ Disabled | ✗ Disabled |
| Ctrl+Z undo | ✓ Enabled | Standard u | Standard u | Randomized |
| Windows shortcuts | ✓ Enabled | ✗ Disabled | ✗ Disabled | ✗ Disabled |
| Easy quit | ✓ Multiple ways | Standard :q | ✗ Blocked | ✗ Blocked |
| zz command | ✓ Enabled | ✓ Enabled | ✗ Blocked | ✗ Blocked |
| Keybindings | Standard | Standard | Standard | Randomized |
| Difficulty | ⭐ Easiest | ⭐⭐ Medium | ⭐⭐⭐ Hard | ⭐⭐⭐⭐⭐ BRUTAL |

---

## Building BrutalVim

```bash
cd brutalvim
make CMAKE_BUILD_TYPE=RelWithDebInfo
sudo make install
```

---

## Troubleshooting

**Q: I'm stuck in HARDER/HARDEST mode and can't quit!**
A: Use `:wq!` or `:x!` to force save and quit. Or kill the terminal.

**Q: In HARDEST mode, nothing works!**
A: Keys are randomized. Try different keys to find what does what. ESC and : still work.

**Q: Ctrl+C doesn't copy in EASY mode!**
A: Make sure you're in visual mode ( press `v` first ). Use `"*p` to paste system clipboard.

**Q: Arrow keys don't work in HARD mode!**
A: That's the point! Use `h` ( left ), `j` ( down ), `k` ( up ), `l` ( right ).

**Q: Can I switch modes without restarting?**
A: Not currently. You must restart nvim with a different flag.

---

## Philosophy

- **EASY**: Learn Vim concepts without the steep learning curve
- **HARD**: Force good habits by removing crutches
- **HARDER**: Build discipline by making quitting harder
- **HARDEST**: Embrace chaos and truly understand Vim's modal nature

Choose your difficulty and enjoy the challenge!

---

*Based on an idea by Laslo Szeremi*
