#include <stdio.h>
#include <cstring>
#include <string>
#include <time.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "pico/util/datetime.h"
#include "pico/multicore.h"
#include "hardware/uart.h"
#include "hardware/rtc.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#include "constants.h"
#include "esp32.h"

char recvBuffer[RECVBUFFERLEN];
char _recvBuffer[RECVBUFFERLEN];

/*******************************************************************************************************************************************
	AT-Commands
*******************************************************************************************************************************************/
static int getATdata(int buffPtr)
{
	try {
		bool tooLong = false;
		while (uart_is_readable(UART_ID)) {
			recvBuffer[buffPtr] = uart_getc(UART_ID);

			if (buffPtr >= RECVBUFFERLEN - 1) {
				tooLong = true; 
			} else {
				buffPtr++;
			}
		}

		recvBuffer[buffPtr] = 0;
		return (tooLong) ? -1 : buffPtr;
	} catch (...) {
		printf("[Server]:[ERROR]: while getting AT data\n");
		return -2;
	}
}

int recvIPD(int64_t allowTimeMs)
{
	try {
		int bufp = 0;
		memset(recvBuffer, 0, RECVBUFFERLEN);

		for(int i=0; i<IPD_TRY; i++){
			absolute_time_t start = get_absolute_time();
			allowTimeMs *= MICROS_MS;
			while (absolute_time_diff_us(start, get_absolute_time()) < allowTimeMs) {
				// Received data to be put to the tail of buffer as long as staying the while loop within allowTimeMs
				bufp = getATdata(bufp);
				if (bufp == -2) {
					return false;
				} else if (bufp == -1) {
					printf("[AT-Command]-[%s]: Command is too long: (%s)\n", recvBuffer);
					return false;
				} else if (strstr(recvBuffer, "\r\n\r\n") != NULL){
					if (strstr(recvBuffer, "+IPD") != NULL){
						return 1;
					} else{
						return 2;
					}
				}
			}
		}
	} catch (...) {
		printf("[IPD]: Exception\n");
	}
	return false;
}

char* recvBody(int blen, int64_t allowTimeMs)
{
	try{
		int bufp = 0;
		char *body;

		_recvBuffer[0] = '\0';
		snprintf(_recvBuffer, RECVBUFFERLEN, "%s%s", _recvBuffer, recvBuffer);
		if((body = strstr(_recvBuffer, "\r\n\r\n")) == NULL)	return NULL;
		body += 4;									// remove \r\n\r\n
		int ofs = strlen(recvBuffer);				// offset to check buffer too long
		bufp = ofs;
		for(int i=0; i<IPD_TRY; i++){
			absolute_time_t start = get_absolute_time();
			allowTimeMs *= MICROS_MS;
			while (absolute_time_diff_us(start, get_absolute_time()) < allowTimeMs) {
				// Received data to be put to the tail of buffer as long as staying the while loop within allowTimeMs
				bufp = getATdata(bufp);
				if (bufp == -2) {
					return NULL;
				} else if (bufp == -1) {
					printf("[AT-Command]-[%s]: Command is too long\n", recvBuffer);
					return NULL;
				} else if((bufp-ofs) >= blen){
					snprintf(_recvBuffer, RECVBUFFERLEN, "%s%s", _recvBuffer, recvBuffer+ofs);
					return body;
				}
			}
		}
	} catch (...) {
		printf("[IPD]: Exception\n");
	}
	return NULL;
}

