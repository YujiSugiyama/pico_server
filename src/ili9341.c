#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "LCDdriver.h"

static inline void lcd_cs_lo() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_CS, 0);
	asm volatile("nop \n nop \n nop");
}

static inline void lcd_cs_hi() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_CS, 1);
	asm volatile("nop \n nop \n nop");
}

static inline void lcd_dc_lo() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_DC, 0);
	asm volatile("nop \n nop \n nop");
}
static inline void lcd_dc_hi() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_DC, 1);
	asm volatile("nop \n nop \n nop");
}

static inline void lcd_reset_lo() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_RESET, 0);
	asm volatile("nop \n nop \n nop");
}
static inline void lcd_reset_hi() {
	asm volatile("nop \n nop \n nop");
	gpio_put(LCD_RESET, 1);
	asm volatile("nop \n nop \n nop");
}

void lcd_writeComm(unsigned char comm){
// Write Command
	lcd_dc_lo();
	lcd_cs_lo();
	spi_write_blocking(SPICH, &comm , 1);
	lcd_cs_hi();
}

void lcd_writeData(unsigned char data)
{
// Write Data
	lcd_dc_hi();
	lcd_cs_lo();
	spi_write_blocking(SPICH, &data , 1);
	lcd_cs_hi();
}

void lcd_writeData2(unsigned short data)
{
// Write Data 2 bytes
	unsigned short d;
	lcd_dc_hi();
	lcd_cs_lo();
	d=(data>>8) | (data<<8);
	spi_write_blocking(SPICH, (unsigned char *)&d, 2);
	lcd_cs_hi();
}

void lcd_writeDataN(unsigned char *b,int n)
{
// Write Data N bytes
	lcd_dc_hi();
	lcd_cs_lo();
	spi_write_blocking(SPICH, b,n);
	lcd_cs_hi();
}

void lcd_init()
{
	lcd_cs_hi();
	lcd_dc_hi();

	// Reset controller
	lcd_reset_hi();
	sleep_ms(1);
	lcd_reset_lo();
	sleep_ms(10);
	lcd_reset_hi();
	sleep_ms(120);

//************* Start Initial Sequence **********//
	lcd_writeComm(0xCB);
	lcd_writeData(0x39);
	lcd_writeData(0x2C);
	lcd_writeData(0x00);
	lcd_writeData(0x34);
	lcd_writeData(0x02);
	lcd_writeComm(0xCF);
	lcd_writeData(0x00);
	lcd_writeData(0XC1);
	lcd_writeData(0X30);
	lcd_writeComm(0xE8);
	lcd_writeData(0x85);
	lcd_writeData(0x00);
	lcd_writeData(0x78);
	lcd_writeComm(0xEA);
	lcd_writeData(0x00);
	lcd_writeData(0x00);
	lcd_writeComm(0xED);
	lcd_writeData(0x64);
	lcd_writeData(0x03);
	lcd_writeData(0X12);
	lcd_writeData(0X81);
	lcd_writeComm(0xF7);
	lcd_writeData(0x20);
	lcd_writeComm(0xC0);
	lcd_writeData(0x23);
	lcd_writeComm(0xC1);
	lcd_writeData(0x10);
	lcd_writeComm(0xC5);
	lcd_writeData(0x3e);
	lcd_writeData(0x28);
	lcd_writeComm(0xC7);
	lcd_writeData(0x86);
	lcd_writeComm(0x36);
#if LCD_ALIGNMENT == VERTICAL
	lcd_writeData(0x48);
#elif LCD_ALIGNMENT == HORIZONTAL
	lcd_writeData(0x0C);
#endif
	lcd_writeComm(0x37);
	lcd_writeData(0x00);
	lcd_writeData(0x00);
	lcd_writeComm(0x3A);
	lcd_writeData(0x55);
	lcd_writeComm(0xB1);
	lcd_writeData(0x00);
	lcd_writeData(0x18);
	lcd_writeComm(0xB6);
	lcd_writeData(0x0A);
	lcd_writeData(0x82);
	lcd_writeData(0x27);
	lcd_writeData(0x00);
//	lcd_writeComm(0xF2);
//	lcd_writeData(0x00);
	lcd_writeComm(0x26);
	lcd_writeData(0x01);
	lcd_writeComm(0xE0);
	lcd_writeData(0x0F);
	lcd_writeData(0x3a);
	lcd_writeData(0x36);
	lcd_writeData(0x0b);
	lcd_writeData(0x0d);
	lcd_writeData(0x06);
	lcd_writeData(0x4c);
	lcd_writeData(0x91);
	lcd_writeData(0x31);
	lcd_writeData(0x08);
	lcd_writeData(0x10);
	lcd_writeData(0x04);
	lcd_writeData(0x11);
	lcd_writeData(0x0c);
	lcd_writeData(0x00);
	lcd_writeComm(0XE1);
	lcd_writeData(0x00);
	lcd_writeData(0x06);
	lcd_writeData(0x0a);
	lcd_writeData(0x05);
	lcd_writeData(0x12);
	lcd_writeData(0x09);
	lcd_writeData(0x2c);
	lcd_writeData(0x92);
	lcd_writeData(0x3f);
	lcd_writeData(0x08);
	lcd_writeData(0x0e);
	lcd_writeData(0x0b);
	lcd_writeData(0x2e);
	lcd_writeData(0x33);
	lcd_writeData(0x0F);
	lcd_writeComm(0x11);
	sleep_ms(120);
	lcd_writeComm(0x29);
}

void lcd_setAddrWindow(unsigned short x,unsigned short y,unsigned short w,unsigned short h)
{
#if LCD_ALIGNMENT == VERTICAL
	lcd_writeComm(0x2a);
	lcd_writeData2(x);
	lcd_writeData2(x+w-1);
	lcd_writeComm(0x2b);
	lcd_writeData2(y);
	lcd_writeData2(y+h-1);
#elif LCD_ALIGNMENT == HORIZONTAL
	lcd_writeComm(0x2a);
	lcd_writeData2(y);
	lcd_writeData2(y+h-1);
	lcd_writeComm(0x2b);
	lcd_writeData2(x);
	lcd_writeData2(x+w-1);
#endif
	lcd_writeComm(0x2c);
}

void lcd_setCursor(unsigned short x, unsigned short y)
{
	lcd_setAddrWindow(x,y,X_RES-x,1);
}

void lcd_continuous_output(unsigned short x,unsigned short y,unsigned short color,int n)
{
	//High speed continuous output
	int i;
	unsigned short d;
	lcd_setAddrWindow(x,y,n,1);
	lcd_dc_hi();
	lcd_cs_lo();
	d=(color>>8) | (color<<8);
	for (i=0; i < n ; i++){
		spi_write_blocking(SPICH, (unsigned char *)&d, 2);
	}
	lcd_cs_hi();
}
void lcd_clear(unsigned short color)
{
	int i;
	unsigned short d;
	lcd_setAddrWindow(0,0,X_RES,Y_RES);
	lcd_dc_hi();
	lcd_cs_lo();
	d=(color>>8) | (color<<8);
	for (i=0; i < X_RES*Y_RES ; i++){
		spi_write_blocking(SPICH, (unsigned char *)&d, 2);
	}
	lcd_cs_hi();
}

void lcd_drawPixel(unsigned short x, unsigned short y, unsigned short color)
{
	lcd_setCursor(x,y);
	lcd_writeData2(color);
}
