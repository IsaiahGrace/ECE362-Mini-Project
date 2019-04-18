/*
 * Display.cpp
 *
 *  Created on: Apr 3, 2019
 *      Author: isaiah
 */

#include "Display.h"
#include "gameBoard.h"
#include "stm32f0xx.h" // This is necessary for the I2C EEPROM functions
// TODO: REMOVE THIS. It's only here to give a "random" number for the sore in the testing mode
#include "stdlib.h"


Display::Display() : screen() {
    screen.begin();
    screen.setOrientation(2); // This is portrait with the pins on the TOP

    // Clear the oldBaord
    for (int i = 0; i < 22; i++) {
        old_board[i] = 0;
    }

    menu_selection = 0;
    EEPROM_CacheNumber = 0;
    EEPROM_CacheScore = 0;
    EEPROM_CacheName[0] = 'A';
    EEPROM_CacheName[1] = 'A';
    EEPROM_CacheName[2] = 'A';
    EEPROM_CacheName[3] = '\0';
    I2C_active = false;
    state = MAIN_MENU;
    stateToMainMenu();
}

void Display::delay(unsigned int ms) {
    for (; ms; ms--) {
        nano_wait(1000000);
    }
}

void Display::nano_wait(unsigned int ns) {
    // Taken from Purdue ECE362 course materials
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(ns) : "r0", "cc");
}

void Display::upButtonPress() {
    switch(state) {
    case MAIN_MENU:
        decrimentMenuSelection();
        drawMainMenuSelection();
        break;
    case HIGH_SCORE:
        break;
    case TETRIS:
        break;
    case NAME_ENTRY:
        // We want an incriment_ammount of -1 because when you push the up button, the letter should move from B to A
        if (menu_selection != 3) {
            updateEEPROM_CacheNameLetter(menu_selection,-1);
            drawNameEntryName();
        }
        break;
    case PERSONAL_HIGH_SCORE:
        break;
    }
}

void Display::downButtonPress() {
    switch(state) {
    case MAIN_MENU:
        incrimentMenuSelection();
        drawMainMenuSelection();
        break;
    case HIGH_SCORE:
        break;
    case TETRIS:
        //TODO: Call Anushka's Code Here
        break;
    case NAME_ENTRY:
        // We want an incriment_ammount of +1 because when you push the down button, the letter should move from A to B
        if (menu_selection != 3) {
            updateEEPROM_CacheNameLetter(menu_selection,1);
            drawNameEntryName();
        }
        break;
    case PERSONAL_HIGH_SCORE:
        break;
    }
}

void Display::leftButtonPress() {
    switch(state) {
    case MAIN_MENU:
        break;
    case HIGH_SCORE:
        break;
    case TETRIS:
        //TODO: Call Anushka's Code Here
        break;
    case NAME_ENTRY:
        decrimentMenuSelection();
        drawNameEntryMenu();
        break;
    case PERSONAL_HIGH_SCORE:
        break;
    }
}

void Display::rightButtonPress() {
    switch(state) {
    case MAIN_MENU:
        break;
    case HIGH_SCORE:
        break;
    case TETRIS:
        //TODO: Call Anushka's Code Here
        break;
    case NAME_ENTRY:
        incrimentMenuSelection();
        drawNameEntryMenu();
        break;
    case PERSONAL_HIGH_SCORE:
        break;
    }
}

void Display::rotateButtonPress() {
    switch(state) {
    case MAIN_MENU:
        switch (menu_selection) {
        case 0:
            stateToTetris();
            break;
        case 1:
            stateToHighScore();
            break;
        }
        break;
    case HIGH_SCORE:
        stateToMainMenu();
        break;
    case TETRIS:
        // TODO: Call Anushka's Code Here

        // This is just testing code. Generate a random score and end the tetris game
        endGame((uint8_t) rand() & 0xFF);
        break;
    case NAME_ENTRY:
        if (menu_selection == 3)
            stateToPersonalHighScore();
        break;
    case PERSONAL_HIGH_SCORE:
        stateToMainMenu();
        break;
    }
}

// Functions that define state transitions
void Display::stateToMainMenu() {
    state = MAIN_MENU;
    menu_selection = 0;
    drawMainMenu();
}

void Display::stateToHighScore() {
    state = HIGH_SCORE;
    drawHighScore();
}

void Display::stateToTetris() {
    state = TETRIS;
    // Clear the oldBaord so that we are sure to draw all the new shapes
    for (int i = 0; i < 22; i++) {
        old_board[i] = 0;
    }

    drawTetrisScreen();
    drawScore(0);

    //TODO: Call Anushka's Code here to trigger the start of the game
}

