#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>

#define MEMORY_SETUP_REGISTER 0xD018

unsigned char BACK_COLOR = COLOR_BLACK;
unsigned char TEXT_COLOR = COLOR_GREEN;

const unsigned char COLUMN_MAX = 39;
const signed char ROW_MAX = 24;

typedef struct column {
    signed char start;
    signed char y;
    signed char length;
    unsigned char chr;
} column;

column columns[40];

unsigned char column_index = 0;
signed char tail_pos = 0;

void init_column(unsigned char column) {
    columns[column].start = -(rand() % 20);
    columns[column].y = columns[column].start;
    columns[column].length = 2 + (rand() % 20);
}

void update_column(unsigned char column) {
    // column is visible, draw char
    if (columns[column].y >= 0 && columns[column].y <= ROW_MAX) {

            // Change color of previos char
            // TODO: Set color at char pos only instead of doing
            // textcolor and drawing the char twice
            if (columns[column].y > 0) {
                textcolor(TEXT_COLOR);
                cputcxy(column, columns[column].y - 1, columns[column].chr);
            }

            columns[column].chr = rand() % 0xFF;

            textcolor(columns[column].y < ROW_MAX ? COLOR_WHITE : TEXT_COLOR);
            cputcxy(column, columns[column].y, columns[column].chr);
        }

        // clear tail
        tail_pos = columns[column].y - columns[column].length;
        if (tail_pos >= 0 && tail_pos <= ROW_MAX) {
            cputcxy(column, tail_pos, ' ');
        }

        // inc y pos
        columns[column].y++;

        // restart column when tail exits screen
    if (columns[column].y - columns[column].length > ROW_MAX) {
        init_column(column);
    }
}

void update(void) {
    for (column_index = 0; column_index <= COLUMN_MAX; column_index++) {
        update_column(column_index);
}
}

int main(void) {
    // randomize rand() function
    _randomize(); 

    // clear screen and set colors
    clrscr();
    bgcolor(BACK_COLOR);
    bordercolor(BACK_COLOR);

    // set character set to upper case
    *(char*)MEMORY_SETUP_REGISTER = 0x15;

    // init all column structs
    for (column_index = 0; column_index <= COLUMN_MAX; column_index++) {
        init_column(column_index);
    }

    while (1) {
        update();
    }

    return 0;
}