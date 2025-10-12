#ifndef UTILS_H
#define UTILS_H

#include "buffer.h"

void insert_char_at_cursor(Buffer *buff, char ch);
void backspace(Buffer *buff);
void delete_row(Buffer *buff, size_t row_index);
void insert_newline(Buffer *buff);

#endif // !UTILS_H
