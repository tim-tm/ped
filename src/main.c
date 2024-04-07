#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <math.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

#include "buffer.h"
#include "defs.h"

Buffer buf = {0};
State state = {0};

char *info_msg = NULL;

const char *mode_get_name(enum Mode mode);

int main(int argc, char **argv) {
    if (argc <= 1 || argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    } else {
        buf.state = &state;
        if (!buffer_read_from_file(&buf, argv[1])) {
            return 1;
        }
    }

    // hacky thing to calculate the length of an integer
    // Example: 1234 -> 4, 12 -> 2, 62332 -> 5
    state.line_size = floor(log10(buf.size)) + 3;

    setlocale(LC_ALL, "");

    initscr();
    move(buf.cursor_y, buf.cursor_x+state.line_size+1);
    refresh();
    noecho();
    raw();

    size_t infobar_height = 2;
    getmaxyx(stdscr, state.max_y, state.max_x);
    WINDOW *line_win = newwin(state.max_y-infobar_height, state.line_size, 0, 0);
    WINDOW *text_win = newwin(state.max_y-infobar_height, state.max_x-state.line_size, 0, state.line_size);
    WINDOW *infobar_win = newwin(infobar_height, state.max_x, state.max_y-infobar_height, 0);
    keypad(text_win, TRUE);
    keypad(infobar_win, TRUE);
    
    state.max_y -= infobar_height;
    buf.cursor_max = state.max_y;

    int c_result;
    wint_t c;
    bool close_requested = false;
    while (c != CTRL('q') && !close_requested) {
        werase(line_win);
        werase(text_win);
        werase(infobar_win);

        if (is_term_resized(state.max_y, state.max_x)) {
            getmaxyx(stdscr, state.max_y, state.max_x);
            state.max_y -= infobar_height;
        }

        wresize(line_win, state.max_y, state.line_size);
        mvwin(line_win, 0, 0);
        wresize(text_win, state.max_y, state.max_x-state.line_size);
        mvwin(text_win, 0, state.line_size);

        Line *itr = buf.first_line;
        for (size_t i = 0; i < buf.size; ++i) {
            size_t l_size = floor(log10(i+1)) + 1;
            if (buf.cursor_y >= state.max_y) {
                buf.scroll_y = buf.cursor_max-state.max_y+1;
            } else {
                // FIXME: This causes a weird clipping that should be removed
                buf.scroll_y = 0;
            }

            mvwprintw(line_win, i-buf.scroll_y, state.line_size-2-l_size, "%zu", i+1);
            if (itr->size != 0) {
                Character *c_itr = itr->first_char;
                size_t j = 0;
                while (c_itr != NULL) {
                    mvwprintw(text_win, i-buf.scroll_y, j, "%C", c_itr->value);
                    j += wcwidth(c_itr->value);
                    c_itr = c_itr->next;
                }
            }
            itr = itr->next;
        }
        wprintw(infobar_win, "%s @ %s\n", mode_get_name(state.current_mode), buf.file_path);
        if (info_msg != NULL) {
            wprintw(infobar_win, "%s", info_msg);
        }
        move(buf.cursor_y-buf.scroll_y, buf.cursor_x+state.line_size);

        wrefresh(line_win);
        wrefresh(text_win);
        wrefresh(infobar_win);
        refresh();

        c_result = wget_wch(text_win, &c);
        if (c_result == ERR) {
            info_msg = "Invalid character!";
            continue;
        }
        if (info_msg != NULL) {
            info_msg = NULL;
        }

        switch (state.current_mode) {
            case MODE_NORMAL: {
                switch (c) {
                    case KEY_DOWN:
                    case 'j': {
                        buffer_move_cursor_down(&buf);
                    } break;
                    case KEY_UP:
                    case 'k': {
                        buffer_move_cursor_up(&buf);
                    } break;
                    case KEY_RIGHT:
                    case 'l': {
                        buffer_move_cursor_right(&buf);
                    } break;
                    case KEY_LEFT:
                    case 'h': {
                        buffer_move_cursor_left(&buf);
                    } break;
                    case 'a': {
                        state.current_mode = MODE_INSERT;
                    } break;
                    case 'v': {
                        state.current_mode = MODE_VISUAL;
                    } break;
                    case '/': {
                        state.current_mode = MODE_SEARCH;
                    } break;
                    case CTRL('s'): {
                        if (buffer_save(&buf, buf.file_path)) {
                            close_requested = true;
                        } else {
                            info_msg = "Failed to save file!";
                        }
                    } break;
                }
            } break;
            case MODE_INSERT: {
                switch (c) {
                    case KEY_DOWN: {
                        buffer_move_cursor_down(&buf);
                    } break;
                    case KEY_UP: {
                        buffer_move_cursor_up(&buf);
                    } break;
                    case KEY_RIGHT: {
                        buffer_move_cursor_right(&buf);
                    } break;
                    case KEY_LEFT: {
                        buffer_move_cursor_left(&buf);
                    } break;
                    case KEY_ESCAPE: {
                        state.current_mode = MODE_NORMAL;
                    } break;
                    case KEY_DC: {
                        buffer_delete_char_at_cursor(&buf);
                    } break;
                    case KEY_BACKSPACE: {
                        Line *lin = buffer_find_line(&buf, buf.cursor_y);
                        if (lin == NULL) break;
                        if (lin->size <= 0 && lin != buf.last_line && lin != buf.first_line) {
                            if (buffer_delete_line(&buf, lin)) {
                                // Update rendering vars
                                state.line_size = floor(log10(buf.size)) + 3;
                            }
                            break;
                        }

                        if (buffer_delete_char_at_cursor_x(&buf, buf.cursor_x-1)) {
                            buf.cursor_x--;
                        }
                    } break;
                    case KEY_TAB: {
                        buffer_append_char_at_cursor(&buf, '\t');
                    } break;
                    case KEY_ENTER1: {
                        if (buffer_insert_line_at_cursor(&buf)) {
                            // Update rendering vars
                            state.line_size = floor(log10(buf.size)) + 3;
                        }
                    } break;
                    default: {
                        buffer_append_char_at_cursor(&buf, c);
                    } break;
                }
            } break;
            case MODE_VISUAL: {
                switch (c) {
                    case KEY_ESCAPE: {
                        state.current_mode = MODE_NORMAL;
                    } break;
                }
            } break;
            case MODE_SEARCH: {
                switch (c) {
                    case KEY_ESCAPE: {
                        state.current_mode = MODE_NORMAL;
                    } break;
                }
            } break;
        }
    }

    delwin(line_win);
    delwin(text_win);
    delwin(infobar_win);
    endwin();

    buffer_free(&buf);
    return 0;
}

const char *mode_get_name(enum Mode mode) {
    switch (mode) {
        case MODE_NORMAL: return "NORMAL";
        case MODE_INSERT: return "INSERT";
        case MODE_VISUAL: return "VISUAL";
        case MODE_SEARCH: return "SEARCH";
    }
}
