/// @file brutal.c
///
/// BrutalVim mode system implementation
/// Provides EASY, HARD, HARDER, and HARDEST difficulty modes

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nvim/ascii_defs.h"
#include "nvim/brutal.h"
#include "nvim/globals.h"
#include "nvim/keycodes.h"
#include "nvim/message.h"
#include "nvim/os/time.h"
#include "nvim/vim_defs.h"
#include "nvim/option.h"

BrutalMode brutal_mode = BRUTAL_NONE;
uint8_t brutal_keymap[256];
uint64_t brutal_esc_hold_start = 0;
int brutal_ctrl_quit_count = 0;
uint64_t brutal_esc_press_times[3] = {0};
int brutal_esc_press_count = 0;
char brutal_easter_egg_buffer[32] = {0};
int brutal_easter_egg_pos = 0;

/// Initialize brutal mode keybinding randomization for HARDEST mode
/// Uses logical replacement groups to maintain some usability
static void brutal_init_keymap_hardest( void )
{
  // First, initialize identity mapping
  for ( int i = 0; i < 256; i++ ) {
    brutal_keymap[i] = ( uint8_t )i;
  }

  // Define logical replacement groups
  // Motion keys - should swap within motion group
  char motion_keys[] = { 'h', 'j', 'k', 'l', 'w', 'b', 'e', 'W', 'B', 'E', 
                         '0', '^', '$', 'G', 'g', 'f', 'F', 't', 'T', '%' };
  int motion_count = sizeof( motion_keys ) / sizeof( motion_keys[0] );

  // Edit keys - swap within edit group
  char edit_keys[] = { 'i', 'a', 'o', 'O', 'I', 'A', 'c', 'd', 'y', 'p', 'P',
                       'x', 'X', 's', 'S', 'r', 'R', 'u', 'U', '~', 'J' };
  int edit_count = sizeof( edit_keys ) / sizeof( edit_keys[0] );

  // Visual/select keys
  char visual_keys[] = { 'v', 'V', Ctrl_V & 0x7F };
  int visual_count = sizeof( visual_keys ) / sizeof( visual_keys[0] );

  // Search keys
  char search_keys[] = { '/', '?', 'n', 'N', '*', '#' };
  int search_count = sizeof( search_keys ) / sizeof( search_keys[0] );

  // Mark/jump keys
  char mark_keys[] = { 'm', '\'', '`' };
  int mark_count = sizeof( mark_keys ) / sizeof( mark_keys[0] );

  // Seed RNG with current time for randomization
  srand( ( unsigned int )time( NULL ) );

  // Shuffle each group using Fisher-Yates algorithm
  auto void shuffle_group( char *keys, int count ) {
    for ( int i = count - 1; i > 0; i-- ) {
      int j = rand() % ( i + 1 );
      char temp = keys[i];
      keys[i] = keys[j];
      keys[j] = temp;
    }
  }

  shuffle_group( motion_keys, motion_count );
  shuffle_group( edit_keys, edit_count );
  shuffle_group( visual_keys, visual_count );
  shuffle_group( search_keys, search_count );
  shuffle_group( mark_keys, mark_count );

  // Apply shuffled mappings
  auto void apply_shuffled_mapping( const char *original, const char *shuffled, int count ) {
    for ( int i = 0; i < count; i++ ) {
      brutal_keymap[( uint8_t )original[i]] = ( uint8_t )shuffled[i];
    }
  }

  char motion_original[] = { 'h', 'j', 'k', 'l', 'w', 'b', 'e', 'W', 'B', 'E',
                             '0', '^', '$', 'G', 'g', 'f', 'F', 't', 'T', '%' };
  char edit_original[] = { 'i', 'a', 'o', 'O', 'I', 'A', 'c', 'd', 'y', 'p', 'P',
                           'x', 'X', 's', 'S', 'r', 'R', 'u', 'U', '~', 'J' };
  char visual_original[] = { 'v', 'V', Ctrl_V & 0x7F };
  char search_original[] = { '/', '?', 'n', 'N', '*', '#' };
  char mark_original[] = { 'm', '\'', '`' };

  apply_shuffled_mapping( motion_original, motion_keys, motion_count );
  apply_shuffled_mapping( edit_original, edit_keys, edit_count );
  apply_shuffled_mapping( visual_original, visual_keys, visual_count );
  apply_shuffled_mapping( search_original, search_keys, search_count );
  apply_shuffled_mapping( mark_original, mark_keys, mark_count );

  // Randomize remaining common keys (numbers, some punctuation)
  char other_keys[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9',
                        '.', ',', ';', ':', '-', '+', '=', '<', '>', '[', ']', '{', '}' };
  int other_count = sizeof( other_keys ) / sizeof( other_keys[0] );
  char other_shuffled[sizeof( other_keys )];
  memcpy( other_shuffled, other_keys, sizeof( other_keys ) );
  shuffle_group( other_shuffled, other_count );
  apply_shuffled_mapping( other_keys, other_shuffled, other_count );
}

