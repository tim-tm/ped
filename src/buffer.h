#ifndef _BUFFER_H_
#define _BUFFER_H_

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
    size_t cursor_max;
    size_t scroll_y;
    
    size_t size;
    Line *lines;
    Line *first_line;
    Line *last_line;
} Buffer;

// All static methods are for internal purposes and not exposed to the one including buffer.h

/**
 *  buffer_init_empty(buf, path)
 *
 *  Purpose:
 *      This function inits the specified buffer with
 *      one empty line. Note that this function is using
 *      printf to print an error if occoured.
 *  Return value:
 *      true - Init was successful
 *      false - Init failed
 */
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
void buffer_append_char_at_cursor(Buffer *buf, char c);

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

/**
 *  buffer_move_cursor_down(buf, max_y)
 *
 *  Purpose:
 *      Calling this function moves the buffer's cursor down by
 *      one if possible. The param 'max_y' should be the size
 *      of the text-field being drawn.
 *  Return value:
 *      void
 */
void buffer_move_cursor_down(Buffer *buf, size_t max_y);

/**
 *  buffer_move_cursor_up(buf)
 *
 *  Purpose:
 *      Calling this function moves the buffer's cursor up by
 *      one if possible.
 *  Return value:
 *      void
 */
void buffer_move_cursor_up(Buffer *buf);

/**
 *  buffer_move_cursor_right(buf)
 *
 *  Purpose:
 *      Calling this function moves the buffer's cursor right by
 *      one if possible.
 *  Return value:
 *      void
 */
void buffer_move_cursor_right(Buffer *buf);

/**
 *  buffer_move_cursor_left(buf)
 *
 *  Purpose:
 *      Calling this function moves the buffer's cursor left by
 *      one if possible.
 *  Return value:
 *      void
 */
void buffer_move_cursor_left(Buffer *buf);

/**
 *  buffer_delete_char_at_cursor_xy(buf, cursor_x, cursor_y)
 *
 *  Purpose:
 *      Functions exactly the same as buffer_delete_char_at_cursor
 *      but allows the caller to specify a custom cursor x and y position.
 *      This function checks for the bounds of the cursor.
 *  Return value:
 *      true - Deletion successful
 *      false - Character could not be deleted
 */
bool buffer_delete_char_at_cursor_xy(Buffer *buf, size_t cursor_x, size_t cursor_y);

/**
 *  buffer_delete_char_at_cursor_x(buf, cursor_x)
 *
 *  Purpose:
 *      Functions exactly the same as buffer_delete_char_at_cursor
 *      but allows the caller to specify a custom cursor_x.
 *      This function checks for the bounds of the cursor.
 *  Return value:
 *      true - Deletion successful
 *      false - Character could not be deleted
 */
bool buffer_delete_char_at_cursor_x(Buffer *buf, size_t cursor_x);

/**
 *  buffer_delete_char_at_cursor(buf)
 *
 *  Purpose:
 *      Deletes a character at the current cursor position.
 *      This function checks for the bounds of the cursor.
 *  Return value:
 *      true - Deletion successful
 *      false - Character could not be deleted
 */
bool buffer_delete_char_at_cursor(Buffer *buf);

/**
 *  buffer_delete_line(buf, lin)
 *
 *  Purpose:
 *      Delete the specified line 'lin' from the given buffer
 *      'buf'.
 *  Return value:
 *      true - Deletion successful
 *      false - Line could not be deleted
 */
bool buffer_delete_line(Buffer *buf, Line *lin);

/**
 *  buffer_insert_line_at_cursor_y(buf, cursor_y)
 *
 *  Purpose:
 *      Insert a new empty line at the specified y position.
 *      This function does not modify the internally saved cursor_x and cursor_y.
 *  Return value:
 *      true - Insertion successful
 *      false - Line could not be inserted
 */
bool buffer_insert_line_at_cursor_y(Buffer *buf, size_t cursor_y);

/**
 *  buffer_insert_line_at_cursor(buf, max_y)
 *
 *  Purpose:
 *      Insert a new empty line at 'cursor_y', which is saved in the buffer 'buf'.
 *      This function does modify the internally saved cursor_x and cursor_y.
 *  Return value:
 *      true - Insertion successful
 *      false - Line could not be inserted
 */
bool buffer_insert_line_at_cursor(Buffer *buf, size_t max_y);

#endif // _BUFFER_H_
