#pragma once

#include <stdbool.h>
#include <stdint.h>

/// Brutal mode types
typedef enum {
  BRUTAL_NONE = 0,     ///< Normal Neovim mode
  BRUTAL_EASY = 1,     ///< Easy mode with extra shortcuts
  BRUTAL_HARD = 2,     ///< Hard mode: no cursor keys
  BRUTAL_HARDER = 3,   ///< Harder mode: no cursor keys, no :q, no zz
  BRUTAL_HARDEST = 4,  ///< Hardest mode: randomized keybindings
} BrutalMode;

/// Global brutal mode state
extern BrutalMode brutal_mode;

/// Randomized keybinding map for HARDEST mode (256 entries for all possible chars)
extern uint8_t brutal_keymap[256];

/// Easy mode state tracking
extern uint64_t brutal_esc_hold_start;  ///< Unused (kept for compatibility)
extern int brutal_ctrl_quit_count;      ///< Count of consecutive ctrl+x/c/q presses
extern uint64_t brutal_esc_press_times[5];  ///< Timestamps of last 5 ESC presses
extern int brutal_esc_press_count;      ///< Count of ESC presses

/// Easter egg buffer for "fuck you let me out"
extern char brutal_easter_egg_buffer[32];
extern int brutal_easter_egg_pos;

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "brutal.h.generated.h"
#endif
