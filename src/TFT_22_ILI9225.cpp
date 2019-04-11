#include "stm32f0xx.h"
#include "TFT_22_ILI9225.h"

//#define DEBUG_PRINT
#ifdef DEBUG_PRINT
    // THis needs porting to the STM32F0. It will not work as is (I think).
    //#define DB_PRINT( x, ... ) { char dbgbuf[60]; sprintf_P( dbgbuf, (const char*) F( x ), __VA_ARGS__ ) ; Serial.print( dbgbuf ); }
    #define DB_PRINT( ... ) { char dbgbuf[60]; sprintf( dbgbuf,   __VA_ARGS__ ) ; Serial.println( dbgbuf ); }
#else
    #define DB_PRINT(  ... ) ;
#endif

// STM    Meaning      DISP
// ----------------------------
// PA3 -> nRESET    -> RST
// PA4 -> NSS (MAN) -> CS
// PA5 -> SPI1_SCK  -> CLK
// PA6 -> DATA/CMD  -> RS
// PA7 -> SPI1_MOSI -> SDI
// +5v -> BackLight -> LED (This is PWM dimmable)
// +5v -> Power     -> Vcc
// GND -> Ground    -> GND

#ifndef pgm_read_byte
 #define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
 #define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
 #define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))

// Control pins
#define SPI_DC_HIGH()           GPIOA->ODR |=  _rs //digitalWrite(_rs, HIGH)
#define SPI_DC_LOW()            GPIOA->ODR &= ~_rs //digitalWrite(_rs, LOW)
#define SPI_CS_HIGH()           GPIOA->ODR |=  _cs //digitalWrite(_cs, HIGH)
#define SPI_CS_LOW()            GPIOA->ODR &= ~_cs //digitalWrite(_cs, LOW)

//Software SPI Macros
#define SSPI_MOSI_HIGH()        GPIOA->ODR |=  _sdi //digitalWrite(_sdi, HIGH)
#define SSPI_MOSI_LOW()         GPIOA->ODR &= ~_sdi //digitalWrite(_sdi, LOW)
#define SSPI_SCK_HIGH()         GPIOA->ODR |=  _sdi //digitalWrite(_clk, HIGH)
#define SSPI_SCK_LOW()          GPIOA->ODR &= ~_sdi //digitalWrite(_clk, LOW)

// Simple Constructor
TFT_22_ILI9225::TFT_22_ILI9225() {
    // STM    Meaning      DISP
    // ----------------------------
    // PA3 -> nRESET    -> RST
    // PA4 -> NSS (MAN) -> CS
    // PA5 -> SPI1_SCK  -> CLK
    // PA6 -> DATA/CMD  -> RS
    // PA7 -> SPI1_MOSI -> SDI
    // +5v -> BackLight -> LED (This is PWM dimmable, but not implemented in this library)
    // +5v -> Power     -> Vcc
    // GND -> Ground    -> GND

    _rst  = GPIO_ODR_3;
    _rs   = GPIO_ODR_6;
    _cs   = GPIO_ODR_4;
    _sdi  = GPIO_ODR_7;
    _clk  = GPIO_ODR_5;
    _led  = 0; // This library does not use LED backlight control at the moment

    // Enable GPIO Port A clock
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Set Pins PA3 PA4 PA5 PA6 PA7 to Output
    GPIOA->MODER &= ~0x0000ffc0;
    GPIOA->MODER |=  0x00005540;

    // Set Clock to HIGH for IDLE state
    GPIOA->ODR |= GPIO_ODR_5;

    // Pull SS high
    GPIOA->ODR |= GPIO_ODR_4;

    // pull RS low
    GPIOA->ODR &= ~GPIO_ODR_6;

    // Set gfxFont to NULL for now
    //gfxFont = NULL;

    // Set maxX and maxY
    _maxX = maxX();
    _maxY = maxY();

    // Set Background color
    _bgColor = COLOR_BLACK;
}

