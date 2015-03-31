#ifndef __ERRORS_H__
#define __ERRORS_H__

enum  ERR_code {
	ERR_PATH_NOT_SET = 1000,
	ERR_GDB_NOT_FOUND,
	ERR_OUT_OF_MEMORY,
	ERR_CANNOT_INIT_GDB,
	ERR_GDB_HANDLE_ALLOC,
	STD_ERR_NO
};

enum ERR_code m_error;

void
log_msg();

#endif
