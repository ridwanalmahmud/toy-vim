#ifndef BUFFER_H
#define BUFFER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    size_t x;
    size_t y;
} Cursor;

typedef struct {
    char *contents;
    size_t length;
    size_t capacity;
    size_t line_num;
} Row;

typedef struct {
    Row *rows;
    size_t num_rows;
    size_t capacity;
    size_t size;
    Cursor cursor;

    // for scrolling
    size_t row_offset;
    size_t col_offset;

    // line numbers
    bool show_line_numbers;
    bool show_relative_numbers;
    int line_number_width;
} Buffer;

void init_buffer(Buffer *buff);
void free_buffer(Buffer *buff);
void write_buffer(char *filename, Buffer *buff);
int load_file_into_buffer(const char *filename, Buffer *buff);
int get_line_len(int line);
void ensure_row_capacity(Row *row, size_t needed);
void ensure_buffer_capacity(Buffer *buff);

#endif // !BUFFER_H
