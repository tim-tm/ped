#ifndef _DEFS_H_
#define _DEFS_H_

#include <stddef.h>

#define MAX_LINE_SIZE 512
#define CTRL(k) ((k) & 0x1f)
#define KEY_ESCAPE 27
#define KEY_TAB 9
#define KEY_ENTER1 10
#define SET_CURSOR_STYLE(style)                                                \
  printf("\033[%d q", (style));                                                \
  fflush(stdout);

enum Mode { MODE_NORMAL, MODE_INSERT, MODE_VISUAL, MODE_SEARCH, MODE_APPEND };
enum CursorStyle {
  // see:
  // https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h4-Functions-using-CSI-_-ordered-by-the-final-character-lparen-s-rparen:CSI-Ps-SP-q.1D81
  CURSOR_BLOCK_BLINK_2 = 0,   // blinking block.
  CURSOR_BLOCK_BLINK = 1,     // blinking block (default).
  CURSOR_BLOCK = 2,           // steady block.
  CURSOR_UNDERLINE_BLINK = 3, // blinking underline.
  CURSOR_UNDERLINE = 4,       // steady underline.
  CURSOR_BAR_BLINK = 5,       // blinking bar, xterm.
  CURSOR_BAR = 6              // steady bar, xterm.
};

typedef struct _State_ {
  size_t line_size;
  size_t max_y;
  size_t max_x;

  enum Mode current_mode;
} State;

#endif // _DEFS_H_
