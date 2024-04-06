#ifndef _BUFFER_H_

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_MAX_LINE_SIZE 512

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
    FILE *fp;
    
    size_t cursor_x;
    size_t cursor_y;
    
    size_t size;
    Line *lines;
    Line *first_line;
    Line *last_line;
} Buffer;

static bool buffer_init_empty(Buffer *buf, char *path);

/**
 *  buffer_read_from_file(buf, path)
 *
 *  Purpose:
 *      This function reads the contents of a file
 *      at the given path into the provided buffer.
 *      Note that this function is using printf to
 *      print an error if occoured.
 *  Return value:
 *      true - Reading was successful
 *      false - Reading was not successful
 */
bool buffer_read_from_file(Buffer *buf, char *path);

/**
 *  buffer_free(buf)
 *
 *  Purpose:
 *      This function free's all of the memory allocated
 *      by buffer_read_from_file.
 *  Return value:
 *      void
 */
void buffer_free(Buffer *buf);

/**
 *  buffer_save(buf, path)
 *
 *  Purpose:
 *      This function writes the content of the specified buffer
 *      to the file being specified by 'path'.
 *      Note that this function is not using printf to
 *      print an error if occoured.
 *  Return value:
 *      true - Successfully saved the file
 *      false - There was an error while saving the file
 */
bool buffer_save(Buffer *buf, char *path);

/**
 *  buffer_append_at_cursor(buf, c)
 *
 *  Purpose:
 *      This function appends a character 'c' to the line
 *      selected by the user, which is being stored in
 *      'cursor_y'.
 *  Return value:
 *      void
 */
void buffer_append_at_cursor(Buffer *buf, char c);

/**
 *  buffer_find_line(buf, index)
 *
 *  Purpose:
 *      This function finds a line inside of the provided buffer
 *      given it's index.
 *  Return value:
 *      NULL - the line could not be found
 *      Line* - the line at the specified index
 */
Line *buffer_find_line(Buffer *buf, size_t index);

/**
 *  line_find_char(buf, lin, index)
 *
 *  Purpose:
 *      This function finds a character inside of the specified
 *      line 'lin' at the given index.
 *  Return value:
 *      NULL - the char could not be found
 *      Character* - the char at the specified index
 */
Character *line_find_char(Buffer *buf, Line *lin, size_t index);

#endif // _BUFFER_H_
