#include "buffer.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // initialize scrolling and line numbers
    buff->row_offset = 0;
    buff->col_offset = 0;
    buff->show_line_numbers = false;
    buff->show_relative_numbers = true;
    buff->line_number_width = 6;
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

int load_file_into_buffer(const char *filename, Buffer *buff) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return -1;
    }

    char line[4096];
    size_t line_count = 0;

    while (fgets(line, sizeof(line), file) && line_count < buff->capacity) {
        // remove newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        if (line_count >= buff->num_rows) {
            ensure_buffer_capacity(buff);
        }

        // allocate or reallocate row content
        ensure_row_capacity(&buff->rows[line_count], len + 1);

        // copy line content
        strcpy(buff->rows[line_count].contents, line);
        buff->rows[line_count].length = len;
        buff->rows[line_count].line_num = line_count + 1;

        line_count++;
    }

    buff->num_rows = line_count;
    fclose(file);
    return 0;
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
