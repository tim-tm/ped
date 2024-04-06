#include "buffer.h"

#include <stdlib.h>
#include <string.h>

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
    if (buf == NULL) return false;

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

    char str[512];
    for (buf->size = 0; fgets(str, BUFFER_MAX_LINE_SIZE, buf->fp) != NULL; ++buf->size) {
        Line *lin = calloc(1, sizeof(Line));
        if (lin == NULL) {
            printf("Failed to allocate space for buffer.\n");
            return false;
        }

        size_t len = strnlen(str, BUFFER_MAX_LINE_SIZE);
        Character *itr = lin->chars;
        // len-1 in order to strip off that \n at the end
        // we can for sure say that there is always a \n because
        // fgets only reads in lines
        for (lin->size = 0; lin->size < len-1; ++lin->size) {
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

        if (buf->lines == NULL) {
            // The first line of a buffer, both first and last line are set to this line
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
    if (buf == NULL) return;

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
    if (buf == NULL) return false;
    if (path == NULL) return false;

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
            fprintf(buf->fp, "%c", char_itr->value);
            char_itr = char_itr->next;
        }
        fprintf(buf->fp, "\n");
        line_itr = line_itr->next;
    }
    fclose(buf->fp);
    return true;
}

void buffer_append_char_at_cursor(Buffer *buf, char c) {
    if (buf == NULL) return;
    if (buf->cursor_y >= buf->size || buf->cursor_x > BUFFER_MAX_LINE_SIZE) return;  
    Line *lin = buffer_find_line(buf, buf->cursor_y);
    if (lin == NULL || lin->size >= BUFFER_MAX_LINE_SIZE) return;

    Character *tmp = calloc(1, sizeof(Character));
    if (tmp == NULL) return;
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
    if (buf->cursor_x >= lin->size-1) {
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
    buf->cursor_x++;
}

Line *buffer_find_line(Buffer *buf, size_t index) {
    if (buf == NULL) return NULL;
    Line *itr = buf->first_line;
    for (size_t i = 0; i < buf->size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}

Character *line_find_char(Buffer *buf, Line *lin, size_t index) {
    if (buf == NULL) return NULL;
    if (lin == NULL) return NULL;
    Character *itr = lin->first_char;
    for (size_t i = 0; i < lin->size && itr != NULL; ++i, itr = itr->next) {
        if (i == index) {
            return itr;
        }
    }
    return NULL;
}

void buffer_move_cursor_down(Buffer *buf, size_t max_y) {
    if (buf != NULL && buf->cursor_y < buf->size-1) {
        buf->cursor_y++;
        buf->cursor_x = 0;
        if (buf->cursor_y > max_y && buf->cursor_y >= buf->cursor_max) {
            buf->cursor_max++;
        }
    }
}

void buffer_move_cursor_up(Buffer *buf) {
    if (buf != NULL && buf->cursor_y > 0) {
        buf->cursor_y--;
        buf->cursor_x = 0;
        if (buf->cursor_y <= buf->scroll_y) {
            buf->cursor_max--;
        }
    }
}

void buffer_move_cursor_right(Buffer *buf) {
    if (buf == NULL) return;
    Line *lin = buffer_find_line(buf, buf->cursor_y);
    if (lin != NULL && lin->size != 0 && buf->cursor_x < lin->size-1) {
        buf->cursor_x++;
    }
}

void buffer_move_cursor_left(Buffer *buf) {
    if (buf != NULL && buf->cursor_x > 0) {
        buf->cursor_x--;
    }
}
