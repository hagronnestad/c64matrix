#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <unistd.h>
#include <6502.h>

#include "characters.h"

#define MEMORY_SETUP_REGISTER 0xD018
#define MEMORY_SETUP_REGISTER_CHAR_SET_UPPER 0x15 // $D018 = %xxxx010x -> charmem is at $1000
#define MEMORY_SETUP_REGISTER_CHAR_SET_LOWER 0x17 // $D018 = %xxxx011x -> charmem is at $1800
#define MEMORY_SETUP_REGISTER_CHAR_SET_3800  0x1F // $D018 = %xxxx111x -> charmem is at $3800

#define SOFT_RESET_VECTOR 0xFCE2

unsigned char DEFAULT_BACK_COLOR = COLOR_BLACK;
unsigned char DEFAULT_TEXT_COLOR = COLOR_GREEN;

unsigned char BACK_COLOR = COLOR_BLACK;
unsigned char TEXT_COLOR = COLOR_GREEN;

unsigned char MAX_START = 20;
unsigned char MAX_LENGTH = 20;

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

unsigned char rainbow_mode = 0;
unsigned char matrix_mode = 0;
unsigned char numbers_symbols_only = 0;
unsigned char is_running = 0;

struct regs *registers;

void start_matrix(void);


void init_column(unsigned char column) {
    columns[column].start = -(rand() % MAX_START);
    columns[column].y = columns[column].start;
    columns[column].length = 2 + (rand() % MAX_LENGTH);
}

void init_all_columns() {
    for (column_index = 0; column_index <= COLUMN_MAX; column_index++) {
        init_column(column_index);
    }
}

void update_column(unsigned char column) {
    // column is visible, draw char
    if (columns[column].y >= 0 && columns[column].y <= ROW_MAX) {

        // Change color of previous char
        // TODO: Set color at char pos only instead of doing
        // textcolor and drawing the char twice
        if (columns[column].y > 0) {
            textcolor(rainbow_mode ? rand() % 15 : TEXT_COLOR);
            cputcxy(column, columns[column].y - 1, columns[column].chr);
        }

        // columns[column].chr = 0x30 + rand() % 0x2A;
        // columns[column].chr = rand() % 0xFF;
        // columns[column].chr = '!' + (rand() % ('z' - '!'));
        // columns[column].chr = 0xA1 + rand() % (0xFF - 0xA1);

        if (numbers_symbols_only) {
            columns[column].chr = (unsigned char)27 + (unsigned char)(rand() % 64);
        } else {
            columns[column].chr = rand() % 0xFF;
        }

        textcolor(columns[column].y < ROW_MAX ?
            COLOR_WHITE : (rainbow_mode ? rand() % 15 : TEXT_COLOR));
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

void randomize_colors() {
    do {
        BACK_COLOR = rand() % 16;
    } while (BACK_COLOR == COLOR_WHITE);
    
    do {
        TEXT_COLOR = rand() % 16;
    } while (TEXT_COLOR == BACK_COLOR || TEXT_COLOR == COLOR_WHITE);
}

void handle_choice(unsigned char choice) {
    switch (choice) {
        case '1':
            randomize_colors();
            textcolor(TEXT_COLOR);
            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            if (is_running) {
                clrscr();
                init_all_columns();
            }
            break;

        case '2':
            TEXT_COLOR = DEFAULT_TEXT_COLOR;
            BACK_COLOR = DEFAULT_BACK_COLOR;
            textcolor(TEXT_COLOR);
            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            if (is_running) {
                clrscr();
                init_all_columns();
            }
            break;

        case '3':
            rainbow_mode = rainbow_mode == 0 ? 1 : 0;
            if (is_running) {
                clrscr();
                init_all_columns();
            }
            break;

        case '4':
            if (MAX_LENGTH > 1) MAX_LENGTH--;
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("max length: %2d", MAX_LENGTH);
            }
            break;

        case '5':
            if (MAX_LENGTH < 20) MAX_LENGTH++;
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("max length: %2d", MAX_LENGTH);
            }
            break;

        case 'r':
            registers->pc = SOFT_RESET_VECTOR;
            _sys(registers);
            // asm("jsr $FCE2");

        case 'n':
            numbers_symbols_only = numbers_symbols_only == 0 ? 1 : 0;
            if (is_running) {
                *(char*)MEMORY_SETUP_REGISTER = numbers_symbols_only == 1 ?
                    MEMORY_SETUP_REGISTER_CHAR_SET_UPPER : MEMORY_SETUP_REGISTER_CHAR_SET_LOWER;
                // clrscr();
                // init_all_columns();
            }
            break;

        case 'm':
            matrix_mode = matrix_mode == 0 ? 1 : 0;
            if (is_running) {
                *(char*)MEMORY_SETUP_REGISTER = matrix_mode == 1 ?
                    MEMORY_SETUP_REGISTER_CHAR_SET_3800 : MEMORY_SETUP_REGISTER_CHAR_SET_LOWER;
                clrscr();
                init_all_columns();
            }
            break;

        case 13: // RETURN
            if (!is_running) {
                start_matrix();
            }

        default:
            is_running = 0;
            break;
    }
}

