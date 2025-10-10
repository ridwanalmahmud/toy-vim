#include "main.h"
#include <ncurses.h>

void backspace(void) {
    int x, y;
    getyx(stdscr, y, x);

    if (x != 0) {
        move(y, x - 1);
        delch();
    } else if (y > 0) {
        move(y - 1, 0);
        while (x < COLS && mvinch(y - 1, x) != ' ') {
            x++;
        }
        if (x > 0) {
            move(y - 1, x - 1);
        }
        delch();
    }
}

void mode_stat(void) {
    int row, col;
    getmaxyx(stdscr, row, col);
    mvprintw(row - 1, 0, "INSERT");
    move(0, 0);
}

int main(void) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    mode_stat();

    int ch = getch();
    addch(ch);
    while (ch != 'q') {
        ch = getch();
        if (ch == BACKSPACE) {
            backspace();
        } else {
            addch(ch);
        }
    }

    refresh();
    endwin();

    return 0;
}
