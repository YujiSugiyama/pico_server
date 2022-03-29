#ifndef __SERVER_H__
#define __SERVER_H__

#include <unordered_map>
#include <string>

bool server_init(void);
void dispatch_server(void);

void echo_server(const std::string&, const std::string&);
void led_server(const std::string&, const std::string&, const std::string&);

// Process on core1
void led_process(void);

#endif		// __SERVER_H__

