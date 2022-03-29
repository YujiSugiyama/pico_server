#ifndef __ESP32_H__
#define __ESP32_H__

// Definitions
#define SOCKET_PORT			"54000"
#define HOSTNAME			"RPi-Pico-ESP32"
#define HMAC_SECRET			"secret"

#define SENDBUFFERLEN		512
#define RECVBUFFERLEN		2048

#define MICROS_MS			1000
#define IPD_TRY				4

// Variables
extern char recvBuffer[RECVBUFFERLEN];
extern char _recvBuffer[RECVBUFFERLEN];

static int getATdata(int);

// AT commands
int recvIPD(int64_t);
char* recvBody(int, int64_t);
bool sendATCommand(const char*, int64_t, const char*, const bool&);
bool sendATCommandOK(const char*, const int64_t, const bool&);
bool clearATBuffer(const int64_t);

// Analyzing command parameters
std::string getParam(const int, const char, const char, const std::string);
std::string getParamEx(const char*, const char, const char*);

// Responding to client
void sendResponse(const char*, const char*, const char*, const char*, int, int);
void sendMessage(const char*, const char*);

// Aggregated Commands
std::string getIp(void);
std::string getSDKVersion(void);
int getWifiState(void);
bool setupUTCTime(void);
u_int64_t getMsecFrom1970(void);

int getHeaderContentValue(const char*, const char*);
std::string getHeaderContentStr(const char*, const char*);
void manualESP32(void);

#endif		// __ESP32_H__
