#include "main.h"
#include "buffer.h"
#include "modes.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

void draw_interface(Buffer *buff, Mode mode, const char *filename) {
    int screen_rows, screen_cols;
    getmaxyx(stdscr, screen_rows, screen_cols);

    clear();

    // adjust row offset if cursor moved off screen
    if (buff->cursor.y < buff->row_offset) {
        buff->row_offset = buff->cursor.y;
    } else if (buff->cursor.y >= buff->row_offset + screen_rows - 1) {
        buff->row_offset = buff->cursor.y - screen_rows + 2;
    }

    // adjust column offset if cursor moved off screen
    if (buff->cursor.x < buff->col_offset) {
        buff->col_offset = buff->cursor.x;
    } else if (buff->cursor.x >= buff->col_offset + screen_cols) {
        buff->col_offset = buff->cursor.x - screen_cols + 1;
    }

    // display visible rows only
    for (int i = 0; i < screen_rows - 1; i++) {
        size_t row_idx = buff->row_offset + i;

        if (row_idx < buff->num_rows) {
            Row *row = &buff->rows[row_idx];

            // calculate how much of the line to display
            int display_length = row->length - buff->col_offset;
            if (display_length > screen_cols) {
                display_length = screen_cols;
            }

            if (display_length > 0 && buff->col_offset < row->length) {
                mvprintw(i,
                         0,
                         "%.*s",
                         display_length,
                         row->contents + buff->col_offset);
            }

            // clear the rest of the line
            if (display_length < screen_cols) {
                mvprintw(
                    i, display_length, "%*s", screen_cols - display_length, "");
            }
        } else {
            // clear empty lines
            mvprintw(i, 0, "%*s", screen_cols, "");
        }
    }

    // draw status bar with scrolling info
    attron(A_REVERSE);
    mvprintw(screen_rows - 1,
             0,
             " %s | %s | Row %zu/%zu, Col %zu ",
             stringify_mode(mode),
             filename,
             buff->cursor.y + 1,
             buff->num_rows,
             buff->cursor.x + 1);
    clrtoeol();
    attroff(A_REVERSE);

    // calculate screen-relative cursor position
    int screen_cursor_y = buff->cursor.y - buff->row_offset;
    int screen_cursor_x = buff->cursor.x - buff->col_offset;

    // ensure cursor stays on screen
    if (screen_cursor_y >= 0 && screen_cursor_y < screen_rows - 1 &&
        screen_cursor_x >= 0 && screen_cursor_x < screen_cols) {
        move(screen_cursor_y, screen_cursor_x);
    }
}

int main(int argc, char *argv[]) {
    initscr();
    raw();
    keypad(stdscr, true);
    noecho();

    Buffer buff;
    init_buffer(&buff);

    char *filename = "untitled.txt";
    if (argc > 1) {
        filename = argv[1];
        if (load_file_into_buffer(filename, &buff) != 0) {
            create_empty_file(filename);
        }
    }

    Mode mode = NORMAL;
    int ch = 0;

    while (ch != CTRL('q')) {
        draw_interface(&buff, mode, filename);
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

    write_buffer(filename, &buff);
    free_buffer(&buff);
    endwin();
    return 0;
}