bool sendATCommand(
	const char* command, 			// If it is "", reads request data from client (Not response).
	int64_t allowTimeMs, 			// 
	const char* successMsg, 		// "\r\nOK\r\n" is normal command
	const bool& surpressOutput		// Does not print the response
) {
	try {
		absolute_time_t start = get_absolute_time();
		allowTimeMs *= MICROS_MS;

		char sendBuffer[SENDBUFFERLEN];
		bool runCommand = true;
		int buffPtr = 0;
	
		if (strlen(command) > 0) {
			snprintf(sendBuffer, SENDBUFFERLEN, "AT+%s\r\n", command);
		}

		memset(recvBuffer, 0, RECVBUFFERLEN);

		while (absolute_time_diff_us(start, get_absolute_time()) < allowTimeMs) {
			if (runCommand && strlen(command) > 0) {
				uart_puts(UART_ID, sendBuffer); 
				printf("[AT-Command]-[%s]: Sending Command\n", command);
	
				runCommand = false;
				buffPtr = 0;
			}

			// Received data to be put to the tail of buffer as long as staying the while loop within allowTimeMs
			buffPtr = getATdata(buffPtr);

			if (buffPtr == -2) {
				return false;
			}

			if (buffPtr == -1) {
				if (!surpressOutput) {
					printf("[AT-Command]-[%s]: Response to command is too long: (%s)\n", command, recvBuffer);
				}
				return false;
			}

			if ((strlen(successMsg) > 0) && (strstr(recvBuffer, successMsg) != NULL)) {
				return true;
			}

//			if (isIPD && strstr(recvBuffer, "+IPD") != NULL && strstr(recvBuffer, "\r\n\r\n") != NULL) {
//				return true;
//			}

			if (strstr(recvBuffer, "busy p...") != NULL) {
				if (!surpressOutput) {
					printf("[AT-Command]-[%s]: ESP32 busy, retry command\n", command);
				}

				memset(recvBuffer, 0, RECVBUFFERLEN);
				sleep_ms(100);
				runCommand = true;
			}

			if (strstr(recvBuffer, "link is not valid") != NULL) {
				return false;
			}
		}

		// Time over handling
//		printf("+++");printf(recvBuffer);printf("+++\n");
		if (strlen(successMsg) > 0) {
			if (buffPtr > 0) {
				if (strstr(recvBuffer, "busy p...") != NULL) {
					if (!surpressOutput) {
						printf("[AT-Command]-[%s]: Timed out waiting on ESP32 busy\n", command);
					}
					return false;
				}

				if (strlen(recvBuffer) == 0) {
					if (!surpressOutput) {
						printf("[AT-Command]-[%s]: No ESP32 response\n", command);
					}
					return false;
				}

				if (strstr(recvBuffer, "busy s...") != NULL) {
					if (!surpressOutput) {
						printf("[AT-Command]-[%s]: ESP32 unable to receive\n", command);
					}
					return false;
				}

				if (strstr(recvBuffer, "ERROR") != NULL) {
					if (!surpressOutput) {
						printf("[AT-Command]-[%s]: ESP32 Error\n", command);
					}
					return false;
				}

				if (!surpressOutput) {
					printf("[AT-Command]-[%s]: Command got unexpected response(%s)\n", command, recvBuffer);
				}
				return false;
			}
			if (!surpressOutput) {
				printf("[AT-Command]-[%s]: Timed out waiting for ESP32 response\n", command);
			}
			return false;
		}

		return (buffPtr > 0) ? true : false;			// if data is not received completely(in the middle), returns true.
	} catch (...) {
		printf("[AT-Command]-[%s]: Exception\n", command);
		return false;
	}
}

bool sendATCommandOK(const char* command, const int64_t allowTimeMs, const bool& surpressOutput) {
//	return sendATCommand(command, allowTimeMs, "\r\nOK\r\n", surpressOutput);
	return sendATCommand(command, allowTimeMs, "OK\r\n", surpressOutput);
}

bool clearATBuffer(const int64_t allowTimeMs) {
	return sendATCommand("", allowTimeMs, "", false);
}

