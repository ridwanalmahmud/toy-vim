#include "main.h"
#include "modes.h"
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

int main(void) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    Mode mode = NORMAL;
    mode_stat(mode);

    int ch = getch();
    while (ch != CTRL('q')) {
        mode_stat(mode);
        ch = getch();
        switch (mode) {
        case NORMAL:
            normal_mode(ch, &mode);
            break;
        case INSERT:
            keypad(stdscr, FALSE);
            insert_mode(ch, &mode);
            break;
        }
    }

    refresh();
    endwin();

    return 0;
}
