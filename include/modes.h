#ifndef __MODES_H__
#define __MODES_H__

typedef enum {
    NORMAL,
    INSERT,
} Mode;

void mode_stat(Mode mode);
void insert_mode(int ch, Mode *mode);
void normal_mode(int ch, Mode *mode);

#endif // !__MODES_H__
