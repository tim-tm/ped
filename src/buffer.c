#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

static bool buffer_init_empty(Buffer *buf, char *path) {
    Line *lin = calloc(1, sizeof(Line));
    if (lin == NULL) {
        printf("Failed to allocate space for buffer.\n");
        return false;
    }
    buf->first_line = lin;
    buf->last_line = lin;
    buf->lines = lin;
    buf->file_path = path;
    buf->size++;
    return true;
}

bool buffer_read_from_file(Buffer *buf, char *path) {
    if (buf == NULL)
        return false;

    buf->fp = fopen(path, "r");
    if (buf->fp == NULL) {
        buf->fp = fopen(path, "w+");
        if (buf->fp != NULL) {
            return buffer_init_empty(buf, path);
        } else {
            printf("Failed to create file: %s\n", path);
            return false;
        }
    }
    buf->file_path = path;

    wchar_t str[512];
    for (buf->size = 0; fgetws(str, BUFFER_MAX_LINE_SIZE, buf->fp) != NULL;
         ++buf->size) {
        Line *lin = calloc(1, sizeof(Line));
        if (lin == NULL) {
            printf("Failed to allocate space for buffer.\n");
            return false;
        }

        size_t len = wcsnlen(str, BUFFER_MAX_LINE_SIZE);
        Character *itr = lin->chars;
        // len-1 in order to strip off that \n at the end
        // we can for sure say that there is always a \n because
        // fgetws only reads in lines
        for (lin->size = 0; lin->size < len - 1; ++lin->size) {
            Character *tmp = calloc(1, sizeof(Character));
            if (tmp == NULL) {
                printf("Failed to allocate space for buffer.\n");
                return false;
            }
            tmp->value = str[lin->size];

            if (lin->size == 0) {
                // The first character of a line, both first and last char are
                // set to this char
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

        if (buf->lines == NULL) {
            // The first line of a buffer, both first and last line are set to
            // this line
            buf->first_line = lin;
            buf->last_line = lin;
        } else {
            // Appending to the end
            //      last -> new
            //      last <-> new
            // Updating the last char
            //      last = new
            buf->last_line->next = lin;
            lin->prev = buf->last_line;
            buf->last_line = lin;
        }
        buf->lines = lin;
    }

    if (buf->size == 0) {
        return buffer_init_empty(buf, path);
    }
    return true;
}

void buffer_free(Buffer *buf) {
    if (buf == NULL)
        return;

    Line *line_itr = buf->first_line;
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

bool buffer_save(Buffer *buf, char *path) {
    if (buf == NULL)
        return false;
    if (path == NULL)
        return false;

    if (buf->fp != NULL) {
        fclose(buf->fp);
    }
    buf->fp = fopen(path, "w+");
    if (buf->fp == NULL) {
        return false;
    }

    Line *line_itr = buf->first_line;
    while (line_itr != NULL) {
        Character *char_itr = line_itr->first_char;
        while (char_itr != NULL) {
            fprintf(buf->fp, "%C", char_itr->value);
            char_itr = char_itr->next;
        }
        fprintf(buf->fp, "\n");
        line_itr = line_itr->next;
    }
    fclose(buf->fp);
    return true;
}

void buffer_append_char_at_cursor(Buffer *buf, wint_t c) {
    if (buf == NULL)
        return;
    if (buf->cursor_y >= buf->size || buf->cursor_x > BUFFER_MAX_LINE_SIZE)
        return;
    Line *lin = buffer_find_line(buf, buf->cursor_y);
    if (lin == NULL || lin->size >= BUFFER_MAX_LINE_SIZE)
        return;

    Character *tmp = calloc(1, sizeof(Character));
    if (tmp == NULL)
        return;
    tmp->value = c;

    Character *ch = line_find_char(buf, lin, buf->cursor_x);
    // the line is empty
    if (ch == NULL) {
        lin->first_char = tmp;
        lin->last_char = tmp;
        lin->size++;
        return;
    }

    // if cursor is at the end of the line
    if (buf->cursor_x >= lin->size - 1) {
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
    buf->render_cursor_x += wcwidth(c);
    buf->cursor_x++;
}

Line *buffer_find_line(Buffer *buf, size_t index) {
    if (buf == NULL)
        return NULL;
    Line *itr = buf->first_line;
    for (size_t i = 0; i < buf->size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}

Character *line_find_char(Buffer *buf, Line *lin, size_t index) {
    if (buf == NULL)
        return NULL;
    if (lin == NULL)
        return NULL;
    Character *itr = lin->first_char;
    for (size_t i = 0; i < lin->size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}

// TODO: Fix wide character handling, since that is still kind of buggy

void buffer_move_cursor_down(Buffer *buf) {
    if (buf != NULL && buf->state != NULL && buf->cursor_y < buf->size - 1) {
        buf->cursor_y++;
        buf->render_cursor_x = 0;
        buf->cursor_x = 0;
        if (buf->cursor_y > buf->state->max_y &&
            buf->cursor_y >= buf->cursor_max) {
            buf->cursor_max++;
        }
    }
}

void buffer_move_cursor_up(Buffer *buf) {
    if (buf != NULL && buf->cursor_y > 0) {
        buf->cursor_y--;
        buf->render_cursor_x = 0;
        buf->cursor_x = 0;
        if (buf->cursor_y <= buf->scroll_y) {
            buf->cursor_max--;
        }
    }
}

void buffer_move_cursor_right(Buffer *buf) {
    if (buf == NULL)
        return;
    Line *lin = buffer_find_line(buf, buf->cursor_y);
    if (lin != NULL && lin->size != 0 && buf->cursor_x < lin->size - 1 &&
        buf->render_cursor_x < lin->size - 1) {
        if (buffer_move_render_cursor(buf, 1)) {
            buf->cursor_x++;
        }
    }
}

void buffer_move_cursor_left(Buffer *buf) {
    if (buf != NULL && buf->cursor_x > 0 && buf->render_cursor_x > 0) {
        if (buffer_move_render_cursor(buf, -1)) {
            buf->cursor_x--;
        }
    }
}

bool buffer_delete_char_at_cursor_xy(Buffer *buf, size_t cursor_x,
                                     size_t cursor_y) {
    if (buf == NULL)
        return false;

    Line *lin = buffer_find_line(buf, cursor_y);
    if (lin == NULL || lin->size <= 0)
        return false;

    Character *ch = line_find_char(buf, lin, cursor_x);
    if (ch == NULL)
        return false;
    if (ch == lin->first_char) {
        // Example:
        //      |a| <-> b <-> c
        //      b <-> c
        lin->first_char = ch->next;
    } else if (ch == lin->last_char) {
        lin->last_char = ch->prev;
        buf->render_cursor_x -= wcwidth(ch->value);
        buf->cursor_x--;
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
    return true;
}

bool buffer_delete_char_at_cursor_x(Buffer *buf, size_t cursor_x) {
    if (buf != NULL) {
        return buffer_delete_char_at_cursor_xy(buf, cursor_x, buf->cursor_y);
    }
    return false;
}

bool buffer_delete_char_at_cursor(Buffer *buf) {
    if (buf != NULL) {
        return buffer_delete_char_at_cursor_xy(buf, buf->cursor_x,
                                               buf->cursor_y);
    }
    return false;
}

bool buffer_delete_line(Buffer *buf, Line *lin) {
    if (buf == NULL || lin == NULL || lin->next == NULL || lin->prev == NULL)
        return false;

    if (lin == buf->first_line) {
        buf->first_line = lin->next;
    } else {
        lin->next->prev = lin->prev;
        lin->prev->next = lin->next;
    }

    if (lin->size > 0) {
        Character *itr = lin->first_char;
        while (itr != NULL) {
            Character *next_itr = itr->next;
            free(itr);
            itr = next_itr;
        }
    }
    free(lin);
    buf->size--;

    if (buf->cursor_y <= buf->scroll_y) {
        buf->cursor_max--;
    }
    return true;
}

bool buffer_insert_line_at_cursor_y(Buffer *buf, size_t cursor_y) {
    if (buf == NULL)
        return false;

    Line *curr_lin = buffer_find_line(buf, cursor_y);
    if (curr_lin == NULL)
        return false;

    Line *lin = calloc(1, sizeof(Line));
    if (lin == NULL)
        return false;

    if (curr_lin == buf->last_line) {
        buf->last_line->next = lin;
        lin->prev = buf->last_line;
        buf->last_line = lin;
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
    buf->size++;
    return true;
}

bool buffer_insert_line_at_cursor(Buffer *buf) {
    if (buf != NULL) {
        bool res = buffer_insert_line_at_cursor_y(buf, buf->cursor_y);
        if (res != false) {
            buf->cursor_y++;
            buf->render_cursor_x = 0;
            buf->cursor_x = 0;
            if (buf->cursor_y > buf->state->max_y &&
                buf->cursor_y >= buf->cursor_max) {
                buf->cursor_max++;
            }
            return true;
        }
    }
    return false;
}

bool buffer_move_render_cursor_x(Buffer *buf, size_t cursor_x,
                                 int direction_x) {
    if (buf == NULL)
        return false;

    Line *lin = buffer_find_line(buf, buf->cursor_y);
    if (lin == NULL)
        return false;
    Character *ch = line_find_char(buf, lin, cursor_x);
    if (ch == NULL)
        return false;

    buf->render_cursor_x += wcwidth(ch->value) * direction_x;
    return true;
}

bool buffer_move_render_cursor(Buffer *buf, int direction_x) {
    return buffer_move_render_cursor_x(buf, buf->cursor_x, direction_x);
}
