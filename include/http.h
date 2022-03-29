#ifndef __HTTP_H__
#define __HPPT_H__

enum class HTTP_STATUS {
	UNSUPPORTED_MEDIA_TYPE,
	INTERNAL_SERVER_ERROR,
	METHOD_NOT_ALLOWED,
	UNAUTHORIZED,
	BAD_REQUEST,
	NOT_FOUND,
	OK
};

static void uri_decode(char* dest, const char* src);
static void post_analyze(char*, const char*);
static std::string HTTP_STATUSES(const HTTP_STATUS&);
static void close_connection(const std::string&, const HTTP_STATUS&);

static void handle_get_request(const std::string&, const std::string&, int);
static void handle_post_request(const std::string&, const std::string&, char*, int);

void http_server(const std::string&, const std::string&, char*, int);

#endif		// __HTTP_H__

