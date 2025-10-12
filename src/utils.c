#include "utils.h"
#include <stdlib.h>
#include <string.h>

void insert_char_at_cursor(Buffer *buff, char ch) {
    if (buff->cursor.y >= buff->num_rows)
        return;

    Row *row = &buff->rows[buff->cursor.y];
    ensure_row_capacity(row, row->length + 1);

    // insert character at cursor position
    if (buff->cursor.x <= row->length) {
        memmove(row->contents + buff->cursor.x + 1,
                row->contents + buff->cursor.x,
                row->length - buff->cursor.x);
        row->contents[buff->cursor.x] = ch;
        row->length++;
        row->contents[row->length] = '\0';
        buff->cursor.x++;
    }
}

void backspace(Buffer *buff) {
    if (buff->cursor.y >= buff->num_rows)
        return;

    Row *row = &buff->rows[buff->cursor.y];

    if (buff->cursor.x > 0) {
        // delete character before cursor
        memmove(row->contents + buff->cursor.x - 1,
                row->contents + buff->cursor.x,
                row->length - buff->cursor.x);
        row->length--;
        row->contents[row->length] = '\0';
        buff->cursor.x--;
    } else if (buff->cursor.y > 0) {
        // merge with previous line
        Row *prev_row = &buff->rows[buff->cursor.y - 1];
        size_t prev_len = prev_row->length;

        ensure_row_capacity(prev_row, prev_row->length + row->length);
        memcpy(prev_row->contents + prev_len, row->contents, row->length);
        prev_row->length += row->length;
        prev_row->contents[prev_row->length] = '\0';

        // remove the current row
        free(row->contents);
        memmove(buff->rows + buff->cursor.y,
                buff->rows + buff->cursor.y + 1,
                (buff->num_rows - buff->cursor.y - 1) * sizeof(Row));
        buff->num_rows--;

        buff->cursor.y--;
        buff->cursor.x = prev_len;
    }
}

void delete_row(Buffer *buff, size_t row_index) {
    if (row_index >= buff->num_rows)
        return;

    // free the row's contents
    free(buff->rows[row_index].contents);

    // shift rows down
    for (size_t i = row_index; i < buff->num_rows - 1; i++) {
        buff->rows[i] = buff->rows[i + 1];
        buff->rows[i].line_num = i + 1;
    }

    buff->num_rows--;

    // ensure we always have at least one row
    if (buff->num_rows == 0) {
        ensure_buffer_capacity(buff);
        buff->rows[0].capacity = 64;
        buff->rows[0].contents = calloc(1, buff->rows[0].capacity);
        buff->rows[0].length = 0;
        buff->rows[0].line_num = 1;
        buff->num_rows = 1;
    }
}

void insert_newline(Buffer *buff) {
    ensure_buffer_capacity(buff);

    if (buff->cursor.y >= buff->num_rows)
        return;
    Row *current_row = &buff->rows[buff->cursor.y];

    // create new row
    size_t split_pos = buff->cursor.x;
    size_t new_line_len = current_row->length - split_pos;

    // move rows down to make space
    memmove(buff->rows + buff->cursor.y + 2,
            buff->rows + buff->cursor.y + 1,
            (buff->num_rows - buff->cursor.y - 1) * sizeof(Row));

    // initialize new row
    Row *new_row = &buff->rows[buff->cursor.y + 1];
    new_row->capacity = new_line_len + 16;
    new_row->contents = malloc(new_row->capacity);
    new_row->length = new_line_len;
    if (new_line_len > 0) {
        memcpy(
            new_row->contents, current_row->contents + split_pos, new_line_len);
    }
    new_row->contents[new_line_len] = '\0';

    // truncate current row
    current_row->length = split_pos;
    current_row->contents[split_pos] = '\0';

    buff->num_rows++;

    // update line numbers
    for (size_t i = buff->cursor.y + 1; i < buff->num_rows; i++) {
        buff->rows[i].line_num = i + 1;
    }

    buff->cursor.y++;
    buff->cursor.x = 0;
}
