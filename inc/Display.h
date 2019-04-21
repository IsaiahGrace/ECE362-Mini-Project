/*
 * Display.h
 *
 *  Created on: Mar 31, 2019
 *      Author: isaiah
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "gameBoard.h"
#include "EEPROM.h"
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

typedef enum STATE {
        MAIN_MENU,
        HIGH_SCORE,
        TETRIS,
        NAME_ENTRY,
        PERSONAL_HIGH_SCORE
} STATE;

class Display {
public:
    Display();
    ~Display();

    // Screen Related Functions:
    // Tetris Functions that Anushka Calls
    // Each function is guarded by a state check and will only do anything if the state is TETRIS
    void drawGameBoard(gameBoard_t new_board);
    void drawScore(uint8_t score);
    void drawNextBlock(block_t next_block);
    void endGame(uint8_t score);

    // This class will call the following tetris functions:
    // void TETRIS_Start();  // Starts the tetris game
    // void TETRIS_Left();   // Response to press of left button
    // void TETRIS_Right();  // Response to press of right button
    // void TETRIS_Down();   // Response to press of down button
    // void TETRIS_Rotate(); // Response to press of rotate button

    // Generic button press responses
    // These are the functions that Meghan Calls when a button is pressed (falling edge)
    void upButtonPress();
    void downButtonPress();
    void leftButtonPress();
    void rightButtonPress();
    void rotateButtonPress();

private:
    TFT_22_ILI9225 screen;
    EEPROM eeprom;
    STATE state;
    int menu_selection;
    gameBoard_t old_board;

    // This is a pretty cool rainbow color scheme (red-orange theme)
    uint16_t colors[22] = {
            0xf6f1, 0xf6b1, 0xee70, 0xee30, 0xee0f, 0xedcf, 0xe58e, 0xe54e,
            0xe50d, 0xdccd, 0xdc8c, 0xdc4c, 0xdc0b, 0xd3eb, 0xd3aa, 0xd36a,
            0xcb2a, 0xcae9, 0xcaa9, 0xca68, 0xc228, 0xc1e7
    };

    // Tetris Screen
    void drawTetrisScreen();

    // Main Menu
    void drawMainMenu();
    void drawMainMenuMenu();

    // High Score
    void drawHighScore();
    void drawHighScoreText();
    void drawHighScoreMenu();

    // Personal High Score
    void drawPersonalHighScore();

    // Name Entry
    void drawNameEntry();
    void drawNameEntryMenu();
    void drawNameEntryName();

    // Functions that define state transitions
    void stateToMainMenu();
    void stateToHighScore();
    void stateToTetris();
    void stateToNameEntry();
    void stateToPersonalHighScore();

    // Menu Controls
    void incrimentMenuSelection();
    void decrimentMenuSelection();

    // backgrounds
    void drawBackground(int colorTheme);
    uint16_t permuteColor(uint16_t color, int colorPermutation);
};

#endif /* DISPLAY_H_ */
