#include <libubus.h>
#include "log.h"
#include "ubus_events.h"


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

	// connect to ubus
	ret =  wifictld_ubus_bind_events(ctx);
	if (ret) {
		log_error("Failed to connect to ubus");
		return 1;
	}

	// add to uloop
	ubus_add_uloop(ctx);

	return 0;
}

// close ubus connection
void wifictld_ubus_close()
{
	ubus_free(ctx);
}
