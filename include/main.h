#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>

#define BACKSPACE 127
#define ENTER 10
#define ESC 27
#define CTRL(x) ((x) & 0x1f)

typedef struct {
    char *contents;
    size_t buf_size;
    size_t buf_x;
    size_t buf_y;
} Buffer;

#endif // !MAIN_H