// delim2=='\0'なら、delimで区切られた文字列において、pos番目(先頭は0番目から始まる)の者を得る
// delim2!='\0'なら、delimで区切られた文字列において、pos番目(先頭は0番目から始まる)の者でdelim2までを得る
std::string getParam(const int pos, const char delim, const char delim2, const std::string from)
{
	try {
		std::string line;
		std::istringstream iss_input(from);

		std::getline(iss_input, line, delim);			// line <= iss_input[0] ~ delim直前までの文字列, iss_inputはdelim直後まで進む
		for (int i = 0; i < pos-1; i++)
			std::getline(iss_input, line, delim);		// line <= delimをpos-1個進み、その直前までの文字列, iss_inputはdelimがpos-1個分直後まで進む
		if (delim2 == '\0' && pos > 0) {
			std::getline(iss_input, line, delim);		// line <= delimをpos個進み、その直前までの文字列
		} else if(pos > 0){
			line = from.substr((int)iss_input.tellg());	// iss_input.tellg()は現在のstreamポインタ位置を得る。ポインタ位置から最後までの文字列を得る
			std::getline(std::istringstream(line), line, delim2);	// delim2直線までの文字れるを得る
		}
		return line;
	} catch (...) {
		printf("[Server]:[ERROR]: while getting param\n");
		return "";
	}
}

std::string getParamEx(const char* start, const char end, const char* from)
{
	try {
		const char* c_line = strstr(from, start);
		if (c_line == NULL) {
			return "";
		}

		std::string line(c_line);
//		line.erase(0, 1);
		line.erase(0, strlen(start));

		if (end != '\0') {
			std::getline(std::istringstream(line), line, end);
		}

		return line;
	} catch (...) {
		printf("[Server]:[ERROR]: while getting c_param\n");
		return "";
	}
}

#define SENDLEN	512
//#define SENDLEN	16
void sendResponse(const char* sock, const char* statusCode, const char *type, const char* body, int len, int alive)
{
	try {
		char sendATBuffer[SENDBUFFERLEN];
		char contentBuffer[SENDBUFFERLEN];
		int	ret;

		snprintf(
			contentBuffer, SENDBUFFERLEN, 
			"HTTP/1.1 %s\r\n%s\r\n%s%s\r\n%s%d\r\n\r\n",
			statusCode, 
			"Host: RPi-Pico",
			"Content-type: ", type, 
			"Content-Length: ", len
		);

		// Response + Header
		snprintf(sendATBuffer, SENDBUFFERLEN, "CIPSENDEX=%s,%d", sock, strlen(contentBuffer));
		sendATCommand(sendATBuffer, 250, ">", true);
		uart_puts(UART_ID, contentBuffer);
		printf(contentBuffer); printf("\n");
		ret = sendATCommandOK("", 3000, true);
		if(ret == false)	printf("NG---\n");

		// Body
		int count = 0;
		int size;
		while(len > 0){
			if(len > SENDLEN)	size = SENDLEN;
			else				size = len;
			snprintf(sendATBuffer, SENDBUFFERLEN, "CIPSEND=%s,%d", sock, size);
			sendATCommand(sendATBuffer, 250, ">", true);
			for(int i=0; i<size; i++){
				char c;
				uart_putc(UART_ID, c = body[count++]);
				if(c == 0x5c && i != size-1)						// last data is not escaped
					uart_putc(UART_ID, 0x5c);
//				printf("0x%02x ", c);
			}
			len -= size;
			ret = sendATCommandOK("", 250, true);
			if(ret == false)	printf("(%d)NG---\n", count);
			else				printf("(%d)DONE---\n", count);
		}
#if 0
//		len += 2;
		int count = 0;
		int size, i, esp;
		while(len > 0){
			if(len > SENDLEN)	size = SENDLEN;
			else				size = len;
			for(i=0, esp=0; i<size; i++){
				char c;
				c = contentBuffer[i+esp] = body[count++];
				if((i != (size-1)) && (c == 0x5c)){				// '\' must be escaped
					esp++;
					contentBuffer[i+esp] = 0x5c;
				}
			}
			snprintf(sendATBuffer, SENDBUFFERLEN, "CIPSEND=%s,%d", sock, size);
			sendATCommand(sendATBuffer, 250, ">", true);
			for(i=0; i<size+esp; i++){
				char c;
				uart_putc(UART_ID, c = contentBuffer[i]);
				printf("0x%02x ", c);
			}
			len -= size;
			ret = sendATCommandOK("", 250, true);
			if(ret == false)	printf("(%d)NG---\n", count);
			else				printf("(%d)DONE---\n", count);
		}
#endif

		if(alive == 0){
			snprintf(sendATBuffer, SENDBUFFERLEN, "CIPCLOSE=%s", sock);
			sendATCommandOK(sendATBuffer, 250, true);
		}
	} catch (...) {
		printf("[HTTP-Server]: Error sending response data\n");
	}
}

