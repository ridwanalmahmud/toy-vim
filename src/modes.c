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

void insert_mode(int ch, Mode *mode) {
    int x, y;
    getyx(stdscr, y, x);
    switch (ch) {
    case ESC:
        *mode = NORMAL;
        keypad(stdscr, TRUE);
        if (x > 0) {
            move(y, x - 1);
        } else {
            move(y, x);
        }
        break;
    case BACKSPACE:
        backspace();
        break;
    case ENTER:
        move(y + 1, 0);
        break;
    default:
        buff[buff_s++] = ch;
        insch(ch);
        move(y, x + 1);
        break;
    }
}

void normal_mode(int ch, Mode *mode) {
    int x, y;
    getyx(stdscr, y, x);
    int max_x, max_y;
    getmaxyx(stdscr, max_y, max_x);
    switch (ch) {
    case 'i':
        *mode = INSERT;
        break;
    case 'a':
        *mode = INSERT;
        move(y, x + 1);
        break;
    case 'o':
        *mode = INSERT;
        move(y + 1, 0);
        break;
    case 'O':
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
}