void TFT_22_ILI9225::begin() {
    // Initialization Code -- Power ON RESET
    GPIOA->ODR |= GPIO_ODR_3; // Pull the reset pin high to release the ILI9225C from the reset status
    _delay(1);
    GPIOA->ODR &= ~GPIO_ODR_3;  // Pull the reset pin low to reset ILI9225
    _delay(10);
    GPIOA->ODR |= GPIO_ODR_3;  // Pull the reset pin high to release the ILI9225C from the reset status
    _delay(50);

    /* Start Initial Sequence */

    /* Set SS bit and direction output from S528 to S1 */
    startWrite();
    _writeRegister(ILI9225_POWER_CTRL1, 0x0000); // Set SAP,DSTB,STB
    _writeRegister(ILI9225_POWER_CTRL2, 0x0000); // Set APON,PON,AON,VCI1EN,VC
    _writeRegister(ILI9225_POWER_CTRL3, 0x0000); // Set BT,DC1,DC2,DC3
    _writeRegister(ILI9225_POWER_CTRL4, 0x0000); // Set GVDD
    _writeRegister(ILI9225_POWER_CTRL5, 0x0000); // Set VCOMH/VCOML voltage
    endWrite();
    _delay(40);

    // Power-on sequence
    startWrite();
    _writeRegister(ILI9225_POWER_CTRL2, 0x0018); // Set APON,PON,AON,VCI1EN,VC
    _writeRegister(ILI9225_POWER_CTRL3, 0x6121); // Set BT,DC1,DC2,DC3
    _writeRegister(ILI9225_POWER_CTRL4, 0x006F); // Set GVDD   /*007F 0088 */
    _writeRegister(ILI9225_POWER_CTRL5, 0x495F); // Set VCOMH/VCOML voltage
    _writeRegister(ILI9225_POWER_CTRL1, 0x0800); // Set SAP,DSTB,STB
    endWrite();
    _delay(10);
    startWrite();
    _writeRegister(ILI9225_POWER_CTRL2, 0x103B); // Set APON,PON,AON,VCI1EN,VC
    endWrite();
    _delay(50);

    startWrite();
    _writeRegister(ILI9225_DRIVER_OUTPUT_CTRL, 0x011C); // set the display line number and display direction
    _writeRegister(ILI9225_LCD_AC_DRIVING_CTRL, 0x0100); // set 1 line inversion
    _writeRegister(ILI9225_ENTRY_MODE, 0x1038); // set GRAM write direction and BGR=1.
    _writeRegister(ILI9225_DISP_CTRL1, 0x0000); // Display off
    _writeRegister(ILI9225_BLANK_PERIOD_CTRL1, 0x0808); // set the back porch and front porch
    _writeRegister(ILI9225_FRAME_CYCLE_CTRL, 0x1100); // set the clocks number per line
    _writeRegister(ILI9225_INTERFACE_CTRL, 0x0000); // CPU interface
    _writeRegister(ILI9225_OSC_CTRL, 0x0D01); // Set Osc  /*0e01*/
    _writeRegister(ILI9225_VCI_RECYCLING, 0x0020); // Set VCI recycling
    _writeRegister(ILI9225_RAM_ADDR_SET1, 0x0000); // RAM Address
    _writeRegister(ILI9225_RAM_ADDR_SET2, 0x0000); // RAM Address

    /* Set GRAM area */
    _writeRegister(ILI9225_GATE_SCAN_CTRL, 0x0000); 
    _writeRegister(ILI9225_VERTICAL_SCROLL_CTRL1, 0x00DB); 
    _writeRegister(ILI9225_VERTICAL_SCROLL_CTRL2, 0x0000); 
    _writeRegister(ILI9225_VERTICAL_SCROLL_CTRL3, 0x0000); 
    _writeRegister(ILI9225_PARTIAL_DRIVING_POS1, 0x00DB); 
    _writeRegister(ILI9225_PARTIAL_DRIVING_POS2, 0x0000); 
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR1, 0x00AF); 
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR2, 0x0000); 
    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR1, 0x00DB); 
    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR2, 0x0000); 

    /* Set GAMMA curve */
    _writeRegister(ILI9225_GAMMA_CTRL1, 0x0000); 
    _writeRegister(ILI9225_GAMMA_CTRL2, 0x0808); 
    _writeRegister(ILI9225_GAMMA_CTRL3, 0x080A); 
    _writeRegister(ILI9225_GAMMA_CTRL4, 0x000A); 
    _writeRegister(ILI9225_GAMMA_CTRL5, 0x0A08); 
    _writeRegister(ILI9225_GAMMA_CTRL6, 0x0808); 
    _writeRegister(ILI9225_GAMMA_CTRL7, 0x0000); 
    _writeRegister(ILI9225_GAMMA_CTRL8, 0x0A00); 
    _writeRegister(ILI9225_GAMMA_CTRL9, 0x0710); 
    _writeRegister(ILI9225_GAMMA_CTRL10, 0x0710); 

    _writeRegister(ILI9225_DISP_CTRL1, 0x0012); 
    endWrite();
    _delay(50);
    startWrite();
    _writeRegister(ILI9225_DISP_CTRL1, 0x1017);
    endWrite();

    // Turn on backlight
    setBacklight(true); // does nothing right now...

    // 0=portrait, 1=right rotated landscape, 2=reverse portrait, 3=left rotated landscape
    setOrientation(0);

    // Initialize variables
    setBackgroundColor(COLOR_BLACK);

    //clear();
}

// ----------------------------------------------------------------------------
// Functions added for STM32F0 Port
// ----------------------------------------------------------------------------

