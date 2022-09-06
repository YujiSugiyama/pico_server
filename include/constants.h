
#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

// LEDs
#define SYS_LED			25			// System LED
#define STATUS_LED_PIN	PICO_DEFAULT_LED_PIN

#define RED_LED			28
#define GREEN_LED		26
#define BLUE_LED		20

// Bottons
#define BTN0_GPIO		2
#define BTN1_GPIO		3

// UART for ESP32
#define UART_ID			uart1
#define BAUD_RATE		115200
#define DATA_BITS		8
#define STOP_BITS		1
#define PARITY			UART_PARITY_NONE
#define UART_TX_PIN		4
#define UART_RX_PIN		5
#define UART_CTS_PIN	6
#define UART_RTS_PIN	7
#define FLOW_CTRL		1

// UART for STDIO
#define STDIO_UART		uart0
#define STDIO_TX_PIN	0
#define STDIO_RX_PIN	1
#define STDIO_BAUD_RATE	115200

// LCD
#define LCD_DISP
#define SOUNDPORT		10
#define PWM_WRAP		4000		// 125MHz/31.25KHz
#define LCD_CS			11			//GPIO11
#define LCD_DC			12			//GPIO12
#define LCD_RESET		13			//GPIO13
#define LCD_SPI			spi1

#endif		// __CONSTANTS_H__


