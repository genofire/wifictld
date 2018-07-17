#ifndef __WIFICTLD_WIFI_CLIENTS_H
#define __WIFICTLD_WIFI_CLIENTS_H

#include <stdint.h>
#include <time.h>
#include <libubox/avl.h>
#include "hostapd/ieee802_11_defs.h"

extern int client_try_threashold;
extern int client_signal_threashold;
extern int clean_every;
extern int clean_older_then;
extern struct avl_tree clients_by_addr;


struct wifi_client {
	struct avl_node avl;
	u8 addr[ETH_ALEN];
	time_t time;
	int try_probe;
	int try_auth;
	bool connected;
	bool authed;
	uint32_t freq_highest;
	uint32_t signal_lowfreq;
	uint32_t signal_highfreq;
};

#define WIFI_CLIENT_FREQ_THREASHOLD 5000

int wifi_clients_init();
void wifi_clients_close();

void wifi_clients_learn(const uint8_t *address, uint32_t freq, uint32_t ssi_signal);
int wifi_clients_try(bool auth, const uint8_t *address, uint32_t freq, uint32_t ssi_signal);
void wifi_clients_disconnect(const uint8_t *address, uint32_t freq, uint32_t ssi_signal);

void wifi_clients_del(const u8 *addr);

#endif
