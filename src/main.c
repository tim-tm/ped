#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>
#include <math.h>

#define MAX_LINE_SIZE 512
#define CTRL(k) ((k)&0x1f)
#define KEY_ESCAPE 27
#define KEY_TAB 9

enum Mode {
    MODE_NORMAL = 0,
    MODE_INSERT = 1,
    MODE_VISUAL = 2,
    MODE_SEARCH = 3
};

typedef struct _Character_ {
    char value;

    struct _Character_ *next;
    struct _Character_ *prev;
} Character;

typedef struct _Line_ {
    size_t size;
    Character *chars;
    Character *first_char;
    Character *last_char;

    struct _Line_ *next;
    struct _Line_ *prev;
} Line;

typedef struct _Buffer_ {
    char *file_path;
    
    size_t size;
    Line *lines;
    Line *first_line;
    Line *last_line;
} Buffer;

FILE *fp;
Buffer buf = {0};

size_t cursor_x = 0;
size_t cursor_y = 0;
size_t max_y = 0;
size_t max_x = 0;
size_t scroll_y = 0;
size_t cursor_max = 0;
char *info_msg = NULL;

enum Mode current_mode = MODE_NORMAL;

const char *mode_get_name(enum Mode mode);

/**
 *  buffer_read_from_file(path)
 *
 *  Purpose:
 *      This function reads the contents of a file
 *      at the given path into the main buffer.
 *      Note that this function is using printf to
 *      print an error if occoured.
 *  Return value:
 *      true - Reading was successful
 *      false - Reading was not successful
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      buffer_read_from_file(buffer, path)
 */
bool buffer_read_from_file(char *path);

/**
 *  buffer_free()
 *
 *  Purpose:
 *      This function free's all of the memory allocated
 *      by buffer_read_from_file.
 *  Return value:
 *      void
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      buffer_free(buffer)
 */
void buffer_free(void);

/**
 *  buffer_save(path)
 *
 *  Purpose:
 *      This function writes the content of the main buffer
 *      to the file being specified by 'path'.
 *      Note that this function is not using printf to
 *      print an error if occoured.
 *  Return value:
 *      true - Successfully saved the file
 *      false - There was an error while saving the file
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      buffer_save(buffer, path)
 */
bool buffer_save(char *path);

/**
 *  buffer_append_at_cursor(c)
 *
 *  Purpose:
 *      This function appends a character 'c' to the line
 *      selected by the user, which is being stored in
 *      'cursor_y'.
 *  Return value:
 *      void
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      buffer_append_at_cursor(buffer, c)
 */
void buffer_append_at_cursor(char c);

/**
 *  buffer_find_line(index)
 *
 *  Purpose:
 *      This function finds a line inside of the main buffer
 *      given it's index.
 *  Return value:
 *      NULL - the line could not be found
 *      Line* - the line at the specified index
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      buffer_find_line(buffer, index)
 */
Line *buffer_find_line(size_t index);

/**
 *  line_find_char(lin, index)
 *
 *  Purpose:
 *      This function finds a character inside of the specified
 *      line 'lin' at the given index.
 *  Return value:
 *      NULL - the char could not be found
 *      Character* - the char at the specified index
 *  Possible extension:
 *      Requring the caller to specify the buffer could
 *      be good for implementing a multi-buffer system.
 *      A new function signature may look like this:
 *      line_find_char(buffer, lin, index)
 */
Character *line_find_char(Line *lin, size_t index);