void TFT_22_ILI9225::_delay(unsigned int ms) {
    _nano_wait(ms * 1000000);
}

void TFT_22_ILI9225::_nano_wait(unsigned int ns) {
    // Taken from Purdue ECE362 course materials
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(ns) : "r0", "cc");
}

uint16_t TFT_22_ILI9225::_min(uint16_t a, uint16_t b) {
    return a > b ? b : a;
}

uint16_t TFT_22_ILI9225::_abs(int16_t a) {
    // If a is less than zero, flip all the bits and add one.
    return a < 0 ? (uint16_t) ~a + 1 : a;
}

int TFT_22_ILI9225::_strlen(STRING str) {
    int i;
    for (i = 0; str[i] != '\0'; i++) ;
    return i;
}

int TFT_22_ILI9225::_bitRead(uint8_t byte, int k) {
    return (byte >> k) & 0x1;
}

// ----------------------------------------------------------------------------
// END of new functions
// ----------------------------------------------------------------------------

void TFT_22_ILI9225::_spiWrite(uint8_t b) {
    for (uint8_t bit = 0x80; bit; bit >>= 1) {
        if (b & bit) {
            // Data is 1, set MOSI to high
            GPIOA->ODR |= _sdi;
        } else {
            // Data is 0, set MOSI to low
            GPIOA->ODR &= ~_sdi;
        }
        GPIOA->ODR |=  _clk;
        GPIOA->ODR &= ~_clk;
    }
}

void TFT_22_ILI9225::_spiWriteCommand(uint8_t c) {
    SPI_DC_LOW();
    SPI_CS_LOW();
    _spiWrite(c);
    SPI_CS_HIGH();
}


void TFT_22_ILI9225::_spiWriteData(uint8_t c) {
    SPI_DC_HIGH();
    SPI_CS_LOW();
    _spiWrite(c);
    SPI_CS_HIGH();
}

void TFT_22_ILI9225::_orientCoordinates(uint16_t &x1, uint16_t &y1) {

    switch (_orientation) {
    case 0:  // ok
        break;
    case 1: // ok
        y1 = _maxY - y1 - 1;
        _swap(x1, y1);
        break;
    case 2: // ok
        x1 = _maxX - x1 - 1;
        y1 = _maxY - y1 - 1;
        break;
    case 3: // ok
        x1 = _maxX - x1 - 1;
        _swap(x1, y1);
        break;
    }
}

void TFT_22_ILI9225::_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    _setWindow( x0, y0, x1, y1, TopDown_L2R ); // default for drawing characters
}

void TFT_22_ILI9225::_setWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, autoIncMode_t mode) {
    DB_PRINT( "setWindows( x0=%d, y0=%d, x1=%d, y1=%d, mode=%d", x0,y0,x1,y1,mode );
    // clip to TFT-Dimensions
    x0 = _min( x0, (uint16_t) (_maxX-1) );
    x1 = _min( x1, (uint16_t) (_maxX-1) );
    y0 = _min( y0, (uint16_t) (_maxY-1) );
    y1 = _min( y1, (uint16_t) (_maxY-1) );
    _orientCoordinates(x0, y0);
    _orientCoordinates(x1, y1);

    if (x1<x0) _swap(x0, x1);
    if (y1<y0) _swap(y0, y1);
    
    startWrite();
    // autoincrement mode
    if ( _orientation > 0 ) mode = modeTab[_orientation-1][mode];
    _writeRegister(ILI9225_ENTRY_MODE, 0x1000 | ( mode<<3) );
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR1,x1);
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR2,x0);

    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR1,y1);
    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR2,y0);
    DB_PRINT( "gedreht: x0=%d, y0=%d, x1=%d, y1=%d, mode=%d", x0,y0,x1,y1,mode );
    // starting position within window and increment/decrement direction
    switch ( mode>>1 ) {
      case 0:
        _writeRegister(ILI9225_RAM_ADDR_SET1,x1);
        _writeRegister(ILI9225_RAM_ADDR_SET2,y1);
        break;
      case 1:
        _writeRegister(ILI9225_RAM_ADDR_SET1,x0);
        _writeRegister(ILI9225_RAM_ADDR_SET2,y1);
        break;
      case 2:
        _writeRegister(ILI9225_RAM_ADDR_SET1,x1);
        _writeRegister(ILI9225_RAM_ADDR_SET2,y0);
        break;
      case 3:
        _writeRegister(ILI9225_RAM_ADDR_SET1,x0);
        _writeRegister(ILI9225_RAM_ADDR_SET2,y0);
        break;
    }
    _writeCommand16( ILI9225_GRAM_DATA_REG );

    //_writeRegister(ILI9225_RAM_ADDR_SET1,x0);
    //_writeRegister(ILI9225_RAM_ADDR_SET2,y0);

    //_writeCommand(0x00, 0x22);

    endWrite();
}


