#ifndef __WIFICTLD_UBUS_EVENT_H
#define __WIFICTLD_UBUS_EVENT_H

#include <libubus.h>

extern int client_probe_learning;

int  wifictld_ubus_bind_events(struct ubus_context *ctx);

#endif
