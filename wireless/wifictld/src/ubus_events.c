#include <stdlib.h>
#include <string.h>
#include <libubox/blobmsg.h>
#include <libubus.h>
#include "hostapd/ieee802_11_defs.h" // ETH_ALEN + hwaddr_aton
#include "hostapd/common.h"
#include "log.h"
#include "wifi_clients.h"
#include "ubus_events.h"

static struct blob_buf b;
static struct ubus_context *ctx_main;


// bind on every hostapd by receive all ubus registered services
static void recieve_interfaces(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv);

/**
 * handle every notify sended by hostapd
 * (and allow or deny client auth)
 */
static int receive_notify(struct ubus_context *ctx, struct ubus_object *obj,
	struct ubus_request_data *req, const char *method, struct blob_attr *msg);

// subscriber handler with callback to handle notifies from hostapd
struct ubus_subscriber sub = {
	.cb = receive_notify,
};

/**
 * init ubus connection and request for all registered hostapd instances
 * (start everthing need and add themselve to uloop)
 */
int wifictld_ubus_init()
{
	int ret = 0;

	// connect to ubus
	ctx_main = ubus_connect(NULL);
	if (!ctx_main) {
		log_error("Failed to connect to ubus");
		return 1;
	}

	// register subscriber on ubus
	ret = ubus_register_subscriber(ctx_main, &sub);
	if (ret) {
		log_error("Error while registering subscriber: %s", ubus_strerror(ret));
		ubus_free(ctx_main);
		return 2;
	}

	// request for all ubus services
	ubus_lookup(ctx_main, NULL, recieve_interfaces, NULL);

	// add to uloop
	ubus_add_uloop(ctx_main);

	return 0;
}

// close ubus connection
void wifictld_ubus_close()
{
	ubus_free(ctx_main);
}

static void recieve_interfaces(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	int ret = 0;
	const char *str = "notify_response";

	if (obj->path == NULL || !strstr(obj->path, "hostapd.")) {
		return;
	}

	//change hostapd to wait for response
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, str, 1);
	ret = ubus_invoke(ctx, obj->id, str, b.head, NULL, NULL, 100);
	if (ret) {
		log_error("Error while register response for event '%s': %s\n", obj->path, ubus_strerror(ret));
	}

	//subscribe hostapd with THIS interface
	ret = ubus_subscribe(ctx, &sub, obj->id);
	if (ret) {
		log_error("Error while register subscribe for event '%s': %s\n", obj->path, ubus_strerror(ret));
	}

	log_info("sub %s: %d:%d\n", obj->path, obj->id, obj->type_id);
}


static int receive_notify(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	const char *attr_name, *str;
	u8 addr[ETH_ALEN];
	uint32_t freq, ssi_signal = -1;
	struct blob_attr *pos;
	int rem = blobmsg_data_len(msg);

	// read msg
	log_debug("ubus_events.receive_notify(): read msg\n");

	blobmsg_for_each_attr(pos, msg, rem) {
		attr_name = blobmsg_name(pos);

		if (!strcmp(attr_name, "address")){
			str = blobmsg_get_string(pos);
			hwaddr_aton(str, addr);
		} else if(!strcmp(attr_name, "signal")){
			ssi_signal = blobmsg_get_u32(pos);
		} else if(!strcmp(attr_name, "freq")){
			freq = blobmsg_get_u32(pos);
		}
	}


	// handle
	log_debug("ubus_events.receive_notify(): handle\n");

	if (!strcmp(method, "probe")) {
		log_verbose("probe["MACSTR"]", MAC2STR(addr));
		if (client_probe_learning) {
			log_verbose(" + learn freq[%d]\n", freq);
			wifi_clients_learn(addr, freq);
		}else{
			log_verbose("\n");
		}
		return WLAN_STATUS_SUCCESS;
	}
	if (!strcmp(method, "auth")) {
		log_info("auth["MACSTR"]", MAC2STR(addr));
		if (wifi_clients_try(addr, freq)) {
			log_info(" -> drop\n");
			return WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY;
		}
		log_info(" -> accept\n");
	} else {
		log_verbose("%s["MACSTR"]\n", method, MAC2STR(addr));
	}

	return WLAN_STATUS_SUCCESS;
}
