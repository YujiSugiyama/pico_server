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
#include "http.h"
#include "server.h"
#include "core1.h"

/*******************************************************************************************************************************************
	Network Server
*******************************************************************************************************************************************/
bool server_init(void)
{
	try {
		char sendBuffer[SENDBUFFERLEN];

		if (!sendATCommandOK("CWMODE=1", 2000, false)) {
			return false;
		}
		printf("[Server]: ESP32 in Station Mode\n");

		snprintf(sendBuffer, SENDBUFFERLEN, "CWHOSTNAME=\"%s\"", HOSTNAME);
		if (!sendATCommandOK(sendBuffer, 2000, false)) {
			return false;
		}
		printf("[Server]: Hostname set to %s\n", HOSTNAME);

		const int wifiState = getWifiState();
		if (wifiState <= 0) {
			printf("\n************************** Network setting is required **************************\n");
			printf("Connect to AP(maybe send AT+CWJAP=\"SSID\",\"Passphrase\" and ctl-Q to quit)*\n");
			printf("To set IP address : Ex. AT+CIPSTA=\"192.168.1.xx\",\"192.168.1.yy\",\"255.255.255.0\"\n");
			printf("To get IP address : Ex. AT+CIFSR\n");
			printf("Anyway you need to set network addresses by manual with AT commands\n");
			printf("*********************************************************************************\n\n");
			manualESP32();
		} else {
			printf("[Server]: Connecting to last WiFi configuration\n");
			if(!sendATCommandOK("CWJAP", 30 * 1000, false)) {
				return false;
			}
		}

		const std::string ipAddress = getIp();
		if (ipAddress == "0.0.0.0" || ipAddress.empty()) {
			printf("[Server]: Failed to get IP address\n");
			return false;
		}
		printf("[Server]: Connected to WiFi\n");
		printf("[Server]: IP Address: '%s'\n", ipAddress.c_str());

		printf("[Server]: Setting the UTC time to RTC\n");
		if (!setupUTCTime()) {
			printf("[Server]: Failed to set the UTC time to RTC\n");
			return false;
		}
		if (1324512000000 > getMsecFrom1970()) {
			printf("[Server]-[RTC]: Check Failed\n");
			return false;
		}

		if (!sendATCommandOK("CIPMUX=1", 2000, false)) {
			return false;
		}
		snprintf(sendBuffer, SENDBUFFERLEN, "CIPSERVER=1,%s", SOCKET_PORT);
		if (!sendATCommandOK(sendBuffer, 2000, false)) {
			return false;
		}
		printf("[Server]: Server running on port '%s'\n", SOCKET_PORT);

		if (!sendATCommandOK("CIPSTO=180", 1000, true)) {
			printf("[Server]: Failed to set client timeout\n");
		} else {
			printf("[Server]: Client timeout set to 180s\n");
		}

#if 0		// ujDebug
		const std::string one_month_signature = generate_signature(31 * 24 * 60 * 60 * (u_int64_t)1000);
		const std::string b64_payload = getParam(0, '.', '\0', one_month_signature);
		const std::string b64_signature = getParam(1, '.', '\0', one_month_signature);

#ifdef IS_DEBUG_MODE
		printf("\n-------------------------------------------\n");
		printf("Debug Signature: \n%s.\n%s\n", b64_payload.c_str(), b64_signature.c_str());
		printf("-------------------------------------------\n\n");
#endif
		gpio_put(STATUS_LED_PIN, 1);
#endif

		return true;
	} catch (...) {
		return false;
	}
}

