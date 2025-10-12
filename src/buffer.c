#include "buffer.h"
#include <stdlib.h>

void init_buffer(Buffer *buff) {
    *buff = (Buffer){0};

    buff->capacity = 16;
    buff->rows = calloc(buff->capacity, sizeof(Row));
    if (!buff->rows) {
        buff->capacity = 0;
        return;
    }

    // Create initial empty row
    buff->num_rows = 1;
    buff->rows[0].capacity = 64;
    buff->rows[0].contents = malloc(buff->rows[0].capacity);
    if (buff->rows[0].contents) {
        buff->rows[0].contents[0] = '\0';
        buff->rows[0].length = 0;
        buff->rows[0].line_num = 1;
    }
}

void free_buffer(Buffer *buff) {
    if (!buff)
        return;

    if (buff->rows) {
        for (size_t i = 0; i < buff->num_rows; i++) {
            free(buff->rows[i].contents);
        }
        free(buff->rows);
    }

    // Reset the buffer to empty state
    buff->rows = NULL;
    buff->num_rows = 0;
    buff->capacity = 0;
    buff->cursor.x = 0;
    buff->cursor.y = 0;
    buff->size = 0;
}
