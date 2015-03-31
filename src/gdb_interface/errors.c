#include <stdio.h>
#include <string.h>
#include "errors.h"
#include "log_printf.h"

void
log_msg() {
	switch (m_error) {
		case ERR_PATH_NOT_SET:
#ifdef MC_LINUX
			log_printf(LOG_WARNING, "Environment variable $PATH  not set.\n\
											 Trying /usr/bin/gdb\n");
#else
			log_printf(LOG_WARNING, "Environment variable $GNU_BD  not set.\n\
    		 \Trying /usr/local/te/tool/Linux-i686/bin/sh-unknown-tmonitor-gdb\n");
#endif
			break;
		case ERR_GDB_NOT_FOUND:
			log_printf(LOG_ERROR, "No executable binary of GDB found!\n");
			break;
		case ERR_OUT_OF_MEMORY:
		   log_printf(LOG_ERROR, "Out of memory!!\n");
		   break;
		case ERR_CANNOT_INIT_GDB:
		   log_printf(LOG_ERROR, "Cannot initialize GDB.\n");
		   break;
		case ERR_GDB_HANDLE_ALLOC:
			log_printf(LOG_ERROR, "Could not allocate memory for GDB handler!\n");
			break;
		case STD_ERR_NO :
			log_printf (LOG_WARNING, (char *) strerror(m_error));
			break;
	}
}
