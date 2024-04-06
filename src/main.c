#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <math.h>

#include "buffer.h"

#define MAX_LINE_SIZE 512
#define CTRL(k) ((k)&0x1f)
#define KEY_ESCAPE 27
#define KEY_TAB 9
#define KEY_ENTER1 10

enum Mode {
    MODE_NORMAL = 0,
    MODE_INSERT = 1,
    MODE_VISUAL = 2,
    MODE_SEARCH = 3
};

Buffer buf = {0};

size_t scroll_y = 0;
size_t cursor_max = 0;
size_t line_size = 0;
size_t max_y = 0;
size_t max_x = 0;
char *info_msg = NULL;

enum Mode current_mode = MODE_NORMAL;

const char *mode_get_name(enum Mode mode);

int main(int argc, char **argv) {
    if (argc <= 1 || argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    } else {
        if (!buffer_read_from_file(&buf, argv[1])) {
            return 1;
        }
    }

    // hacky thing to calculate the length of an integer
    // Example: 1234 -> 4, 12 -> 2, 62332 -> 5
    line_size = floor(log10(buf.size)) + 3;

    initscr();
    move(buf.cursor_y, buf.cursor_x+line_size+1);
    refresh();
    noecho();
    raw();

    size_t infobar_height = 2;
    getmaxyx(stdscr, max_y, max_x);
    WINDOW *line_win = newwin(max_y-infobar_height, line_size, 0, 0);
    WINDOW *text_win = newwin(max_y-infobar_height, max_x-line_size, 0, line_size);
    WINDOW *infobar_win = newwin(infobar_height, max_x, max_y-infobar_height, 0);
    keypad(text_win, TRUE);
    keypad(infobar_win, TRUE);
    
    max_y -= infobar_height;
    cursor_max = max_y;

    int c;
    bool close_requested = false;
    while (c != CTRL('q') && !close_requested) {
        werase(line_win);
        werase(text_win);
        werase(infobar_win);

        if (is_term_resized(max_y, max_x)) {
            getmaxyx(stdscr, max_y, max_x);
            max_y -= infobar_height;
        }

        wresize(line_win, max_y, line_size);
        mvwin(line_win, 0, 0);
        wresize(text_win, max_y, max_x-line_size);
        mvwin(text_win, 0, line_size);

        Line *itr = buf.first_line;
        for (size_t i = 0; i < buf.size; ++i) {
            size_t l_size = floor(log10(i+1)) + 1;
            if (buf.cursor_y >= max_y) {
                scroll_y = cursor_max-max_y+1;
            } else {
                // FIXME: This causes a weird clipping that should be removed
                scroll_y = 0;
            }

            mvwprintw(line_win, i-scroll_y, line_size-2-l_size, "%zu", i+1);
            if (itr->size != 0) {
                Character *c_itr = itr->first_char;
                for (size_t j = 0; j < itr->size; ++j) {
                    mvwprintw(text_win, i-scroll_y, j, "%c", c_itr->value);
                    c_itr = c_itr->next;
                }
            }
            itr = itr->next;
        }
        wprintw(infobar_win, "%s @ %s\n", mode_get_name(current_mode), buf.file_path);
        if (info_msg != NULL) {
            wprintw(infobar_win, "%s", info_msg);
        }
        move(buf.cursor_y-scroll_y, buf.cursor_x+line_size);

        wrefresh(line_win);
        wrefresh(text_win);
        wrefresh(infobar_win);
        refresh();

        c = wgetch(text_win);
        if (info_msg != NULL) {
            info_msg = NULL;
        }

        switch (current_mode) {
            case MODE_NORMAL: {
                switch (c) {
                    case KEY_DOWN:
                    case 'j': {
                        if (buf.cursor_y < buf.size-1) {
                            buf.cursor_y++;
                            buf.cursor_x = 0;
                            if (buf.cursor_y > max_y && buf.cursor_y >= cursor_max) {
                                cursor_max++;
                            }
                        }
                    } break;
                    case KEY_UP:
                    case 'k': {
                        if (buf.cursor_y > 0) {
                            buf.cursor_y--;
                            buf.cursor_x = 0;
                            if (buf.cursor_y <= scroll_y) {
                                cursor_max--;
                            }
                        }
                    } break;
                    case KEY_RIGHT:
                    case 'l': {
                        Line *lin = buffer_find_line(&buf, buf.cursor_y);
                        if (lin != NULL && buf.cursor_x < lin->size-1) {
                            buf.cursor_x++;
                        }
                    } break;
                    case KEY_LEFT:
                    case 'h': {
                        if (buf.cursor_x > 0) {
                            buf.cursor_x--;
                        }
                    } break;
                    case 'a': {
                        current_mode = MODE_INSERT;
                    } break;
                    case 'v': {
                        current_mode = MODE_VISUAL;
                    } break;
                    case '/': {
                        current_mode = MODE_SEARCH;
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
                        if (buf.cursor_y < buf.size-1) {
                            buf.cursor_y++;
                            buf.cursor_x = 0;
                            if (buf.cursor_y > max_y && buf.cursor_y >= cursor_max) {
                                cursor_max++;
                            }
                        }
                    } break;
                    case KEY_UP: {
                        if (buf.cursor_y > 0) {
                            buf.cursor_y--;
                            buf.cursor_x = 0;
                            if (buf.cursor_y <= scroll_y) {
                                cursor_max--;
                            }
                        }
                    } break;
                    case KEY_RIGHT: {
                        Line *lin = buffer_find_line(&buf, buf.cursor_y);
                        if (lin != NULL && buf.cursor_x < lin->size-1) {
                            buf.cursor_x++;
                        }
                    } break;
                    case KEY_LEFT: {
                        if (buf.cursor_x > 0) {
                            buf.cursor_x--;
                        }
                    } break;
                    case KEY_ESCAPE: {
                        current_mode = MODE_NORMAL;
                    } break;
                    case KEY_DC: {
                        Line *lin = buffer_find_line(&buf, buf.cursor_y);
                        if (lin == NULL || lin->size <= 0) break;
                        
                        Character *ch = line_find_char(&buf, lin, buf.cursor_x);
                        if (ch == NULL) break;
                        if (ch == lin->first_char) {
                            // Example:
                            //      |a| <-> b <-> c
                            //      b <-> c
                            lin->first_char = ch->next;
                        } else {
                            // Example:
                            //      a <-> |b| <-> c
                            //      
                            // 1.   ______________
                            //     |             |
                            //     a <-> |b| ->  c
                            // 2.
                            //     a <-> c
                            ch->next->prev = ch->prev;
                            ch->prev->next = ch->next;
                        }
                        free(ch);
                        lin->size--;
                    } break;
                    case KEY_BACKSPACE: {
                        Line *lin = buffer_find_line(&buf, buf.cursor_y);
                        if (lin == NULL) break;
                        if (lin->size <= 0 && lin != buf.last_line && lin != buf.first_line) {
                            if (lin == buf.first_line) {
                                buf.first_line = lin->next;
                            } else {
                                lin->next->prev = lin->prev;
                                lin->prev->next = lin->next;
                            }
                            free(lin);
                            buf.size--;

                            // Update rendering vars
                            line_size = floor(log10(buf.size)) + 3;
                            if (buf.cursor_y <= scroll_y) {
                                cursor_max--;
                            }
                            break;
                        }

                        long long del_x_index = buf.cursor_x-1;
                        Character *ch = line_find_char(&buf, lin, del_x_index);
                        if (ch == NULL) break;
                        if (ch == lin->first_char) {
                            // Example:
                            //      |a| <-> b <-> c
                            //      b <-> c
                            lin->first_char = ch->next;
                        } else {
                            // Example:
                            //      a <-> |b| <-> c
                            //      
                            // 1.   ______________
                            //     |             |
                            //     a <-> |b| ->  c
                            // 2.
                            //     a <-> c
                            ch->next->prev = ch->prev;
                            ch->prev->next = ch->next;
                        }
                        free(ch);
                        lin->size--;
                        buf.cursor_x--;
                    } break;
                    case KEY_TAB: {
                        buffer_append_at_cursor(&buf, '\t');
                    } break;
                    case KEY_ENTER1: {
                        Line *curr_lin = buffer_find_line(&buf, buf.cursor_y);
                        if (curr_lin == NULL) break;

                        Line *lin = calloc(1, sizeof(Line));
                        if (lin == NULL) break;

                        if (curr_lin == buf.last_line) {
                            buf.last_line->next = lin;
                            lin->prev = buf.last_line;
                            buf.last_line = lin;
                        } else {
                            // Example:
                            // 'a' is being placed between 'c' and 'd'
                            //      b <-> c <-> d
                            
                            //      b <-> c  a -> d
                            lin->next = curr_lin->next;
                            
                            //      b <-> c <- a -> d
                            lin->prev = curr_lin;
                            
                            //      b <-> c <- a <-> d
                            curr_lin->next->prev = lin;
                            
                            //      b <-> c <-> a <-> d
                            curr_lin->next = lin;
                        }
                        buf.size++;
                        buf.cursor_y++;
                        buf.cursor_x = 0;
                        
                        // Update rendering vars
                        line_size = floor(log10(buf.size)) + 3;
                        if (buf.cursor_y > max_y && buf.cursor_y >= cursor_max) {
                            cursor_max++;
                        }
                    } break;
                    default: {
                        buffer_append_at_cursor(&buf, c);
                    } break;
                }
            } break;
            case MODE_VISUAL: {
                switch (c) {
                    case KEY_ESCAPE: {
                        current_mode = MODE_NORMAL;
                    } break;
                }
            } break;
            case MODE_SEARCH: {
                switch (c) {
                    case KEY_ESCAPE: {
                        current_mode = MODE_NORMAL;
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