/// Initialize brutal mode system
void brutal_init( void )
{
  if ( brutal_mode == BRUTAL_HARDEST ) {
    brutal_init_keymap_hardest();
  }
  
  // EASY mode: Enable shift-arrow text selection and clipboard operations
  if ( brutal_mode == BRUTAL_EASY ) {
    // Set keymodel to include "startsel" for shift-selection
    km_startsel = true;
    
    // Set clipboard to use system clipboard by default
    // This makes y/d/p operations use the + register automatically
    // Note: Full Ctrl+C/X/V integration requires normal.c hooks
    // For now, users can use "+y, "+d, "+p or set clipboard=unnamedplus
  }
}

/// Check if we should start EASY mode in insert mode
/// Called after initialization to set restart_edit
bool brutal_should_start_insert( void )
{
  return brutal_mode == BRUTAL_EASY;
}

/// Display brutal mode startup banner
void brutal_show_banner( void )
{
  if ( brutal_mode == BRUTAL_NONE ) {
    return;
  }

  // ASCII art banner
  msg_puts( "\n" );
  msg_puts( "██████╗ ██████╗ ██╗   ██╗████████╗ █████╗ ██╗     ██╗   ██╗██╗███╗   ███╗\n" );
  msg_puts( "██╔══██╗██╔══██╗██║   ██║╚══██╔══╝██╔══██╗██║     ██║   ██║██║████╗ ████║\n" );
  msg_puts( "██████╔╝██████╔╝██║   ██║   ██║   ███████║██║     ██║   ██║██║██╔████╔██║\n" );
  msg_puts( "██╔══██╗██╔══██╗██║   ██║   ██║   ██╔══██║██║     ╚██╗ ██╔╝██║██║╚██╔╝██║\n" );
  msg_puts( "██████╔╝██║  ██║╚██████╔╝   ██║   ██║  ██║███████╗ ╚████╔╝ ██║██║ ╚═╝ ██║\n" );
  msg_puts( "╚═════╝ ╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝╚══════╝  ╚═══╝  ╚═╝╚═╝     ╚═╝\n" );
  msg_puts( "\n" );

  // Mode indicator
  switch ( brutal_mode ) {
    case BRUTAL_EASY:
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "                              [ EASY MODE ]                                 \n" );
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "\n" );
      msg_puts( "Enabled features:\n" );
      msg_puts( "  • Starts in INSERT mode (ready to type immediately)\n" );
      msg_puts( "  • Arrow keys enabled for navigation\n" );
      msg_puts( "  • Shift+Arrow keys for text selection (like most editors)\n" );
      msg_puts( "  • Ctrl+Arrow keys for word-wise movement\n" );
      msg_puts( "  • Press ESC to enter command mode\n" );
      msg_puts( "  • In command mode: Press ESC three times (within 5s) to quit\n" );
      msg_puts( "  • In command mode: Hold ESC for 5 seconds to quit\n" );
      msg_puts( "  • Easter egg: Type 'fuck you let me out' anywhere to quit!\n" );
      msg_puts( "\n" );
      msg_puts( "Modified keybindings (Windows-style):\n" );
      msg_puts( "  • Ctrl+Z  →  Undo (u)\n" );
      msg_puts( "  • Ctrl+C  →  Copy to clipboard (in visual mode)\n" );
      msg_puts( "  • Ctrl+X  →  Cut to clipboard (in visual mode)\n" );
      msg_puts( "  • Enter   →  Copy to clipboard (in visual mode)\n" );
      msg_puts( "  • Ctrl+V  →  Paste from clipboard\n" );
      msg_puts( "\n" );
      msg_puts( "Tip: Use Shift+Arrow to select, Enter or Ctrl+C to copy.\n" );
      msg_puts( "     Ctrl+X cuts, Ctrl+V pastes. Ctrl+Arrow jumps words.\n" );
      msg_puts( "     Ctrl+C does NOT exit in insert mode - only in command mode!\n" );
      break;

    case BRUTAL_HARD:
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "                              [ HARD MODE ]                                 \n" );
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "\n" );
      msg_puts( "Restrictions:\n" );
      msg_puts( "  • Arrow keys disabled\n" );
      msg_puts( "  • Use h/j/k/l for navigation\n" );
      break;

    case BRUTAL_HARDER:
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "                             [ HARDER MODE ]                                \n" );
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "\n" );
      msg_puts( "Restrictions:\n" );
      msg_puts( "  • Arrow keys disabled\n" );
      msg_puts( "  • Standard quit commands disabled\n" );
      msg_puts( "  • Exit commands blocked\n" );
      break;

    case BRUTAL_HARDEST:
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "                            [ HARDEST MODE ]                                \n" );
      msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
      msg_puts( "\n" );
      msg_puts( "Restrictions:\n" );
      msg_puts( "  • All keybindings randomized within logical groups\n" );
      msg_puts( "  • Arrow keys disabled\n" );
      msg_puts( "  • Copy/paste/yank operations remapped\n" );
      msg_puts( "  • Undo operation remapped\n" );
      msg_puts( "  • Visual mode keys remapped\n" );
      msg_puts( "  • Standard quit commands disabled\n" );
      msg_puts( "\n" );
      msg_puts( "  Good luck! Figure out the new mappings yourself.\n" );
      break;

    default:
      break;
  }

  msg_puts( "\n" );
  msg_puts( "═══════════════════════════════════════════════════════════════════════════\n" );
  msg_puts( "\n" );
}

