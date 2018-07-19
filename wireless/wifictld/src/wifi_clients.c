#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <libubox/avl.h>
#include <libubox/uloop.h>
#include "hostapd/ieee802_11_defs.h" // ETH_ALEN
#include "log.h"
#include "wifi_clients.h"

int client_try_threashold = 3;
int client_signal_threashold = -75;

int clean_every = 600; //in ms  = 10min
int clean_older_then = 3600;  //in sec = 1h

struct avl_tree clients_by_addr = {};


void clean_cbhandler(struct uloop_timeout *t)
{
	int i = 0;
	time_t now;
	time(&now);
	now -= clean_older_then;
	struct wifi_client *client, *ptr;
	avl_for_each_element_safe(&clients_by_addr, client, avl, ptr) {
		if (client->time < now && client->authed == 0) {
			avl_delete(&clients_by_addr, &client->avl);
			log_verbose("clean_client(): "MACSTR" remove from memory\n", MAC2STR(client->addr));
			free(client);
			i++;
		}
	}
	uloop_timeout_set(t, clean_every * 1000);

	if (i > 0) {
		log_info("remove %d clients from memory\n", i);
	}else{
		log_verbose("remove %d clients from memory\n", i);
	}
}

struct uloop_timeout clean = {
	.cb = clean_cbhandler
};

static int avl_compare_macaddr(const void *k1, const void *k2, void *ptr)
{
	return memcmp(k1, k2, ETH_ALEN);
}

int wifi_clients_init() {
	avl_init(&clients_by_addr, avl_compare_macaddr, false, NULL);
	uloop_timeout_set(&clean, clean_every * 1000);
	uloop_timeout_add(&clean);
	return 0;
}

void wifi_clients_close() {

}

struct wifi_client *__get_client(const u8 *address){
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, address, client, avl);
	if (client) {
		log_debug("wifi_clients.__get_client("MACSTR"): found existing client\n", MAC2STR(address));
		return client;
	}
	log_debug("wifi_clients.__get_client("MACSTR"): gen new client\n", MAC2STR(address));
	client = malloc(sizeof(*client));
	memcpy(client->addr, address, sizeof(client->addr));
	client->try_probe = 0;
	client->try_auth = 0;
	time(&client->time);
	client->authed = 0;
	client->connected = 0;
	client->freq_highest = 0;
	client->signal_lowfreq = 0;
	client->signal_highfreq = 0;
	client->time = 0;
	client->avl.key = client->addr;
	log_debug("wifi_clients.__get_client("MACSTR"): add client to mem\n", MAC2STR(address));
	avl_insert(&clients_by_addr, &client->avl);
	return client;
}

void __client_learn(struct wifi_client *client, uint32_t freq, uint32_t ssi_signal) {
	log_debug("wifi_clients.__client_learn(., %d):", freq);
	if (client->freq_highest < freq) {
		client->freq_highest = freq;
		log_debug(" new highest freq");
	}
	if (freq > WIFI_CLIENT_FREQ_THREASHOLD) {
		client->signal_highfreq = ssi_signal;
	}else{
		client->signal_lowfreq = ssi_signal;
	}
	log_debug("\n");
	time(&client->time);
}

void wifi_clients_learn(const u8 *address, uint32_t freq, uint32_t ssi_signal) {
	struct wifi_client *client;
	client = __get_client(address);
	__client_learn(client, freq, ssi_signal);
}

int wifi_clients_try(bool auth, const u8 *address, uint32_t freq, uint32_t ssi_signal) {
	struct wifi_client *client;
	client = __get_client(address);
	__client_learn(client, freq, ssi_signal);

	if (auth) {
		log_info("auth(try=%d mac="MACSTR" freq=%d ssi=%d): ", client->try_auth, MAC2STR(address), freq, ssi_signal);
		client->try_auth++;
		client->try_probe = 0;
	}else{
		log_verbose("probe(try=%d mac="MACSTR" freq=%d ssi=%d): ", client->try_probe, MAC2STR(address), freq, ssi_signal);
		client->try_probe++;
	}
	if (freq > WIFI_CLIENT_FREQ_THREASHOLD) {
		if(!auth){
			client->try_probe = 0;
			log_verbose("accept\n");
			return 0;
		}
		log_info("accept\n");
		client->try_auth = 0;
		client->authed = 1;
		client->connected = 1;
		return 0;
	}

	if (client->freq_highest > WIFI_CLIENT_FREQ_THREASHOLD &&
		ssi_signal > client_signal_threashold
		) {
		if(!auth){
			log_verbose("reject - learned higher freq + ssi is high enough\n");
			return -1;
		}
		log_info("reject - learned higher freq + ssi is high enough\n");
		return -1;
	}

	if(auth && client->try_auth > client_try_threashold ||
		!auth && client->try_probe > client_try_threashold
		) {
		if(!auth){
			client->try_probe = 0;
			log_verbose("accept - threashold\n");
			return 0;
		}
		log_info("accept - threashold\n");
		client->try_auth = 0;
		client->authed = 1;
		client->connected = 1;
		return 0;
	}
	if(!auth){
		log_verbose("reject\n");
		return client->try_probe;
	}
	log_info("reject\n");
	return client->try_auth;
}

void wifi_clients_disconnect(const u8 *address, uint32_t freq, uint32_t ssi_signal) {
	struct wifi_client *client;
	client = __get_client(address);
	__client_learn(client, freq, ssi_signal);
	client->connected = 0;
}

void wifi_clients_del(const u8 *addr) {
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, addr, client, avl);
	if (!client)
			return;

	avl_delete(&clients_by_addr, &client->avl);
	free(client);
}