void Display::stateToNameEntry() {
    state = NAME_ENTRY;
    menu_selection = 0;

    // Reset the CacheName so the player is presented with AAA instead of whatever was last used
    EEPROM_CacheName[0] = 'A';
    EEPROM_CacheName[1] = 'A';
    EEPROM_CacheName[2] = 'A';
    EEPROM_CacheName[3] = '\0';

    drawNameEntry();
}

void Display::stateToPersonalHighScore() {
    state = PERSONAL_HIGH_SCORE;

    uint32_t packet_data = EEPROM_CreateDataPacket();
    EEPROM_InsertDataPacket(packet_data);

    drawPersonalHighScore();
}

void Display::endGame(uint8_t score) {
    if (state != TETRIS) return;
    EEPROM_CacheScore = score;
    stateToNameEntry();
}

void Display::drawTetrisScreen(){
    // Draw A red-orange background
     drawBackground(0);

     // Draw a boarder around the play area
     screen.drawLine(39, 0, 39, screen.maxY(), COLOR_WHITE);
     screen.drawLine(141, 0, 141, screen.maxY(), COLOR_WHITE);

     // Fill the block area with black
     screen.fillRectangle(40, 0, 140, screen.maxY(), COLOR_BLACK);

     // Prepare the SCORE area
     screen.setFont((uint8_t*) &Terminal6x8);
     char score_text[] = "SCORE\0";
     screen.drawText(3, 5, score_text, COLOR_BLUE);

     // Prepare the NEXT BLOCK area
     char next_text[] =  "NEXT\0";
     char block_text[] = "BLOCK\0";
     screen.setFont((uint8_t*) &Terminal6x8);
     screen.drawText(145, 3, next_text, COLOR_BLUE);
     screen.drawText(142, 14, block_text, COLOR_BLUE);
     screen.drawRectangle(143, 29, 175, 71, COLOR_BLUE);
     screen.fillRectangle(144, 30, 174, 70, E_BLOCK_COLOR);
}

void Display::drawMainMenu(){
    // Draw a nice green background
    drawBackground(1);

    // Write out a title
    screen.setFont((uint8_t*) &Terminal11x16);
    char title[] = "ECE362 Tetris!\0";
    int text_width = screen.getTextWidth(title);
    screen.drawText((screen.maxX() - text_width) / 2, 5, title, COLOR_GOLD);

    drawMainMenuSelection();

    // Print The names of the team members at the bottom of the screen
    screen.setFont((uint8_t*) &Terminal11x16);
    char credits[4][25] = {"Isaiah Grace\0", "Ashley Maddock\0", "Meghan Jarriel\0", "Anushka Madwesh\0"};
    for (unsigned int i = 0; i < (sizeof credits / sizeof credits[0]); i++) {
        text_width = screen.getTextWidth(credits[i]);
        screen.drawText((screen.maxX() - text_width) / 2, 130 + i * 20, credits[i], COLOR_BLUE);
    }


}

void Display::drawMainMenuSelection() {
    int text_width;
    uint16_t play_color, scores_color;

    switch (menu_selection) {
    case 0:
        play_color = COLOR_GREEN;
        scores_color = COLOR_RED;
        break;
    case 1:
        play_color = COLOR_RED;
        scores_color = COLOR_GREEN;
        break;
    default:
        play_color = COLOR_BLUE;
        scores_color = COLOR_BLUE;
    }

    // Print the "Play Tetris" Option
    screen.setFont((uint8_t*) &Terminal11x16);
    char play[] = "Play Tetris\0";
    text_width = screen.getTextWidth(play);
    screen.drawText((screen.maxX() - text_width) / 2, 45, play, play_color);

    // Print out the "View High Scores" option
    screen.setFont((uint8_t*) &Terminal11x16);
    char highScore[] = "View High Scores\0";
    text_width = screen.getTextWidth(highScore);
    screen.drawText((screen.maxX() - text_width) / 2, 75, highScore, scores_color);
}

