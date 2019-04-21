/*
 * EEPROM.h
 *
 *  Created on: Apr 19, 2019
 *      Author: igrace
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "stm32f0xx.h"
#include "I2C.h"

#define EEPROM_ADDR 0x50
#define SECRET_CHAR '-'

class EEPROM {
public:
    EEPROM();
    virtual ~EEPROM();

    // EEPROM related functions
    // These variables reduce the number of reads and writes to the EEPPROM
    // These will be useful because the user will call getName and then getScore for the same number
    int CacheNumber;
    uint8_t CacheScore;
    char CacheName[4];

    // TODO: Make the Score field two bytes allowing at most 999 scores. 999 is the maximum number we can print on our display...

    // Increments the letter in the CacheName by the increment amount.
    // Used in the NameEntry screen and also to ensure that bytes read from the EEPROM are printable ASCII
    // @param       letter_index must be in the range 0-2
    // @param       incriment_ammount will be added to the ASCII letter variable (usually -1 or 1)
    void updateCacheNameLetter(int letter_index, int incriment_ammount);

    // High level function returns the name of person ranked number (starting from 0)
    // returns a word that is actually three chars and a terminating character
    uint32_t GetName(uint8_t number);

    // High level function returns the score of person ranked number (starting from 0)
    uint8_t GetScore(uint8_t number);

    // Update the cache from the EEPROM device by reading the data packet for the person ranked number
    void UpdateCache(uint8_t number);

    // Returns the total number of scores present in the EEPROM
    uint8_t GetNumberOfScores();

    // Returns the rank of the score given lowest is 1, highest is EEPROM_GetNumberOfScores() + 1
    int GetRank(uint8_t score);

    // Generates a data packet including name and score for the EEPROM based on the Cache
    uint32_t CreateDataPacket();

    // Writes the given data packet in the appropriate location to maintain a sorted data array on the EEPROM
    // Implements a simple insertion sort
    void InsertDataPacket(uint32_t dataPacket);

    void ClearScores();

    void write(uint16_t wr_addr, uint8_t data);
    void read(uint16_t rd_addr, uint8_t* data);

    // Calculates the physical address of the start of a 32-bit High-Score entry
    uint16_t Addr(int number);

private:
    I2C i2c;

    void delay(unsigned int ms);
    void nano_wait(unsigned int ns);

};

#endif /* EEPROM_H_ */