void dispatch_server(void)
{
	int ipd_ready;
	int core1_busy = 0;
	std::string core1_sock;

	printf("[Server]: Serving clients\n");

	while(1) {
		ipd_ready = 0;

		if(uart_is_readable(UART_ID) && recvIPD(250)){			// IPD received?
			ipd_ready = 1;
		}

		if(ipd_ready){		// IPD receive
			const std::string response(recvBuffer);

			const int conn = response.find("CONNECT");
			if (conn != std::string::npos) {
				printf("[Server]: Client '%c' connected\n", response[conn - 2]);
			}

			const int cls = response.find("CLOSED");
			if (cls != std::string::npos) {
				printf("[Server]: Client '%c' disconnected\n", response[cls - 2]);
			}

			const char* ipd = strstr(recvBuffer, "+IPD");
			if (ipd != NULL) {
				printf("---------------------------------------------------------------------\n");
				printf("[IPD]: %s", ipd);
				const std::string sock = getParam(1, ',', '\0', ipd);		// cut "+IPD,"
				const std::string len = getParam(2, ',', ':', ipd);			// cut "id,"
				std::string line = getParam(2, ',', '\r', ipd);				// get "len:........\r" thus HEADR
				line = getParam(1, ':', '\r', line);
//				printf("Request = %s", (line+"\n").c_str());

				const std::string srv = getParam(0, ' ', '\0', line);
				int alive;
				if(srv == "GET" || srv == "POST"){
					if(getHeaderContentStr(recvBuffer,"Connection: ") == "close")
						alive = 0;
					else
						alive = 1;
				}

				if(srv == "GET"){
					http_server(sock, line, NULL, alive);
				} else if(srv == "POST"){
					int blen = getHeaderContentValue(recvBuffer,"Content-Length: ");
					char* body = recvBody(blen, 1000);
					printf("Body = %s\n", body);
					http_server(sock, line, body, alive);
				} else if(srv == "ECHO" || srv == "echo"){
					line = getParam(1, ' ', '\r', line);
					printf("[Server(%s)]: ", sock.c_str());
					printf("serve='%s' command='%s'\n", srv.c_str(), line.c_str());
					echo_server(sock, line);
				} else if(srv == "LED" || srv == "led"){
					const std::string cmd = getParam(1, ' ', '\0', line);
					const std::string pm = getParam(2, ' ', '\r', line);
					printf("[Server(%s)]: ", sock.c_str());
					printf("serve='%s' command='%s' param='%s'\n", srv.c_str(), cmd.c_str(), pm.c_str());
					led_server(sock, cmd, pm);
				} else if(!core1_thrd && (srv == "THRD" || srv == "thrd")){
					const std::string thrd = getParam(1, ' ', '\0', line);
					const std::string pm = getParam(2, ' ', '\r', line);
					printf("[Server(%s)]: ", sock.c_str());
					printf("serve='%s' thread='%s' param='%s'\n", srv.c_str(), thrd.c_str(), pm.c_str());
					multicore_fifo_push_timeout_us((uintptr_t)sock.c_str(), 1);
					multicore_fifo_push_timeout_us((uintptr_t)thrd.c_str(), 1);
					multicore_fifo_push_timeout_us((uintptr_t)pm.c_str(), 1);
					// must wait for core1 to receive 3 fifos, as 3 fifo data are queued in stack. Otherwise, fifo data must be in static memory
					uint32_t ret = multicore_fifo_pop_blocking();
					sleep_ms(1);			// short sleep is needed after fifo blocking. I do not know the reason...
					if(!ret){
						printf("[Server]: FIFO is not working\n");
					} else{
						// It is not allowed to call ESP32 from core1, because return data from ESP32 does not have the sock ID.
						sendMessage(sock.c_str(), "Core1 is busy\r\n");
						core1_busy = 1;
						core1_sock = sock;
					}	
				} else if(srv == "KILL" || srv == "kill"){
					const std::string pm = getParam(1, ' ', '\r', line);
					printf("[Server(%s)]: ", sock.c_str());
					printf("serve='%s' param='%s'\n", srv.c_str(), pm.c_str());
					mutex_enter_blocking(&mtx_core1_thrd);
					core1_thrd = 0;
					mutex_exit(&mtx_core1_thrd);
				}
			}
		}

		if(core1_busy == 1 && core1_thrd == 0){
			// It is not allowed to call ESP32 from core1
			sendMessage(core1_sock.c_str(), "Core1 is free\r\n");
			core1_busy = 0;
		}
	}
}

/*******************************************************************************************************************************************
	Echo server: Executed on core0
*******************************************************************************************************************************************/
void echo_server(const std::string& sock, const std::string& inp)
{
	try{
		std::string msg(inp);
		printf("[Server(%s)]: echo='%s'\n", sock.c_str(), msg.c_str());
		msg = msg + "\r\n";
		sendMessage(sock.c_str(), msg.c_str());
	} catch(...){
		printf("[Server]:Echo server error\n");
	}
}

/*******************************************************************************************************************************************
	LED on/off server: Executed on core0
*******************************************************************************************************************************************/
void led_server(const std::string& sock, const std::string& color, const std::string& pm)
{
	try{
		const std::string stat = getParam(0, ' ', '\0', pm);
		printf("[Server(%s)]: color='%s' stat='%s'\n", sock.c_str(), color.c_str(), stat.c_str());

		if(color=="RED" || color=="red"){
			if(stat == "1"){
				gpio_put(RED_LED, 1);
				printf("[Serever]: RED_LED ON\n"); 
			} else{
				gpio_put(RED_LED, 0);
				printf("[Serever]: RED_LED OFF\n");
			}
		} else if(color=="GREEN" || color=="green"){
			if(stat == "1"){
				gpio_put(GREEN_LED, 1);
				printf("[Serever]: GREEN_LED ON\n"); 
			} else{
				gpio_put(GREEN_LED, 0);
				printf("[Serever]: GREEN_LED OFF\n"); 
			}
		} else if(color=="BLUE" || color=="blue"){
			if(stat == "1"){
				gpio_put(BLUE_LED, 1);
				printf("[Serever]: BLUE_LED ON\n"); 
			} else{
				gpio_put(BLUE_LED, 0);
				printf("[Serever]: BLUE_LED OFF\n"); 
			}
		}
	} catch(...){
		printf("[Server]:LED on/off server error\n");
	}
}