void Display::drawHighScore(){
    // Draw a nice green background
    drawBackground(2);

    // Print out Title
    int text_width;
    screen.setFont((uint8_t*) &Terminal11x16);
    char title[] = "HIGH SCORES\0";
    text_width = screen.getTextWidth(title);
    screen.drawText((screen.maxX() - text_width) / 2, 5, title, COLOR_RED);

    // Print out a Table of names and scores
    char name[4];
    uint8_t score;
    uint32_t packaged_name;
    for (unsigned int i = 0; i < 10; i++) {
        packaged_name = EEPROM_GetName(i);
        name[0] = (char) (packaged_name >> 24);
        name[1] = (char) (packaged_name >> 16);
        name[2] = (char) (packaged_name >> 8);
        name[3] = (char)  packaged_name;
        score = EEPROM_GetScore(i);
        screen.drawText(40, 30 + i * 18, name, COLOR_GOLD);
        screen.drawNumber(100, 30 + i * 18, score, COLOR_GOLD);
    }

    // Draw the back button, this is the only menu button available
    screen.setFont((uint8_t*) &Terminal6x8);
    char back[] = "<- Back\0";
    text_width = screen.getTextWidth(back);
    screen.drawText(screen.maxX() - text_width, screen.maxY() - 8, back, COLOR_GREEN);

}

void Display::drawPersonalHighScore(){
    int text_width;
    uint8_t personal_score = EEPROM_CacheScore;

    // Draw a nice background
    drawBackground(3);

    // Print out Title
    screen.setFont((uint8_t*) &Terminal11x16);
    char title[] = "HIGH SCORES\0";
    text_width = screen.getTextWidth(title);
    screen.drawText((screen.maxX() - text_width) / 2, 5, title, COLOR_RED);

    char subtitle1[] = "You\0";
    char subtitle2[] = "placed:\0";
    screen.drawText(100, 30, subtitle1, COLOR_RED);
    screen.drawText(100, 48, subtitle2, COLOR_RED);
    screen.drawNumber(100, 66, (uint8_t) EEPROM_GetRank(personal_score), COLOR_GREEN);

    char subtitle3[] = "Out of:\0";
    char subtitle4[] = "players\0";
    screen.drawText(100, 93, subtitle3, COLOR_RED);
    screen.drawNumber(100, 111, (uint8_t) EEPROM_GetNumberOfScores(), COLOR_GREEN);
    screen.drawText(100, 129, subtitle4, COLOR_RED);

    char subtitle5[] = "Your\0";
    char subtitle6[] = "Score:\0";
    screen.drawText(100, 156, subtitle5, COLOR_RED);
    screen.drawText(100, 174, subtitle6, COLOR_RED);
    screen.drawNumber(100, 192, (uint8_t) personal_score, COLOR_GREEN);

    // Print out a Table of names and scores
    char name[4];
    uint8_t score;
    uint32_t packaged_name;
    for (unsigned int i = 0; i < 10; i++) {
        packaged_name = EEPROM_GetName(i);
        name[0] = (char) (packaged_name >> 24);
        name[1] = (char) (packaged_name >> 16);
        name[2] = (char) (packaged_name >> 8);
        name[3] = (char)  packaged_name;
        score = EEPROM_GetScore(i);
        screen.drawText(10, 30 + i * 18, name, COLOR_GOLD);
        screen.drawNumber(50, 30 + i * 18, score, COLOR_GOLD);
    }

    // Draw the back button, this is the only menu button available
    screen.setFont((uint8_t*) &Terminal6x8);
    char back[] = "<- Done\0";
    text_width = screen.getTextWidth(back);
    screen.drawText(screen.maxX() - text_width, screen.maxY() - 8, back, COLOR_GREEN);
}

void Display::drawNameEntry(){
    // Display a background
    drawBackground(4);

    // Draw Title
    int text_width;
    screen.setFont((uint8_t*) &Terminal11x16);
    char title[] = "Enter your tag:\0";
    text_width = screen.getTextWidth(title);
    screen.drawText((screen.maxX() - text_width) / 2, 5, title, COLOR_RED);

    // Draw the Menu Selection
    drawNameEntryMenu();

    // Draw the player's score
    screen.drawNumber(100, 80, EEPROM_CacheScore, COLOR_GOLD);

    // Draw the initial name, to be updated by every button press
    drawNameEntryName();
}

