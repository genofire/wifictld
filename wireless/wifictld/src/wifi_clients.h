#ifndef __WIFICTLD_WIFI_CLIENTS_H
#define __WIFICTLD_WIFI_CLIENTS_H

#include "include.h"



struct hostapd_client {
	u8 *address;
	const char *method;
	bool auth;
	uint32_t freq;
	uint32_t ssi_signal;
};

int wifi_clients_init();
void wifi_clients_close();

void wifi_clients_learn(struct hostapd_client *hclient);
int wifi_clients_try(struct hostapd_client *hclient);
void wifi_clients_disconnect(struct hostapd_client *hclient);

void wifi_clients_del(const u8 *address);

#endif
