#ifndef __WIFICTLD_UBUS_H
#define __WIFICTLD_UBUS_H

#include <libubus.h>
#include "log.h"
#include "ubus_events.h"
#include "ubus_service.h"


static struct ubus_context *ctx;

/**
 * init ubus connection
 * (start everthing need and add themselve to uloop)
 */
int wifictld_ubus_init()
{
	int ret = 0;

	// connect to ubus
	ctx = ubus_connect(NULL);
	if (!ctx) {
		log_error("Failed to connect to ubus");
		return 1;
	}

	// add to uloop
	ubus_add_uloop(ctx);

	// add bbs
	ret =  wifictld_ubus_add_bss(ctx);
	if (ret) {
		log_error("Failed to add ubus service");
		return 1;
	}

	// bind events
	ret =  wifictld_ubus_bind_events(ctx);
	if (ret) {
		log_error("Failed to bind for ubus events");
		return 2;
	}

	return 0;
}

// close ubus connection
void wifictld_ubus_close()
{
	ubus_free(ctx);
}

#endif