void Display::drawNameEntryMenu() {
    uint16_t letter1_color, letter2_color, letter3_color, done_color;
    int text_width;

    switch (menu_selection) {
    case 0:
        letter1_color = COLOR_GREEN;
        letter2_color = COLOR_RED;
        letter3_color = COLOR_RED;
        done_color = COLOR_RED;
        break;
    case 1:
        letter1_color = COLOR_RED;
        letter2_color = COLOR_GREEN;
        letter3_color = COLOR_RED;
        done_color = COLOR_RED;
        break;
    case 2:
        letter1_color = COLOR_RED;
        letter2_color = COLOR_RED;
        letter3_color = COLOR_GREEN;
        done_color = COLOR_RED;
        break;
    case 3:
        letter1_color = COLOR_RED;
        letter2_color = COLOR_RED;
        letter3_color = COLOR_RED;
        done_color = COLOR_GREEN;
        break;
    default:
        letter1_color = COLOR_BLUE;
        letter2_color = COLOR_BLUE;
        letter3_color = COLOR_BLUE;
        done_color = COLOR_BLUE;
        break;
    }

    // Draw triangles above and below the first letter
    int letterX = 29;
    int letterY = 80;
    screen.fillTriangle(letterX, letterY - 4, letterX + 12 , letterY - 4, letterX + 6, letterY - 16, letter1_color);
    screen.fillTriangle(letterX, letterY + 4 + 16, letterX + 12 , letterY + 4 + 16, letterX + 6, letterY + 16 + 16, letter1_color);

    // Draw triangles above and below the second letter
    letterX += 20;
    letterY = 80;
    screen.fillTriangle(letterX, letterY - 4, letterX + 12 , letterY - 4, letterX + 6, letterY - 16, letter2_color);
    screen.fillTriangle(letterX, letterY + 4 + 16, letterX + 12 , letterY + 4 + 16, letterX + 6, letterY + 16 + 16, letter2_color);

    // Draw triangles above and below the third letter
    letterX += 20;
    letterY = 80;
    screen.fillTriangle(letterX, letterY - 4, letterX + 12 , letterY - 4, letterX + 6, letterY - 16, letter3_color);
    screen.fillTriangle(letterX, letterY + 4 + 16, letterX + 12 , letterY + 4 + 16, letterX + 6, letterY + 16 + 16, letter3_color);

    // Draw the confirm option
    screen.setFont((uint8_t*) &Terminal11x16);
    char done_text[] = "Confirm\0";
    text_width = screen.getTextWidth(done_text);
    screen.drawText((screen.maxX() - text_width) / 2, 150, done_text, done_color);
}

void Display::drawNameEntryName() {
    // Draw the name, to be updated by every button press
    char letter1[] = {EEPROM_CacheName[0], '\0'};
    char letter2[] = {EEPROM_CacheName[1], '\0'};
    char letter3[] = {EEPROM_CacheName[2], '\0'};

    screen.setFont((uint8_t*) &Terminal12x16);
    screen.drawText(30, 80, letter1, COLOR_GOLD);
    screen.drawText(50, 80, letter2, COLOR_GOLD);
    screen.drawText(70, 80, letter3, COLOR_GOLD);
}

void Display::incrimentMenuSelection() {
    switch(state) {
    case MAIN_MENU:
        menu_selection = menu_selection >= 1 ? 0 : menu_selection + 1;
        break;
    case HIGH_SCORE:
        // Note: no menu choices on this screen
        break;
    case TETRIS:
        // Note: no menu on this screen
        break;
    case NAME_ENTRY:
        menu_selection = menu_selection >= 3 ? 0 : menu_selection + 1;
        break;
    case PERSONAL_HIGH_SCORE:
        // Note: no menu choices on this screen
        break;
    }
}

void Display::decrimentMenuSelection() {
    switch(state) {
    case MAIN_MENU:
        menu_selection = menu_selection <= 0 ? 1 : menu_selection - 1;
        break;
    case HIGH_SCORE:
        // Note: no menu choices on this screen
        break;
    case TETRIS:
        // Note: no menu on this screen
        break;
    case NAME_ENTRY:
        menu_selection = menu_selection <= 0 ? 3 : menu_selection - 1;
        break;
    case PERSONAL_HIGH_SCORE:
        // Note: no menu choices on this screen
        break;
    }
}

void Display::drawGameBoard(gameBoard_t new_board) {
    if (state != TETRIS) return;

    // We're going to be a little clever with how we do this.
    // We're only going to send what has changed since the last time we rendered the board
    uint32_t bitmask;
    uint32_t new_row;
    uint16_t x0, x1, y0, y1;
    uint16_t new_color;
    block_t new_block;

    for (int row = 21; row >= 0; row--) {
        new_row = new_board[row];
        // If this row of the board hasn't changed, then don't render it!
        if (old_board[row] == new_row) continue;

        // Now we want to go through every column on this row
        for(int col = 0; col < 10; col++) {
            bitmask = 0x7 << (col * 3);
            if ((old_board[row] & bitmask) == (new_row & bitmask)) continue;

            // At this point we know the square we are looking at has updated
            new_block = (block_t) ((new_row & bitmask) >> (col * 3));
            x0 = 40 + 10 * col;
            x1 = x0 + 10;
            y0 = 10 * row;
            y1 = y0 + 10;
            switch (new_block) {
            case E_BLOCK:
                new_color = E_BLOCK_COLOR;
                break;
            case I_BLOCK:
                new_color = I_BLOCK_COLOR;
                break;
            case J_BLOCK:
                new_color = J_BLOCK_COLOR;
                break;
            case L_BLOCK:
                new_color = L_BLOCK_COLOR;
                break;
            case O_BLOCK:
                new_color = O_BLOCK_COLOR;
                break;
            case S_BLOCK:
                new_color = S_BLOCK_COLOR;
                break;
            case T_BLOCK:
                new_color = T_BLOCK_COLOR;
                break;
            case Z_BLOCK:
                new_color = Z_BLOCK_COLOR;
                break;
            }
            screen.fillRectangle(x0, y0, x1, y1, new_color);
        }
    }
    for (int i = 0; i < 22; i++) {
        old_board[i] = new_board[i];
    }
}

