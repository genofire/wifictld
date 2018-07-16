#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <libubox/avl.h>
#include "hostapd/ieee802_11_defs.h" // ETH_ALEN
#include "wifi_clients.h"

static void wifi_clients_del(const u8 *addr);

static struct avl_tree clients_by_addr;

struct wifi_client {
	struct avl_node avl;
	u8 addr[ETH_ALEN];
	int time;
	int try;
	uint32_t highfreq;
};

int wifi_clients_init() {
	return 0;
}

void wifi_clients_close() {

}

struct wifi_client *__get_client(const u8 *addr){
	struct wifi_client *client;

	client = avl_find_element(&clients_by_addr, addr, client, avl);
	if (client)
			return client;
		client = malloc(sizeof(*client));
		memcpy(client->addr, addr, sizeof(client->addr));
		client->highfreq = 0;
		client->try = 0;
		client->time = 0;
		client->avl.key = client->addr;
		avl_insert(&clients_by_addr, &client->avl);
		return client;
}

void __client_learn(struct wifi_client *client, uint32_t freq) {
	if (client->highfreq < freq) {
		client->highfreq = freq;
	}
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

	if (freq > 5000) {
		client->try = 0;
	} else {
		if(client->try > client_freq_try_threashold) {
			return 0;
		}
		client->try++;
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