void TFT_22_ILI9225::_resetWindow() {
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR1, 0x00AF); 
    _writeRegister(ILI9225_HORIZONTAL_WINDOW_ADDR2, 0x0000); 
    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR1, 0x00DB); 
    _writeRegister(ILI9225_VERTICAL_WINDOW_ADDR2, 0x0000); 

}

void TFT_22_ILI9225::clear() {
    uint8_t old = _orientation;
    setOrientation(0);
    fillRectangle(0, 0, _maxX - 1, _maxY - 1, COLOR_BLACK);
    setOrientation(old);
    _delay(10);
}


void TFT_22_ILI9225::invert(boolean flag) {
    startWrite();
    _writeCommand16(flag ? ILI9225C_INVON : ILI9225C_INVOFF);
    //_writeCommand(0x00, flag ? ILI9225C_INVON : ILI9225C_INVOFF);
    endWrite();
}


void TFT_22_ILI9225::setBacklight(boolean flag) {
    // THIS FUNCTION DOES NOTHING AT THE MOMENT
    blState = flag;
    // Update PWM duty cycle (based on _brightness) of LED pin on display here if you want to implement LED control.
}


void TFT_22_ILI9225::setBacklightBrightness(uint8_t brightness) {
    // THIS FUNCTION DOES NOTHING AT THE MOMENT
    _brightness = brightness;
    setBacklight(blState);
}


void TFT_22_ILI9225::setDisplay(boolean flag) {
    if (flag) {
        startWrite();
        _writeRegister(0x00ff, 0x0000);
        _writeRegister(ILI9225_POWER_CTRL1, 0x0000);
        endWrite();
        _delay(50);
        startWrite();
        _writeRegister(ILI9225_DISP_CTRL1, 0x1017);
        endWrite();
        _delay(200);
    } else {
        startWrite();
        _writeRegister(0x00ff, 0x0000);
        _writeRegister(ILI9225_DISP_CTRL1, 0x0000);
        endWrite();
        _delay(50);
        startWrite();
        _writeRegister(ILI9225_POWER_CTRL1, 0x0003);
        endWrite();
        _delay(200);
    }
}


void TFT_22_ILI9225::setOrientation(uint8_t orientation) {

    _orientation = orientation % 4;

    switch (_orientation) {
    case 0:
        _maxX = ILI9225_LCD_WIDTH;
        _maxY = ILI9225_LCD_HEIGHT;

        break;
    case 1:
        _maxX = ILI9225_LCD_HEIGHT;
        _maxY = ILI9225_LCD_WIDTH;
        break;
    case 2:
        _maxX = ILI9225_LCD_WIDTH;
        _maxY = ILI9225_LCD_HEIGHT;
        break;
    case 3:
        _maxX = ILI9225_LCD_HEIGHT;
        _maxY = ILI9225_LCD_WIDTH;
        break;
    }
}


uint8_t TFT_22_ILI9225::getOrientation() {
    return _orientation;
}


void TFT_22_ILI9225::drawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {
    startWrite();
    drawLine(x1, y1, x1, y2, color);
    drawLine(x1, y1, x2, y1, color);
    drawLine(x1, y2, x2, y2, color);
    drawLine(x2, y1, x2, y2, color);
    endWrite();
}


void TFT_22_ILI9225::fillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {

    _setWindow(x1, y1, x2, y2,L2R_BottomUp);

    startWrite();
    for(uint16_t t=(y2 - y1 + 1) * (x2 - x1 + 1); t > 0; t--)
        _writeData16(color);
    endWrite();
    _resetWindow();
}


void TFT_22_ILI9225::drawCircle(uint16_t x0, uint16_t y0, uint16_t r, uint16_t color) {

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    startWrite();

    drawPixel(x0, y0 + r, color);
    drawPixel(x0, y0-  r, color);
    drawPixel(x0 + r, y0, color);
    drawPixel(x0 - r, y0, color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawPixel(x0 + x, y0 + y, color);
        drawPixel(x0 - x, y0 + y, color);
        drawPixel(x0 + x, y0 - y, color);
        drawPixel(x0 - x, y0 - y, color);
        drawPixel(x0 + y, y0 + x, color);
        drawPixel(x0 - y, y0 + x, color);
        drawPixel(x0 + y, y0 - x, color);
        drawPixel(x0 - y, y0 - x, color);
    }
    endWrite();
}


void TFT_22_ILI9225::fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color) {

    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;

    startWrite();
    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        drawLine(x0 + x, y0 + y, x0 - x, y0 + y, color); // bottom
        drawLine(x0 + x, y0 - y, x0 - x, y0 - y, color); // top
        drawLine(x0 + y, y0 - x, x0 + y, y0 + x, color); // right
        drawLine(x0 - y, y0 - x, x0 - y, y0 + x, color); // left
    }
    endWrite();
    fillRectangle(x0-x, y0-y, x0+x, y0+y, color);
}


