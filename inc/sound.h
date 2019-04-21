/*
 * sound.h
 *
 *  Created on: Apr 19, 2019
 *      Author: igrace
 */

#ifndef SOUND_H_
#define SOUND_H_


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

#define A 4550 //220.21 //A3
#define B 4070 //246.22 //B3
#define C 3830 //261.63 C4
#define D 3415 //293.52
#define E 3040 //E4 329.63Hz
#define GS 4825 //G3# 207.652
#define F4 2870 //F4 349.228
#define G4 2563 //391.995
#define A4 2275 //440.000
#define G4S 2412 //415.305
#define R 0 //0
//next is *12th root of 2 for a half step
//audible frequency is 20 - 20,000Hz
//#define S 1000 //slow
//#define F 1500 //fast

void setup_gpio(void);
void setup_tim1(void);
void setup_tim14(void);

void playSong(void);
void stopSong(void);

void micro_wait(int);

#endif /* SOUND_H_ */
