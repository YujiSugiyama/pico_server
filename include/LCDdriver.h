#ifdef __cplusplus
extern "C" {
#endif

#define VERTICAL	0
#define HORIZONTAL	1
#define LCD_ALIGNMENT	HORIZONTAL
//#define LCD_ALIGNMENT	HORIZONTAL

#if LCD_ALIGNMENT == VERTICAL
	#define X_RES 240	// �������𑜓x
	#define Y_RES 320	// �c�����𑜓x
#else
	#define X_RES 320	// �������𑜓x
	#define Y_RES 240	// �c�����𑜓x
#endif

#define LCD_CS 11		//GPIO11
#define LCD_DC 12		//GPIO12
#define LCD_RESET 13	//GPIO13
#define SPICH spi1

void lcd_writeComm(unsigned char comm);
void lcd_writeData(unsigned char data);
void lcd_writeData2(unsigned short data);
void lcd_writeDataN(unsigned char *b,int n);
void lcd_init(void);
void lcd_setCursor(unsigned short x, unsigned short y);
void lcd_clear(unsigned short color);
void lcd_drawPixel(unsigned short x, unsigned short y, unsigned short color);

#ifdef __cplusplus
}
#endif /* End of CPP guard */
