#include "include.h"
#include "config.h"
#include "log.h"
#include "wifi_clients.h"

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
 */
int wifictld_ubus_bind_events(struct ubus_context *ctx)
{
	// register subscriber on ubus
	int ret = ubus_register_subscriber(ctx, &sub);
	if (ret) {
		log_error("Error while registering subscriber: %s", ubus_strerror(ret));
		ubus_free(ctx);
		return 2;
	}

	// request for all ubus services
	ubus_lookup(ctx, NULL, recieve_interfaces, NULL);
	return 0;
}

static void recieve_interfaces(struct ubus_context *ctx, struct ubus_object_data *obj, void *priv)
{
	int ret = 0;
	const char *str = "notify_response",
		*path_prefix = "hostapd.";

	size_t lenpre = strlen(path_prefix),
		lenpath = strlen(obj->path);

	if (lenpath < lenpre || strncmp(path_prefix, obj->path, lenpre) != 0) {
		return;
	}
	char *path = malloc((lenpath+1) * sizeof(char));
	strncpy(path, obj->path, lenpath);
	path[lenpath] = '\0';

	//change hostapd to wait for response
	struct blob_buf b = {};
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, str, 1);
	ret = ubus_invoke(ctx, obj->id, str, b.head, NULL, NULL, 100);

	blob_buf_free(&b);

	if (ret) {
		log_error("Error while register response for event '%s': %s\n", path, ubus_strerror(ret));
	}

	//subscribe hostapd with THIS interface
	ret = ubus_subscribe(ctx, &sub, obj->id);
	if (ret) {
		log_error("Error while register subscribe for event '%s': %s\n", path, ubus_strerror(ret));
	}

	log_info("subscribe %s\n", path);
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
	log_verbose("%s["MACSTR"] freq: %d signal %d", method, MAC2STR(addr), freq, ssi_signal);
	if (!strcmp(method, "auth")) {
		if (wifi_clients_try(true, addr, freq, ssi_signal)) {
			log_debug(" -> reject\n");
			return WLAN_STATUS_ASSOC_REJECTED_TEMPORARILY;
		}
		log_debug(" -> accept\n");
		return WLAN_STATUS_SUCCESS;
	}

	if (!strcmp(method, "probe")) {
		if(client_probe_steering) {
			if (wifi_clients_try(false, addr, freq, ssi_signal)) {
				log_debug(" -> reject\n");
				return WLAN_STATUS_UNSPECIFIED_FAILURE;
			}
			log_debug(" -> accept\n");
			return WLAN_STATUS_SUCCESS;
		}
		if(client_probe_learning) {
			log_verbose(" learn");
		wifi_clients_learn(addr, freq, ssi_signal);
		}
	}

	if (!strcmp(method, "deauth")) {
		wifi_clients_disconnect(addr, freq, ssi_signal);
		log_verbose(" -> disconnect\n");
		return WLAN_STATUS_SUCCESS;
	}
	log_verbose("\n");

	return WLAN_STATUS_SUCCESS;
}