void TFT_22_ILI9225::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color) {

    // Classic Bresenham algorithm
    int16_t steep = _abs((int16_t)(y2 - y1)) > _abs((int16_t)(x2 - x1));

    int16_t dx, dy;

    if (steep) {
        _swap(x1, y1);
        _swap(x2, y2);
    }

    if (x1 > x2) {
        _swap(x1, x2);
        _swap(y1, y2);
    }

    dx = x2 - x1;
    dy = _abs((int16_t)(y2 - y1));

    int16_t err = dx / 2;
    int16_t ystep;

    if (y1 < y2) ystep = 1;
    else ystep = -1;

    startWrite();
    for (; x1<=x2; x1++) {
        if (steep) drawPixel(y1, x1, color);
        else       drawPixel(x1, y1, color);

        err -= dy;
        if (err < 0) {
            y1 += ystep;
            err += dx;
        }
    }
    endWrite();
}


void TFT_22_ILI9225::drawPixel(uint16_t x1, uint16_t y1, uint16_t color) {
    if((x1 >= _maxX) || (y1 >= _maxY)) return;

    _orientCoordinates(x1, y1);
    startWrite();
    _writeRegister(ILI9225_RAM_ADDR_SET1,x1);
    _writeRegister(ILI9225_RAM_ADDR_SET2,y1);
    _writeRegister(ILI9225_GRAM_DATA_REG,color);
    
    endWrite();
}


uint16_t TFT_22_ILI9225::maxX() {
    return _maxX;
}


uint16_t TFT_22_ILI9225::maxY() {
    return _maxY;
}


uint16_t TFT_22_ILI9225::setColor(uint8_t red8, uint8_t green8, uint8_t blue8) {
    // rgb16 = red5 green6 blue5
    return (red8 >> 3) << 11 | (green8 >> 2) << 5 | (blue8 >> 3);
}


void TFT_22_ILI9225::splitColor(uint16_t rgb, uint8_t &red, uint8_t &green, uint8_t &blue) {
    // rgb16 = red5 green6 blue5
    red   = (rgb & 0b1111100000000000) >> 11 << 3;
    green = (rgb & 0b0000011111100000) >>  5 << 2;
    blue  = (rgb & 0b0000000000011111)       << 3;
}


void TFT_22_ILI9225::_swap(uint16_t &a, uint16_t &b) {
    uint16_t w = a;
    a = b;
    b = w;
}

void TFT_22_ILI9225::_writeCommand16(uint16_t command) {
    SPI_DC_LOW();
    SPI_CS_LOW();
    _spiWrite((uint8_t) (command >> 8));
    _spiWrite((uint8_t) command); // was 0x00ff & command, but this is not necessary
    SPI_CS_HIGH();
}

void TFT_22_ILI9225::_writeData16(uint16_t data) {
    SPI_DC_HIGH();
    SPI_CS_LOW();
    _spiWrite((uint8_t) (data >> 8));
    _spiWrite((uint8_t) data); // was 0x00ff & command, but this is not necessary
    SPI_CS_HIGH();
}

void TFT_22_ILI9225::_writeRegister(uint16_t reg, uint16_t data) {
    _writeCommand16(reg);
    _writeData16(data);
}

void TFT_22_ILI9225::drawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {
    startWrite();
    drawLine(x1, y1, x2, y2, color);
    drawLine(x2, y2, x3, y3, color);
    drawLine(x3, y3, x1, y1, color);
    endWrite();
}

