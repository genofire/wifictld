#include <stdlib.h>
#include <string.h>
#include <libubox/blobmsg.h>
#include <libubox/ulog.h>
#include <libubus.h>
#include "temp_defs.h"
#include "log.h"

static struct blob_buf b;
struct ubus_context *ctx_main;



static int receive_request(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	const char *attr_name, *addr;
	uint16_t freq, ssi_signal = -1;
	struct blob_attr *pos;
	int rem = blobmsg_data_len(msg);

	// read msg
	blobmsg_for_each_attr(pos, msg, rem) {
		attr_name = blobmsg_name(pos);

		if (!strcmp(attr_name, "address")){
			addr = blobmsg_get_string(pos);
		} else if(!strcmp(attr_name, "signal")){
			ssi_signal = blobmsg_get_u32(pos);
		} else if(!strcmp(attr_name, "freq")){
			freq = blobmsg_get_u32(pos);
		}
	}

	/*
	blob_buf_init(&b, 0);
	blob_put_u32(&b, 0, WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA);
	ubus_send_reply(ctx, req, b.head);
	*/

	if (!strcmp(method, "probe")) {
		ULOG_NOTE("probe\n");
		return WLAN_STATUS_SUCCESS;
	}
	if (!strcmp(method, "auth")) {
		if (freq <= 5000) {
			ULOG_WARN("auth [drop]-> %s\n", addr);
			return WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;
		}
		ULOG_WARN("auth [accept]-> %s\n", addr);
	}

	// learn by freq and address

	ULOG_INFO("%s\n", method);
	return WLAN_STATUS_SUCCESS;
}




struct ubus_subscriber sub = {
	.cb = receive_request,
};


void wifictld_ubus_interfaces(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	int ret = 0;
	const char *str = "notify_response";

	if (obj->path == NULL || !strstr(obj->path, "hostapd.")) {
		return;
	}

	//switch to notify
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, str, 1);
	ret = ubus_invoke(ctx, obj->id, str, b.head, NULL, NULL, 100);
	if (ret) {
		ULOG_ERR("Error while register response for event '%s': %s\n", obj->path, ubus_strerror(ret));
	}

	//subscribe
	ret = ubus_subscribe(ctx, &sub, obj->id);
	if (ret) {
		ULOG_ERR("Error while register subscribe for event '%s': %s\n", obj->path, ubus_strerror(ret));
	}

	printf("sub %s: %d:%d\n", obj->path, obj->id, obj->type_id);
}

int wifictld_ubus_init()
{
	int ret = 0;

	ctx_main = ubus_connect(NULL);
	if (!ctx_main) {
		ULOG_ERR("Failed to connect to ubus");
		return 1;
	}

	ret = ubus_register_subscriber(ctx_main, &sub);
	if (ret) {
		ULOG_ERR("Error while registering subscriber: %s", ubus_strerror(ret));
		ubus_free(ctx_main);
		return 2;
	}

	ubus_lookup(ctx_main, NULL, wifictld_ubus_interfaces, NULL);

	ubus_add_uloop(ctx_main);

	return 0;
}


void wifictld_ubus_close()
{
	ubus_free(ctx_main);
}
