#include "main.h"
#include "modes.h"
#include <ncurses.h>
#include <stdlib.h>

int main(void) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    Buffer *buff = malloc(sizeof(Buffer));
    buff->contents = calloc(1024, sizeof(char));
    buff->buf_size = 0;
    buff->buf_x = 0;
    buff->buf_y = 0;

    Mode mode = NORMAL;
    mode_stat(mode);

    int ch = 0;
    while (ch != CTRL('q')) {
        mode_stat(mode);
        ch = getch();
        switch (mode) {
        case NORMAL:
            keypad(stdscr, TRUE);
            normal_mode(ch, buff, &mode);
            break;
        case INSERT:
            keypad(stdscr, FALSE);
            insert_mode(ch, buff, &mode);
            break;
        }
    }

    refresh();
    free(buff);
    endwin();

    return 0;
}
