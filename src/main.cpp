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
#include "stdlib.h"

Display* tetrisPTR;
uint16_t buttonHistory[5];

#define T_1 0x0000000b
#define T_2 0x000DF700
#define T_3 0x0E000000

#define L_1 0x00800009
#define L_2 0x004CF310
#define L_3 0x06000020

#define I_1 0x00100005
#define I_2 0x008AFA20
#define I_3 0x05000040

#define J_1 0x00200003
#define J_2 0x0019F640
#define J_3 0x0C000080

#define S_1 0x00000083
#define S_2 0x1009F600
#define S_3 0x4C200000

#define O_1 0x0000000F
#define O_2 0xF000FF00
#define O_3 0x00000000

#define Z_1 0x00000019
#define Z_2 0x200CF300
#define Z_3 0x86400000

#define GAME_SPEED 500
gameBoard_t board;
gameBoard_t endboard;
gameBoard_t mboard;
gameBoard_t dispboard;

int *rowarr;
int *colarr;
int score = 0;

//these variables are global to communicate with button presses
volatile int gameRunning = 0;
volatile int difficulty = 0;
volatile int numRot;
volatile int rotFlag;
volatile int leftButton;
volatile int rightButton;
volatile int downButton;

void TETRIS_Start() {
    // Starts the tetris game
    gameRunning = 1;
    difficulty = 0;
    score = 0;
}

void TETRIS_Left() {
    // Response to press of left button
    leftButton = 1;
}

void TETRIS_Right() {
    // Response to press of right button
    rightButton = 1;
}

void TETRIS_Down() {
    // Response to press of down button
    downButton = 1;
}

void TETRIS_Rotate() {
    // Response to press of rotate button
    numRot++;
    rotFlag = 1;
}

int collides(uint32_t row1, uint32_t row2);
int checkBoard(int row);
int moveDownCheck();
int translateCheck(int moveLeft, int moveRight);
int insertShape(block_t blockType, int row, int col, int rotation);

void init_gameboard(void){
    for (int i = 0; i < 27; i++) {
        board[i] = E_BLOCK;
        endboard[i] = E_BLOCK;
        mboard[i] = E_BLOCK;
        dispboard[i] = E_BLOCK;
    }
}
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

uint32_t blockVal(uint32_t row, int col) {
    // Clear the bits of the block we want to change
    uint32_t bitmask = (row >> (col * 3));
    bitmask &= 0x7; //111
    //if there are values in the given row, it will not return 0
    return bitmask;
}

block_t randblock (){
    //generate random block
    int num = rand() % 7; //range of 0 to 6
    block_t block;
    switch (num) {
    case 0:
        block = I_BLOCK;
        break;
    case 1:
        block = J_BLOCK;
        break;
    case 2:
        block = L_BLOCK;
        break;
    case 3:
        block = O_BLOCK;
        break;
    case 4:
        block = S_BLOCK;
        break;
    case 5:
        block = T_BLOCK;
        break;
    case 6:
        block = Z_BLOCK;
        break;
    }
    return block;

}

int leftRight(int col, int leftButton, int rightButton){
    //given the left/right button input, function moves the current
    if (leftButton){
        if (col > 0){
            col--;
        }
    }
    else if (rightButton){
        if (col < 9){
            col++;
        }
    }
    return col;

}

int endGame(){
    int end = 0;
    end |= board[0] & 0xFFFFFFFF;
    if (end){
        return 1;
    }
    return 0;
}

void getScore(gameBoard_t board){
    int count;

     for (int i = 1; i < 27; i++) {
         //stepping through each row
         count = 0;
                for (int j = 0; j <= 9; j++) {
                    //stepping through each column
                    if (blockVal(board[i], j)!= 0){
                        count = count +1 ; //counting each space that has block
                    }
                }
                if (count ==10){
                    //at the end if all 10 were full, remove the row and increment score
                    for(int k =i; k>0; k--){
                        //stepping backwards through rows
                        board[k] = board[k-1];
                    }
                    score++;
                }
     }
}

int translateCheck(int moveLeft, int moveRight) {
    if (moveLeft && moveRight) return 0; // You can't go left and right!

    uint32_t shiftedRow;
    for(unsigned int i = 0; i < sizeof board / sizeof board[0]; i++) {
        // If this row of the mboard is empty, we don't need to look at it anymore
        if (!mboard[i]) continue;

        // At this point the mboard is not empty.
        // 1. Check if the block is next to either edge of the board
        if((mboard[i] & 0x38000000) && moveRight) return 0;
        if((mboard[i] & 0x7) && moveLeft) return 0;

        // 2. Apply the translation

        if(moveLeft) shiftedRow = mboard[i] >> 3;
        if(moveRight) shiftedRow = mboard[i] << 3;

        // 3. Check if the result collides with any other blocks
        if (collides(board[i],shiftedRow)) return 0;
    }
    return 1;
}

int collides(uint32_t row1, uint32_t row2) {
    for(unsigned int mask = 0x7; mask ^ 0xc0000000; mask <<= 3) {
        if((row1 & mask) && (row2 & mask)) return 1;
    }
    // Return FALSE, the two rows do NOT collide
    return 0;
}

