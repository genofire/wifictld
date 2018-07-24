#include "include.h"
#include "config.h"
#include "log.h"
#include "data.h"
#include "wifi_clients.h"

static struct blob_buf b = {};

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

		blobmsg_add_u32(&b, "time", el->time);
		blobmsg_add_u32(&b, "try_probe", el->try_probe);
		blobmsg_add_u32(&b, "try_auth", el->try_auth);
		blobmsg_add_u32(&b, "connected", el->connected);
		blobmsg_add_u32(&b, "authed", el->authed);
		blobmsg_add_u32(&b, "freq_highest", el->freq_highest);
		blobmsg_add_u32(&b, "signal_lowfreq", el->signal_lowfreq);
		blobmsg_add_u32(&b, "signal_highfreq", el->signal_highfreq);

		blobmsg_close_table(&b, c);
	}

	blobmsg_close_table(&b, list);

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

static int ubus_get_config(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	blob_buf_init(&b, 0);
	void *list = blobmsg_open_table(&b, "config");

	blobmsg_add_bool(&b, "verbose", config_verbose);
	blobmsg_add_u32(&b, "client_try_threashold", config_client_try_threashold);
	blobmsg_add_u32(&b, "client_signal_threashold", config_client_signal_threashold);
	blobmsg_add_u32(&b, "client_clean_every", config_client_clean_every);
	blobmsg_add_u32(&b, "client_clean_older_then", config_client_clean_older_then);
	blobmsg_add_bool(&b, "client_clean_authed", config_client_clean_authed);
	blobmsg_add_bool(&b, "client_force", config_client_force);
	blobmsg_add_bool(&b, "client_force_probe", config_client_force_probe);
	blobmsg_add_bool(&b, "client_probe_steering", config_client_probe_steering);
	blobmsg_add_bool(&b, "client_probe_learning", config_client_probe_learning);

	blobmsg_close_table(&b, list);

	ubus_send_reply(ctx, req, b.head);

	return 0;
}


enum {
	SET_CONFIG_VERBOSE,
	SET_CONFIG_CLIENT_TRY_THREASHOLD,
	SET_CONFIG_CLIENT_SIGNAL_THREASHOLD,
	SET_CONFIG_CLIENT_CLEAN_EVERY,
	SET_CONFIG_CLIENT_CLEAN_OLDER_THEN,
	SET_CONFIG_CLIENT_CLEAN_AUTHED,
	SET_CONFIG_CLIENT_FORCE,
	SET_CONFIG_CLIENT_FORCE_PROBE,
	SET_CONFIG_CLIENT_PROBE_STEERING,
	SET_CONFIG_CLIENT_PROBE_LEARNING,
	__SET_CONFIG_MAX
};

static const struct blobmsg_policy ubus_set_config_policy[__SET_CONFIG_MAX] = {
	[SET_CONFIG_VERBOSE] = { "verbose", BLOBMSG_TYPE_BOOL },
	[SET_CONFIG_CLIENT_TRY_THREASHOLD] = { "client_try_threashold", BLOBMSG_TYPE_INT32 },
	[SET_CONFIG_CLIENT_SIGNAL_THREASHOLD] = { "client_signal_threashold", BLOBMSG_TYPE_INT32 },
	[SET_CONFIG_CLIENT_CLEAN_EVERY] = { "client_clean_every", BLOBMSG_TYPE_INT32 },
	[SET_CONFIG_CLIENT_CLEAN_OLDER_THEN] = { "client_clean_older_then", BLOBMSG_TYPE_INT32 },
	[SET_CONFIG_CLIENT_CLEAN_AUTHED] = { "client_clean_authed", BLOBMSG_TYPE_BOOL },
	[SET_CONFIG_CLIENT_FORCE] = { "client_force", BLOBMSG_TYPE_BOOL },
	[SET_CONFIG_CLIENT_FORCE_PROBE] = { "client_force_probe", BLOBMSG_TYPE_BOOL },
	[SET_CONFIG_CLIENT_PROBE_STEERING] = { "client_probe_steering", BLOBMSG_TYPE_BOOL },
	[SET_CONFIG_CLIENT_PROBE_LEARNING] = { "client_probe_learning", BLOBMSG_TYPE_BOOL },
};

