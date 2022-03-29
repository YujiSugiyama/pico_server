#include <stdio.h>
#include <string>

#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include "hardware/rtc.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#include "constants.h"
#include "esp32.h"
#include "http.h"
#include "server.h"
#include "core1.h"
#include "LCDdriver.h"
#include "graphlib.h"
#include "disp.h"

/*******************************************************************************************************************************************
	Reboot at the initial failure
*******************************************************************************************************************************************/
void reboot_board(void)
{
	printf("[MAIN]: Rebooting board\n");
	sleep_ms(1000);

	if (watchdog_caused_reboot()) {
		sleep_ms(1000);
	}

	watchdog_enable(10, false);
	while (1);
}

/*******************************************************************************************************************************************
	Init 3.3V power to PWM
*******************************************************************************************************************************************/
void power3v3_init(void)
{
// Set PWM mode for 3.3V regulator
	gpio_init(23);
	gpio_set_dir(23, GPIO_OUT);
	gpio_put(23, 1);
}

/*******************************************************************************************************************************************
	Init GPIO, STDIO_UART
*******************************************************************************************************************************************/
void io_init(void)
{
// Init UART0 for STDIO
	gpio_set_function(STDIO_TX_PIN, GPIO_FUNC_UART);
	gpio_set_function(STDIO_RX_PIN, GPIO_FUNC_UART);
	gpio_pull_up(STDIO_RX_PIN);
	uart_init(uart0, STDIO_BAUD_RATE);
	uart_set_fifo_enabled(STDIO_UART, true);
	sleep_ms(100);

// Init RPI pico on-board LED(Green)
	gpio_init(STATUS_LED_PIN);
	gpio_set_dir(STATUS_LED_PIN, GPIO_OUT);
	gpio_put(STATUS_LED_PIN, 1);

// Init buttons on bread board
	gpio_init(BTN0_GPIO);
	gpio_init(BTN1_GPIO);
	gpio_set_dir(BTN0_GPIO, GPIO_IN);
	gpio_set_dir(BTN1_GPIO, GPIO_IN);
	gpio_pull_up(BTN0_GPIO);
	gpio_pull_up(BTN1_GPIO);

// Init LEDs on bread board
	gpio_init(RED_LED);
	gpio_set_dir(RED_LED, GPIO_OUT);
	gpio_put(RED_LED, 1);
	gpio_init(GREEN_LED);
	gpio_set_dir(GREEN_LED, GPIO_OUT);
	gpio_put(GREEN_LED, 1);
	gpio_init(BLUE_LED);
	gpio_set_dir(BLUE_LED, GPIO_OUT);
	gpio_put(BLUE_LED, 1);
}

/*******************************************************************************************************************************************
	Init Sound PWM port(GPIO10)
*******************************************************************************************************************************************/
void sound_init(int sw)
{
	uint	pwm_slice_num;

	// サウンド用PWM設定
	if(sw){
		gpio_set_function(SOUNDPORT, GPIO_FUNC_PWM);
		pwm_slice_num = pwm_gpio_to_slice_num(SOUNDPORT);
		pwm_set_wrap(pwm_slice_num, PWM_WRAP-1);
		// duty 50%
		pwm_set_chan_level(pwm_slice_num, PWM_CHAN_A, PWM_WRAP/2);
	} else{
		gpio_set_function(SOUNDPORT, GPIO_FUNC_SIO);
		gpio_init(SOUNDPORT);
		gpio_set_dir(SOUNDPORT, GPIO_OUT);
		gpio_put(SOUNDPORT, false);
	}
}

/*******************************************************************************************************************************************
	Init LCD display(ili9341)
*******************************************************************************************************************************************/
void disp_init(void)
{	// 液晶用ポート設定
	// Enable SPI 0 at 40 MHz and connect to GPIOs
	spi_init(LCD_SPI, 40000 * 1000);
	gpio_set_function(14, GPIO_FUNC_SPI);
	gpio_set_function(15, GPIO_FUNC_SPI);

	gpio_init(LCD_CS);
	gpio_put(LCD_CS, 1);
	gpio_set_dir(LCD_CS, GPIO_OUT);
	gpio_init(LCD_DC);
	gpio_put(LCD_DC, 1);
	gpio_set_dir(LCD_DC, GPIO_OUT);
	gpio_init(LCD_RESET);
	gpio_put(LCD_RESET, 1);
	gpio_set_dir(LCD_RESET, GPIO_OUT);

	init_graphic();					// 液晶利用開始
//	lcd_writeComm(0x37);			// 画面中央にするためスクロール設定
//	lcd_writeData2(272);
}

