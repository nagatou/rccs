#ifndef __LOG_PRINTF_H__
#define __LOG_PRINTF_H__


#define RED "\x1b[31;01m"
#define DARKRED "\x1b[31;06m"
#define RESET "\x1b[0m"
#define GREEN "\x1b[32;06m"
#define YELLOW "\x1b[33;06m"

enum {
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

void
log_printf(int dp, char *format, ...);

#endif
