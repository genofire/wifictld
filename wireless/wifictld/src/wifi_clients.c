#include "include.h"
#include "config.h"
#include "log.h"
#include "data.h"
#include "wifi_clients.h"


void clean_cbhandler(struct uloop_timeout *t)
{
	int count = 0,
		auth = 0,
		all = 0;
	time_t now;
	time(&now);
	now -= config_client_clean_older_then;
	struct wifi_client *client, *ptr;
	avl_for_each_element_safe(&clients_by_addr, client, avl, ptr) {
		all++;
		if(!config_client_clean_authed && client->authed) {
			auth++;
			continue;
		}
		if (client->time < now) {
			avl_delete(&clients_by_addr, &client->avl);
			log_verbose("clean_client(): "MACSTR" remove from memory\n", MAC2STR(client->addr));
			free(client);
			count++;
		}
	}
	uloop_timeout_set(t, config_client_clean_every * 1000);

	if (count > 0) {
		log_info("remove %d of %d clients", count, all);
		if(!config_client_clean_authed)
			log_info(" (skipped %d authed clients)\n", auth);
		log_info(" from memory\n");
	}else{
		log_verbose("remove %d of %d clients", count, all);
		if(!config_client_clean_authed)
			log_verbose(" (skipped %d authed clients)\n", auth);
		log_verbose(" from memory\n");
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
	uloop_timeout_set(&clean, config_client_clean_every * 1000);
	uloop_timeout_add(&clean);
	return 0;
}

void wifi_clients_close() {

}

void __client_setvalues(struct wifi_client *client, struct hostapd_client *hclient) {
	log_debug("wifi_clients.__client_setvalues(., %d):", hclient->freq);
	if (client->freq_highest < hclient->freq) {
		client->freq_highest = hclient->freq;
		log_debug(" new highest freq");
	}
	if (hclient->freq > WIFI_CLIENT_FREQ_THREASHOLD) {
		client->signal_highfreq = hclient->ssi_signal;
	}else{
		client->signal_lowfreq = hclient->ssi_signal;
	}
	log_debug("\n");
	time(&client->time);
	if(!client->authed) {
		client->authed = hclient->auth;
	}
}

struct wifi_client *__get_client(struct hostapd_client *hclient){
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, hclient->address, client, avl);
	if (client) {
		__client_setvalues(client, hclient);
		log_debug("wifi_clients.__get_client("MACSTR"): found existing client\n", MAC2STR(hclient->address));
		return client;
	}
	log_debug("wifi_clients.__get_client("MACSTR"): gen new client\n", MAC2STR(hclient->address));
	client = malloc(sizeof(*client));
	memcpy(client->addr, hclient->address, sizeof(client->addr));
	client->try_probe = 0;
	client->try_auth = 0;
	client->connected = 0;
	client->freq_highest = 0;
	client->signal_lowfreq = 0;
	client->signal_highfreq = 0;
	__client_setvalues(client, hclient);
	client->avl.key = client->addr;
	log_debug("wifi_clients.__get_client("MACSTR"): add client to mem\n", MAC2STR(hclient->address));
	avl_insert(&clients_by_addr, &client->avl);
	return client;
}

void wifi_clients_learn(struct hostapd_client *hclient) {
	__get_client(hclient);
}

int wifi_clients_try(struct hostapd_client *hclient) {
	struct wifi_client *client;

	client = __get_client(hclient);

	if (hclient->auth) {
		log_info("auth(try=%d mac="MACSTR" freq=%d ssi=%d): ", client->try_auth, MAC2STR(hclient->address), hclient->freq, hclient->ssi_signal);
		client->try_auth++;
		client->try_probe = 0;
	}else{
		if(config_client_force_probe){
			log_verbose("probe(try=%d mac="MACSTR" freq=%d ssi=%d): ", client->try_auth, MAC2STR(hclient->address), hclient->freq, hclient->ssi_signal);
		}else{
			log_verbose("probe(try=%d mac="MACSTR" freq=%d ssi=%d): ", client->try_probe, MAC2STR(hclient->address), hclient->freq, hclient->ssi_signal);
			client->try_probe++;
		}
	}
	if (hclient->freq > WIFI_CLIENT_FREQ_THREASHOLD) {
		if(!hclient->auth){
			client->try_probe = 0;
			log_verbose("accept\n");
			return 0;
		}
		log_info("accept\n");
		client->try_auth = 0;
		client->connected = 1;
		return 0;
	}
	if (client->freq_highest > WIFI_CLIENT_FREQ_THREASHOLD) {
		if (config_client_force || config_client_force_probe && !hclient->auth) {
			if(!hclient->auth){
				log_verbose("reject - force\n");
				return -1;
			}
			log_info("reject - force\n");
			return -1;
		}

		if (hclient->ssi_signal > config_client_signal_threashold) {
			if(!hclient->auth){
				log_verbose("reject - learned higher freq + ssi is high enough\n");
				return -1;
			}
			log_info("reject - learned higher freq + ssi is high enough\n");
			return -1;
		}
	}

	if(hclient->auth && client->try_auth > config_client_try_threashold ||
		!hclient->auth && client->try_probe > config_client_try_threashold
		) {
		if(!hclient->auth){
			client->try_probe = 0;
			log_verbose("accept - threashold\n");
			return 0;
		}
		log_info("accept - threashold\n");
		client->try_auth = 0;
		client->connected = 1;
		return 0;
	}
	if(!hclient->auth){
		log_verbose("reject\n");
		return client->try_probe;
	}
	log_info("reject\n");
	return client->try_auth;
}

void wifi_clients_disconnect(struct hostapd_client *hclient) {
	struct wifi_client *client;
	client = __get_client(hclient);
	client->connected = 0;
}

void wifi_clients_del(const u8 *address) {
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, address, client, avl);
	if (!client)
			return;

	avl_delete(&clients_by_addr, &client->avl);
	free(client);
}