int moveDownCheck() {
    for(unsigned int i = 0; i < sizeof board / sizeof board[0]; i++) {
        if (checkBoard(i)) return 0;
    }
    // return TRUE, you CAN move the board down one row
    return 1;
}

int cmpChunk(uint32_t currChunk, uint32_t nextChunk){
    uint32_t key = 0x7; //111
    int ovrlpFlag; // there is overlap
    if ((currChunk ^ key) == key){
        //if 111 then must have been 000 so ovr not possible

        ovrlpFlag=0;//no overlap
    }
    else if ((nextChunk ^ key) != key){
        //if ~= to 111 then must have contained value so will overlap

        ovrlpFlag = 1;
    }

    return ovrlpFlag;
}
int checkBoard(int row){
    int ovrFlag = 0;
    int cCol; //current col - go through all columns
    int nRow = row + 1; //destination row

    //int useless = 0;
    uint32_t boardCnk;
    uint32_t mboardCnk;
    for(cCol = 0; cCol < 10; cCol++){
        boardCnk = blockVal(board[nRow], cCol);
        mboardCnk = blockVal(mboard[row], cCol);
        if(mboardCnk && (nRow > 21)){
        //exceeds or is last row no need to check
        //is >= correct??
            ovrFlag = 1;
            return ovrFlag;
        }
        if ((mboardCnk && boardCnk)) {
            //useless++;
            ovrFlag = 1;
            break;
        }
    }
    return ovrFlag;
}

void moveBoardDown(){
    for (int i = sizeof mboard / sizeof mboard[0]; i > 0; i--){
        mboard[i] = mboard[i-1];
    }
    mboard[0] = E_BLOCK;
}

int insertShape(block_t blockType, int row, int col, int rotation) {
    // Rotation must be a number between 0 and 3
    // function returns a 0 is placement is impossible
    // (becasue of a conflict with a block on the static board or the edge)
    // function returns a 1 is the block was sucsessfully placed in the mboard
    uint32_t blockMask[3];
    switch(blockType) {
    case I_BLOCK:
        blockMask[0] = I_1;
        blockMask[1] = I_2;
        blockMask[2] = I_3;
        break;
    case J_BLOCK:
        blockMask[0] = J_1;
        blockMask[1] = J_2;
        blockMask[2] = J_3;
        break;
  case L_BLOCK:
      blockMask[0] = L_1;
      blockMask[1] = L_2;
      blockMask[2] = L_3;
      break;
  case O_BLOCK:
      blockMask[0] = O_1;
      blockMask[1] = O_2;
      blockMask[2] = O_3;
      break;
  case S_BLOCK:
      blockMask[0] = S_1;
      blockMask[1] = S_2;
      blockMask[2] = S_3;
      break;
  case T_BLOCK:
      blockMask[0] = T_1;
      blockMask[1] = T_2;
      blockMask[2] = T_3;
      break;
  case Z_BLOCK:
      blockMask[0] = Z_1;
      blockMask[1] = Z_2;
      blockMask[2] = Z_3;
      break;
  default:
      return 0;
      break;
  }

    uint32_t mask = 0x11111111; // This mask selects all the bits from the 0th rotation

    mask <<= rotation; // This shifts the mask to select the correct rotation

    // This will filter out only the bits relevant for this rotation
    blockMask[0] &= mask;
    blockMask[1] &= mask;
    blockMask[2] &= mask;

    // These series of for loops will iterate over the 5x5 grid, centered on the 12th square.
    block_t insert_block;
    int blockNum;
    int insert_row;
    int insert_col;
    // These two for loops will iterate over the blocks defined by blockMask[0]
    // Those are blocks 0-7
    for (int maskSelect = 0; maskSelect < 3; maskSelect++) {
        // Now we can re-purpose the mask variable to help up detect when a block should be inserted
        mask = 0xF0000000;
        for (int i = 0; i < 8; i ++) {
            // this is the block we're looking at in the 5x5 grid as defined on the google doc
            blockNum = maskSelect * 8 + i;

            // Calculate the coordinates of the block we're looking at on the mboard.
            // Block 12 should be the center and unchanged (row,col)
            insert_row  = (blockNum / 5) - 2 + row;
            insert_col = (blockNum % 5) - 2 + col;

            // Calculate the type of block that we will try to insert into the mboard
            insert_block = blockMask[maskSelect] & mask ? blockType : E_BLOCK;

            //Update the mask so we will select the next block next time
            mask >>= 4;

            // Now lets try to insert the bloc into the mboard
            // First do some boundary checks to make sure we can insert the block
            if(insert_block == E_BLOCK) continue; // inserting an E_Block does nothing, so skip the rest
            if(insert_row <  0) continue; // We're trying to insert a block out of bounds, return FALSE
            //if(insert_row > 22) return 1;
            if(insert_col <  0) return 1;
            if(insert_col > 9) return -1;

            //Now we need to check if there is already a block in the static board that is occupying our destination
            // this figures out if there's a block on the static board at this position (insert_row,insert_col). If there is, return 2;
            if ((blockVal(board[insert_row], insert_col))) return 2;

            // finally, insert the block into the correct position
            // This is pseudo code, I don't know how you actually insert a block at a specific location
            // mboard[insert_row][insert_col] = blockType;
            mboard[insert_row] = insertBlock(mboard[insert_row], insert_col, blockType);
        }

    }

  // Block successfully placed on mboard. Return TRUE
  return 0;
}


