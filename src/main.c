#include "main.h"
#include "buffer.h"
#include "modes.h"
#include <ncurses.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

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

void create_empty_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file) {
        fclose(file);
    }
}

int main(int argc, char *argv[]) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    Buffer buff;
    init_buffer(&buff);

    char *filename = NULL;

    if (argc > 1) {
        filename = argv[1];

        if (load_file_into_buffer(filename, &buff) == 0) {
            mvprintw(0, 0, "Loaded: %s", filename);
        } else {
            create_empty_file(filename);
            mvprintw(0, 0, "New file: %s", filename);
        }
    } else {
        filename = "untitled.txt";
        mvprintw(0, 0, "No file specified. Using: %s", filename);
    }

    Mode mode = NORMAL;

    int ch = 0;
    while (ch != CTRL('q')) {
        clear();

        // display file content
        for (size_t i = 0; i < buff.num_rows && i < (size_t)(LINES - 2); i++) {
            if (buff.rows[i].contents) {
                mvprintw(i, 0, "%s", buff.rows[i].contents);
            }
        }

        // draw status bar with filename and mode
        int row, col;
        getmaxyx(stdscr, row, col);
        attron(A_REVERSE);
        mvprintw(row - 1, 0, " %s | %s ", stringify_mode(mode), filename);
        clrtoeol();
        attroff(A_REVERSE);

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

    // save before quitting if filename was provided
    if (argc > 1) {
        write_buffer(filename, &buff);
    } else {
        write_buffer("untitled.txt", &buff);
    }

    free_buffer(&buff);
    endwin();
    return 0;
}