void Display::drawScore(uint8_t score) {
    if (state != TETRIS) return;
    uint8_t* oldFont = (uint8_t*) screen.getFont().font;
    screen.setFont((uint8_t*) &Terminal11x16);
    screen.drawNumber(2, 20, score, COLOR_BLUE);
    screen.setFont(oldFont);
}

void Display::drawNextBlock(block_t next_block) {
    if (state != TETRIS) return;

    // Blocks look like this:
    /// E   I   J   L   O   S   T   Z
    // --- -#- -#- -#- --- --- --- ---
    // --- -#- -#- -#- -## -## -#- ##-
    // --- -#- ##- -## -## ##- ### -##
    // --- -#- --- --- --- --- --- ---
    screen.fillRectangle(144, 30, 174, 70, E_BLOCK_COLOR);
    switch (next_block) {
    case E_BLOCK:
        screen.fillRectangle(144, 30, 174, 70, E_BLOCK_COLOR);
        break;
    case I_BLOCK:
        screen.fillRectangle(154, 30, 164, 70, I_BLOCK_COLOR);
        break;
    case J_BLOCK:
        screen.fillRectangle(144, 50, 164, 60, J_BLOCK_COLOR);
        screen.fillRectangle(154, 30, 164, 50, J_BLOCK_COLOR);
        break;
    case L_BLOCK:
        screen.fillRectangle(154, 50, 174, 60, L_BLOCK_COLOR);
        screen.fillRectangle(154, 30, 164, 50, L_BLOCK_COLOR);
        break;
    case O_BLOCK:
        screen.fillRectangle(154, 40, 174, 60, O_BLOCK_COLOR);
        break;
    case S_BLOCK:
        screen.fillRectangle(144, 50, 164, 60, S_BLOCK_COLOR);
        screen.fillRectangle(154, 40, 174, 50, S_BLOCK_COLOR);
        break;
    case T_BLOCK:
        screen.fillRectangle(144, 50, 174, 60, T_BLOCK_COLOR);
        screen.fillRectangle(154, 40, 164, 50, T_BLOCK_COLOR);
        break;
    case Z_BLOCK:
        screen.fillRectangle(154, 50, 174, 60, Z_BLOCK_COLOR);
        screen.fillRectangle(144, 40, 164, 50, Z_BLOCK_COLOR);
        break;
    }
}

void Display::drawBackground(int colorTheme) {
    for (int i = 210; i >= 0; i -= 10) {
        screen.fillRectangle(0, i, screen.maxX(), i + 10, permuteColor(colors[i / 10], colorTheme));
    }
}

uint16_t Display::permuteColor(uint16_t color, int colorPermutation) {
    uint8_t red, green, blue = 0;
    screen.splitColor(color, red, green, blue);

    // The first three (0,1,2) are interesting, the rest are less pretty
    switch (colorPermutation) {
    case 0:
        color = screen.setColor(red, green, blue); // A nice red-orange gradient
        break;
    case 1:
        color = screen.setColor(blue, red, green); // A nice green-blue gradient
        break;
    case 2:
        color = screen.setColor(green, blue, red); // An okay blue-purple gradient
        break;
    case 3:
        color = screen.setColor(red, blue, green); // A nice purple gradient
        break;
    case 4:
        color = screen.setColor(green, red, blue); // A nice green gradient
        break;
    case 5:
        color = screen.setColor(blue, green, red); // A nice blue gradient
        break;
    }
    return color;
}

