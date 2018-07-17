#include <libubus.h>
#include <libubox/avl.h>
#include "hostapd/common.h"
#include "log.h"
#include "wifi_clients.h"
#include "ubus_events.h"

static struct blob_buf b;

static int ubus_get_clients(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	void *list, *c;
	char mac_buf[20];
	struct wifi_client *el;

	blob_buf_init(&b, 0);
	list = blobmsg_open_table(&b, "clients");
	avl_for_each_element(&clients_by_addr, el, avl) {

		sprintf(mac_buf, MACSTR, MAC2STR(el->addr));
		c = blobmsg_open_table(&b, mac_buf);
		blobmsg_add_u32(&b, "try", el->try);
		blobmsg_add_u32(&b, "connected", el->connected);
		blobmsg_add_u32(&b, "freq_highest", el->freq_highest);
		blobmsg_add_u32(&b, "signal_lowfreq", el->signal_lowfreq);
		blobmsg_add_u32(&b, "signal_highfreq", el->signal_highfreq);
		blobmsg_close_table(&b, c);
	}

	blobmsg_close_array(&b, list);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

enum {
	DEL_CLIENT_ADDR,
	__DEL_CLIENT_MAX
};

static const struct blobmsg_policy ubus_del_client_policy[__DEL_CLIENT_MAX] = {
	[DEL_CLIENT_ADDR] = { "addr", BLOBMSG_TYPE_STRING },
};

static int ubus_del_client(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__DEL_CLIENT_MAX];
	u8 addr[ETH_ALEN];

	blobmsg_parse(ubus_del_client_policy, __DEL_CLIENT_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[DEL_CLIENT_ADDR])
		return UBUS_STATUS_INVALID_ARGUMENT;
	if (hwaddr_aton(blobmsg_data(tb[DEL_CLIENT_ADDR]), addr))
		return UBUS_STATUS_INVALID_ARGUMENT;

	wifi_clients_del(addr);

	return 0;
}

static int ubus_get_client_threasholds(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "try", client_try_threashold);
	blobmsg_add_u32(&b, "signal", client_signal_threashold);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}


enum {
	SET_CLIENT_TRY_THREASHOLD,
	SET_CLIENT_SIGNAL_THREASHOLD,
	__SET_CLIENT_THREASHOLD_VALUE_MAX
};

static const struct blobmsg_policy ubus_set_client_threashold_policy[__SET_CLIENT_THREASHOLD_VALUE_MAX] = {
	[SET_CLIENT_TRY_THREASHOLD] = { "try", BLOBMSG_TYPE_INT32 },
	[SET_CLIENT_SIGNAL_THREASHOLD] = { "signal", BLOBMSG_TYPE_INT32 },
};

static int ubus_set_client_threashold(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__SET_CLIENT_THREASHOLD_VALUE_MAX];

	blobmsg_parse(ubus_set_client_threashold_policy, __SET_CLIENT_THREASHOLD_VALUE_MAX, tb, blob_data(msg), blob_len(msg));
	if (!tb[SET_CLIENT_TRY_THREASHOLD] && !tb[SET_CLIENT_SIGNAL_THREASHOLD])
		return UBUS_STATUS_INVALID_ARGUMENT;
	if (tb[SET_CLIENT_TRY_THREASHOLD])
		client_try_threashold = blobmsg_get_u32(tb[SET_CLIENT_TRY_THREASHOLD]);
	if (tb[SET_CLIENT_SIGNAL_THREASHOLD])
		client_signal_threashold = blobmsg_get_u32(tb[SET_CLIENT_SIGNAL_THREASHOLD]);
	return 0;
}

static int ubus_is_client_probe_learning(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "current", client_probe_learning);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}

static int ubus_toggle_client_probe_learning(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	blobmsg_add_u32(&b, "before", client_probe_learning);
	client_probe_learning = !client_probe_learning;
	blobmsg_add_u32(&b, "current", client_probe_learning);
	ubus_send_reply(ctx, req, b.head);
	return 0;
}


static const struct ubus_method wifictld_ubus_methods[] = {
	// mgmt client db of bandsteering
	UBUS_METHOD_NOARG("get_clients", ubus_get_clients),
	UBUS_METHOD("del_client", ubus_del_client, ubus_del_client_policy),

	// client threasholds
	UBUS_METHOD_NOARG("get_client_threasholds", ubus_get_client_threasholds),
	UBUS_METHOD("set_client_threasholds", ubus_set_client_threashold, ubus_set_client_threashold_policy),

	// learn by probe (or only auth)
	UBUS_METHOD_NOARG("is_client_probe_learning", ubus_is_client_probe_learning),
	UBUS_METHOD_NOARG("toggle_client_probe_learning", ubus_toggle_client_probe_learning),
};

static struct ubus_object_type bss_object_type =
	UBUS_OBJECT_TYPE("wifictld_bss", wifictld_ubus_methods);

static struct ubus_object obj = {
		.name = "wifictld",
		.type = &bss_object_type,
		.methods = wifictld_ubus_methods,
		.n_methods = ARRAY_SIZE(wifictld_ubus_methods),
	};

int wifictld_ubus_add_bss(struct ubus_context *ctx) {
	return ubus_add_object(ctx, &obj);
}
