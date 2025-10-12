#include "buffer.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>

void init_buffer(Buffer *buff) {
    *buff = (Buffer){0};

    buff->capacity = 16;
    buff->rows = calloc(buff->capacity, sizeof(Row));
    if (!buff->rows) {
        buff->capacity = 0;
        return;
    }

    // start with one empty row
    buff->num_rows = 1;
    buff->rows[0].capacity = 64;
    buff->rows[0].contents = malloc(buff->rows[0].capacity);
    if (buff->rows[0].contents) {
        buff->rows[0].contents[0] = '\0';
        buff->rows[0].length = 0;
        buff->rows[0].line_num = 1;
    }

    // initialize scrolling
    buff->row_offset = 0;
    buff->col_offset = 0;
}

void free_buffer(Buffer *buff) {
    if (!buff)
        return;

    if (buff->rows) {
        for (size_t i = 0; i < buff->num_rows; i++) {
            free(buff->rows[i].contents);
        }
        free(buff->rows);
    }

    // reset the buffer to empty state
    buff->rows = NULL;
    buff->num_rows = 0;
    buff->capacity = 0;
    buff->cursor.x = 0;
    buff->cursor.y = 0;
    buff->size = 0;
}

void write_buffer(char *filename, Buffer *buff) {
    FILE *file = fopen(filename, "w");
    if (file) {
        for (size_t i = 0; i < buff->num_rows; i++) {
            fwrite(buff->rows[i].contents, 1, buff->rows[i].length, file);
            if (i < buff->num_rows - 1) {
                fputc('\n', file);
            }
        }
        fclose(file);
    }
}

int get_line_len(int line) {
    int line_len = 0;
    for (int i = COLS - 1; i >= 0; i--) {
        chtype ch = mvwinch(stdscr, line, i);
        if ((ch & A_CHARTEXT) != ' ') {
            line_len = i + 1;
            break;
        }
    }
    return line_len;
}

void ensure_row_capacity(Row *row, size_t needed) {
    if (needed >= row->capacity) {
        size_t new_capacity = row->capacity ? row->capacity * 2 : 64;
        char *new_contents = realloc(row->contents, new_capacity);
        if (new_contents) {
            row->contents = new_contents;
            row->capacity = new_capacity;
        }
    }
}

void ensure_buffer_capacity(Buffer *buff) {
    if (buff->num_rows >= buff->capacity) {
        size_t new_capacity = buff->capacity ? buff->capacity * 2 : 16;
        Row *new_rows = realloc(buff->rows, new_capacity * sizeof(Row));
        if (new_rows) {
            buff->rows = new_rows;
            buff->capacity = new_capacity;
        }
    }
}