void TFT_22_ILI9225::fillTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3, uint16_t color) {

    uint16_t a, b, y, last;

    // Sort coordinates by Y order (y3 >= y2 >= y1)
    if (y1 > y2) {
        _swap(y1, y2); _swap(x1, x2);
    }
    if (y2 > y3) {
        _swap(y3, y2); _swap(x3, x2);
    }
    if (y1 > y2) {
        _swap(y1, y2); _swap(x1, x2);
    }

    startWrite();
    if (y1 == y3) { // Handle awkward all-on-same-line case as its own thing
        a = b = x1;
        if (x2 < a)      a = x2;
        else if (x2 > b) b = x2;
        if (x3 < a)      a = x3;
        else if (x3 > b) b = x3;
            drawLine(a, y1, b, y1, color);
        return;
    }

    int16_t dx11 = x2 - x1,
            dy11 = y2 - y1,
            dx12 = x3 - x1,
            dy12 = y3 - y1,
            dx22 = x3 - x2,
            dy22 = y3 - y2;
    int32_t sa   = 0,
            sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y2=y3 (flat-bottomed triangle), the scanline y2
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y2 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y1=y2
    // (flat-topped triangle).
    if (y2 == y3) last = y2;   // Include y2 scanline
    else          last = y2 - 1; // Skip it

    for (y = y1; y <= last; y++) {
        a   = x1 + sa / dy11;
        b   = x1 + sb / dy12;
        sa += dx11;
        sb += dx12;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x1 + (x3 - x1) * (y - y1) / (y3 - y1);
        */
        if (a > b) _swap(a,b);
        drawLine(a, y, b, y, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y2=y3.
    sa = dx22 * (y - y2);
    sb = dx12 * (y - y1);
    for (; y<=y3; y++) {
        a   = x2 + sa / dy22;
        b   = x1 + sb / dy12;
        sa += dx22;
        sb += dx12;
        /* longhand:
        a = x2 + (x3 - x2) * (y - y2) / (y3 - y2);
        b = x1 + (x3 - x1) * (y - y1) / (y3 - y1);
        */
        if (a > b) _swap(a,b);
            drawLine(a, y, b, y, color);
    }
    endWrite();
}


void TFT_22_ILI9225::setBackgroundColor(uint16_t color) {
    _bgColor = color;
}

void TFT_22_ILI9225::setFont(uint8_t* font, bool monoSp) {

    cfont.font     = font;
    cfont.width    = readFontByte(0);
    cfont.height   = readFontByte(1);
    cfont.offset   = readFontByte(2);
    cfont.numchars = readFontByte(3);
    cfont.nbrows   = cfont.height / 8;
    cfont.monoSp   = monoSp;

    if (cfont.height % 8) cfont.nbrows++;  // Set number of bytes used by height of font in multiples of 8
}

_currentFont TFT_22_ILI9225::getFont() {
    return cfont;
}

uint16_t TFT_22_ILI9225::drawText(uint16_t x, uint16_t y, STRING s, uint16_t color) {

    uint16_t currx = x;

    // Print every character in string
    for (uint8_t k = 0; k < _strlen(s); k++) {
        currx += drawChar(currx, y, s[k], color) + 1;
    }
    return currx;
}

void TFT_22_ILI9225::drawNumber(uint16_t x, uint16_t y, uint8_t number, uint16_t color) {
    int digit;
    char number_text[] = "999\0";
    int number_index = 0;
    for (int i = 1000; i > 1; i /= 10) {
        digit = number % i;                    // Isolates only the relevant digit
        digit /= i / 10;                           // Scales the digit to the ones place
        number_text[number_index] = digit + 48; // 0 is 48 in ASCII
        number_index++;
    }
    drawText(x, y, number_text, color);
}

uint16_t TFT_22_ILI9225::getTextWidth( STRING s ) {

    uint16_t width = 0;
    // Count every character in string ( +1 for spacing )
    for (uint8_t k = 0; k < _strlen(s); k++) {
        width += getCharWidth(s[k]) + 1;
    }
    return width;
}

uint16_t TFT_22_ILI9225::drawChar(uint16_t x, uint16_t y, uint16_t ch, uint16_t color) {

    uint8_t charData, charWidth;
    uint8_t h, i, j;
    uint16_t charOffset;
    bool fastMode;
    charOffset = (cfont.width * cfont.nbrows) + 1;  // bytes used by each character
    charOffset = (charOffset * (ch - cfont.offset)) + FONT_HEADER_SIZE;  // char offset (add 4 for font header)
    if ( cfont.monoSp ) charWidth = cfont.width;      // monospaced: get char width from font
    else                charWidth  = readFontByte(charOffset);  // get chracter width from 1st byte
    charOffset++;  // increment pointer to first character data byte

    startWrite();
    
    // use autoincrement/decrement feature, if character fits completely on screen
    fastMode = ( (x+charWidth+1) < _maxX && (y+cfont.height-1) < _maxY ) ;
    
    if ( fastMode )_setWindow( x,y,x+charWidth+1, y+cfont.height-1 );  // set character Window

    for (i = 0; i <= charWidth; i++) {  // each font "column" (+1 blank column for spacing)
        h = 0;  // keep track of char height
        for (j = 0; j < cfont.nbrows; j++)     {  // each column byte
            if (i == charWidth) charData = (uint8_t)0x0; // Insert blank column
            else                charData = readFontByte(charOffset);
            charOffset++;
            
            // Process every row in font character
            for (uint8_t k = 0; k < 8; k++) {
                if (h >= cfont.height ) break;  // No need to process excess bits
                if (fastMode ) _writeData16( _bitRead(charData, k) ? color : _bgColor );
                else drawPixel( x + i, y + (j * 8) + k, _bitRead(charData, k) ? color : _bgColor );
                h++;
            }
        }
    }
    endWrite();
    _resetWindow();
    return charWidth;
}

uint16_t TFT_22_ILI9225::getCharWidth(uint16_t ch) {
    uint16_t charOffset;
    charOffset = (cfont.width * cfont.nbrows) + 1;  // bytes used by each character
    charOffset = (charOffset * (ch - cfont.offset)) + FONT_HEADER_SIZE;  // char offset (add 4 for font header)

    return readFontByte(charOffset);  // get font width from 1st byte
}

// Draw a 1-bit image (bitmap) at the specified (x,y) position from the
// provided bitmap buffer (must be PROGMEM memory) using the specified
// foreground color (unset bits are transparent).
void TFT_22_ILI9225::drawBitmap(int16_t x, int16_t y,
const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  0, true, true, false );
}

// Draw a 1-bit image (bitmap) at the specified (x,y) position from the
// provided bitmap buffer (must be PROGMEM memory) using the specified
// foreground (for set bits) and background (for clear bits) colors.
void TFT_22_ILI9225::drawBitmap(int16_t x, int16_t y,
const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  bg, false, true, false );
}