void start_matrix(void) {
    clrscr();

    // set character set to upper case
    // *(char*)MEMORY_SETUP_REGISTER = MEMORY_SETUP_REGISTER_CHAR_SET_UPPER;
    *(char*)MEMORY_SETUP_REGISTER = matrix_mode == 1 ?
        MEMORY_SETUP_REGISTER_CHAR_SET_3800 : numbers_symbols_only == 1 ?
        MEMORY_SETUP_REGISTER_CHAR_SET_UPPER : MEMORY_SETUP_REGISTER_CHAR_SET_LOWER;

    // init all column structs
    init_all_columns();

    is_running = 1;

    while (is_running) {
        update();

        if (kbhit()) {
            handle_choice(cgetc());
        }
    }
}

void show_menu() {   
    unsigned char option = 0;

    // clear screen and set colors
    clrscr();
    textcolor(TEXT_COLOR);
    bgcolor(BACK_COLOR);
    bordercolor(BACK_COLOR);

    // set character set to lower case
    *(char*)MEMORY_SETUP_REGISTER = MEMORY_SETUP_REGISTER_CHAR_SET_LOWER;

    cputsxy(0, 0, "c64matrix - cmatrix for the Commodore 64");
    cputsxy(0, 2, "SETTINGS");
    chlinexy(0, 3, 40);

    cputsxy(8, 6, "1: Randomize colors");
    cputsxy(8, 7, "2: Default colors");
    cputsxy(8, 8, "3: Rainbow mode");
    printf("        (%s)", rainbow_mode ? "ON" : "OFF");
    cputsxy(8, 9, "4: Increase length");
    printf("     (%d)", MAX_LENGTH);
    cputsxy(8, 10, "5: Decrease length");
    printf("     (%d)", MAX_LENGTH);
    cputsxy(8, 11, "M: Matrix mode");
    printf("         (%s)", matrix_mode ? "ON" : "OFF");

    cputsxy(3, 14, "Return: START");
    cputsxy(6, 16, "Any: MENU");

    cputsxy(8, 18, "R: Soft Reset");

    chlinexy(0, 23, 40);
    cputsxy(0, 24, "C0d3d by HAG - http://hag.yt");

    option = cgetc();
    handle_choice(option);

    show_menu();
}

void copy_matrix_code_chars() {
    memcpy((const void *) 0x3800, MATRIX_CODE_CHARS_2, 4096);
}

int main(void) {
    copy_matrix_code_chars();

    // randomize rand() function
    _randomize(); 

    // clear screen and set colors
    clrscr();
    textcolor(TEXT_COLOR);
    bgcolor(BACK_COLOR);
    bordercolor(BACK_COLOR);

loop:
    start_matrix();
    show_menu();
    goto loop;

    return 0;
}