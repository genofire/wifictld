#ifndef __WIFICTLD_UBUS_EVENT_H
#define __WIFICTLD_UBUS_EVENT_H

#include <libubus.h>

extern bool client_probe_steering;
/*
	steering contains learning already:
	when client_probe_steering is set,
	client_probe_learning is ignored
*/
extern bool client_probe_learning;

int  wifictld_ubus_bind_events(struct ubus_context *ctx);

#endif
