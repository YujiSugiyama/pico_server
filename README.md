PIN 1/GPIO0	: UART0_Tx	  // To USB-COM 
PIN 2/GPIO1	: UART0_Rx	  // To USB-COM
PIN 3/	: GND

PIN 6/GPIO4	: UART1_Tx	  // To ESP32_Tx (PIN31/IO19)
PIN 7/GPIO5	: UART1_Rx	  // To ESP32_Rx (PIN36/IO22)
PIN 8/	: GND
PIN 9/GPIO6	: UART1_CTS	  // To ESP32_RTS(PIN13/IO14) 
PIN10/GPIO7	: UART1_RTS	  // To ESP32_CTS(PIN23/IO15) 

PIN14/GPIO10: Speaker IO out	// To Speaker (Active High)
PIN15/GPIO11: CS IO out			  // To 240x320LCD_CS(PIN3)
PIN16/GPIO12: D/C IO out		  // To 240x320LCD_D/C(PIN5)
PIN17/GPIO13: RESET IO out		// To 240x320LCD_RESET(PIN4)
PIN18/	: GND					        // To 240x320GND
PIN19/GPIO14: SPI1_CLK 			  // To 240x320LCD_SCK(PIN7)
PIN20/GPIO15: SPI1_MOSI(TX)		// To 240x320LCD_MOSI(PIN6)

PIN26/GPIO20	: BLUE_LED(PWM)
PIN31/GPIO26	: GREEN_LED (PWM)
PIN34/GPIO28	: RED_LED (PWM)

Come on IP-addr:54000/index.html with your browser like Chrome
	Simple HTTP server supporting GET/POST.
	The input from operator to be replied on POST method.
Connect to IP-arrd:54000 and send one command of the follows;
	ECHO xxx yyy zzz ...  => echo "xxx yyy zzz ..."
	LED RED/GREEN/BLUE 1/0 => LED on or off of RED(GPIO28) or BLUE(GPIO20) or GREEN(GPIO26)
	THRD LED interval num-blink => LED blink on core1, interval(msec) num-blink(0 then infinite)
