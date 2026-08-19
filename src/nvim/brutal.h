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
extern bool brutal_bypass_quit_block;

/// Randomized keybinding map for HARDEST mode (256 entries for all possible chars)
extern uint8_t brutal_keymap[256];

/// Easy mode state tracking
extern uint64_t brutal_esc_hold_start;  ///< Unused (kept for compatibility)
extern int brutal_ctrl_quit_count;      ///< Count of consecutive ctrl+x/c/q presses
extern uint64_t brutal_ctrl_quit_times[5];  ///< Timestamps of last 5 Ctrl-C/X/Q presses
extern uint64_t brutal_esc_press_times[5];  ///< Timestamps of last 5 ESC presses
extern int brutal_esc_press_count;      ///< Count of ESC presses

/// Easter egg buffer for "fuck you let me out"
extern char brutal_easter_egg_buffer[32];
extern int brutal_easter_egg_pos;

/// Function prototypes
void brutal_init(void);
void brutal_show_banner(void);
bool brutal_should_block_key(int c);
int brutal_apply_easy_mode_mappings(int c);
int brutal_remap_key(int c);
bool brutal_windows_keys_active(void);
bool brutal_should_block_quit(bool force);
bool brutal_easy_mode_quit_check(int c);
bool brutal_easy_mode_esc_repeated(void);
bool brutal_check_easter_egg(void);
void brutal_record_char(int c);
void brutal_handle_esc_press(void);
bool brutal_check_repeated_esc(void);

// New functions for managing quit counter explicitly
void brutal_reset_ctrl_quit_count(void);
void brutal_increment_ctrl_quit_count(void);
bool brutal_check_ctrl_quit(void);

#ifdef INCLUDE_GENERATED_DECLARATIONS
# include "brutal.h.generated.h"
#endif