/// Check if a key should be blocked in current brutal mode
/// @param c The character to check
/// @return true if the key should be blocked, false otherwise
bool brutal_should_block_key( int c )
{
  // Allow all keys in headless or embedded mode
  if ( headless_mode || embedded_mode ) {
    return false;
  }

  // EASY mode: Allow all arrow keys including shift-arrow for text selection
  if ( brutal_mode == BRUTAL_EASY ) {
    return false;
  }

  // HARD, HARDER, HARDEST: Block arrow keys (including shift variants)
  if ( brutal_mode >= BRUTAL_HARD ) {
    if ( c == K_UP || c == K_DOWN || c == K_LEFT || c == K_RIGHT ||
         c == K_S_UP || c == K_S_DOWN || c == K_S_LEFT || c == K_S_RIGHT ||
         c == K_PAGEUP || c == K_PAGEDOWN || c == K_HOME || c == K_END ) {
      return true;
    }
  }

  return false;
}

/// Apply Windows-style key mappings for EASY mode
/// @param c The character to remap
/// @return The remapped character (or original if no mapping)
int brutal_apply_easy_mode_mappings( int c )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return c;
  }

  // Windows-style shortcuts for EASY mode
  switch ( c ) {
    case Ctrl_Z:
      // Map Ctrl+Z to 'u' (undo)
      return 'u';
    
    default:
      return c;
  }
}

// EASY mode Ctrl+C/X/V and Enter mappings are implemented in normal.c
// See normal_execute() function around line 1253 for the implementation

/// Apply key remapping for HARDEST mode
/// @param c The character to remap
/// @return The remapped character
int brutal_remap_key( int c )
{
  if ( brutal_mode == BRUTAL_HARDEST && c >= 0 && c < 256 ) {
    return brutal_keymap[c];
  }
  return c;
}

/// Check if quit command should be blocked (HARDER and HARDEST modes)
/// @return true if quit should be blocked
bool brutal_should_block_quit( void )
{
  // Always allow quit in headless or embedded mode
  if ( headless_mode || embedded_mode ) {
    return false;
  }

  return ( brutal_mode == BRUTAL_HARDER || brutal_mode == BRUTAL_HARDEST );
}

/// Handle easy mode special quit sequences
/// @param c The character pressed
/// @return true if quit was triggered
bool brutal_easy_mode_quit_check( int c )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return false;
  }

  // Check for Ctrl+X, Ctrl+C, or Ctrl+Q
  if ( c == Ctrl_X || c == Ctrl_C || c == Ctrl_Q ) {
    brutal_ctrl_quit_count++;
    if ( brutal_ctrl_quit_count >= 3 ) {
      return true;  // Trigger quit
    }
  } else {
    brutal_ctrl_quit_count = 0;  // Reset counter
  }

  return false;
}

