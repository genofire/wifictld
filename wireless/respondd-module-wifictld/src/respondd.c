#include <respondd.h>
#include <json-c/json.h>

#include <libubus.h>
#include <libubox/blobmsg.h>

#define UBUS_SOCKET "/var/run/ubus.sock"

static struct blob_attr *msg_config_store, *msg_client_store;

static void cb_config(struct ubus_request *req, int type, struct blob_attr *msg)
{
	if (!msg) return;
	msg_config_store = msg;
}

static void cb_client(struct ubus_request *req, int type, struct blob_attr *msg)
{
	if (!msg) return;
	msg_client_store = msg;
}

static int ubus_fetch(const char *method, ubus_data_handler_t cb) {
	int ret;
	uint32_t ubus_id;

	struct ubus_context *ctx = ubus_connect(UBUS_SOCKET);
	if (!ctx) {
		return 1;
	}

	ret = ubus_lookup_id(ctx, "wifictld", &ubus_id);
	if (ret) {
		ubus_free(ctx);
		return 2;
	}
	ret = ubus_invoke(ctx, ubus_id, method, NULL, cb, NULL, 100);
	if (ret) {
		ubus_free(ctx);
		return 3;
	}
	ubus_free(ctx);
	return 0;
}


struct json_object *respondd_provider_nodeinfo(void) {
	struct json_object *ret = json_object_new_object(),
			   *wifictld = json_object_new_object();
	json_object_object_add(ret, "wifictld", wifictld);

	int ubus_ret = ubus_fetch("get_config", cb_config);
	if (ubus_ret) {
		json_object_object_add(wifictld, "error", json_object_new_int(ubus_ret));
	}

	if (!msg_config_store) {
		json_object_object_add(wifictld, "error", json_object_new_string("no data"));
		return ret;
	}

	struct blob_attr *pos;
	int rem;

	const char *attr_name;

	blobmsg_for_each_attr(pos, blobmsg_data(msg_config_store), rem) {
		attr_name = blobmsg_name(pos);
		switch(blobmsg_type(pos)) {
			case BLOBMSG_TYPE_UNSPEC:
				json_object_object_add(wifictld, attr_name, json_object_new_string("null"));
				break;
			/*
			case BLOBMSG_TYPE_BOOL:
				json_object_object_add(wifictld, attr_name, json_object_new_boolean(blobmsg_get_bool(pos)));
				break;
			*/
			case BLOBMSG_TYPE_INT8:
				json_object_object_add(wifictld, attr_name, json_object_new_int(blobmsg_get_u8(pos)));
				break;
			case BLOBMSG_TYPE_INT16:
				json_object_object_add(wifictld, attr_name, json_object_new_int(blobmsg_get_u16(pos)));
				break;
			case BLOBMSG_TYPE_INT32:
				json_object_object_add(wifictld, attr_name, json_object_new_int(blobmsg_get_u32(pos)));
				break;
			case BLOBMSG_TYPE_INT64:
				json_object_object_add(wifictld, attr_name, json_object_new_int64(blobmsg_get_u64(pos)));
				break;
			case BLOBMSG_TYPE_DOUBLE:
				json_object_object_add(wifictld, attr_name, json_object_new_double(blobmsg_get_double(pos)));
				break;
			case BLOBMSG_TYPE_STRING:
				json_object_object_add(wifictld, attr_name, json_object_new_string(blobmsg_get_string(pos)));
				break;
			default:
				json_object_object_add(wifictld, attr_name, json_object_new_string("error"));
			}
	}

	return ret;
}

struct json_object *respondd_provider_statistics(void) {
	struct json_object *ret = json_object_new_object(),
			   *wifictld = json_object_new_object();
	json_object_object_add(ret, "wifictld", wifictld);

	int ubus_ret = ubus_fetch("get_clients", cb_client);
	if (ubus_ret) {
		json_object_object_add(wifictld, "error", json_object_new_int(ubus_ret));
	}

	if (!msg_client_store) {
		json_object_object_add(wifictld, "error", json_object_new_string("no data"));
		return ret;
	}

	struct blob_attr *client, *pos;
	int rem, remC;

	const char *attr_name;
	int value;


	int total = 0,
	    client24 = 0,
	    client5 = 0,
	    authed = 0,
	    connected = 0,
	    highest_try_probe = 0,
	    highest_try_auth = 0;

	blobmsg_for_each_attr(client, blobmsg_data(msg_client_store), rem) {
		total++;
		remC = blobmsg_data_len(client);
		blobmsg_for_each_attr(pos, client, remC) {
			attr_name = blobmsg_name(pos);
			value = blobmsg_get_u32(pos);

			//TODO counter by frequence, try_auth and try_probe
			if (!strcmp(attr_name, "freq_highest")){
				if (value > 5000)
					client5++;
				else
					client24++;
			} else if(!strcmp(attr_name, "authed")){
				if(value == 1) authed++;
			} else if(!strcmp(attr_name, "connected")){
				if(value == 1) connected++;
			} else if(!strcmp(attr_name, "try_probe")){
				if(value > highest_try_probe) highest_try_probe = value;
			} else if(!strcmp(attr_name, "try_auth")){
				if(value > highest_try_auth) highest_try_auth = value;
			}
		}
	}

	json_object_object_add(wifictld, "total", json_object_new_int(total));
	json_object_object_add(wifictld, "client24", json_object_new_int(client24));
	json_object_object_add(wifictld, "client5", json_object_new_int(client5));
	json_object_object_add(wifictld, "authed", json_object_new_int(authed));
	json_object_object_add(wifictld, "connected", json_object_new_int(connected));
	json_object_object_add(wifictld, "highest_try_probe", json_object_new_int(highest_try_probe));
	json_object_object_add(wifictld, "highest_try_auth", json_object_new_int(highest_try_auth));

	return ret;
}

const struct respondd_provider_info respondd_providers[] = {
	{"nodeinfo", respondd_provider_nodeinfo},
	{"statistics", respondd_provider_statistics},
	{0, 0},
};
