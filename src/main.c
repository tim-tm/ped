#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <math.h>

#define MAX_LINE_SIZE 512
#define CTRL(k) ((k)&0x1f)
#define KEY_ESCAPE 27

enum Mode {
    MODE_NORMAL = 0,
    MODE_INSERT = 1,
    MODE_VISUAL = 2,
    MODE_SEARCH = 3
};

FILE *fp;
size_t fbuf_max = 64;
size_t fbuf_count;
char **fbuf;

size_t cursor_x = 0;
size_t cursor_y = 0;
size_t max_y = 0;
size_t max_x = 0;
size_t scroll_y = 0;
size_t cursor_max = 0;

enum Mode current_mode = MODE_NORMAL;

int main(int argc, char **argv) {
    if (argc <= 1 || argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    } else {
        fp = fopen(argv[1], "r+");
        if (fp == NULL) {
            printf("Failed to open file: %s\n", argv[1]);
            return 1;
        }

        fbuf = calloc(fbuf_max, sizeof(char*));
        if (fbuf == NULL) {
            printf("Failed to allocate space for buffer.\n");
            return 1;
        }
        char str[512];
        for (fbuf_count = 0; fgets(str, MAX_LINE_SIZE, fp) != NULL; ++fbuf_count) {
            if (fbuf_count >= fbuf_max) {
                fbuf_max *= 2;
                fbuf = realloc(fbuf, fbuf_max*sizeof(char*));
            }
            fbuf[fbuf_count] = calloc(MAX_LINE_SIZE, sizeof(char));
            if (fbuf[fbuf_count] == NULL) {
                printf("Failed to allocate space for buffer.\n");
                return 1;
            }
            strncpy(fbuf[fbuf_count], str, 512);
        }
    }

    size_t line_size = floor(log10(fbuf_count)) + 3;

    initscr();
    move(cursor_y, cursor_x+line_size+1);
    refresh();
    noecho();
    raw();

    getmaxyx(stdscr, max_y, max_x);
    cursor_max = max_y;
    WINDOW *line_win = newwin(max_y, line_size, 0, 0);
    WINDOW *text_win = newwin(max_y, max_x-line_size, 0, line_size);
    keypad(text_win, TRUE);

    int c;
    while (c != CTRL('q')) {
        werase(line_win);
        werase(text_win);

        if (is_term_resized(max_y, max_x)) {
            getmaxyx(stdscr, max_y, max_x);
        }

        for (size_t i = 0; i < fbuf_count; ++i) {
            size_t l_size = floor(log10(i+1)) + 1;
            mvwprintw(line_win, i-scroll_y, line_size-2-l_size, "%zu\n", i+1);
            mvwprintw(text_win, i-scroll_y, 0, "%s", fbuf[i]);
        }
        move(cursor_y, cursor_x+3);
        
        wrefresh(line_win);
        wrefresh(text_win);
        refresh();

        c = wgetch(text_win);
        switch (current_mode) {
            case MODE_NORMAL: {
                switch (c) {
                    case KEY_DOWN:
                    case 'j': {
                        if (cursor_y < fbuf_count-1) {
                            if (cursor_y > cursor_max) {
                                scroll_y++;
                                cursor_max = cursor_y;
                            }
                            
                            cursor_y++;
                            cursor_x = 0;
                        }
                    } break;
                    case KEY_UP:
                    case 'k': {
                        if (cursor_y > 0) {
                            // TODO: Implement scrolling back up
                            cursor_y--;
                            cursor_x = 0;
                        }
                    } break;
                    case KEY_RIGHT:
                    case 'l': {
                        char *curr_line = fbuf[cursor_y];
                        size_t len = strnlen(curr_line, MAX_LINE_SIZE);
                        if (cursor_x < len-1) {
                            cursor_x++;
                        }
                    } break;
                    case KEY_LEFT:
                    case 'h': {
                        if (cursor_x > 0) {
                            cursor_x--;
                        }
                    } break;
                    case 'a': {
                        current_mode = MODE_INSERT;
                    } break;
                }
            } break;
            case MODE_INSERT: {
                switch (c) {
                    case KEY_ESCAPE: {
                        current_mode = MODE_NORMAL;
                    } break;
                    default: {
                        // TODO: Implement
                        char *dir = &fbuf[cursor_y][cursor_x];
                    } break;
                }
            } break;
            case MODE_VISUAL: {

            } break;
            case MODE_SEARCH: {

            } break;
        }
    }

    delwin(line_win);
    delwin(text_win);
    endwin();

    for (size_t i = 0; i < fbuf_count; ++i) {
        free(fbuf[i]);
    }
    free(fbuf);
    return 0;
}
