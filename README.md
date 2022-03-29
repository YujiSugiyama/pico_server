# Raspberry Pi Pico Pin assignment
PIN 1/GPIO0	: UART0_Tx	// To USB-COM<br/>
PIN 2/GPIO1	: UART0_Rx	// To USB-COM<br/>
PIN 3/		: GND<br/>

PIN 6/GPIO4	: UART1_Tx	// To ESP32_Tx (PIN31/IO19)<br/>
PIN 7/GPIO5	: UART1_Rx	// To ESP32_Rx (PIN36/IO22)<br/>
PIN 8/		: GND<br/>
PIN 9/GPIO6	: UART1_CTS	// To ESP32_RTS(PIN13/IO14)<br/>
PIN10/GPIO7	: UART1_RTS	// To ESP32_CTS(PIN23/IO15)<br/>

PIN14/GPIO10	: Speaker IO out// To Speaker (Active High)<br/>
PIN15/GPIO11	: CS IO out	// To 240x320LCD_CS(PIN3)<br/>
PIN16/GPIO12	: D/C IO out	// To 240x320LCD_D/C(PIN5)<br/>
PIN17/GPIO13	: RESET IO out	// To 240x320LCD_RESET(PIN4)<br/>
PIN18/		: GND		// To 240x320GND<br/>
PIN19/GPIO14	: SPI1_CLK 	// To 240x320LCD_SCK(PIN7)<br/>
PIN20/GPIO15	: SPI1_MOSI(TX)	// To 240x320LCD_MOSI(PIN6)<br/>

PIN26/GPIO20	: BLUE_LED<br/>
PIN31/GPIO26	: GREEN_LED<br/>
PIN34/GPIO28	: RED_LED<br/>

## How to use
Come on IP-addr:54000/index.html with your browser like Chrome<br/>
	Simple HTTP server supporting GET/POST.<br/>
	The input from operator to be replied on POST method.<br/>
Connect to IP-arrd:54000 and send one command of the follows;<br/>
	ECHO xxx yyy zzz ...  => echo "xxx yyy zzz ..."<br/>
	LED RED/GREEN/BLUE 1/0 => LED on or off of RED(GPIO28) or BLUE(GPIO20) or GREEN(GPIO26)<br/>
	THRD LED interval num-blink => LED blink on core1, interval(msec) num-blink(0 then infinite)<br/>

## Breadboard
![](pico_server.JPG)