void sendMessage(const char* sock, const char* message)
{
	try {
		char data[SENDBUFFERLEN];
		char cmd[SENDBUFFERLEN];

		snprintf(data, SENDBUFFERLEN, "%s%s", message, "\0");
		printf("Echo is '%s'\n", message);
		snprintf(cmd, SENDBUFFERLEN, "CIPSENDEX=%s,%d", sock, strlen(message));
		if(sendATCommandOK(cmd, 250, true) && sendATCommand("", 250, ">", true)){	// AT+CIPSEND=0,LEN and rceive ">"
			uart_puts(UART_ID, data);
			sendATCommandOK("", 250, true);
		}
	} catch (...) {
		printf("[HTTP-Server]: Error sending response\n");
	}
}

/*******************************************************************************************************************************************
	Aggregated Commands
*******************************************************************************************************************************************/
std::string getIp(void)
{
	try {
		if (!sendATCommandOK("CIFSR", 2000, false)) {
			return "";
		}

		const char* buff = strstr(recvBuffer, "STAIP");
		if (buff == NULL) {
			return "";
		}

		return getParam(1, '"', '\0', buff);
	} catch (...) {
		printf("[Server]:[ERROR]: while getting IP\n");
		return "";
	}
}

std::string getSDKVersion(void)
{
	try {
		if (!sendATCommandOK("GMR", 1000, false)) {
			return "";
		}

		const char* buff = strstr(recvBuffer, "SDK");
		if (buff == NULL) {
			return "";
		}

		return getParam(1, ':', '\r', buff);
	} catch (...) {
		printf("[Server]:[ERROR]: while getting sdk version\n");
		return "";
	}
}

/*
 * -1: Error
 * 0: ESP station has not started any Wi-Fi connection.
 * 1: ESP station has connected to an AP, but does not get an IPv4 address yet.
 * 2: ESP station has connected to an AP, and got an IPv4 address.
 * 3: ESP station is in Wi-Fi connecting or reconnecting state.
 * 4: ESP station is in Wi-Fi disconnected state.
 */
int getWifiState(void)
{
	try {
		if (!sendATCommandOK("CWSTATE?", 1000, false)) {
			return -1;
		}

		const char* buff = strstr(recvBuffer, "+CWSTATE:");
		if (buff == NULL) {
			return -1;
		}

		const std::string s_state = getParam(1, ':', ',', buff);
		if (s_state.empty()) {
			return -1;
		}

		return stoi(s_state);
	} catch (...) {
		printf("[Server]:[ERROR]: while getting wifi state\n");
		return -1;
	}
}

