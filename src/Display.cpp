/*
 * Display.cpp
 *
 *  Created on: Apr 3, 2019
 *      Author: isaiah
 */

#include "Display.h"
#include "gameBoard.h"

Display::Display() : screen() {
    screen.begin();
    screen.setOrientation(2); // This is portrait with the pins on the BOTTOM

    // Clear the oldBaord
    for (int i = 0; i < 22; i++) {
        old_board[i] = 0;
    }

    drawBackground();

    // Draw a boarder around the play area
    screen.drawLine(39, 0, 39, screen.maxY(), COLOR_WHITE);
    screen.drawLine(141, 0, 141, screen.maxY(), COLOR_WHITE);

    // Fill the block area with black
    screen.fillRectangle(40, 0, 140, screen.maxY(), COLOR_BLACK);

    // Prepare the SCORE area
    screen.setFont((uint8_t*) &Terminal6x8);
    char score_text[] = "SCORE\0";
    screen.drawText(3, 5, score_text, COLOR_BLUE);
    drawScore(0);

    // Prepare the NEXT BLOCK area
    char next_text[] =  "NEXT\0";
    char block_text[] = "BLOCK\0";
    screen.drawText(145, 3, next_text, COLOR_BLUE);
    screen.drawText(142, 14, block_text, COLOR_BLUE);
    screen.drawRectangle(143, 29, 175, 71, COLOR_BLUE);
    screen.fillRectangle(144, 30, 174, 70, E_BLOCK_COLOR);
}

void Display::drawGameBoard(gameBoard_t new_board) {
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
    int digit;
    char score_text[] = "999\0";
    int score_index = 0;
    for (int i = 1000; i > 1; i /= 10) {
        digit = score % i;                    // Isolates only the relevant digit
        digit /= i / 10;                           // Scales the digit to the ones place
        score_text[score_index] = digit + 48; // 0 is 48 in ASCII
        score_index++;
    }
    screen.setFont((uint8_t*) &Terminal11x16);
    screen.drawText(2, 20, score_text, COLOR_BLUE);
    screen.setFont((uint8_t*) &Terminal6x8);
}

void Display::drawNextBlock(block_t next_block) {
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

void Display::drawBackground() {
    // This is a pretty cool rainbow color scheme
    uint16_t colors[] = {
            0xf6f1, 0xf6b1, 0xee70, 0xee30, 0xee0f, 0xedcf, 0xe58e, 0xe54e,
            0xe50d, 0xdccd, 0xdc8c, 0xdc4c, 0xdc0b, 0xd3eb, 0xd3aa, 0xd36a,
            0xcb2a, 0xcae9, 0xcaa9, 0xca68, 0xc228, 0xc1e7
    };

    int color_index = 21;
    for (int i = 210; i >= 0; i -= 10) {
        screen.fillRectangle(0, i, screen.maxX(), i + 10, colors[color_index]);
        color_index--;
    }
}