static int ubus_set_config(struct ubus_context *ctx, struct ubus_object *obj,
		struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[__SET_CONFIG_MAX];

	blobmsg_parse(ubus_set_config_policy, __SET_CONFIG_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[SET_CONFIG_VERBOSE] &&
			!tb[SET_CONFIG_CLIENT_TRY_THREASHOLD] &&
			!tb[SET_CONFIG_CLIENT_SIGNAL_THREASHOLD] &&
			!tb[SET_CONFIG_CLIENT_CLEAN_EVERY]  &&
			!tb[SET_CONFIG_CLIENT_CLEAN_OLDER_THEN] &&
			!tb[SET_CONFIG_CLIENT_CLEAN_AUTHED] &&
			!tb[SET_CONFIG_CLIENT_FORCE] &&
			!tb[SET_CONFIG_CLIENT_FORCE_PROBE] &&
			!tb[SET_CONFIG_CLIENT_PROBE_STEERING] &&
			!tb[SET_CONFIG_CLIENT_PROBE_LEARNING])
		return UBUS_STATUS_INVALID_ARGUMENT;
	if (tb[SET_CONFIG_VERBOSE])
			config_verbose = blobmsg_get_bool(tb[SET_CONFIG_VERBOSE]);
	if (tb[SET_CONFIG_CLIENT_TRY_THREASHOLD])
		config_client_try_threashold = blobmsg_get_u32(tb[SET_CONFIG_CLIENT_TRY_THREASHOLD]);
	if (tb[SET_CONFIG_CLIENT_SIGNAL_THREASHOLD])
		config_client_signal_threashold = blobmsg_get_u32(tb[SET_CONFIG_CLIENT_SIGNAL_THREASHOLD]);
	if (tb[SET_CONFIG_CLIENT_CLEAN_EVERY])
		config_client_clean_every = blobmsg_get_u32(tb[SET_CONFIG_CLIENT_CLEAN_EVERY]);
	if (tb[SET_CONFIG_CLIENT_CLEAN_OLDER_THEN])
		config_client_clean_older_then = blobmsg_get_u32(tb[SET_CONFIG_CLIENT_CLEAN_OLDER_THEN]);
	if (tb[SET_CONFIG_CLIENT_CLEAN_AUTHED])
		config_client_clean_authed = blobmsg_get_bool(tb[SET_CONFIG_CLIENT_CLEAN_AUTHED]);
	if (tb[SET_CONFIG_CLIENT_FORCE])
		config_client_force = blobmsg_get_bool(tb[SET_CONFIG_CLIENT_FORCE]);
	if (tb[SET_CONFIG_CLIENT_FORCE_PROBE])
		config_client_force_probe = blobmsg_get_bool(tb[SET_CONFIG_CLIENT_FORCE_PROBE]);
	if (tb[SET_CONFIG_CLIENT_PROBE_STEERING])
		config_client_probe_steering = blobmsg_get_bool(tb[SET_CONFIG_CLIENT_PROBE_STEERING]);
	if (tb[SET_CONFIG_CLIENT_PROBE_LEARNING])
		config_client_probe_learning = blobmsg_get_bool(tb[SET_CONFIG_CLIENT_PROBE_LEARNING]);

	return 0;
}




static const struct ubus_method wifictld_ubus_methods[] = {
	// mgmt client db of bandsteering
	UBUS_METHOD_NOARG("get_clients", ubus_get_clients),
	UBUS_METHOD("del_client", ubus_del_client, ubus_del_client_policy),

	// config threasholds
	UBUS_METHOD_NOARG("get_config", ubus_get_config),
	UBUS_METHOD("set_config", ubus_set_config, ubus_set_config_policy),
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
