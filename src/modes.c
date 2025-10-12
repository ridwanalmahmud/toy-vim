#include "modes.h"
#include "main.h"
#include "buffer.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

char *stringify_mode(Mode mode) {
    switch (mode) {
    case NORMAL:
        return "NORMAL";
    case INSERT:
        return "INSERT";
    default:
        return "NORMAL";
    }
}

void mode_stat(Mode mode) {
    int row, col, x, y;
    getmaxyx(stdscr, row, col);
    getyx(stdscr, y, x);
    mvprintw(row - 1, 0, "%s", stringify_mode(mode));
    move(y, x);
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

// Helper function to ensure row has enough capacity
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

// Helper function to ensure buffer has enough rows
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

void insert_char_at_cursor(Buffer *buff, char ch) {
    if (buff->cursor.y >= buff->num_rows)
        return;

    Row *row = &buff->rows[buff->cursor.y];
    ensure_row_capacity(row, row->length + 1);

    // Insert character at cursor position
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

void insert_mode(int ch, Buffer *buff, Mode *mode) {
    switch (ch) {
    case ESC:
        *mode = NORMAL;
        if (buff->cursor.x > 0) {
            buff->cursor.x--;
        }
        break;
    case BACKSPACE:
        backspace(buff);
        break;
    case ENTER:
        insert_newline(buff);
        break;
    default:
        insert_char_at_cursor(buff, ch);
        break;
    }
}

void normal_mode(int ch, Buffer *buff, Mode *mode) {
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);

    switch (ch) {
    case CTRL('w'):
        write_buffer("buff.txt", buff);
        break;
    case 'i':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        break;
    case 'a':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        if (buff->cursor.x < buff->rows[buff->cursor.y].length) {
            buff->cursor.x++;
        }
        break;
    case 'o':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        // insert new line below current line
        ensure_buffer_capacity(buff);
        memmove(buff->rows + buff->cursor.y + 2,
                buff->rows + buff->cursor.y + 1,
                (buff->num_rows - buff->cursor.y - 1) * sizeof(Row));
        buff->rows[buff->cursor.y + 1] = (Row){0};
        buff->rows[buff->cursor.y + 1].contents = strdup("");
        buff->rows[buff->cursor.y + 1].length = 0;
        buff->rows[buff->cursor.y + 1].capacity = 64;
        buff->rows[buff->cursor.y + 1].line_num = buff->cursor.y + 2;
        buff->num_rows++;
        buff->cursor.y++;
        buff->cursor.x = 0;
        break;
    case 'O':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        // insert new line above current line
        ensure_buffer_capacity(buff);
        memmove(buff->rows + buff->cursor.y + 1,
                buff->rows + buff->cursor.y,
                (buff->num_rows - buff->cursor.y) * sizeof(Row));
        buff->rows[buff->cursor.y] = (Row){0};
        buff->rows[buff->cursor.y].contents = strdup("");
        buff->rows[buff->cursor.y].length = 0;
        buff->rows[buff->cursor.y].capacity = 64;
        buff->rows[buff->cursor.y].line_num = buff->cursor.y + 1;
        buff->num_rows++;
        buff->cursor.x = 0;
        // update line numbers for all subsequent rows
        for (size_t i = buff->cursor.y + 1; i < buff->num_rows; i++) {
            buff->rows[i].line_num++;
        }
        break;
    case 'h':
        if (buff->cursor.x > 0)
            buff->cursor.x--;
        break;
    case 'j':
        if (buff->cursor.y < buff->num_rows - 1)
            buff->cursor.y++;
        break;
    case 'k':
        if (buff->cursor.y > 0)
            buff->cursor.y--;
        break;
    case 'l':
        if (buff->cursor.x < buff->rows[buff->cursor.y].length - 1)
            buff->cursor.x++;
        break;
    case '0':
        buff->cursor.x = 0;
        break;
    case '$':
        buff->cursor.x = buff->rows[buff->cursor.y].length;
        break;
    case 'I':
        *mode = INSERT;
        buff->cursor.x = 0;
        break;
    case 'A':
        *mode = INSERT;
        buff->cursor.x = buff->rows[buff->cursor.y].length;
        break;
    case 'g':
        buff->cursor.y = 0;
        buff->cursor.x = 0;
        break;
    case 'G':
        if (buff->num_rows > 0) {
            buff->cursor.y = buff->num_rows - 1;
            buff->cursor.x = 0;
        }
        break;
    case 's':
        *mode = INSERT;
        if (buff->cursor.x < buff->rows[buff->cursor.y].length) {
            // delete character at cursor
            Row *row = &buff->rows[buff->cursor.y];
            memmove(row->contents + buff->cursor.x,
                    row->contents + buff->cursor.x + 1,
                    row->length - buff->cursor.x - 1);
            row->length--;
            row->contents[row->length] = '\0';
        }
        break;
    case 'x':
        if (buff->cursor.x < buff->rows[buff->cursor.y].length) {
            Row *row = &buff->rows[buff->cursor.y];
            memmove(row->contents + buff->cursor.x,
                    row->contents + buff->cursor.x + 1,
                    row->length - buff->cursor.x - 1);
            row->length--;
            row->contents[row->length] = '\0';
        }
        break;
    case 'D':
        // delete to end of line
        if (buff->cursor.y < buff->num_rows) {
            Row *row = &buff->rows[buff->cursor.y];
            row->length = buff->cursor.x;
            row->contents[buff->cursor.x] = '\0';
        }
        break;
    }
}
