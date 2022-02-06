#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>
#include <6502.h>
#include <cbm.h>

#define MEMORY_SETUP_REGISTER 0xD018
#define SOFT_RESET_VECTOR 0xFCE2

unsigned char DEFAULT_BACK_COLOR = COLOR_BLACK;
unsigned char DEFAULT_TEXT_COLOR = COLOR_LIGHTGREEN;

unsigned char BACK_COLOR = COLOR_BLACK;
unsigned char TEXT_COLOR = COLOR_LIGHTGREEN;

unsigned char TEXT_COLORS_MAX = 15;
unsigned char text_colors[16];

unsigned char MAX_START = 20;
unsigned char MAX_LENGTH = 12;

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

unsigned char reverse_mode = 0;
unsigned char shifted_mode = 0;
unsigned char symbols_only_mode = 0;
unsigned char is_running = 0;

struct regs *registers;


void set_text_colors_all() {
    text_colors[0] = COLOR_BLACK;
    text_colors[1] = COLOR_WHITE;
    text_colors[2] = COLOR_RED;
    text_colors[3] = COLOR_CYAN;
    text_colors[4] = COLOR_VIOLET;
    text_colors[5] = COLOR_GREEN;
    text_colors[6] = COLOR_BLUE;
    text_colors[7] = COLOR_YELLOW;
    text_colors[8] = COLOR_ORANGE;
    text_colors[9] = COLOR_BROWN;
    text_colors[10] = COLOR_LIGHTRED;
    text_colors[11] = COLOR_GRAY1;
    text_colors[12] = COLOR_GRAY2;
    text_colors[13] = COLOR_LIGHTGREEN;
    text_colors[14] = COLOR_LIGHTBLUE;
    text_colors[15] = COLOR_GRAY3;
    TEXT_COLORS_MAX = 15;
}

void set_text_colors_green() {
    text_colors[0] = COLOR_GREEN;
    text_colors[1] = COLOR_LIGHTGREEN;
    TEXT_COLORS_MAX = 2;
}

void set_text_colors_blue() {
    text_colors[0] = COLOR_BLUE;
    text_colors[1] = COLOR_LIGHTBLUE;
    TEXT_COLORS_MAX = 2;
}


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

void update_column(unsigned char col) {
    signed char y = columns[col].y;

    // Check if column is visible on screen
    if (y >= 0 && y <= ROW_MAX) {

        // Change color of previos char
        // TODO: Set color at char pos only instead of doing
        // textcolor and drawing the char twice
        if (y > 0) {
            textcolor(text_colors[rand() % TEXT_COLORS_MAX]);
            cputcxy(col, y - 1, columns[col].chr);
        }

        if (symbols_only_mode) { // shifted_mode must be off
            columns[col].chr = 0x5B + (rand() % (0xBF-0x5B));
        } else {
            columns[col].chr = 0x21 + (rand() % (0xBF-0x21));
        }

        textcolor((y < ROW_MAX) ? COLOR_WHITE : text_colors[rand() % TEXT_COLORS_MAX]);
        cputcxy(col, y, columns[col].chr);
    }

    // clear tail
    tail_pos = y - columns[col].length;
    if (tail_pos >= 0 && tail_pos <= ROW_MAX) {
        if (reverse_mode) textcolor(BACK_COLOR);
        cputcxy(col, tail_pos, ' ');
    }

    // inc y pos
    columns[col].y++;

    // restart column when tail exits screen
    if (y - columns[col].length > ROW_MAX) {
        init_column(col);
    }
}

void update(void) {
    for (column_index = 0; column_index <= COLUMN_MAX; column_index++) {
        update_column(column_index);
    }
}

