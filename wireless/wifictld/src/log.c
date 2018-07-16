#include <stdio.h>
#include <stdarg.h>
#include "log.h"

#ifdef DEBUG
void log_debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#else
	void log_debug(const char *format, ...) {
	}
#endif


void log_verbose(const char *format, ...) {
	if (!verbose)
		return;
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void log_info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void log_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
