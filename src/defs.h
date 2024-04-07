#ifndef _DEFS_H_
#define _DEFS_H_

#include <stddef.h>

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

typedef struct _State_ {  
    size_t line_size;
    size_t max_y;
    size_t max_x;
    
    enum Mode current_mode;
} State;

#endif // _DEFS_H_
