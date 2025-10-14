#include "main.h"
#include "buffer.h"
#include "modes.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    // adjust scrolling (your existing code)
    if (buff->cursor.y < buff->row_offset) {
        buff->row_offset = buff->cursor.y;
    } else if (buff->cursor.y >= buff->row_offset + screen_rows - 1) {
        buff->row_offset = buff->cursor.y - screen_rows + 2;
    }

    if (buff->cursor.x < buff->col_offset) {
        buff->col_offset = buff->cursor.x;
    } else if (buff->cursor.x >= buff->col_offset + screen_cols) {
        buff->col_offset = buff->cursor.x - screen_cols + 1;
    }

    // calculate line number area width
    int line_num_area = 0;
    if (buff->show_line_numbers || buff->show_relative_numbers) {
        // calculate needed width based on total lines
        int max_line_num = buff->num_rows;
        buff->line_number_width = 0;
        while (max_line_num > 0) {
            max_line_num /= 10;
            buff->line_number_width++;
        }
        buff->line_number_width += 4; // add some padding
        if (buff->line_number_width < 4)
            buff->line_number_width = 4;
        if (buff->line_number_width > 8)
            buff->line_number_width = 8;

        line_num_area = buff->line_number_width;
    }

    // display visible rows with line numbers
    for (int i = 0; i < screen_rows - 1; i++) {
        size_t row_idx = buff->row_offset + i;

        // draw line numbers
        if (line_num_area > 0 && row_idx < buff->num_rows) {
            char line_num_str[16];

            if (buff->show_relative_numbers) {
                // relative line numbers
                int relative_num = abs((int)row_idx - (int)buff->cursor.y);
                if (relative_num == 0) {
                    // current line shows absolute number
                    snprintf(line_num_str,
                             sizeof(line_num_str),
                             "%*zu ",
                             buff->line_number_width - 1,
                             row_idx + 1);
                } else {
                    snprintf(line_num_str,
                             sizeof(line_num_str),
                             "%*d ",
                             buff->line_number_width - 1,
                             relative_num);
                }
            } else if (buff->show_line_numbers) {
                // absolute line numbers
                snprintf(line_num_str,
                         sizeof(line_num_str),
                         "%*zu ",
                         buff->line_number_width - 1,
                         row_idx + 1);
            }

            // draw line number with different attributes
            attron(COLOR_PAIR(4) | A_DIM);
            mvprintw(i, 0, "%s  ", line_num_str);
            attroff(COLOR_PAIR(4) | A_DIM);
        } else if (line_num_area > 0) {
            // empty line in line number area
            mvprintw(i, 0, "%*s", line_num_area, "");
        }

        // display text content
        if (row_idx < buff->num_rows) {
            Row *row = &buff->rows[row_idx];

            // calculate how much of the line to display (account for line number area)
            int text_start_col = line_num_area;
            int available_width = screen_cols - text_start_col;
            int display_length = row->length - buff->col_offset;

            if (display_length > available_width) {
                display_length = available_width;
            }

            if (display_length > 0 && buff->col_offset < row->length) {
                mvprintw(i,
                         text_start_col,
                         "%.*s",
                         display_length,
                         row->contents + buff->col_offset);
            }

            // clear the rest of the line
            if (display_length < available_width) {
                mvprintw(i,
                         text_start_col + display_length,
                         "%*s",
                         available_width - display_length,
                         "");
            }
        } else {
            // clear empty lines (text area only)
            mvprintw(i, line_num_area, "%*s", screen_cols - line_num_area, "");
        }
    }

    // draw status bar with line number mode info
    attron(A_REVERSE);
    char line_mode[16];
    if (buff->show_relative_numbers) {
        snprintf(line_mode, sizeof(line_mode), "REL");
    } else if (buff->show_line_numbers) {
        snprintf(line_mode, sizeof(line_mode), "ABS");
    } else {
        snprintf(line_mode, sizeof(line_mode), "NO");
    }

    mvprintw(screen_rows - 1,
             0,
             " %s | %s | Line: %s | Row %zu/%zu, Col %zu ",
             stringify_mode(mode),
             filename,
             line_mode,
             buff->cursor.y + 1,
             buff->num_rows,
             buff->cursor.x + 1);
    clrtoeol();
    attroff(A_REVERSE);

    // calculate screen-relative cursor position (account for line numbers)
    int screen_cursor_y = buff->cursor.y - buff->row_offset;
    int screen_cursor_x = buff->cursor.x - buff->col_offset + line_num_area;

    // ensure cursor stays on screen
    if (screen_cursor_y >= 0 && screen_cursor_y < screen_rows - 1 &&
        screen_cursor_x >= line_num_area && screen_cursor_x < screen_cols) {
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
