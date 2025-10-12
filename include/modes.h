#ifndef MODES_H
#define MODES_H

#include "buffer.h"

typedef enum {
    NORMAL,
    INSERT,
} Mode;

void mode_stat(Mode mode);
char *stringify_mode(Mode mode);
void insert_mode(int ch, Buffer *buff, Mode *mode);
void normal_mode(int ch, Buffer *buff, Mode *mode);

#endif // !MODES_H
