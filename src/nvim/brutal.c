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

BrutalMode brutal_mode = BRUTAL_NONE;
uint8_t brutal_keymap[256];
uint64_t brutal_esc_hold_start = 0;
int brutal_ctrl_quit_count = 0;
uint64_t brutal_esc_press_times[5] = {0};
int brutal_esc_press_count = 0;
char brutal_easter_egg_buffer[32] = {0};
int brutal_easter_egg_pos = 0;

/// Helper function to shuffle array using Fisher-Yates algorithm
static void shuffle_char_array( char *keys, int count )
{
  for ( int i = count - 1; i > 0; i-- ) {
    int j = rand() % ( i + 1 );
    char temp = keys[i];
    keys[i] = keys[j];
    keys[j] = temp;
  }
}

/// Helper function to apply shuffled mappings
static void apply_shuffled_mapping( const char *original, const char *shuffled, int count )
{
  for ( int i = 0; i < count; i++ ) {
    brutal_keymap[( uint8_t )original[i]] = ( uint8_t )shuffled[i];
  }
}

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

  // Shuffle each group
  shuffle_char_array( motion_keys, motion_count );
  shuffle_char_array( edit_keys, edit_count );
  shuffle_char_array( visual_keys, visual_count );
  shuffle_char_array( search_keys, search_count );
  shuffle_char_array( mark_keys, mark_count );

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
  shuffle_char_array( other_shuffled, other_count );
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
    // This enables Shift+Arrow to start selection
    do_cmdline_cmd( "set keymodel=startsel,stopsel" );
    
    // Set selectmode to "key" so Shift+Arrow enters select mode
    do_cmdline_cmd( "set selectmode=key" );
  }
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
      msg_puts( "  • Arrow keys enabled for navigation\n" );
      msg_puts( "  • Easter egg: Type 'fuck you let me out' in INSERT mode to force quit!\n" );
      msg_puts( "  • You can use :quit! and :q! to force quit without saving\n" );
      msg_puts( "\n" );
      msg_puts( "Modified keybindings (Windows-style):\n" );
      msg_puts( "  • Ctrl+Z  →  Undo (u)\n" );
      msg_puts( "  • Ctrl+C  →  Copy/Yank (use in visual mode)\n" );
      msg_puts( "  • Ctrl+X  →  Cut (use in visual mode)\n" );
      msg_puts( "  • Ctrl+V  →  Paste (use \"+p or \"*p for clipboard)\n" );
      msg_puts( "\n" );
      msg_puts( "Tip: Select text in visual mode (v), then Ctrl+C to copy,\n" );
      msg_puts( "     or Ctrl+X to cut. Use Ctrl+V or \"*p to paste.\n" );
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
  // HARD, HARDER, HARDEST: Block arrow keys
  if ( brutal_mode >= BRUTAL_HARD ) {
    if ( c == K_UP || c == K_DOWN || c == K_LEFT || c == K_RIGHT ||
         c == K_PAGEUP || c == K_PAGEDOWN || c == K_HOME || c == K_END ) {
      return true;
    }
  }

  return false;
}

/// Apply Windows-style key mappings for default mode (always on)
/// Handles Shift+Arrow for visual selection, Ctrl+C/V/X for clipboard
/// @param c The character to remap
/// @return The remapped character (or K_IGNORE if handled internally)
int brutal_apply_easy_mode_mappings( int c )
{
  // Map Ctrl+Z to undo (always available, not just EASY mode)
  if ( c == Ctrl_Z ) {
    return 'u';
  }
  
  // Shift+Arrow keys are handled in normal.c/visual mode code
  // Ctrl+C/V/X are handled in normal.c/visual mode code
  // This function is for simple character remapping only
  
  return c;
}

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

/// Check if Windows-style keybindings should be active (EASY mode only)
/// @return true if EASY mode is active
bool brutal_windows_keys_active( void )
{
  return brutal_mode == BRUTAL_EASY;
}

/// Check if quit command should be blocked (HARDER and HARDEST modes)
/// @param force true if using :quit! or :q!
/// @return true if quit should be blocked
bool brutal_should_block_quit( bool force )
{
  // Never block quit in headless or embedded modes (build system, automation, etc.)
  if ( headless_mode || embedded_mode ) {
    return false;
  }
  
  // Default mode (BRUTAL_NONE) and EASY/HARD: never block
  if ( brutal_mode == BRUTAL_NONE || brutal_mode == BRUTAL_EASY || brutal_mode == BRUTAL_HARD ) {
    return false;
  }
  
  // EASY mode with force quit: allow
  if ( brutal_mode == BRUTAL_EASY && force ) {
    return false;
  }
  
  // HARDER/HARDEST: block ALL quit commands including forced quit
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

/// Check if ESC has been pressed repeatedly (5 times within 10 seconds)
/// @return true if threshold met
bool brutal_easy_mode_esc_repeated( void )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return false;
  }

  // Check if we have 5 ESC presses
  if ( brutal_esc_press_count >= 5 ) {
    uint64_t now = os_hrtime();
    uint64_t ten_seconds_ns = 10000000000ULL;  // 10 seconds
    
    // Check if oldest press (4th back) is within 10 seconds
    if ( now - brutal_esc_press_times[4] <= ten_seconds_ns ) {
      return true;
    }
  }

  return false;
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
/// @return true if "fuckyouletmeout" detected (exact match, case-insensitive)
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
  
  // Need at least the target length
  if ( norm_pos < ( int )strlen( target ) ) {
    return false;
  }
  
  // Check if the normalized string contains the target
  normalized[norm_pos] = '\0';
  bool found = strstr( normalized, target ) != NULL;
  
  // DEBUG: Write to log
  if ( norm_pos > 10 ) {  // Only log significant strings
    FILE *f = fopen( "/tmp/brutal_debug.log", "a" );
    if ( f ) {
      fprintf( f, "Checking: '%s' (len=%d) against '%s' -> %s\n",
               normalized, norm_pos, target, found ? "MATCH" : "no match" );
      fclose( f );
    }
  }
  
  return found;
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

/// Handle ESC press in EASY mode (for repeated-press detection)
void brutal_handle_esc_press( void )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return;
  }
  
  uint64_t now = os_hrtime();
  
  // Shift timestamps (keep last 5)
  brutal_esc_press_times[4] = brutal_esc_press_times[3];
  brutal_esc_press_times[3] = brutal_esc_press_times[2];
  brutal_esc_press_times[2] = brutal_esc_press_times[1];
  brutal_esc_press_times[1] = brutal_esc_press_times[0];
  brutal_esc_press_times[0] = now;
  
  // Increment counter
  if ( brutal_esc_press_count < 5 ) {
    brutal_esc_press_count++;
  }
}

/// Check if 5 ESC presses detected in EASY mode within 10 seconds
bool brutal_check_repeated_esc( void )
{
  if ( brutal_mode != BRUTAL_EASY ) {
    return false;
  }
  
  if ( brutal_esc_press_count >= 5 ) {
    uint64_t now = os_hrtime();
    uint64_t ten_seconds_ns = 10000000000ULL;
    
    // Check if 5th press back is within 10 seconds
    if ( brutal_esc_press_times[4] != 0 && 
         ( now - brutal_esc_press_times[4] ) <= ten_seconds_ns ) {
      // Reset counter
      brutal_esc_press_count = 0;
      for ( int i = 0; i < 5; i++ ) {
        brutal_esc_press_times[i] = 0;
      }
      return true;
    }
  }
  return false;
}
