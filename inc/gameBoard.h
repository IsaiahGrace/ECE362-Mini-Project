/*
 * gameBoard.h
 *
 *  Created on: Mar 31, 2019
 *      Author: isaiah
 */

#ifndef GAMEBOARD_H_
#define GAMEBOARD_H_

#include <stdint.h>

//typedef struct gameBoard_t {
//    uint32_t row[22];
//} gameBoard_t;

typedef uint32_t gameBoard_t[22];

// Blocks look like this:
/// E   I   J   L   O   S   T   Z
// --- -#- -#- -#- --- --- --- ---
// --- -#- -#- -#- -## -## -#- ##-
// --- -#- ##- -## -## ##- ### -##
// --- -#- --- --- --- --- --- ---

typedef enum block_t {
    E_BLOCK = 0,
    I_BLOCK = 1,
    J_BLOCK = 2,
    L_BLOCK = 3,
    O_BLOCK = 4,
    S_BLOCK = 5,
    T_BLOCK = 6,
    Z_BLOCK = 7
} block_t;

#endif /* GAMEBOARD_H_ */
