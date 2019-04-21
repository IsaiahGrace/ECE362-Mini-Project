/*
 * I2C.h
 *
 *  Created on: Apr 19, 2019
 *      Author: igrace
 */

#ifndef I2C_H_
#define I2C_H_

#include "stm32f0xx.h"

// Define some constants for the I2C functions
#define I2C_FAIL -1
#define I2C_SUCCESS 0
#define I2C_WR 0
#define I2C_RD 1

class I2C {
public:
    I2C();
    virtual ~I2C();

    void init();
    void waitidle();
    void start(uint8_t addr, uint32_t dir);
    void stop();

    int senddata(uint8_t* data, uint32_t size);
    int readdata(uint8_t* data, uint32_t size);
};

#endif /* I2C_H_ */