/// Check if ESC has been held long enough in EASY mode
/// @return true if 5 seconds have elapsed
bool brutal_easy_mode_esc_held( void )
{
  if ( brutal_mode != BRUTAL_EASY || brutal_esc_hold_start == 0 ) {
    return false;
  }

  uint64_t now = os_hrtime();
  uint64_t elapsed_ns = now - brutal_esc_hold_start;
  uint64_t five_seconds_ns = 5000000000ULL;  // 5 seconds in nanoseconds

  return elapsed_ns >= five_seconds_ns;
}

/// Calculate Levenshtein distance between two strings
static int levenshtein_distance( const char *s1, const char *s2 )
{
  int len1 = ( int )strlen( s1 );
  int len2 = ( int )strlen( s2 );
  
  // Create distance matrix
  int matrix[64][64];  // Max 64 chars each
  
  if ( len1 > 63 || len2 > 63 ) {
    return 999;  // Too long
  }
  
  // Initialize first row and column
  for ( int i = 0; i <= len1; i++ ) {
    matrix[i][0] = i;
  }
  for ( int j = 0; j <= len2; j++ ) {
    matrix[0][j] = j;
  }
  
  // Calculate distances
  for ( int i = 1; i <= len1; i++ ) {
    for ( int j = 1; j <= len2; j++ ) {
      int cost = ( s1[i-1] == s2[j-1] ) ? 0 : 1;
      
      int del = matrix[i-1][j] + 1;
      int ins = matrix[i][j-1] + 1;
      int sub = matrix[i-1][j-1] + cost;
      
      matrix[i][j] = del < ins ? del : ins;
      matrix[i][j] = matrix[i][j] < sub ? matrix[i][j] : sub;
    }
  }
  
  return matrix[len1][len2];
}

/// Check if easter egg phrase was typed
/// @return true if "fuck you let me out" detected (with ~5 char tolerance)
bool brutal_check_easter_egg( void )
{
  const char *target = "fuckyouletmeout";
  char normalized[32] = {0};
  int norm_pos = 0;
  
  // Normalize buffer: lowercase, remove spaces/punctuation
  for ( int i = 0; i < brutal_easter_egg_pos && norm_pos < 31; i++ ) {
    char c = brutal_easter_egg_buffer[i];
    if ( c >= 'A' && c <= 'Z' ) {
      normalized[norm_pos++] = c + 32;  // to lowercase
    } else if ( c >= 'a' && c <= 'z' ) {
      normalized[norm_pos++] = c;
    }
    // Skip spaces, punctuation, etc.
  }
  
  if ( norm_pos < 10 ) {
    return false;  // Too short to match
  }
  
  int distance = levenshtein_distance( normalized, target );
  return distance <= 5;  // Allow ~5 character differences
}

/// Record a character for easter egg detection
void brutal_record_char( int c )
{
  // Only record printable ASCII and space
  if ( ( c >= 32 && c <= 126 ) || c == ' ' ) {
    brutal_easter_egg_buffer[brutal_easter_egg_pos] = ( char )c;
    brutal_easter_egg_pos++;
    
    // Keep only last 31 characters
    if ( brutal_easter_egg_pos >= 31 ) {
      brutal_easter_egg_pos = 31;
      // Shift buffer left
      for ( int i = 0; i < 30; i++ ) {
        brutal_easter_egg_buffer[i] = brutal_easter_egg_buffer[i+1];
      }
      brutal_easter_egg_pos = 30;
    }
  }
}

/// Handle ESC press in EASY mode (for triple-press detection)
void brutal_handle_esc_press( void )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return;
  }
  
  uint64_t now = os_hrtime();
  uint64_t five_seconds_ns = 5000000000ULL;
  
  // Shift timestamps
  brutal_esc_press_times[2] = brutal_esc_press_times[1];
  brutal_esc_press_times[1] = brutal_esc_press_times[0];
  brutal_esc_press_times[0] = now;
  
  // Check if 3 presses within 5 seconds
  if ( brutal_esc_press_times[2] != 0 ) {
    uint64_t elapsed = now - brutal_esc_press_times[2];
    if ( elapsed <= five_seconds_ns ) {
      brutal_esc_press_count = 3;
    } else {
      brutal_esc_press_count = 0;
    }
  }
}

/// Check if 3 ESC presses detected in EASY mode
bool brutal_check_triple_esc( void )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return false;
  }
  
  if ( brutal_esc_press_count >= 3 ) {
    brutal_esc_press_count = 0;  // Reset
    return true;
  }
  return false;
}
