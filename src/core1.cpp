#include <stdio.h>
#include <cstring>
#include <string>
#include <ctime>
#include <iomanip>

#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"

#include "constants.h"
#include "esp32.h"
#include "core1.h"

int	core1_thrd = 0;
mutex_t mtx_core1_thrd;

/*******************************************************************************************************************************************
	Core1 process main: Executed on core1
*******************************************************************************************************************************************/
void core1_entry(void)
{
	mutex_init(&mtx_core1_thrd);

	while(1){
		const char* sock = (char *)multicore_fifo_pop_blocking();
		const char* thrd = (char *)multicore_fifo_pop_blocking();
		const std::string func(thrd);
		const char* pm = (char *)multicore_fifo_pop_blocking();
		sleep_ms(1);			// short sleep is needed after fifo blocking. I do not know the reason...

		if(func == "LED" || func == "led"){
			mutex_enter_blocking(&mtx_core1_thrd);
			core1_thrd = 1;
			mutex_exit(&mtx_core1_thrd);
			multicore_fifo_push_timeout_us((uint32_t)1, 1);

			const std::string tm = getParam(0, ' ', '\0', pm);
			const std::string lp = getParam(1, ' ', '\0', pm);
			led_process(sock, atoi(tm.c_str()), atoi(lp.c_str()));

			mutex_enter_blocking(&mtx_core1_thrd);
			core1_thrd = 0;
			mutex_exit(&mtx_core1_thrd);
		} else{
			multicore_fifo_push_timeout_us((uint32_t)1, 1);
		}
	}
}

/*******************************************************************************************************************************************
	LED process server: Executed on core1
*******************************************************************************************************************************************/
void led_process(const char *sock, int interval, int loop)
{
	try{
		printf("[Server(%s)]: interval='%d' loop='%d'\n", sock, interval, loop);
		while(core1_thrd){
			gpio_put(RED_LED, 1);
			sleep_ms(interval);
			gpio_put(RED_LED, 0);
			gpio_put(GREEN_LED, 1);
			sleep_ms(interval);
			gpio_put(GREEN_LED, 0);
			gpio_put(BLUE_LED, 1);
			sleep_ms(interval);
			gpio_put(BLUE_LED, 0);
			if(loop == 1)	break;
			else if(loop > 0) loop--;
		}
	} catch(...){
		printf("[Server]:LED on/off server error\n");
	}
}