// Updates the name in Cache, used in the name entry screen
// @param       letter_index must be in the range 0-2
// @param       incriment_ammount will be added to the ASCII letter variable (usually -1 or 1)
void Display::updateEEPROM_CacheNameLetter(int letter_index, int incriment_ammount) {
    /*
    Abreviated ASCII Table used for tags, only numbers and Capital Letters allowed

    48  0
    49  1
    50  2
    51  3
    52  4
    53  5
    54  6
    55  7
    56  8
    57  9
    ------------------
    58  :
    59  ;
    60  <
    61  =
    62  >
    63  ?
    64  @
    ------------------
    65  A
    66  B
    67  C
    68  D
    69  E
    70  F
    71  G
    72  H
    73  I
    74  J
    75  K
    76  L
    77  M
    78  N
    79  O
    80  P
    81  Q
    82  R
    83  S
    84  T
    85  U
    86  V
    87  W
    88  X
    89  Y
    90  Z
    */

    EEPROM_CacheName[letter_index] += incriment_ammount;

    // Apply the wrap-arround rules to insure that only the correct letters possible
    if (EEPROM_CacheName[letter_index] > 90)
        EEPROM_CacheName[letter_index] = '0';

    if (EEPROM_CacheName[letter_index] < 48)
        EEPROM_CacheName[letter_index] = 'Z';

    if (EEPROM_CacheName[letter_index] > 57 && EEPROM_CacheName[letter_index] < 65) {
        if (incriment_ammount > 0)
            EEPROM_CacheName[letter_index] = 'A';
        else
            EEPROM_CacheName[letter_index] = '9';
    }
}

// High level function returns the name of person ranked number (starting from 0)
uint32_t Display::EEPROM_GetName(uint8_t number) {
    if (number != EEPROM_CacheNumber)
        EEPROM_UpdateCache(number);

    return EEPROM_CacheName[0] << 24 | EEPROM_CacheName[1] << 16 | EEPROM_CacheName[2] << 8 | EEPROM_CacheName[3];
}

// High level function returns the score of person ranked number (starting from 1)
uint8_t Display::EEPROM_GetScore(uint8_t number) {
    if (number != EEPROM_CacheNumber)
        EEPROM_UpdateCache(number);

    return EEPROM_CacheScore;
}

void Display::EEPROM_UpdateCache(uint8_t number) {
    EEPROM_CacheNumber = number;

    // We need to read 4 bytes from address number + 1
    uint32_t read_buf = 0xFFFFFFFF;

    I2C1_waitidle();
    I2C1_start(number + 1, I2C_RD);
    int fail = I2C1_readdata((uint8_t*) &read_buf, 4);
    I2C1_stop();

    delay(5);

    if (fail) {
        for(;;);
    }

    // Just concatenate the read_buffer into an 8-bit score
    EEPROM_CacheScore = (uint8_t) read_buf;

    EEPROM_CacheName[0] = (char) (read_buf >> 8);
    EEPROM_CacheName[0] = (char) (read_buf >> 16);
    EEPROM_CacheName[0] = (char) (read_buf >> 24);
    EEPROM_CacheName[0] = '\0';

    // This is mostly for testing. It ensures that the characters in EEPROM_CacheName are printable
    // TODO: Remove these lines when I know the EEPROM code works
    updateEEPROM_CacheNameLetter(0,0);
    updateEEPROM_CacheNameLetter(1,0);
    updateEEPROM_CacheNameLetter(2,0);

//    uint8_t scores[10] = {45, 22, 20, 15, 0, 0, 0, 0, 0, 0};
//    EEPROM_CacheScore = scores[number];
//
//    char names[10][5] = {"ISA\0", "ASH\0", "MEG\0", "ANU\0", "N05\0", "N06\0", "N07\0", "N08\0", "N09\0", "N10\0"};
//    EEPROM_CacheName[0] = names[number][0];
//    EEPROM_CacheName[1] = names[number][1];
//    EEPROM_CacheName[2] = names[number][2];
//    EEPROM_CacheName[3] = names[number][3];
}

// Returns the total number of scores present in the EEPROM
uint8_t Display::EEPROM_GetNumberOfScores() {
    uint8_t read_buf;

    I2C1_waitidle();
    I2C1_start(0, I2C_RD);
    int fail = I2C1_readdata(&read_buf, 1);
    I2C1_stop();
    delay(5);

    if (fail) {
        for(;;);
    }

    return read_buf;
}

// Returns the rank of the score given lowest is 1, highest is EEPROM_GetNumberOfScores() + 1
int Display::EEPROM_GetRank(uint8_t score) {
    for (int i = EEPROM_GetNumberOfScores() - 1; i >= 0; i--) {
        if (EEPROM_GetScore(i) > score)
            return i + 2;
    }
    return 1;
}

