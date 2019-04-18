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

Display* tetrisPTR;
uint16_t buttonHistory[5];

void nano_wait(unsigned int ns) {
    // Taken from Purdue ECE362 course materials
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(ns) : "r0", "cc");
}

void delay(unsigned int ms) {
    for (; ms > 0; ms -= 1) {
    nano_wait(1000000);
    }
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

void init_TIM2_buttons() {
    // Clear the button histories
    buttonHistory[0] = 0;
    buttonHistory[1] = 0;
    buttonHistory[2] = 0;
    buttonHistory[3] = 0;
    buttonHistory[4] = 0;

    // Enable the Clock to GPIOA
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

    // Set PA0, PA1, PA2, PA3, PA4 to Input
    GPIOB->MODER &= ~(GPIO_MODER_MODER0 |
                      GPIO_MODER_MODER1 |
                      GPIO_MODER_MODER2 |
                      GPIO_MODER_MODER3 |
                      GPIO_MODER_MODER4);

    // Enable APB Bus clock to TIM2
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Counting direction up
    TIM2->CR1 &= ~TIM_CR1_DIR;

    // Set the timer prescaler
    TIM2->PSC = 480 - 1;

    // Set the ARR
    TIM2->ARR = 100 - 1;

    // Enable the Timer Update Interrupt
    TIM2->DIER |= TIM_DIER_UIE;

    // Start the timer!
    TIM2->CR1 |= TIM_CR1_CEN;

    // Enable the Interrupt in the NVIC
    NVIC->ISER[0] |= 1 << TIM2_IRQn;
}

// THIS IS ABSOLUTLY NECESSARY FOR ANY INTERRUPT HANDLER IN C++ CODE!!!
extern "C" {

void TIM2_IRQHandler() {
    // Acknowledge the interrupt
    TIM2->SR &= ~TIM_SR_UIF;

    // Sample only the relevant IDR values
    int IDR = GPIOB->IDR & 0x1F;

    // iterate over the IDR inputs
    for (int i = 0; i < 5; i++) {
        // Update the button history samples
        buttonHistory[i] <<= 1;
        buttonHistory[i] |= 0x1 & (IDR >> i);

        // Check if the most recent four samples are 1, AND the oldest four samples are 0
        if (((buttonHistory[i] & 0x000F) == 0x000F) && ((buttonHistory[i] & 0xF000) == 0)) {
            // Give us some hysteresis
            buttonHistory[i] = 0xFFFF;

            // Call the appropriate function
            switch (i) {
            case 0:
                tetrisPTR->upButtonPress();
                break;
            case 1:
                tetrisPTR->leftButtonPress();
                break;
            case 2:
                tetrisPTR->downButtonPress();
                break;
            case 3:
                tetrisPTR->rightButtonPress();
                break;
            case 4:
                tetrisPTR->rotateButtonPress();
                break;
            }
        }
    }
}
}

int main(void) {
    // Clear the board
    gameBoard_t board;
    for (int i = 0; i < 22; i++) {
        board[i] = E_BLOCK;
    }

    // Draw the example from the google drive
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


    // Start the test!
    Display tetris = Display();
    tetrisPTR = &tetris;

    // Start the button code
    init_TIM2_buttons();

    // Do some testing using only button presses and Public API calls
    int test_speed = 1000;

    for(;;);

    while(1) {
        delay(test_speed);
        tetris.upButtonPress();
        delay(test_speed);
        tetris.rotateButtonPress(); // This takes us to the HIGH_SCORE state
        delay(test_speed);
        tetris.upButtonPress(); // Does nothing
        delay(test_speed);
        tetris.rotateButtonPress(); // This takes us back to the main menu
        delay(test_speed);
        tetris.downButtonPress();
        delay(test_speed);
        tetris.upButtonPress();
        delay(test_speed);
        tetris.rotateButtonPress(); // This takes us to the TETRIS state
        tetris.drawGameBoard(board);
        delay(test_speed);
        tetris.drawScore(56);
        tetris.drawNextBlock(Z_BLOCK);
        tetris.drawGameBoard(board);
        delay(test_speed);
        delay(test_speed);
        tetris.endGame(123); // End Game event takes us to NAME_ENTRY state
        delay(test_speed);
        tetris.upButtonPress(); // Change the first character to 0
        delay(test_speed);
        tetris.upButtonPress(); // Change the first character to 1
        delay(test_speed);
        tetris.rightButtonPress(); // Select the second character
        delay(test_speed);
        tetris.downButtonPress(); // Change the second character to B
        delay(test_speed);
        tetris.downButtonPress(); // Change the second character to B
        delay(test_speed);
        tetris.rightButtonPress(); // Select the third character
        delay(test_speed);
        tetris.rightButtonPress(); // Select the "Confirm" option
        delay(test_speed);
        tetris.rotateButtonPress(); // This takes us to the PERSONAL_HIGH_SCORE state
        delay(test_speed);
        tetris.rotateButtonPress(); // This takes us all the way back to MAIN_MENU state
    }

    for(;;);
}