// drawBitmap() variant for RAM-resident (not PROGMEM) bitmaps.
void TFT_22_ILI9225::drawBitmap(int16_t x, int16_t y,
uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  0, true, false, false );
}

// drawBitmap() variant w/background for RAM-resident (not PROGMEM) bitmaps.
void TFT_22_ILI9225::drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  bg, false, false, false );
}

//Draw XBitMap Files (*.xbm), exported from GIMP,
//Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
//C Array can be directly used with this function
void TFT_22_ILI9225::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  0, true, true, true );
}

void TFT_22_ILI9225::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg) {
    _drawBitmap( x,  y, bitmap,  w,  h, color,  bg, false, true, true );
}


// internal function for drawing bitmaps with/without transparent bg, or from ram or progmem
void TFT_22_ILI9225::_drawBitmap(int16_t x, int16_t y,
const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg, bool transparent, bool progmem,bool Xbit) {
    bool noAutoInc = false;     // Flag set when transparent pixel was 'written'
    int16_t i, j, byteWidth = (w + 7) / 8;
    int16_t wx0,wy0,wx1,wy1,wh,ww;  // Window-position and size
    uint8_t byte, maskBit;
    maskBit = Xbit? 0x01:0x80;
    // adjust window hight/width to displaydimensions
    DB_PRINT( "DrawBitmap.. maxX=%d, maxY=%d", _maxX,_maxY );
    wx0 = x<0?0:x;
    wy0 = y<0?0:y;
    wx1 = (x+w>_maxX?_maxX:x+w)-1;
    wy1 = (y+h>_maxY?_maxY:y+h)-1;
    wh  = wy1-wy0 +1;
    ww  = wx1-wx0 +1;
    _setWindow( wx0,wy0,wx1,wy1,L2R_TopDown);
    startWrite();
    for (j = y>=0?0:-y; j < (y>=0?0:-y)+wh; j++) {
        for (i = 0; i < w; i++ ) {
            if (i & 7) { if ( Xbit ) byte >>=1; else byte <<= 1; }
            else {  if ( progmem ) byte   = pgm_read_byte(bitmap + j * byteWidth + i / 8);
                    else      byte   = bitmap[j * byteWidth + i / 8];
            }
            if ( x+i >= wx0 && x+i <= wx1 ) {
                // write only if pixel is within window
                if (byte & maskBit) {
                    if (noAutoInc) {
                        //there was a transparent area, set pixelkoordinates again
                        drawPixel(x + i, y + j, color);
                        noAutoInc = false;
                    }
                    else  { 
                        _writeData16(color);
                    }
                }
                else  {
                    if (transparent) noAutoInc = true; // no autoincrement in transparent area!
                    else _writeData16( bg);
                }
            }
        }
    }
    endWrite();
}



//Draw XBitMap Files (*.xbm), exported from GIMP,
//Usage: Export from GIMP to *.xbm, rename *.xbm to *.c and open in editor.
//C Array can be directly used with this function
/*void TFT_22_ILI9225::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {

    int16_t i, j, byteWidth = (w + 7) / 8;
    uint8_t byte;

    startWrite();
    for (j = 0; j < h; j++) {
        for (i = 0; i < w; i++ ) {
            if (i & 7) byte >>= 1;
            else      byte   = pgm_read_byte(bitmap + j * byteWidth + i / 8);
            if (byte & 0x01) drawPixel(x + i, y + j, color);
        }
    }
    endWrite();
}
*/

//High speed color bitmap
void TFT_22_ILI9225::drawBitmap(uint16_t x1, uint16_t y1, 
const uint16_t** bitmap, int16_t w, int16_t h) {
    _setWindow(x1, y1, x1+w, y1+h);
    startWrite();
    SPI_DC_HIGH();
    SPI_CS_LOW();
    for (uint16_t x = 0; x < w; x++) {
        for (uint16_t y = 0; y < h; y++) {
            uint16_t col = bitmap[x][y];
            _spiWrite(col>>8);
            _spiWrite(col);
        }
    }
    SPI_CS_HIGH();
    endWrite();
}

