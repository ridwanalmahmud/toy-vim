#include "modes.h"
#include "main.h"
#include <ncurses.h>

char *stringify_mode(Mode mode) {
    switch (mode) {
    case NORMAL:
        return "NORMAL";
        break;
    case INSERT:
        return "INSERT";
        break;
    default:
        return "NORMAL";
        break;
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
        fwrite(buff->contents, 1, buff->buf_size, file);
        buff->contents[buff->buf_size] = '\0';
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
void backspace(Buffer *buff) {
    int x, y;
    getyx(stdscr, y, x);

    if (x != 0) {
        move(y, x - 1);
        delch();
        if (buff->buf_x > 0) {
            buff->buf_x -= 1;
            buff->buf_size -= 1;
        }
    } else if (y > 0) {
        int prev_line_len = get_line_len(y - 1);
        move(y - 1, prev_line_len);
        delch();
        if (buff->buf_y > 0) {
            buff->buf_y -= 1;
            buff->buf_x = prev_line_len;
            buff->buf_size -= 1;
        }
    }
}

void insert_mode(int ch, Buffer *buff, Mode *mode) {
    int x, y;
    getyx(stdscr, y, x);
    switch (ch) {
    case ESC:
        *mode = NORMAL;
        if (x > 0) {
            move(y, x - 1);
        } else {
            move(y, x);
        }
        break;
    case BACKSPACE:
        backspace(buff);
        break;
    case ENTER:
        buff->contents[buff->buf_size++] = '\n';
        move(y + 1, 0);
        break;
    default:
        buff->contents[buff->buf_size++] = ch;
        insch(ch);
        move(y, x + 1);
        buff->buf_x = x + 1;
        buff->buf_y = y;
        break;
    }
}

void normal_mode(int ch, Buffer *buff, Mode *mode) {
    int x, y;
    getyx(stdscr, y, x);
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
        move(y, x + 1);
        break;
    case 'o':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        move(y + 1, 0);
        break;
    case 'O':
        keypad(stdscr, FALSE);
        *mode = INSERT;
        move(y - 1, 0);
        break;
    case 'h':
        move(y, x - 1);
        break;
    case 'j':
        move(y + 1, x);
        break;
    case 'k':
        move(y - 1, x);
        break;
    case 'l':
        move(y, x + 1);
        break;
    case '0':
        move(y, 0);
        break;
    case '$':
        move(y, max_x);
        break;
    case 'I':
        *mode = INSERT;
        move(y, 0);
        break;
    case 'A':
        *mode = INSERT;
        move(y, max_x - 1);
        break;
    case 'g':
        move(0, x);
        break;
    case 'G':
        move(max_y - 2, x);
        break;
    case 's':
        *mode = INSERT;
        delch();
        break;
    case 'x':
        delch();
        break;
    case 'D':
        move(y, 0);
        clrtoeol();
        move(y - 1, 0);
        break;
    }

    buff->buf_x = x;
    buff->buf_y = y;
}