// Generates a data packet including name and score for the EEPROM based on the Cache
uint32_t Display::EEPROM_CreateDataPacket() {
    return EEPROM_CacheName[0] << 24 | EEPROM_CacheName[1] << 16 | EEPROM_CacheName[2] << 8 | EEPROM_CacheScore;
}

// Writes the given data packet in the appropriate location to maintain a sorted data array on the EEPROM
// Implements a simple insertion sort
void Display::EEPROM_InsertDataPacket(uint32_t dataPacket) {
    uint8_t numScores = EEPROM_GetNumberOfScores();
    uint8_t score = (uint8_t) dataPacket;
    int rank = EEPROM_GetRank(score);

    uint32_t readData;
    uint32_t writeData = dataPacket;
    for (int i = rank - 1; i < numScores; i++) {
        I2C1_waitidle();
        I2C1_start(i + 1, I2C_RD);
        I2C1_readdata((uint8_t*) &readData, 4);
        I2C1_stop();

        delay(5);

        I2C1_waitidle();
        I2C1_start(i + 1, I2C_WR);
        I2C1_senddata((uint8_t*) &writeData, 4);
        I2C1_stop();

        writeData = readData;
    }
}

void Display::EEPROM_ClearScores() {
    // Set the number of scores saved to zero.
    // We don't have to overwrite the data already on the device.
    // As we save new scores, we will automatically overwrite what's already there
    write_EEPROM(0,0);
}

void Display::write_EEPROM(uint16_t wr_addr, uint8_t data) {
    if(!I2C_active) init_I2C1();

    uint8_t addr = 0x50;

    // 1) Initialize an array write_buf[3]  where the first element is the higher byte of parameter ‘wr_addr’,
    //    the second element is the lower byte of ‘wr_addr’, and the third element is ‘data’.
    uint8_t write_buf[3];
    write_buf[0] = (uint8_t) wr_addr >> 8;
    write_buf[1] = (uint8_t) wr_addr & 0x00ff;
    write_buf[2] = data;

    // 2) Call I2C1_waitidle().
    I2C1_waitidle();

    // 3) Call I2C1_start() with I2C address of the EEPROM (see datasheet), with WR as the direction.
    I2C1_start(addr, I2C_WR);

    // 4) Call I2C1_senddata(write_buf, 3).
    I2C1_senddata(write_buf, 3);

    // 5) Call I2C1_stop().
    I2C1_stop();

    // 6) Wait for 5ms.
    delay(5);
}

