#include "modes.h"
#include "main.h"
#include "buffer.h"
#include "utils.h"
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
    case KEY_LEFT:
        if (buff->cursor.x > 0) {
            buff->cursor.x--;
        }
        break;
    case 'j':
    case KEY_DOWN:
        if (buff->cursor.y < buff->num_rows - 1) {
            buff->cursor.y++;
            // Ensure cursor x doesn't go beyond new line length
            if (buff->cursor.x > buff->rows[buff->cursor.y].length - 1) {
                buff->cursor.x = buff->rows[buff->cursor.y].length - 1;
            }
        }
        break;
    case 'k':
    case KEY_UP:
        if (buff->cursor.y > 0) {
            buff->cursor.y--;
            // Ensure cursor x doesn't go beyond new line length
            if (buff->cursor.x > buff->rows[buff->cursor.y].length - 1) {
                buff->cursor.x = buff->rows[buff->cursor.y].length - 1;
            }
        }
        break;
    case 'l':
    case KEY_RIGHT:
        if (buff->cursor.x < buff->rows[buff->cursor.y].length - 1) {
            buff->cursor.x++;
        }
        break;
    case '0':
        buff->cursor.x = 0;
        break;
    case '$':
        buff->cursor.x = buff->rows[buff->cursor.y].length - 1;
        break;
    case '_':
        if (buff->cursor.y < buff->num_rows) {
            Row *row = &buff->rows[buff->cursor.y];

            // find first non-whitespace character
            size_t first_non_ws = 0;
            while (first_non_ws < row->length &&
                   (row->contents[first_non_ws] == ' ' ||
                    row->contents[first_non_ws] == '\t')) {
                first_non_ws++;
            }

            // if line is empty or all whitespace, go to start
            if (first_non_ws >= row->length) {
                buff->cursor.x = 0;
            } else {
                buff->cursor.x = first_non_ws;
            }
        }
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
        if (buff->cursor.y < buff->num_rows) {
            Row *row = &buff->rows[buff->cursor.y];
            row->length = buff->cursor.x;
            row->contents[buff->cursor.x] = '\0';
        }
        break;
    case CTRL('d'):
        if (buff->num_rows > 0) {
            delete_row(buff, buff->cursor.y);

            if (buff->cursor.y >= buff->num_rows && buff->num_rows > 0) {
                buff->cursor.y = buff->num_rows - 1;
            }
            buff->cursor.x = 0;
        }
        break;
    }
}