void handle_choice(unsigned char choice) {
    textcolor(TEXT_COLOR);

    switch (choice) {
        case '1':
            do {
                BACK_COLOR++;
                if (BACK_COLOR == 15) BACK_COLOR = 0;
            } while (BACK_COLOR == TEXT_COLOR || BACK_COLOR == COLOR_WHITE);

            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
            }
            break;

        case '2':
            do {
                TEXT_COLOR++;
                if (TEXT_COLOR == 15) TEXT_COLOR = 0;
            } while (TEXT_COLOR == BACK_COLOR || TEXT_COLOR == COLOR_WHITE);

            text_colors[0] = TEXT_COLOR;
            TEXT_COLORS_MAX = 1;
            textcolor(TEXT_COLOR);
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
            }
            break;

        case '3':
            TEXT_COLOR = DEFAULT_TEXT_COLOR;
            BACK_COLOR = DEFAULT_BACK_COLOR;
            textcolor(TEXT_COLOR);
            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            set_text_colors_green();
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("green theme");
            }
            break;

        case '4':
            TEXT_COLOR = COLOR_LIGHTBLUE;
            BACK_COLOR = COLOR_BLACK;
            textcolor(TEXT_COLOR);
            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            set_text_colors_blue();
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("blue theme");
            }
            break;

        case '5':
            TEXT_COLOR = COLOR_WHITE;
            BACK_COLOR = COLOR_BLACK;
            textcolor(TEXT_COLOR);
            bgcolor(BACK_COLOR);
            bordercolor(BACK_COLOR);
            set_text_colors_all();
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("rainbow theme");
            }
            break;

        case 'R':
        case 'r':
            reverse_mode = reverse_mode == 0 ? 1 : 0;
            if (is_running) {
                revers(reverse_mode);
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("reverse mode: %s", reverse_mode == 0 ? "off" : "on");
            }
            break;

        case 'S':
        case 's':
            shifted_mode = shifted_mode == 0 ? 1 : 0;
            if (shifted_mode) symbols_only_mode = 0;
            if (is_running) {
                *(char*)MEMORY_SETUP_REGISTER = shifted_mode == 0 ? 0x15 : 0x17;
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("shifted mode: %s", shifted_mode == 0 ? "off" : "on");
            }
            break;

        case 'P':
        case 'p':
            symbols_only_mode = symbols_only_mode == 0 ? 1 : 0;
            if (symbols_only_mode) shifted_mode = 0;
            if (is_running) {
                *(char*)MEMORY_SETUP_REGISTER = shifted_mode == 0 ? 0x15 : 0x17;
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("symbols only mode: %s", symbols_only_mode == 0 ? "off" : "on");
            }
            break;

        case '-':
            if (MAX_LENGTH > 1) MAX_LENGTH--;
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("max tail length: %2d", MAX_LENGTH);
            }
            break;

        case '+':
            if (MAX_LENGTH < 20) MAX_LENGTH++;
            if (is_running) {
                clrscr();
                init_all_columns();
                gotoxy(0, 24);
                printf("max tail length: %2d", MAX_LENGTH);
            }
            break;

        case 'X':
        case 'x':
            registers->pc = SOFT_RESET_VECTOR;
            _sys(registers);
            // asm("jsr $FCE2");

        case 13: // RETURN
            if (is_running) {
                clrscr();
                is_running = 0;
            } else {
                clrscr();
                start_matrix();
            }
            break;

        default:
            if (is_running) clrscr();
            is_running = 0;
            break;
    }
}

void start_matrix(void) {
    revers(reverse_mode);

    // set character set
    *(char*)MEMORY_SETUP_REGISTER = shifted_mode == 0 ? 0x15 : 0x17;

    // // Print all chars for debugging
    // gotoxy(0, 0);
    // // All printable chars
    // for (column_index = 0x00; column_index <= 0xFE; column_index++) {
    //     cputc(column_index);
    // }
    // cputc(0x0D);
    // cputc(0x0D);
    // // Only graphical chars (must be unshifted)
    // for (column_index = 0x5E; column_index <= 0xBF; column_index++) {
    //     cputc(column_index);
    // }
    // cgetc();

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

    // set colors
    textcolor(TEXT_COLOR);
    bgcolor(BACK_COLOR);
    bordercolor(BACK_COLOR);

    // set character set to lower case
    *(char*)MEMORY_SETUP_REGISTER = 0x17;

    // draw menu
    revers(1);
    cputsxy(0, 0, "c64matrix - cmatrix for the Commodore 64");
    revers(0);
    printf("\x92");
    cputsxy(0, 2, "SETTINGS");
    chlinexy(0, 3, 40);

    cputsxy(8, 5, "1: Change Background Color");
    cputsxy(8, 6, "2: Change Text Color");
    cputsxy(8, 7, "3: Green Theme");
    cputsxy(8, 8, "4: Blue Theme");
    cputsxy(8, 9, "5: Rainbow Theme");

    cputsxy(8, 10, "R: Reverse Mode");
    gotoxy(32, 10);
    printf("(%s) ", reverse_mode ? "ON" : "OFF");

    cputsxy(8, 11, "S: Shifted Mode");
    gotoxy(32, 11);
    printf("(%s) ", shifted_mode ? "ON" : "OFF");

    cputsxy(8, 12, "P: Symbols Only Mode");
    gotoxy(32, 12);
    printf("(%s) ", symbols_only_mode ? "ON" : "OFF");
    
    cputsxy(6, 13, "+/-: Max Tail Length");
    gotoxy(32, 13);
    printf("(%d) ", MAX_LENGTH);

    cputsxy(3, 16, "Return: Enter The Matrix");
    cputsxy(6, 18, "Any: This Menu");

    cputsxy(8, 21, "X: Soft Reset");

    revers(1);
    cputsxy(0, 24, "C0d3d by HAG'S LAB - http://hag.yt      ");
    revers(0);

    // get menu option and handle it
    option = cgetc();
    handle_choice(option);

    show_menu();
}

int main(void) {
    // randomize rand() function
    _randomize(); 

    // clear screen and set colors
    clrscr();
    textcolor(TEXT_COLOR);
    bgcolor(BACK_COLOR);
    bordercolor(BACK_COLOR);
    set_text_colors_green();

    //gotoxy(0, 24);
    //printf("press return for menu");

loop:
    start_matrix();
    clrscr();
    show_menu();
    goto loop;

    return 0;
}