//High speed color bitmap
void TFT_22_ILI9225::drawBitmap(uint16_t x1, uint16_t y1, 
uint16_t** bitmap, int16_t w, int16_t h) {
    _setWindow(x1, y1, x1+w, y1+h);
    startWrite();
    SPI_DC_HIGH();
    SPI_CS_LOW();
    for (uint16_t x = 0; x < w; x++) {
        for (uint16_t y = 0; y < h; y++) {
            uint16_t col = bitmap[x][y];
            _spiWrite(col>>8);
            _spiWrite(col);
        }
    }
    SPI_CS_HIGH();
    endWrite();
}

void TFT_22_ILI9225::startWrite(void){
    if (writeFunctionLevel++ == 0) {
        //SPI_BEGIN_TRANSACTION(); //TODO: Confirm that this is not necessary
        SPI_CS_LOW();
    }
}


void TFT_22_ILI9225::endWrite(void){
    if (--writeFunctionLevel == 0) {
        SPI_CS_HIGH();
        //SPI_END_TRANSACTION(); //TODO: Confirm that this is not necessary
    }
}


// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------

// It seems at first glance that GFX fonts are an Arduino thing...

//void TFT_22_ILI9225::setGFXFont(const GFXfont *f) {
//    gfxFont = (GFXfont *)f;
//}


// Draw a string
//void TFT_22_ILI9225::drawGFXText(int16_t x, int16_t y, STRING s, uint16_t color) {
//
//    int16_t currx = x;
//
//    if(gfxFont) {
//        // Print every character in string
//        #ifdef USE_STRING_CLASS
//        for (uint8_t k = 0; k < s.length(); k++) {
//            currx += drawGFXChar(currx, y, s.charAt(k), color) + 1;
//        }
//        #else
//        for (uint8_t k = 0; k < strlen(s); k++) {
//            currx += drawGFXChar(currx, y, s[k], color) + 1;
//        }
//        #endif
//    }
//}


// Draw a character
//uint16_t TFT_22_ILI9225::drawGFXChar(int16_t x, int16_t y, unsigned char c, uint16_t color) {
//
//    c -= (uint8_t)pgm_read_byte(&gfxFont->first);
//    GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
//    uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);
//
//    uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
//    uint8_t  w  = pgm_read_byte(&glyph->width),
//             h  = pgm_read_byte(&glyph->height),
//             xa = pgm_read_byte(&glyph->xAdvance);
//    int8_t   xo = pgm_read_byte(&glyph->xOffset),
//             yo = pgm_read_byte(&glyph->yOffset);
//    uint8_t  xx, yy, bits = 0, bit = 0;
//
//    // Add character clipping here one day
//
//    startWrite();
//    for(yy=0; yy<h; yy++) {
//        for(xx=0; xx<w; xx++) {
//            if(!(bit++ & 7)) {
//                bits = pgm_read_byte(&bitmap[bo++]);
//            }
//            if(bits & 0x80) {
//                drawPixel(x+xo+xx, y+yo+yy, color);
//            }
//            bits <<= 1;
//        }
//    }
//    endWrite();
//
//    return (uint16_t)xa;
//}


//void TFT_22_ILI9225::getGFXCharExtent(uint8_t c, int16_t *gw, int16_t *gh, int16_t *xa) {
//    uint8_t first = pgm_read_byte(&gfxFont->first),
//            last  = pgm_read_byte(&gfxFont->last);
//    // Char present in this font?
//    if((c >= first) && (c <= last)) {
//        GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c - first]);
//        *gw = pgm_read_byte(&glyph->width);
//        *gh = pgm_read_byte(&glyph->height);
//        *xa = pgm_read_byte(&glyph->xAdvance);
//        // int8_t  xo = pgm_read_byte(&glyph->xOffset),
//        //         yo = pgm_read_byte(&glyph->yOffset);
//    }
//}


//void TFT_22_ILI9225::getGFXTextExtent(STRING str, int16_t x, int16_t y, int16_t *w, int16_t *h) {
//    *w  = *h = 0;
//    #ifdef USE_STRING_CLASS
//    for (uint8_t k = 0; k < str.length(); k++) {
//        uint8_t c = str.charAt(k);
//    #else
//    for (uint8_t k = 0; k < strlen(str); k++) {
//        uint8_t c = str[k];
//    #endif
//        int16_t gw, gh, xa;
//        getGFXCharExtent(c, &gw, &gh, &xa);
//        if(gh > *h) {
//            *h = gh;
//        }
//        *w += xa;
//    }
//}