/*******************************************************************************************************************************************
	Init ESP32
*******************************************************************************************************************************************/
bool esp32_init(void)
{
	try {
		clearATBuffer(1000);

		if (sendATCommandOK("RST", 1000, false)) {
			// Just dummy read because ESP32 is just RESET state on unstable response.
			sendATCommand("", 1000, "\r\n", false);
			sendATCommand("", 1000, "\r\n", false);
			sendATCommand("", 1000, "\r\n", false);
			sendATCommand("", 1000, "\r\n", false);

			std::string sdkVersion = getSDKVersion();
			if (sdkVersion.empty()) {
				printf("[ESP32]: Failed to get version\n");
				return false;
			}

			printf("[ESP32]: SDK Version: %s\n", sdkVersion.c_str());

			uart_puts(UART_ID, "ATE0\r\n");
			if (!sendATCommandOK("", 1000, false)) {
				printf("[ESP32]: Failed to set ATE0\n");
				return false;
			}

			return true;
		} else {
			printf("[ESP32]: Failed to connect\n");
			return false;
		}
	} catch (...) {
		printf("[ESP32]: ESP Init Exception\n");
		return false;
	}
}

/*******************************************************************************************************************************************
	Init UART for ESP32
*******************************************************************************************************************************************/
bool uart_init(void)
{
	try {
		gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
		gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
		gpio_set_function(UART_CTS_PIN, GPIO_FUNC_UART);
		gpio_set_function(UART_RTS_PIN, GPIO_FUNC_UART);
		uart_init(UART_ID, BAUD_RATE);

		gpio_pull_up(UART_RX_PIN);
		gpio_pull_up(UART_CTS_PIN);

		uart_set_translate_crlf(UART_ID, false);
		uart_set_hw_flow(UART_ID, true, true);
		uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
		uart_set_fifo_enabled(UART_ID, true);

		return true;
	} catch (...) {
		printf("[ESP32]: UART Exception\n");
		return false;
	}
}

/*******************************************************************************************************************************************
	Main for the test program
*******************************************************************************************************************************************/
int main(void)
{
// 3.3V PWM
	power3v3_init();

// Should reset core1 also
	multicore_reset_core1();

// Init STDIO
	stdio_init_all();
	sleep_ms(100);
	io_init();

// Init RTC
	rtc_init();

// Init LCD and Sound
	disp_init();
	sound_init(0);

// Init LED
	sleep_ms(500);
	gpio_put(STATUS_LED_PIN, 0);
	printf("\n\n\n------------ Boot up RPi-Pico-Server ------------\n");
	gpio_put(STATUS_LED_PIN, 0);
	gpio_put(RED_LED, 0);
	gpio_put(GREEN_LED, 0);
	gpio_put(BLUE_LED, 0);

	if(!uart_init()){
		reboot_board();
		return -1;
	}

	gpio_put(STATUS_LED_PIN, 1);
	sleep_ms(1000);
	if(uart_is_readable(STDIO_UART)){
		printf("\n*Manual WiFi mode(ctl-Q to quit)*\n");
		manualESP32();
	}
	gpio_put(STATUS_LED_PIN, 0);

	if(!esp32_init()){
		reboot_board();
		return -1;
	}

	if(!server_init()){
		printf("[Server]: Failed to start server\n");
		reboot_board();
		return -1;
	}

// Now, server ready!!!
	gpio_put(STATUS_LED_PIN, 1);
	disp_ascii();
	disp_icon();

//	multicore_launch_core1(led_process);
	multicore_launch_core1_with_stack(core1_entry, (uint32_t *)PICO_CORE1_STACK_BOTTOM, PICO_CORE1_STACK_SIZE);
	dispatch_server();
}