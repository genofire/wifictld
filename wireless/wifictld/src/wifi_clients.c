#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <libubox/avl.h>
#include "hostapd/ieee802_11_defs.h" // ETH_ALEN
#include "log.h"
#include "wifi_clients.h"

static void wifi_clients_del(const u8 *addr);

static struct avl_tree clients_by_addr;

static int avl_compare_macaddr(const void *k1, const void *k2, void *ptr)
{
	return memcmp(k1, k2, ETH_ALEN);
}

struct wifi_client {
	struct avl_node avl;
	u8 addr[ETH_ALEN];
	int time;
	int try;
	uint32_t highfreq;
};

int wifi_clients_init() {
	avl_init(&clients_by_addr, avl_compare_macaddr, false, NULL);
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
	client = calloc(sizeof(*client), 1);
	memcpy(client->addr, address, sizeof(client->addr));
	client->highfreq = 0;
	client->try = 0;
	client->time = 0;
	client->avl.key = client->addr;
	log_debug("wifi_clients.__get_client("MACSTR"): add client to mem\n", MAC2STR(address));
	avl_insert(&clients_by_addr, &client->avl);
	return client;
}

void __client_learn(struct wifi_client *client, uint32_t freq) {
	log_debug("wifi_clients.__client_learn(., %d): ", freq);
	if (client->highfreq < freq) {
		client->highfreq = freq;
		log_debug("new highfreq");
	}
	log_debug("\n");
	//TODO time set and reset clean
}

void wifi_clients_learn(const u8 *address, uint32_t freq) {
	struct wifi_client *client;
	client = __get_client(address);
	__client_learn(client, freq);
}

int wifi_clients_try(const u8 *address, uint32_t freq) {
	struct wifi_client *client;
	client = __get_client(address);
	__client_learn(client, freq);

	log_debug("wifi_clients.wifi_clients_try("MACSTR", %d): ", MAC2STR(address), freq);
	if (freq > 5000) {
		log_debug("used correct freq\n");
		client->try = 0;
	} else {
		if(client->try > client_freq_try_threashold) {
			log_debug("clients %d try over threashold %d\n",client->try,  client_freq_try_threashold);
			return 0;
		}
		client->try++;
		log_debug("clients->try now by %d\n",client->try);
	}

	return client->try;
}


static void wifi_clients_del(const u8 *addr) {
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, addr, client, avl);
	if (!client)
			return;

	avl_delete(&clients_by_addr, &client->avl);
	free(client);
}