void Display::I2C1_waitidle(void) {
    if(!I2C_active) init_I2C1();

    // Check wait for the bus to be idle.
    while ((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

// See lab document for description
void Display::I2C1_start(uint8_t addr, uint32_t dir) {
    if(!I2C_active) init_I2C1();

    // Clear the SADD bits
    I2C1->CR2 &= ~I2C_CR2_SADD;

    // Set the SADD address
    I2C1->CR2 |= addr << 1;

    // Check the direction bit with RD, if DIR == RD, then set the RD_WRN bit in the CR2 register
    if (dir == I2C_RD) {
        I2C1->CR2 |= I2C_CR2_RD_WRN;
    } else {
        I2C1->CR2 &= ~I2C_CR2_RD_WRN;
    }

    // Set the START Bit in CR2
    I2C1->CR2 |= I2C_CR2_START;
}

// See lab document for description
void Display::I2C1_stop() {
    if(!I2C_active) init_I2C1();

    // Check if the STOPF flag is set. If so, return
    if (I2C1->ISR & I2C_ISR_STOPF) return;

    // Set the STOP bit in CR2
    I2C1->CR2 |= I2C_CR2_STOP;

    // Wait for STOPF flag to be set in ISR
    while (!(I2C1->ISR & I2C_ISR_STOPF));

    // Clear the STOPF flag
    I2C1->ICR |= I2C_ICR_STOPCF;
}

// See lab document for description
int Display::I2C1_senddata(uint8_t* data, uint32_t size) {
    if(!I2C_active) init_I2C1();

    // a. Clear the NBYTES of CR2.
    I2C1->CR2 &= ~I2C_CR2_NBYTES;

    // b.Set the NBYTES bits to the parameter size.
    I2C1->CR2 |= (size & 0xff) << 16;

    // c.Write a ‘for’ loop that iterates ‘size’ number of times.
    uint32_t i;
    int timeout;
    for (i = 0; i < size; i++) {
        // i.Initialize a variable timeout to 0.
        timeout = 0;
        // ii.Wait for I2C_ISR_TXIS to be 1.
        while (!(I2C1->ISR & I2C_ISR_TXIS)) {
            // 1.While waiting for the bit to be set increment timeout.
            timeout++;
            // 2.If timeout exceeds 1 000 000,
            if(timeout > 1000000) {
                // 3.Return FAIL
                return I2C_FAIL;
            }
        }
        // iii.Set TXDR to data[i]; where i is the ith iteration of the ‘for’ loop.
        I2C1->TXDR = data[i];
    }

    // d.Wait until TC flag is set or the NACK flag is set.
    while (!(I2C1->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

    // e.If NACKF flag is set, return FAIL.
    if (I2C1->ISR & I2C_ISR_NACKF) return I2C_FAIL;

    // f.Else return success
    return I2C_SUCCESS;
}

// See lab document for description
int Display::I2C1_readdata(uint8_t* data, uint32_t size) {
    if(!I2C_active) init_I2C1();

    // a. Clear the NBYTES of CR2.
    I2C1->CR2 &= ~I2C_CR2_NBYTES;

    // b.Set the NBYTES bits to the parameter size.
    I2C1->CR2 |= (size & 0xff) << 16;

    // c.Write a ‘for’ loop that iterates ‘size’ number of times.
    uint32_t i;
    int timeout;
    for (i = 0; i < size; i++) {
        // i.Initialize a variable timeout to 0.
        timeout = 0;
        // ii.Wait for I2C_ISR_RXNE to be 1.
        while (!(I2C1->ISR & I2C_ISR_RXNE)) {
            // 1.While waiting for the bit to be set increment timeout.
            timeout++;
            // 2.If timeout exceeds 1 000 000,
            if(timeout > 1000000) {
                // 3.Return FAIL
                return I2C_FAIL;
            }
        }
        // iii.Read RXDR to data[i]; where i is the ith iteration of the ‘for’ loop.
        data[i] = I2C1->RXDR;
    }

    // d.Wait until TC flag is set or the NACK flag is set.
    while (!(I2C1->ISR & (I2C_ISR_TC | I2C_ISR_NACKF)));

    // e.If NACKF flag is set, return FAIL.
    if (I2C1->ISR & I2C_ISR_NACKF) return I2C_FAIL;

    // f.Else return success
    return I2C_SUCCESS;
}

void Display::init_I2C1() {
    // Initialize I2C1
    /*
    1.  Enable clock to GPIOB
    2.  Configure PB6 and PB7 to alternate functions I2C1_SCL and I2C1_SDA
    3.  Enable clock to I2C1
    4.  Set I2C1 to 7 bit mode
    5.  Enable NACK generation for I2C1
    6.  Configure the I2C1 timing register so that PSC is 4, SCLDEL is 3 and SDADEL is 1 and SCLH is 3 and SCLL is 9
    7.  Disable own address1 and own address 2 and set the 7 bit own address to 1
    8.  Enable I2C1
     */

    // Set the active flag, so we only execute this code once
    I2C_active = true;

    // Enable clock to GPIOB
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Configure PB6 and PB7 to AF
    GPIOB->MODER &= ~0x0000f000;
    GPIOB->MODER |=  0x0000a000;

    // Set PB6 and PB7 to AF1
    GPIOB->AFR[0] &= ~0xff000000;
    GPIOB->AFR[0] |=  0x11000000;

    // Enable clock to I2C1
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    // Set I2C1 to 7 bit mode
    I2C1->CR2 &= ~I2C_CR2_ADD10;

    // Enable NACK generation for I2C1
    I2C1->CR2 |= I2C_CR2_NACK;

    // Clear the Timing Register
    I2C1->TIMINGR &= ~0xf0ffffff;

    // Set I2C1 timing register PSC to 4
    I2C1->TIMINGR |= 0x4 << 28;

    // Set I2C1 timing register SCLDEL to 3
    I2C1->TIMINGR |= 0x3 << 20;

    // Set I2C1 timing register SDADEL to 1
    I2C1->TIMINGR |= 0x1 << 16;

    // Set I2C1 timing register SCLH to 3
    I2C1->TIMINGR |= 0x3 << 8;

    // Set I2C1 timing register SCLL to 9
    I2C1->TIMINGR |= 0x9;

    // Disable own address1 and own address2
    I2C1->OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1->OAR2 &= ~I2C_OAR2_OA2EN;

    // set the 7bit own address to 1
    I2C1->OAR1 |= 1 << 1;
    I2C1->OAR2 |= 1 << 1;

    // Enable own address 1
    I2C1->OAR1 |= I2C_OAR1_OA1EN;

    // Enable I2C1
    I2C1->CR1 |= I2C_CR1_PE;
}
