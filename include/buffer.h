#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} Cursor;

typedef struct {
    char *contents;
    size_t length;
    size_t capacity;
    uint32_t line_num;
} Row;

typedef struct {
    Row *rows;
    size_t num_rows;
    size_t capacity;
    size_t size;
    Cursor cursor;
} Buffer;

void init_buffer(Buffer *buff);
void free_buffer(Buffer *buff);
void write_buffer(char *filename, Buffer *buff);
int get_line_len(int line);
void ensure_row_capacity(Row *row, size_t needed);
void ensure_buffer_capacity(Buffer *buff);

#endif // !BUFFER_H
