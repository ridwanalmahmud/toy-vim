#include "main.h"
#include "buffer.h"
#include "modes.h"
#include <ncurses.h>
#include <stdlib.h>

int main(void) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    Buffer buff;
    init_buffer(&buff);

    Mode mode = NORMAL;
    mode_stat(mode);

    // load initial content or display empty buffer
    if (buff.num_rows > 0 && buff.rows[0].contents) {
        mvprintw(0, 0, "%s", buff.rows[0].contents);
    }
    move(buff.cursor.y, buff.cursor.x);

    int ch = 0;
    while (ch != CTRL('q')) {
        // clear and redisplay buffer content
        clear();
        for (size_t i = 0; i < buff.num_rows && i < (size_t)LINES; i++) {
            if (buff.rows[i].contents) {
                mvprintw(i, 0, "%s", buff.rows[i].contents);
            }
        }

        mode_stat(mode);

        // move cursor to current position
        move(buff.cursor.y, buff.cursor.x);
        refresh();

        ch = getch();
        switch (mode) {
        case NORMAL:
            keypad(stdscr, TRUE);
            normal_mode(ch, &buff, &mode);
            break;
        case INSERT:
            keypad(stdscr, FALSE);
            insert_mode(ch, &buff, &mode);
            break;
        }
    }

    free_buffer(&buff);
    endwin();

    return 0;
}
