#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "log_printf.h"

void
log_printf(int dp,char *format, ...) {
#ifdef MC_LOG
   va_list arglist;
	va_start(arglist,format);
	if(dp == LOG_INFO)
	{
		printf(GREEN"\nINFO:\t"RESET);
	}
	else if(dp == LOG_WARNING)
	{
		printf(YELLOW"\nWARNING:\t"RESET);
	}
	else
	{
		printf(DARKRED"\nERROR:\t"RESET);
	}
	vprintf(format, arglist);
	if(format[strlen(format)-1] != '\n')
	{
		printf("\n");
	}
	va_end(arglist);
#endif
}
