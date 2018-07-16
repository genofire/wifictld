#ifndef __WIFICTLD_LOG_H
#define __WIFICTLD_LOG_H

#ifdef DEBUG
	#define DEBUG_COMPILE 1
#else
	#define DEBUG_COMPILE 0
#endif

#define debug_print(fmt) \
	do { if (DEBUG_COMPILE) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
		__LINE__, __func__); } while (0)

#endif
