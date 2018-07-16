#ifndef __WIFICTLD_LOG_H
#define __WIFICTLD_LOG_H

static int verbose = 0;

void log_info(const char *format, ...);
void log_verbose(const char *format, ...);
void log_debug(const char *format, ...);
void log_error(const char *format, ...);

#endif
