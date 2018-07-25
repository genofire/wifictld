#ifndef __WIFICTLD_UBUS_EVENT_H
#define __WIFICTLD_UBUS_EVENT_H

#include "include.h"

extern struct avl_tree ubus_hostapd_binds;


struct ubus_hostapd_bind {
	struct avl_node avl;
	int id;
	char *path;
};

int  wifictld_ubus_bind_events(struct ubus_context *ctx);
int  wifictld_ubus_unbind_events(struct ubus_context *ctx);

#endif