bool setupUTCTime(void) 
{
	if (!sendATCommandOK("CIPSNTPCFG=1,0,\"pool.ntp.org\"", 2000, false)) {
		return false;
	}

	int retries = 10;
	do {
		sleep_ms(1000);

		sendATCommandOK("CIPSNTPTIME?", 2000, false);
		if (strstr(recvBuffer, "1970") == NULL) {
			const std::string s_date = getParamEx(":", '\r', recvBuffer);
			if (!s_date.empty()) {
				printf("[Server]-[INFO]: UTC time: '%s'\n", s_date.c_str());
				std::istringstream ss(s_date);

				if (!ss.fail()) {
//					std::string s;
//					while (ss >> s) {
//						std::cout << s << std::endl;
//					}
					std::tm t = {};
					ss >> std::get_time(&t, "%a %b %d %H:%M:%S %Y");
					std::time_t jp_time = std::mktime(&t) + 9*60*60;
					gmtime_r(&jp_time, &t);
					datetime_t dt = {
						.year  = (int16_t)(t.tm_year + 1900),
						.month = (int8_t)(t.tm_mon + 1),
						.day   = (int8_t)t.tm_mday,
						.dotw  = (int8_t)t.tm_wday, 
						.hour  = (int8_t)t.tm_hour,
						.min   = (int8_t)t.tm_min,
						.sec   = (int8_t)t.tm_sec
					};
					printf("[Server]: Setting RTC time to %d/%d/%d %d:%d:%d\n", dt.year, dt.month, dt.day, dt.hour, dt.min, dt.sec);
					rtc_set_datetime(&dt);
					break;
				}
			}
		}
	} while (--retries);

	if (!retries) {
		return false;
	}

	return true;
}

u_int64_t getMsecFrom1970(void)
{
	std::tm epoch_start;					// Base time
	epoch_start.tm_sec = 0;					// 00 sec
	epoch_start.tm_min = 0;					// 00 min
	epoch_start.tm_hour = 0;				// 00 O'clock
	epoch_start.tm_mday = 1;				// 1st day
	epoch_start.tm_mon = 0;					// January
	epoch_start.tm_year = 1970 - 1900;		// since 1900
	std::time_t basetime = std::mktime(&epoch_start);	// basetime = 1970/Jan/1

	datetime_t dt;
	rtc_get_datetime(&dt);					// PICO SDK function
	std::tm now = {};						// Current time
	now.tm_year = dt.year - 1900;			// since 1900
	now.tm_mon = dt.month - 1;
	now.tm_mday = dt.day;
	now.tm_hour = dt.hour;
	now.tm_min = dt.min;
	now.tm_sec = dt.sec;

	// Difference b/w current and basetime(1970/Jan/1) in ulonglong
	const u_int64_t ms = std::difftime(std::mktime(&now), basetime);
	return ms * 1000;
}

int getHeaderContentValue(const char* request, const char* content)
{
	try{
		char* header = strstr(request, "\r\n") + 2;				// remove \r\n
		const std::string value = getParamEx(content, '\r', header);
		if(!value.empty()){
			int n = atoi(value.c_str());
//			printf("Header+Body = %s, value = %s,%d\n", header, value.c_str(), n);
			printf("%s = %d\n", content, n);
			return n;
		} else
			return 0;
	} catch(...){
		printf("[Server]:[ERROR]: while getting Header contents\n");
		return 0;
	}
}

std::string getHeaderContentStr(const char* request, const char* content)
{
	try{
		char* header = strstr(request, "\r\n") + 2;				// remove \r\n
		const std::string str = getParamEx(content, '\r', header);
		if(!str.empty()){
//			printf("Header+Body = %s, str = %s\n", header, str);
			printf("%s = %s\n", content, str.c_str());
			return str;
		} else
			return "";
	} catch(...){
		printf("[Server]:[ERROR]: while getting Header contents\n");
		return "";
	}
}

void manualESP32(void)
{
	while (true) {
		int	n;
		char c; 
		while(n = uart_is_readable(UART_ID)){
			for(int i=0; i<n; i++){
				c = uart_getc(UART_ID);
				putchar(c);
			}
		}
		if(uart_is_readable(STDIO_UART)){
			c = getchar();
			if(c == 0x11)	break;			// ctl-Q
			uart_putc(UART_ID, c);
		}
	}
}