void clearmBoard(){
    for (unsigned int i = 0; i < sizeof mboard / sizeof mboard[0]; i++){
        mboard[i] = 0;
    }
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
    // Start the test!
    Display tetris = Display();
    tetrisPTR = &tetris;

    // Start the button code
    init_TIM2_buttons();

    while(1) {

    init_gameboard();
    // Render the GameBoard, It's empty right now, so nothing should change
    tetris.drawGameBoard(board);

    block_t currblock = randblock();
    block_t nextblock = randblock();
    int rot;
    leftButton = 0;
    rightButton = 0;
    int flag = 0;
    int flag2 = 0;
    int col;
    int row;
    gameBoard_t tempboard;

    //flag2 = insertShape(L_BLOCK, 12, 4, 0);
    //tetris.drawGameBoard(mboard);

    while(gameRunning){
        difficulty = GAME_SPEED - score * 10;
        col = 4;
        flag = 0;
        numRot = 0;
        currblock = nextblock;
        nextblock = randblock();
        tetris.drawNextBlock(nextblock);
        rot = 0;
        row = -1; //5X5 grid - row 2 col 2 of grid is axis
        //place new block on top of mboard
        flag2 = insertShape(currblock, row, col, rot);
        for (int j = 0; j < 25; j++) { //19
            if (row < 3) {
                insertShape(currblock, row, col, rot);
            }

            //display mboard on top of static board
            for (int i = 0; i < 27; i++) {
                dispboard[i] = board[i] | mboard[i];
            }

            //display board after every iteration
            tetris.drawGameBoard(dispboard);

            for (int wait = 0; wait < GAME_SPEED; wait++) {

                delay(1);

                if(!gameRunning) break;

                if (rotFlag  && (currblock != O_BLOCK)){
                    //if rotate button is pressed
                    rot = numRot % 4; //scale to be between 0-3
                    //numRot = 0;
                    for (int i = 0; i < 27; i++) {
                        tempboard[i] = mboard[i];
                    }
                    clearmBoard();
                    flag2 = insertShape(currblock, row, col, rot);
                    rotFlag = 0;
                    if (flag2 == 2){
                        for (int i = 0; i < 27; i++) {
                            mboard[i] = tempboard[i];
                        }
                    }
                    if (flag2 == -1){
                        //move to left
                        rotFlag=1;
                        leftButton=1;
                    }
                    if (flag2 == 1){
                        //move to right
                        rotFlag=1;
                        rightButton=1;
                    }

                    //display mboard on top of static board
                    for (int i = 0; i < 27; i++) {
                        dispboard[i] = board[i] | mboard[i];
                    }
                    //display board after every iteration
                    tetris.drawGameBoard(dispboard);
                }

                if (leftButton || rightButton){
                    //if left/right button is pressed
                    if (translateCheck(leftButton, rightButton)){
                        //update column value
                        col = leftRight(col, leftButton, rightButton);
                        //update mboard
                        clearmBoard();
                        flag2 = insertShape(currblock, row, col, rot);

                        //display mboard on top of static board
                        for (int i = 0; i < 27; i++) {
                            dispboard[i] = board[i] | mboard[i];
                        }
                        //display board after every iteration
                        tetris.drawGameBoard(dispboard);
                    }
                    leftButton = 0;
                    rightButton = 0;
                }
                if (downButton){
                    while (moveDownCheck()){
                        //moveBoardDown();
                        clearmBoard();
                        flag2 = insertShape(currblock, row, col, rot);
                        row++;
                        //display mboard on top of static board
                        for (int i = 0; i < 27; i++) {
                            dispboard[i] = board[i] | mboard[i];
                        }
                        //display board after every iteration
                        tetris.drawGameBoard(dispboard);
                    }
                    downButton=0;
                    break;
                }
            }
            //row is the row value that the pivot point is at

            flag = moveDownCheck();
            if (!flag){
                //shape could not move further down
                //need to break out of the for loop (start new shape)
                break;
            }

            //move the mboard down
            moveBoardDown();
            row++;
        }

        //reach this point when the shape can't move further down
            //and we are ready for a new shape

        //before starting new shape, move contents of moving board to static board
        for (int i = 0; i < 27; i++) {
            board[i] |= mboard[i];
            mboard[i] = E_BLOCK;
        }

        //see if a row is complete
        //update score
        getScore(board);
        tetris.drawScore(score);
        tetris.drawGameBoard(board);

        //check end game conditions
        int endTetris = endGame();
        if (endTetris){
            gameRunning = 0;
            tetris.endGame(score);
            break;
        }

    }
    }

    for(;;);
}