int main(int argc, char **argv) {
    if (argc <= 1 || argv[1] == NULL) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    } else {
        if (!buffer_read_from_file(argv[1])) {
            return 1;
        }
    }

    // hacky thing to calculate the length of an integer
    // Example: 1234 -> 4, 12 -> 2, 62332 -> 5
    size_t line_size = floor(log10(buf.size)) + 3;

    initscr();
    move(cursor_y, cursor_x+line_size+1);
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

        Line *itr = buf.first_line;
        for (size_t i = 0; i < buf.size; ++i) {
            size_t l_size = floor(log10(i+1)) + 1;
            if (cursor_y >= max_y) {
                scroll_y = cursor_max-max_y+1;
            } else {
                // FIXME: This causes a weird clipping that should be removed
                scroll_y = 0;
            }

            mvwprintw(line_win, i-scroll_y, line_size-2-l_size, "%zu\n", i+1);
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
        move(cursor_y-scroll_y, cursor_x+line_size);

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
                        if (cursor_y < buf.size-1) {
                            cursor_y++;
                            cursor_x = 0;
                            if (cursor_y > max_y && cursor_y >= cursor_max) {
                                cursor_max++;
                            }
                        }
                    } break;
                    case KEY_UP:
                    case 'k': {
                        if (cursor_y > 0) {
                            cursor_y--;
                            cursor_x = 0;
                            if (cursor_y <= scroll_y) {
                                cursor_max--;
                            }
                        }
                    } break;
                    case KEY_RIGHT:
                    case 'l': {
                        Line *lin = buffer_find_line(cursor_y);
                        if (lin != NULL && cursor_x < lin->size-1) {
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
                    case 'v': {
                        current_mode = MODE_VISUAL;
                    } break;
                    case '/': {
                        current_mode = MODE_SEARCH;
                    } break;
                    case CTRL('s'): {
                        if (buffer_save(buf.file_path)) {
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
                        if (cursor_y < buf.size-1) {
                            cursor_y++;
                            cursor_x = 0;
                            if (cursor_y > max_y && cursor_y >= cursor_max) {
                                cursor_max++;
                            }
                        }
                    } break;
                    case KEY_UP: {
                        if (cursor_y > 0) {
                            cursor_y--;
                            cursor_x = 0;
                            if (cursor_y <= scroll_y) {
                                cursor_max--;
                            }
                        }
                    } break;
                    case KEY_RIGHT: {
                        Line *lin = buffer_find_line(cursor_y);
                        if (lin != NULL && cursor_x < lin->size-1) {
                            cursor_x++;
                        }
                    } break;
                    case KEY_LEFT: {
                        if (cursor_x > 0) {
                            cursor_x--;
                        }
                    } break;
                    case KEY_ESCAPE: {
                        current_mode = MODE_NORMAL;
                    } break;
                    case KEY_DC: {
                        Line *lin = buffer_find_line(cursor_y);
                        if (lin == NULL || lin->size <= 0) break;
                        
                        Character *ch = line_find_char(lin, cursor_x);
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
                        Line *lin = buffer_find_line(cursor_y);
                        if (lin == NULL || lin->size <= 0) break;
                        
                        long long del_x_index = cursor_x-1;
                        Character *ch = line_find_char(lin, del_x_index);
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
                        cursor_x--;
                    } break;
                    case KEY_TAB: {
                        buffer_append_at_cursor('\t');
                    } break;
                    default: {
                        buffer_append_at_cursor(c);
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

    buffer_free();
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

bool buffer_read_from_file(char *path) {
    // NOTE: The file will be opened later for writing
    fp = fopen(path, "r");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", path);
        return false;
    }
    buf.file_path = path;

    char str[512];
    for (buf.size = 0; fgets(str, MAX_LINE_SIZE, fp) != NULL; ++buf.size) {
        Line *lin = calloc(1, sizeof(Line));
        if (lin == NULL) {
            printf("Failed to allocate space for buffer.\n");
            return false;
        }

        size_t len = strnlen(str, MAX_LINE_SIZE);
        Character *itr = lin->chars;
        for (lin->size = 0; lin->size < len; ++lin->size) {
            Character *tmp = calloc(1, sizeof(Character));
            if (tmp == NULL) {
                printf("Failed to allocate space for buffer.\n");
                return false;
            }
            tmp->value = str[lin->size];
            
            if (lin->size == 0) {
                // The first character of a line, both first and last char are set to this char
                lin->first_char = tmp;
                lin->last_char = tmp;
            } else {
                // Appending to the end
                //      last -> new
                //      last <-> new
                // Updating the last char
                //      last = new
                lin->last_char->next = tmp;
                tmp->prev = lin->last_char;
                lin->last_char = tmp;
            }
            itr = tmp;
        }

        if (buf.lines == NULL) {
            // The first line of a buffer, both first and last line are set to this line
            buf.first_line = lin;
            buf.last_line = lin;
        } else {
            // Appending to the end
            //      last -> new
            //      last <-> new
            // Updating the last char
            //      last = new
            buf.last_line->next = lin;
            lin->prev = buf.last_line;
            buf.last_line = lin;
        }
        buf.lines = lin;
    }
    return true;
}

void buffer_free(void) {
    Line *line_itr = buf.first_line;
    while (line_itr != NULL) {
        Line *next_line_itr = line_itr->next;
        Character *char_itr = line_itr->first_char;
        while (char_itr != NULL) {
            Character *next_char_itr = char_itr->next;
            free(char_itr);
            char_itr = next_char_itr;
        }
        free(line_itr);
        line_itr = next_line_itr;
    }
}

bool buffer_save(char *path) {
    if (path == NULL) return false;

    if (fp != NULL) {
        fclose(fp);
    }
    fp = fopen(path, "w+");
    if (fp == NULL) {
        return false;
    }

    Line *line_itr = buf.first_line;
    while (line_itr != NULL) {
        Character *char_itr = line_itr->first_char;
        while (char_itr != NULL) {
            fprintf(fp, "%c", char_itr->value);
            char_itr = char_itr->next;
        }
        line_itr = line_itr->next;
    }
    fclose(fp);
    return true;
}

void buffer_append_at_cursor(char c) {
    if (cursor_y >= buf.size || cursor_x > MAX_LINE_SIZE) return;  
    Line *lin = buffer_find_line(cursor_y);
    if (lin == NULL || lin->size >= MAX_LINE_SIZE) return;
    
    Character *ch = line_find_char(lin, cursor_x);
    if (ch == NULL) return;

    Character *tmp = calloc(1, sizeof(Character));
    if (tmp == NULL) return;
    tmp->value = c;
    
    // if cursor is at the end of the line
    if (cursor_x >= lin->size-1) {
        // Appending to the end
        //      last -> new
        //      last <-> new
        // Updating the last char
        //      last = new
        lin->last_char->next = tmp;
        tmp->prev = lin->last_char;
        lin->last_char = tmp;
    } else {
        // Example:
        // 'a' is being placed between 'c' and 'd'
        //      b <-> c <-> d
        
        //      b <-> c  a -> d
        tmp->next = ch->next;
        
        //      b <-> c <- a -> d
        tmp->prev = ch;
        
        //      b <-> c <- a <-> d
        ch->next->prev = tmp;
        
        //      b <-> c <-> a <-> d
        ch->next = tmp;
    }
    lin->size++;
    cursor_x++;    
}

Line *buffer_find_line(size_t index) {
    Line *itr = buf.first_line;
    for (size_t i = 0; i < buf.size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}

Character *line_find_char(Line *lin, size_t index) {
    if (lin == NULL) return NULL;
    Character *itr = lin->first_char;
    for (size_t i = 0; i < lin->size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}
