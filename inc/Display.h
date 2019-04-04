/*
 * Display.h
 *
 *  Created on: Mar 31, 2019
 *      Author: isaiah
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "gameBoard.h"
#include "TFT_22_ILI9225.h"

// Red 5 Green 6 Blue 5
#define E_BLOCK_COLOR COLOR_BLACK //  R    G    B
#define I_BLOCK_COLOR 0x363d      // 049, 199, 239
#define J_BLOCK_COLOR 0x5b35      // 090, 101, 173
#define L_BLOCK_COLOR 0xebc3      // 239, 121, 033
#define O_BLOCK_COLOR 0xf681      // 247, 211, 008
#define S_BLOCK_COLOR 0x45a8      // 066, 182, 066
#define T_BLOCK_COLOR 0xaa73      // 173, 077, 156
#define Z_BLOCK_COLOR 0xe904      // 239, 032, 041

class Display {
public:
    Display();
    ~Display();

    void drawGameBoard(gameBoard_t new_board);
    void drawScore(uint8_t score);
    void drawNextBlock(block_t next_block);

private:
    void drawBackground();

    TFT_22_ILI9225 screen;
    gameBoard_t old_board;
};

#endif /* DISPLAY_H_ */
