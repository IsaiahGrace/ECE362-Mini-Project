/**
  ******************************************************************************
  * @file    main.c
  * @author  Isaiah Grace
  * @version V1.0
  * @date    04-April-2019
  * @brief   Simple Demo of TFT_22_ILI9225 Class.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "TFT_22_ILI9225.h"
#include "Display.h"

void nano_wait(unsigned int ns) {
    // Taken from Purdue ECE362 course materials
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(ns) : "r0", "cc");
}

void delay(unsigned int ms) {
    nano_wait(ms * 1000000);
}

// This function inserts a block of type block into the row represented by the row
uint32_t insertBlock(uint32_t row, int col, block_t block) {
    // Clear the bits of the block we want to change
    uint32_t bitmask = ~(0x7 << (col * 3));
    row &= bitmask;
    // now set the bits according to the block type and return the result
    bitmask = (block << (col * 3));
    row |= bitmask;
    return row;
}

int main(void) {
    // We're wiring the board like this:
    // STM    Meaning      DISP
    // ----------------------------
    // PA3 -> nRESET    -> RST
    // PA4 -> NSS (MAN) -> CS
    // PA5 -> SPI1_SCK  -> CLK
    // PA6 -> DATA/CMD  -> RS
    // PA7 -> SPI1_MOSI -> SDI
    // +5v -> BackLight -> LED
    // +5v -> Power     -> Vcc
    // GND -> Ground    -> GND

    Display tetris = Display();

    // Clear the board
    gameBoard_t board;
    for (int i = 0; i < 22; i++) {
        board[i] = E_BLOCK;
    }

    // Render the GameBoard, It's empty right now, so nothing should change
    tetris.drawGameBoard(board);

    // This will insert one square with the color of Z blocks in the first row and 3rd column of the board
    board[0] = insertBlock(board[0], 2, Z_BLOCK);

    // Display the changes we just made
    tetris.drawGameBoard(board);

    // Wait one second (1000 ms)
    delay(1000);

    // Remove the Z block we just inserted
    board[0] = insertBlock(board[0], 2, E_BLOCK);

    // Draw the example from the google drive
    // Insert the Z block
    board[5] = insertBlock(board[5], 4, Z_BLOCK);
    board[5] = insertBlock(board[5], 5, Z_BLOCK);
    board[6] = insertBlock(board[6], 5, Z_BLOCK);
    board[6] = insertBlock(board[6], 6, Z_BLOCK);

    // Insert the L block
    board[19] = insertBlock(board[19], 0, L_BLOCK);
    board[20] = insertBlock(board[19], 0, L_BLOCK);
    board[21] = insertBlock(board[21], 0, L_BLOCK);
    board[21] = insertBlock(board[21], 1, L_BLOCK);

    // Insert the O block
    board[19] = insertBlock(board[19], 1, O_BLOCK);
    board[19] = insertBlock(board[19], 2, O_BLOCK);
    board[20] = insertBlock(board[20], 1, O_BLOCK);
    board[20] = insertBlock(board[20], 2, O_BLOCK);

    // Insert the S block
    board[20] = insertBlock(board[20], 3, S_BLOCK);
    board[20] = insertBlock(board[20], 4, S_BLOCK);
    board[21] = insertBlock(board[21], 2, S_BLOCK);
    board[21] = insertBlock(board[21], 3, S_BLOCK);

    // Insert the T block
    board[20] = insertBlock(board[20], 5, T_BLOCK);
    board[21] = insertBlock(board[21], 4, T_BLOCK);
    board[21] = insertBlock(board[21], 5, T_BLOCK);
    board[21] = insertBlock(board[21], 6, T_BLOCK);

    // Insert the Z block
    board[20] = insertBlock(board[20], 7, Z_BLOCK);
    board[20] = insertBlock(board[20], 8, Z_BLOCK);
    board[21] = insertBlock(board[21], 8, Z_BLOCK);
    board[21] = insertBlock(board[21], 9, Z_BLOCK);

    // Insert the J block
    board[19] = insertBlock(board[19], 7, J_BLOCK);
    board[19] = insertBlock(board[19], 8, J_BLOCK);
    board[19] = insertBlock(board[19], 9, J_BLOCK);
    board[20] = insertBlock(board[20], 9, J_BLOCK);

    // Insert the I block
    board[15] = insertBlock(board[15], 9, I_BLOCK);
    board[16] = insertBlock(board[16], 9, I_BLOCK);
    board[17] = insertBlock(board[17], 9, I_BLOCK);
    board[18] = insertBlock(board[18], 9, I_BLOCK);

    // Update the display
    tetris.drawGameBoard(board);

    delay(1000);

    // Move the Z block down one step
    // First, remove the block,
    board[5] = insertBlock(board[5], 4, E_BLOCK);
    board[5] = insertBlock(board[5], 5, E_BLOCK);
    board[6] = insertBlock(board[6], 5, E_BLOCK);
    board[6] = insertBlock(board[6], 6, E_BLOCK);

    // Now put the block back in one row lower
    board[6] = insertBlock(board[6], 4, Z_BLOCK);
    board[6] = insertBlock(board[6], 5, Z_BLOCK);
    board[7] = insertBlock(board[7], 5, Z_BLOCK);
    board[7] = insertBlock(board[7], 6, Z_BLOCK);

    // Update the display
    tetris.drawGameBoard(board);

    // now have it go down some more
    for (int i = 6; i < 18; i++) {
        delay(750);

        // Move the Z block down one step
        // First, remove the block,
        board[i] = insertBlock(board[i], 4, E_BLOCK);
        board[i] = insertBlock(board[i], 5, E_BLOCK);
        board[i + 1] = insertBlock(board[i + 1], 5, E_BLOCK);
        board[i + 1] = insertBlock(board[i + 1], 6, E_BLOCK);

        // Now put the block back in one row lower
        board[i + 1] = insertBlock(board[i + 1], 4, Z_BLOCK);
        board[i + 1] = insertBlock(board[i + 1], 5, Z_BLOCK);
        board[i + 2] = insertBlock(board[i + 2], 5, Z_BLOCK);
        board[i + 2] = insertBlock(board[i + 2], 6, Z_BLOCK);

        // Update the display
        tetris.drawGameBoard(board);
    }

    // Cycle through block types for NEXT BLOCK
    for (uint8_t i = 1; i < 8; i++) {
        tetris.drawNextBlock((block_t) i);
        delay(1000);
    }

    // Count up on the score
    for (int i = 0; i < 256; i++) {
        tetris.drawScore(i);
        delay(10);
    }

    // Count back down
    for (int i = 255; i >= 0; i--) {
        tetris.drawScore(i);
        delay(10);
    }

	for(;;);
}
